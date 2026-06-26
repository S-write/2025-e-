/**
 * @file   button.h
 * @brief  按键模块: PB21模式选择/运行, PA18圈数选择
 *
 * 按键功能:
 *   PB21 短按 → 切换模式 (循迹/云台)
 *   PB21 长按(≥1s) → 运行当前模式
 *   PB21 短按(运行时) → 停止, 回到空闲
 *   PA18 短按 → 选择圈数 1~5 (仅模式1有效)
 *
 * 转弯检测: ch0-ch2 三个灰度传感器同时检测到黑线 = 1次转弯
 *           4次转弯 = 1圈
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

/* ──────── 硬件引脚 (使用 SysConfig 生成的宏) ──────── */

/* PB21: 模式按键, SysConfig Pin Group = GPIO_SWITCHES_PB21 */
#define BTN_MODE_PORT       GPIO_SWITCHES_PB21_PORT
#define BTN_MODE_PIN        GPIO_SWITCHES_PB21_USER_SWITCH_1_PIN

/* PA18: 圈数按键, SysConfig Pin Group = USER_SWITCHES_PA18 */
#define BTN_LAP_PORT        USER_SWITCHES_PA18_PORT
#define BTN_LAP_PIN         USER_SWITCHES_PA18_USER_SWITCH_PA18_PIN

/* ──────── 时序参数 ──────── */

#define BTN_DEBOUNCE_MS     50      /* 消抖时间 */
#define BTN_LONG_MS         1000    /* 长按阈值: 1秒 */

/* ──────── 系统状态 ──────── */

typedef enum {
    SYS_IDLE = 0,           /* 空闲: 等待选模式 */
    SYS_MODE_SELECT,        /* 模式选择中 */
    SYS_RUNNING             /* 运行中 */
} SysState_t;

/* ──────── 运行模式 ──────── */

typedef enum {
    MODE_TRACKING = 0,      /* 模式1: 循迹 1~5 圈 (PA18选) */
    MODE_GIMBAL,            /* 模式2: 仅云台追踪 */
    MODE_SCAN,              /* 模式3: X轴扫描 + 收到数据锁定追踪 */
    MODE_TRACK_GIMBAL_1,    /* 模式4: 边循迹边云台追踪, 1圈 */
    MODE_TRACK_GIMBAL_2     /* 模式5: 边循迹边云台追踪, 2圈 */
} OpMode_t;

/* ──────── 公开接口 ──────── */

void     Button_Init(void);
void     Button_OLED_Init(void);
void     Button_Task(void);

/**
 * @brief 刷新 OLED 显示: 模式、圈数、运行状态
 *        内部自动做脏检测, 仅在内容变化时真正刷新屏幕
 */
void     Button_UpdateDisplay(void);

SysState_t Button_GetState(void);
OpMode_t   Button_GetMode(void);
uint8_t    Button_GetLapTarget(void);
void       Button_SetLapTarget(uint8_t laps);  /* 模式4/5 固定圈数 */

/**
 * @brief 由外部(主循环)调用: 通知"检测到一次转弯"
 *        内部自动累加并判断是否达到目标圈数
 * @return true=已完成目标圈数, 需停车
 */
bool     Button_OnCorner(void);

/**
 * @brief 查询 CH0-CH2 是否刚检测到一次新转弯 (上升沿, 只触发一次)
 *        用于触发云台转弯补偿
 */
bool     Button_CornerEdge(void);

/**
 * @brief 获取当前已完成的转弯数和圈数 (用于调试/显示)
 */
uint8_t  Button_GetCornerCount(void);
uint8_t  Button_GetLapCount(void);

#endif /* BUTTON_H */
