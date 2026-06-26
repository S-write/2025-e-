/**
 * @file   button.c
 * @brief  按键模块实现: 消抖、长短按识别、模式状态机、圈数计数
 */

#include "button.h"
#include "gray_sensor.h"
#include "motor.h"
#include "oled.h"
#include "delay.h"
#include <stddef.h>

/* 引用 motor.c 中的全局变量, 停车时清零 */
extern float   target_speed_1;
extern float   target_speed_2;
extern uint8_t corner_mode;

/* ──────── 内部状态变量 ──────── */

static SysState_t  g_sys_state   = SYS_IDLE;
static OpMode_t    g_op_mode     = MODE_TRACKING;
static uint8_t     g_lap_target  = 1;

/* 按键消抖状态机 */
typedef enum {
    KP_IDLE = 0,
    KP_DEBOUNCE,        /* 按下消抖中 */
    KP_PRESSED,         /* 已确认按下 */
    KP_WAIT_RELEASE     /* 长按已触发, 等待释放 */
} KeyState_t;

static KeyState_t  g_mode_key_state  = KP_IDLE;
static uint32_t    g_mode_key_down_tick = 0;

static KeyState_t  g_lap_key_state   = KP_IDLE;
static uint32_t    g_lap_key_down_tick = 0;

/* 圈数/转弯计数 */
static uint8_t     g_corner_count = 0;
static uint8_t     g_lap_count    = 0;
static bool        g_corner_active = false;
static bool        g_corner_edge   = false;  /* 刚检测到转弯上升沿 */

/* ──────── 静态函数声明 ──────── */

static bool Read_Mode_Key(void);
static bool Read_Lap_Key(void);
static void Stop_Motors(void);
static void Mode_Key_FSM(void);
static void Lap_Key_FSM(void);

/* ──────── 按键读取 (遵循 TI DriverLib 风格) ──────── */

static bool Read_Mode_Key(void)
{
    /* 按键低电平有效: 按下 → 读回 0 → 取反后返回 true */
    if (!DL_GPIO_readPins(BTN_MODE_PORT, BTN_MODE_PIN)) {
        return true;
    } else {
        return false;
    }
}

static bool Read_Lap_Key(void)
{
    if (!DL_GPIO_readPins(BTN_LAP_PORT, BTN_LAP_PIN)) {
        return true;
    } else {
        return false;
    }
}

/* ──────── 停车 ──────── */

static void Stop_Motors(void)
{
    corner_mode = 1;
    target_speed_1 = 0.0f;
    target_speed_2 = 0.0f;

    motor_set_duty(1, 0);
    motor_set_duty(2, 0);
    motor_set_direction(1, 0);
    motor_set_direction(2, 0);
    motor_reset_pid();
}

/* ──────── 初始化 ──────── */

void Button_Init(void)
{
    /* PB21: SysConfig 未配置上拉, 此处补上 */
    DL_GPIO_initDigitalInputFeatures(
        GPIO_SWITCHES_PB21_USER_SWITCH_1_IOMUX,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);

    /* PA18: 显式重新初始化, 确保输入+上拉生效 */
    DL_GPIO_initDigitalInputFeatures(
        USER_SWITCHES_PA18_USER_SWITCH_PA18_IOMUX,
        DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
}

void Button_OLED_Init(void)
{
    OLED_Init();
}

/* ──────── 模式按键 (PB21) 状态机 ──────── */

static void Mode_Key_FSM(void)
{
    bool pressed = Read_Mode_Key();

    switch (g_mode_key_state) {

    case KP_IDLE:
        if (pressed) {
            g_mode_key_down_tick = g_ms_tick;
            g_mode_key_state = KP_DEBOUNCE;
        }
        break;

    case KP_DEBOUNCE:
        if (!pressed) {
            g_mode_key_state = KP_IDLE;
        } else if ((g_ms_tick - g_mode_key_down_tick) >= BTN_DEBOUNCE_MS) {
            g_mode_key_state = KP_PRESSED;
        }
        break;

    case KP_PRESSED:
        if (!pressed) {
            uint32_t duration = g_ms_tick - g_mode_key_down_tick;
            if (duration >= BTN_DEBOUNCE_MS && duration < BTN_LONG_MS) {
                /* ─── 短按 ─── */
                switch (g_sys_state) {
                case SYS_IDLE:
                    g_sys_state = SYS_MODE_SELECT;
                    break;

                case SYS_MODE_SELECT:
                    /* 五模式循环 */
                    if (g_op_mode == MODE_TRACKING) {
                        g_op_mode = MODE_GIMBAL;
                    } else if (g_op_mode == MODE_GIMBAL) {
                        g_op_mode = MODE_SCAN;
                    } else if (g_op_mode == MODE_SCAN) {
                        g_op_mode = MODE_TRACK_GIMBAL_1;
                    } else if (g_op_mode == MODE_TRACK_GIMBAL_1) {
                        g_op_mode = MODE_TRACK_GIMBAL_2;
                    } else {
                        g_op_mode = MODE_TRACKING;
                    }
                    break;

                case SYS_RUNNING:
                    Stop_Motors();
                    g_sys_state = SYS_IDLE;
                    g_lap_count    = 0;
                    g_corner_count = 0;
                    g_corner_active = false;
                    break;
                }
            }
            g_mode_key_state = KP_IDLE;
        } else {
            uint32_t duration = g_ms_tick - g_mode_key_down_tick;
            if (duration >= BTN_LONG_MS) {
                /* ─── 长按 ─── */
                if (g_sys_state == SYS_MODE_SELECT) {
                    g_sys_state    = SYS_RUNNING;
                    g_lap_count    = 0;
                    g_corner_count = 0;
                    g_corner_active = false;
                }
                g_mode_key_state = KP_WAIT_RELEASE;
            }
        }
        break;

    case KP_WAIT_RELEASE:
        if (!pressed) {
            g_mode_key_state = KP_IDLE;
        }
        break;
    }
}

/* ──────── 圈数按键 (PA18) 状态机 ──────── */

static void Lap_Key_FSM(void)
{
    bool pressed = Read_Lap_Key();

    switch (g_lap_key_state) {

    case KP_IDLE:
        if (pressed) {
            g_lap_key_down_tick = g_ms_tick;
            g_lap_key_state = KP_DEBOUNCE;
        }
        break;

    case KP_DEBOUNCE:
        if (!pressed) {
            g_lap_key_state = KP_IDLE;
        } else if ((g_ms_tick - g_lap_key_down_tick) >= BTN_DEBOUNCE_MS) {
            g_lap_key_state = KP_PRESSED;
        }
        break;

    case KP_PRESSED:
        if (!pressed) {
            uint32_t duration = g_ms_tick - g_lap_key_down_tick;
            if (duration >= BTN_DEBOUNCE_MS && duration < BTN_LONG_MS) {
                if (g_sys_state == SYS_MODE_SELECT
                    && g_op_mode == MODE_TRACKING) {
                    g_lap_target++;
                    if (g_lap_target > 5) g_lap_target = 1;
                }
            }
            g_lap_key_state = KP_IDLE;
        } else {
            if ((g_ms_tick - g_lap_key_down_tick) >= BTN_LONG_MS) {
                g_lap_key_state = KP_WAIT_RELEASE;
            }
        }
        break;

    case KP_WAIT_RELEASE:
        if (!pressed) {
            g_lap_key_state = KP_IDLE;
        }
        break;
    }
}

/* ──────── 主任务: 每循环调用一次 ──────── */

void Button_Task(void)
{
    Mode_Key_FSM();
    Lap_Key_FSM();
}

/* ──────── 状态/模式查询 ──────── */

SysState_t Button_GetState(void)
{
    return g_sys_state;
}

OpMode_t Button_GetMode(void)
{
    return g_op_mode;
}

uint8_t Button_GetLapTarget(void)
{
    return g_lap_target;
}

void Button_SetLapTarget(uint8_t laps)
{
    if (laps >= 1 && laps <= 5) {
        g_lap_target = laps;
    }
}

uint8_t Button_GetCornerCount(void)
{
    return g_corner_count;
}

bool Button_CornerEdge(void)
{
    if (g_corner_edge) {
        g_corner_edge = false;
        return true;
    }
    return false;
}

uint8_t Button_GetLapCount(void)
{
    return g_lap_count;
}

/* ──────── 转弯检测与圈数计数 ──────── */

bool Button_OnCorner(void)
{
    /* 仅循迹相关模式才计数: M1 / M4 / M5 */
    if (g_sys_state != SYS_RUNNING) return false;
    if (g_op_mode != MODE_TRACKING
            && g_op_mode != MODE_TRACK_GIMBAL_1
            && g_op_mode != MODE_TRACK_GIMBAL_2) {
        return false;
    }

    /* CH0/CH1/CH2: 三个灰度传感器同时检测到黑线 = 转弯 */
    uint8_t ch0 = gray_data[0];
    uint8_t ch1 = gray_data[1];
    uint8_t ch2 = gray_data[2];

    bool all_black = (ch0 && ch1 && ch2);

    if (all_black && !g_corner_active) {
        g_corner_active = true;
        g_corner_edge   = true;
        g_corner_count++;

        if (g_corner_count >= 4) {
            g_corner_count = 0;
            g_lap_count++;

            if (g_lap_count >= g_lap_target) {
                Stop_Motors();
                g_sys_state    = SYS_IDLE;
                g_lap_count    = 0;
                g_corner_count = 0;
                g_corner_active = false;
                return true;
            }
        }
    } else if (!all_black) {
        g_corner_active = false;
    }

    return false;
}

/* ──────── OLED 显示更新 ──────── */

void Button_UpdateDisplay(void)
{
    static SysState_t last_state      = (SysState_t)(-1);
    static OpMode_t   last_mode       = (OpMode_t)(-1);
    static uint8_t    last_lap_target = 255;
    static uint8_t    last_lap_count  = 255;
    static uint8_t    last_corner_cnt = 255;
    static bool       need_redraw     = true;

    if (g_sys_state != last_state)        { need_redraw = true; last_state = g_sys_state; }
    if (g_op_mode   != last_mode)         { need_redraw = true; last_mode  = g_op_mode;   }
    if (g_lap_target != last_lap_target)  { need_redraw = true; last_lap_target = g_lap_target; }

    if (g_sys_state == SYS_RUNNING && g_op_mode == MODE_TRACKING) {
        if (g_lap_count   != last_lap_count)  { need_redraw = true; last_lap_count  = g_lap_count;  }
        if (g_corner_count != last_corner_cnt) { need_redraw = true; last_corner_cnt = g_corner_count; }
    }

    if (!need_redraw) return;
    need_redraw = false;

    OLED_Clear();

    switch (g_sys_state) {

    case SYS_IDLE:
        OLED_ShowString(0,  0, (u8 *)"DianSai V1", 16);
        OLED_ShowString(0, 24, (u8 *)"PB21 -> Start", 12);
        break;

    case SYS_MODE_SELECT:
        if (g_op_mode == MODE_TRACKING) {
            OLED_ShowString(0,  0, (u8 *)"M1: Track", 16);
            OLED_ShowString(0, 24, (u8 *)"Laps:", 12);
            OLED_ShowNum(36, 24, g_lap_target, 1, 12);
            OLED_ShowString(56, 24, (u8 *)"(PA18)", 12);
        } else if (g_op_mode == MODE_GIMBAL) {
            OLED_ShowString(0,  0, (u8 *)"M2: Gimbal", 16);
            OLED_ShowString(0, 24, (u8 *)"Track Only", 12);
        } else if (g_op_mode == MODE_SCAN) {
            OLED_ShowString(0,  0, (u8 *)"M3: Scan", 16);
            OLED_ShowString(0, 24, (u8 *)"Sweep + Track", 12);
        } else if (g_op_mode == MODE_TRACK_GIMBAL_1) {
            OLED_ShowString(0,  0, (u8 *)"M4: T+G 1Lap", 16);
            OLED_ShowString(0, 24, (u8 *)"Track+Gimbal", 12);
        } else {
            OLED_ShowString(0,  0, (u8 *)"M5: T+G 2Lap", 16);
            OLED_ShowString(0, 24, (u8 *)"Track+Gimbal", 12);
        }
        OLED_ShowString(0, 44, (u8 *)"Long PB21: Run", 12);
        break;

    case SYS_RUNNING:
        if (g_op_mode == MODE_TRACKING) {
            OLED_ShowString(0,  0, (u8 *)"M1: Track", 16);
            OLED_ShowString(0, 24, (u8 *)"Lap:", 12);
            OLED_ShowNum(30, 24, g_lap_count, 1, 12);
            OLED_ShowString(40, 24, (u8 *)"/", 12);
            OLED_ShowNum(48, 24, g_lap_target, 1, 12);
            OLED_ShowString(0, 40, (u8 *)"Corner:", 12);
            OLED_ShowNum(48, 40, g_corner_count, 1, 12);
            OLED_ShowString(60, 40, (u8 *)"/4", 12);
        } else if (g_op_mode == MODE_GIMBAL) {
            OLED_ShowString(0,  0, (u8 *)"M2: Gimbal", 16);
            OLED_ShowString(0, 24, (u8 *)"Running...", 12);
        } else if (g_op_mode == MODE_SCAN) {
            OLED_ShowString(0,  0, (u8 *)"M3: Scan", 16);
            OLED_ShowString(0, 24, (u8 *)"Sweeping...", 12);
        } else if (g_op_mode == MODE_TRACK_GIMBAL_1) {
            OLED_ShowString(0,  0, (u8 *)"M4: T+G 1Lap", 16);
            OLED_ShowString(0, 24, (u8 *)"Lap:", 12);
            OLED_ShowNum(30, 24, g_lap_count, 1, 12);
            OLED_ShowString(40, 24, (u8 *)"/1", 12);
            OLED_ShowString(0, 40, (u8 *)"Corner:", 12);
            OLED_ShowNum(48, 40, g_corner_count, 1, 12);
            OLED_ShowString(60, 40, (u8 *)"/4", 12);
        } else {
            OLED_ShowString(0,  0, (u8 *)"M5: T+G 2Lap", 16);
            OLED_ShowString(0, 24, (u8 *)"Lap:", 12);
            OLED_ShowNum(30, 24, g_lap_count, 1, 12);
            OLED_ShowString(40, 24, (u8 *)"/2", 12);
            OLED_ShowString(0, 40, (u8 *)"Corner:", 12);
            OLED_ShowNum(48, 40, g_corner_count, 1, 12);
            OLED_ShowString(60, 40, (u8 *)"/4", 12);
        }
        OLED_ShowString(0, 52, (u8 *)"PB21: Stop", 12);
        break;
    }

    OLED_Refresh();
}
