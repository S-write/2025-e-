/**
 * @file   servo.h
 * @brief  双舵机驱动模块 (X轴270°, Y轴220°)
 *
 * 硬件连接:
 *   PA17 → X轴舵机 (TIMA1 CC0)
 *   PB18 → Y轴舵机 (TIMA1 CC1)
 *
 * PWM: 50Hz, 20ms周期, 500~2500us脉宽
 */

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

/* 舵机索引 */
#define SERVO_X  0
#define SERVO_Y  1

/* 舵机角度范围 */
#define SERVO_X_MAX_ANGLE  270.0f
#define SERVO_Y_MAX_ANGLE  220.0f
#define SERVO_X_CENTER     135.0f
#define SERVO_Y_CENTER     110.0f

/**
 * @brief 初始化舵机模块 (设置中位并启动PWM)
 */
void Servo_Init(void);

/**
 * @brief 设置舵机角度
 * @param ch     SERVO_X 或 SERVO_Y
 * @param deg    目标角度(°), 0 ~ 最大角
 */
void Servo_SetAngle(uint8_t ch, float deg);

/**
 * @brief 舵机回中
 */
void Servo_Center(void);

/**
 * @brief 同时设置两个舵机
 */
void Servo_SetBoth(float x_deg, float y_deg);

#endif /* SERVO_H */
