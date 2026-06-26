/**
 * @file   track.c
 * @brief  PI闭环瞄准控制器实现
 *
 * 数据流:
 *   像素偏移 → 方向校正 → 标定换算 → PI控制器 → 增量角度 → 舵机PWM
 *
 *   err    = offset * DIR / PIXELS_PER_DEG
 *   int   += err
 *   output = Kp * err + Ki * int
 *   angle += output
 */

#include "track.h"
#include "servo.h"
#include <stdbool.h>

/* ──────────── 标定参数 (按实际测试调整) ──────────── */

#define PIXELS_PER_DEG  14.0f    /* 1°对应多少像素 (需标定!) */
#define KP              0.45f     /* 比例增益: 远处快速响应 */
#define KP_LOW          0.01f    /* 比例增益: 近处轻柔防抖 */
#define KP_NEAR_PX      20       /* 低于此像素用低增益 */
#define KI              0.02f    /* 积分增益: 消除稳态误差 */
#define DIR_X           1.0f     /* X轴方向校正 (1 或 -1) */
#define DIR_Y           1.0f     /* Y轴方向校正 (1 或 -1) */
#define INTEGRAL_LIMIT  50.0f    /* 积分限幅, 防止饱和失控 */
#define DEAD_ZONE_IN    4       /* 进入死区阈值: ±5像素 */
#define DEAD_ZONE_OUT   7       /* 退出死区阈值: ±10像素 (迟滞) */
#define MIN_STEP_DEG    0.1f     /* 最小步进(°), 近处保持丝滑 */
#define SPEED_FACTOR    1.2f     /* 速度因子: 误差的N倍=最大步进 */

/* ──────────── 内部状态 ──────────── */

static float g_angle_x;           /* X轴当前角度 */
static float g_angle_y;           /* Y轴当前角度 */
static float g_int_x;             /* X轴积分累加器 */
static float g_int_y;             /* Y轴积分累加器 */
static bool  g_locked = false;    /* 死区锁定标志 */

/* ──────────── 辅助: 限幅函数 ──────────── */

static inline float clamp(float val, float lo, float hi)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

/* ──────────── 公开接口 ──────────── */

void Track_Init(void)
{
    g_angle_x = SERVO_X_CENTER;
    g_angle_y = SERVO_Y_CENTER;
    g_int_x   = 0.0f;
    g_int_y   = 0.0f;
}

void Track_ResetIntegral(void)
{
    g_int_x = 0.0f;
    g_int_y = 0.0f;
}

void Track_Center(void)
{
    g_angle_x = SERVO_X_CENTER;
    g_angle_y = SERVO_Y_CENTER;
    Servo_SetBoth(g_angle_x, g_angle_y);
}

void Track_SyncAngleX(float deg)
{
    g_angle_x = clamp(deg, 0.0f, SERVO_X_MAX_ANGLE);
    g_int_x   = 0.0f;   /* 清积分防跳变 */
}

float Track_GetAngleX(void)
{
    return g_angle_x;
}

void Track_Update(int o_x, int o_y)
{
    /* 迟滞死区: 进易出难, 防止边界振荡 */
    if (g_locked) {
        /* 已锁定 — 需超出 OUT 阈值才唤醒 */
        if (o_x >= -DEAD_ZONE_OUT && o_x <= DEAD_ZONE_OUT &&
            o_y >= -DEAD_ZONE_OUT && o_y <= DEAD_ZONE_OUT) {
            return;  /* 还在死区内, 不动 */
        }
        /* 偏差够大, 解锁并清积分防猛冲 */
        g_locked = false;
        g_int_x  = 0.0f;
        g_int_y  = 0.0f;
    } else {
        /* 未锁定 — 进入 IN 阈值则锁定 */
        if (o_x >= -DEAD_ZONE_IN && o_x <= DEAD_ZONE_IN &&
            o_y >= -DEAD_ZONE_IN && o_y <= DEAD_ZONE_IN) {
            g_locked = true;
            return;  /* 锁定, 不动 */
        }
    }

    /* ① 像素偏移 → 角度误差 */
    float err_x = (float)o_x * DIR_X / PIXELS_PER_DEG;
    float err_y = (float)o_y * DIR_Y / PIXELS_PER_DEG;

    /* ② 积分分离: 靠近中心(pixel < OUT)时清零积分, 防过冲振荡 */
    if (o_x >= -DEAD_ZONE_OUT && o_x <= DEAD_ZONE_OUT) g_int_x = 0.0f;
    if (o_y >= -DEAD_ZONE_OUT && o_y <= DEAD_ZONE_OUT) g_int_y = 0.0f;

    /* ②.⑤ 积分累加 + 限幅 */
    g_int_x = clamp(g_int_x + err_x, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);
    g_int_y = clamp(g_int_y + err_y, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);

    /* ③ PI输出 = P项(非线性增益) + I项 */
    float gain_x = (o_x >= -KP_NEAR_PX && o_x <= KP_NEAR_PX) ? KP_LOW : KP;
    float gain_y = (o_y >= -KP_NEAR_PX && o_y <= KP_NEAR_PX) ? KP_LOW : KP;
    float out_x = gain_x * err_x + KI * g_int_x;
    float out_y = gain_y * err_y + KI * g_int_y;

    /* ③.⑤ 动态速率限制: 远快近慢, 兼顾响应和丝滑 */
    float limit_x = MIN_STEP_DEG + (err_x > 0.0f ? err_x : -err_x) * SPEED_FACTOR;
    float limit_y = MIN_STEP_DEG + (err_y > 0.0f ? err_y : -err_y) * SPEED_FACTOR;
    out_x = clamp(out_x, -limit_x, limit_x);
    out_y = clamp(out_y, -limit_y, limit_y);

    /* ④ 增量式更新 + 物理限幅 */
    g_angle_x = clamp(g_angle_x + out_x, 0.0f, SERVO_X_MAX_ANGLE);
    g_angle_y = clamp(g_angle_y + out_y, 0.0f, SERVO_Y_MAX_ANGLE);

    /* ⑤ 输出到舵机 */
    Servo_SetBoth(g_angle_x, g_angle_y);
}
