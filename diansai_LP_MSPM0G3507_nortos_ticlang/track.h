/**
 * @file   track.h
 * @brief  PI闭环瞄准控制器 — 将像素偏移转为舵机角度修正
 */

#ifndef TRACK_H
#define TRACK_H

#include <stdint.h>

/**
 * @brief 初始化追踪控制器 (舵机回中, 积分归零)
 */
void Track_Init(void);

/**
 * @brief 输入一帧像素偏移, PI控制器自动修正舵机
 * @param o_x  X轴像素偏移 (光斑在靶心右侧为正)
 * @param o_y  Y轴像素偏移 (光斑在靶心下方为正)
 */
void Track_Update(int o_x, int o_y);

/**
 * @brief 重置积分累加器 (目标丢失/切换时调用)
 */
void Track_ResetIntegral(void);

/**
 * @brief 舵机回到中位
 */
void Track_Center(void);

/**
 * @brief 同步 X 轴内部角度 (扫描模式切换到追踪时使用,
 *        避免 Track_Update 从旧角度起算造成跳变)
 * @param deg  当前 X 轴舵机实际角度
 */
void Track_SyncAngleX(float deg);

/**
 * @brief 获取当前 X 轴内部角度 (用于转弯补偿等)
 */
float Track_GetAngleX(void);

#endif /* TRACK_H */
