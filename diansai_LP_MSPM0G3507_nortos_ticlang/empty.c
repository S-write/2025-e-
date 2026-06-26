#include "ti_msp_dl_config.h"
#include "motor.h"
#include "interrupt.h"
#include "gray_sensor.h"
#include "servo.h"
#include "uart.h"
#include "track.h"
#include "button.h"
#include "delay.h"
#include <stdio.h>

extern float target_speed_1;
extern float target_speed_2;

#define BASE_SPEED          200.0f
#define STEER_COEFF         0.11f
#define CORNER_TH_ENTER     1500
#define CORNER_TH_EXIT      800
#define CORNER_DUTY         650
#define CORNER_MIN_TICK     2500
#define CORNER_FORWARD_MS   500

/* ──────── 模式3 扫描参数 ──────── */

#define SCAN_SWEEP_SPEED    3.0f     /* X轴扫描速度 (°/s), 可调 */

/* 扫描状态 */
static float    g_scan_x_angle     = 0.0f;
static int8_t   g_scan_dir         = 1;      /* 1=角度递增, -1=递减 */
static uint32_t g_scan_last_tick   = 0;
static bool     g_scan_init        = true;   /* 首次进入扫描需复位 */
static bool     g_scan_is_tracking = false;  /* 已收到数据→永久锁定追踪 */

/* ──────── 模式3: X轴扫描 + 收到数据后锁定追踪 ──────── */

static void Scan_Task(void)
{
    if (g_scan_init) {
        g_scan_x_angle     = 0.0f;
        g_scan_dir         = 1;
        g_scan_last_tick   = g_ms_tick;
        g_scan_is_tracking = false;
        g_scan_init        = false;
    }

    /* 检查是否有 K230 发来的像素偏移数据 */
    bool got_data = false;

    if (UART_LineReady()) {
        int o_x = 0, o_y = 0;
        if (sscanf(UART_GetLine(), "o_x=%d o_y=%d", &o_x, &o_y) == 2) {
            got_data = true;

            if (!g_scan_is_tracking) {
                /* 首次收到有效数据: 立即停止扫描, 同步角度, 锁定追踪 */
                g_scan_is_tracking = true;

                /* 将舵机明确停到当前扫描位置, 避免惯性视觉上还在动 */
                Servo_SetAngle(SERVO_X, g_scan_x_angle);
                Track_SyncAngleX(g_scan_x_angle);
            }
            Track_Update(o_x, o_y);
        }
        UART_NextLine();
    }

    if (!g_scan_is_tracking) {
        /* 未锁定: X轴匀速扫描 */
        uint32_t now = g_ms_tick;
        float    dt  = (float)(now - g_scan_last_tick) / 1000.0f;
        g_scan_last_tick = now;

        if (dt > 0.1f)  dt = 0.01f;
        if (dt <= 0.0f) dt = 0.001f;

        g_scan_x_angle += (float)g_scan_dir * SCAN_SWEEP_SPEED * dt;

        if (g_scan_x_angle >= SERVO_X_MAX_ANGLE) {
            g_scan_x_angle = SERVO_X_MAX_ANGLE;
            g_scan_dir = -1;
        } else if (g_scan_x_angle <= 0.0f) {
            g_scan_x_angle = 0.0f;
            g_scan_dir = 1;
        }

        Servo_SetAngle(SERVO_X, g_scan_x_angle);
    }
}

/* ──────── 主函数 ──────── */

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(GPIO_MOTOR_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(GPIO_MOTOR_GPIOB_INT_IRQN);
    motor_init(1);
    motor_init(2);
    gray_sensor_init();
    UART_Init();

    /* 舵机上电回中: Servo_Init() 已将 X/Y 舵机设到中位并启动 PWM,
     * 延时 500ms 等待舵机物理到达初始位置 */
    Servo_Init();
    delay_ms(500);
    Track_Init();

    Button_Init();
    Button_OLED_Init();   /* OLED 初始化: 若 OLED 未接/I2C 异常会在此卡住,
                             暂时不需要 OLED 可注释此行 */

    uint32_t corner_enter_tick  = 0;
    uint8_t  forward_pending    = 0;
    bool     tracking_reset     = true;

    while (1)
    {
        /* ── ① 按键状态机 ── */
        Button_Task();

        /* ── ② OLED 显示 ── */
        Button_UpdateDisplay();

        SysState_t state = Button_GetState();
        OpMode_t   mode  = Button_GetMode();

        /* ── ③ 模式2: 仅云台追踪, 无循迹 ── */
        if (mode == MODE_GIMBAL && state == SYS_RUNNING) {
            if (UART_LineReady()) {
                int o_x = 0, o_y = 0;
                if (sscanf(UART_GetLine(), "o_x=%d o_y=%d", &o_x, &o_y) == 2) {
                    Track_Update(o_x, o_y);
                }
                UART_NextLine();
            }
            continue;
        }

        /* ── ④ 空闲 / 模式选择 → 跳过 ── */
        if (state == SYS_IDLE) {
            tracking_reset = true;
            g_scan_init    = true;
            continue;
        }

        if (state == SYS_MODE_SELECT) {
            tracking_reset = true;
            g_scan_init    = true;
            continue;
        }

        /* ── ⑤ 模式3: X轴扫描 + 收到数据锁定追踪 (无循迹) ── */
        if (mode == MODE_SCAN) {
            Scan_Task();
            continue;
        }

        /* ── ⑥ 模式4/5: 边循迹边云台追踪 ── */
        if ((mode == MODE_TRACK_GIMBAL_1 || mode == MODE_TRACK_GIMBAL_2)
                && state == SYS_RUNNING) {
            if (UART_LineReady()) {
                int o_x = 0, o_y = 0;
                if (sscanf(UART_GetLine(), "o_x=%d o_y=%d", &o_x, &o_y) == 2) {
                    Track_Update(o_x, o_y);
                }
                UART_NextLine();
            }
            /* 不 continue — 继续执行下面的循迹逻辑 */
        }

        /* ── ⑦ 循迹 (模式1 / 模式4 / 模式5) ── */

        if (tracking_reset) {
            tracking_reset    = false;
            corner_mode       = 0;
            forward_pending   = 0;
            corner_enter_tick = 0;
            motor_reset_pid();

            /* 模式4/5 固定圈数 */
            if (mode == MODE_TRACK_GIMBAL_1) {
                Button_SetLapTarget(1);
            } else if (mode == MODE_TRACK_GIMBAL_2) {
                Button_SetLapTarget(2);
            }
        }

        Gray_Task_Loop();
        if (gray_data_ready != 2) continue;
        gray_data_ready = 0;

        /* 转弯计数: 模式1 / 模式4 / 模式5 */
        if (mode == MODE_TRACKING
                || mode == MODE_TRACK_GIMBAL_1
                || mode == MODE_TRACK_GIMBAL_2) {
            bool lap_done = Button_OnCorner();
            if (lap_done) {
                continue;
            }
        }

        int16_t error = Gray_Calc_Track_Error();
        float abs_err = (error > 0) ? (float)error : (float)(-error);

        if (forward_pending) {
            if ((g_ms_tick - corner_enter_tick) > CORNER_FORWARD_MS) {
                forward_pending = 0;
                corner_mode = 1;
                corner_enter_tick = g_ms_tick;
                if (error > 0) {
                    motor_set_direction(1, 2);
                    motor_set_direction(2, 1);
                } else {
                    motor_set_direction(1, 1);
                    motor_set_direction(2, 2);
                }
                motor_set_duty(1, CORNER_DUTY);
                motor_set_duty(2, CORNER_DUTY);
                target_speed_1 = 0;
                target_speed_2 = 0;
            } else {
                target_speed_1 = BASE_SPEED;
                target_speed_2 = BASE_SPEED;
                motor_set_direction(1, 1);
                motor_set_direction(2, 1);
            }
        }
        else if (!corner_mode) {
            if (abs_err > CORNER_TH_ENTER) {
                forward_pending = 1;
                corner_enter_tick = g_ms_tick;
            } else {
                float steer = (float)error * STEER_COEFF;
                target_speed_1 = BASE_SPEED - steer;
                target_speed_2 = BASE_SPEED + steer;
                if (target_speed_1 < 20) target_speed_1 = 20;
                if (target_speed_2 < 20) target_speed_2 = 20;
                motor_set_direction(1, 1);
                motor_set_direction(2, 1);
            }
        }
        else {
            if (abs_err < CORNER_TH_EXIT && (g_ms_tick - corner_enter_tick) > CORNER_MIN_TICK) {
                motor_reset_pid();
                corner_mode = 0;
                float steer = (float)error * STEER_COEFF;
                target_speed_1 = BASE_SPEED - steer;
                target_speed_2 = BASE_SPEED + steer;
                if (target_speed_1 < 20) target_speed_1 = 20;
                if (target_speed_2 < 20) target_speed_2 = 20;
                motor_set_direction(1, 1);
                motor_set_direction(2, 1);
            } else {
                if (error > 0) {
                    motor_set_direction(1, 2);
                    motor_set_direction(2, 1);
                } else {
                    motor_set_direction(1, 1);
                    motor_set_direction(2, 2);
                }
                motor_set_duty(1, CORNER_DUTY);
                motor_set_duty(2, CORNER_DUTY);
            }
        }
    }
}
