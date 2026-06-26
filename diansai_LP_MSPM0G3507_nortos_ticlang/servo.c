/**
 * @file   servo.c
 * @brief  双舵机驱动模块实现
 */

#include "servo.h"
#include "ti_msp_dl_config.h"

/* PWM参数 */
#define PWM_PERIOD     19999
#define PULSE_MIN      500
#define PULSE_MAX      2500
#define PWM_CC_PERIOD  (PWM_PERIOD - PULSE_MIN)   /* 19999 - 500 = 19499 */
#define PWM_CC_0       19499                      /* 对应 500us */
#define PWM_CC_MAX     17499                      /* 对应 2500us */

void Servo_Init(void)
{
    /* 先设中位再启动PWM, 避免上电冲到极限 */
    Servo_SetAngle(SERVO_X, SERVO_X_CENTER);
    Servo_SetAngle(SERVO_Y, SERVO_Y_CENTER);
    DL_TimerA_startCounter(PWM_0_INST);
}

void Servo_SetAngle(uint8_t ch, float deg)
{
    float    max_angle;
    float    pulse;
    uint16_t cmp;

    max_angle = (ch == SERVO_X) ? SERVO_X_MAX_ANGLE : SERVO_Y_MAX_ANGLE;

    /* 限幅 */
    if (deg < 0.0f) deg = 0.0f;
    if (deg > max_angle) deg = max_angle;

    /* 角度 → 脉宽(us): 500 + (deg/max) * 2000 */
    pulse = (float)PULSE_MIN + (deg / max_angle) * (float)(PULSE_MAX - PULSE_MIN);

    /* PWM极性INIT_VAL_LOW: 实际脉宽 = 周期 - CC */
    cmp = (uint16_t)((float)PWM_PERIOD - pulse);

    if (ch == SERVO_X) {
        DL_TimerA_setCaptureCompareValue(PWM_0_INST, cmp, DL_TIMER_CC_0_INDEX);
    } else {
        DL_TimerA_setCaptureCompareValue(PWM_0_INST, cmp, DL_TIMER_CC_1_INDEX);
    }
}

void Servo_Center(void)
{
    Servo_SetAngle(SERVO_X, SERVO_X_CENTER);
    Servo_SetAngle(SERVO_Y, SERVO_Y_CENTER);
}

void Servo_SetBoth(float x_deg, float y_deg)
{
    Servo_SetAngle(SERVO_X, x_deg);
    Servo_SetAngle(SERVO_Y, y_deg);
}
