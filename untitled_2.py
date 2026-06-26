# ============================================================
# MicroPython 灰度图矩形中心检测 + 激光检测（动态ROI + 圆形光斑验证）
# 激光存在时：连线为 激光点 → 矩形中心
# 激光不存在时：连线为 屏幕中心 → 矩形中心
# ============================================================

# 导入亚博智能串口通信库
# (Import Yahboom UART communication library)
from ybUtils.YbUart import YbUart

import time, os, sys, gc
from machine import Pin
from media.sensor import *
from media.display import *
from media.media import *
import _thread
import cv_lite
import ulab.numpy as np


# 创建串口实例，设置波特率为115200
# (Create UART instance with baud rate set to 115200)
# 波特率是指每秒传输的比特数，115200是常用的高速通信速率
# (Baud rate refers to bits per second, 115200 is a commonly used high-speed communication rate)
uart = YbUart(baudrate=115200)

# -------------------------------
# 图像尺寸设置
# -------------------------------
image_shape = [480, 640]

# -------------------------------
# 初始化摄像头（灰度图模式）
# -------------------------------
sensor = Sensor(id=2, width=1280, height=960, fps=90)
sensor.reset()
sensor.set_framesize(width=image_shape[1], height=image_shape[0])
sensor.set_pixformat(Sensor.GRAYSCALE)

# -------------------------------
# 初始化显示器
# -------------------------------
Display.init(Display.ST7701, width=image_shape[1], height=image_shape[0],
             to_ide=True, quality=50)

# -------------------------------
# 初始化媒体系统
# -------------------------------
MediaManager.init()
sensor.run()

# -------------------------------
# 可选增益设置
# -------------------------------
gain = k_sensor_gain()
gain.gain[0] = 40
sensor.again(gain)

# -------------------------------
# 启动帧率计时
# -------------------------------
clock = time.clock()

# -------------------------------
# 矩形检测可调参数
# -------------------------------
canny_thresh1      = 50
canny_thresh2      = 150
approx_epsilon     = 0.04
area_min_ratio     = 0.001
max_angle_cos      = 0.3
gaussian_blur_size = 5

# 矩形筛选参数
target_aspect    = 1.414    # 横切半幅(A5) ≈ 1.414
aspect_tolerance = 0.2      # 宽高比容差

# -------------------------------
# 激光检测可调参数
# -------------------------------
laser_threshold = 230
laser_roi_size  = 5

# -------------------------------
# 圆形光斑验证参数
# -------------------------------
laser_spot_check_radius = 8
laser_spot_roundness    = 0.4

# -------------------------------
# 动态ROI跟踪变量
# -------------------------------
tracking_roi_w = 80
tracking_roi_h = 80
laser_initialized = False
roi_x = 0
roi_y = 0
lost_frames = 0
max_lost_frames = 10


# ============================================================
# ★辅助函数：手动计算标准差（ulab.numpy 没有 np.std）
# ============================================================
def manual_std(arr):
    """手动计算数组的标准差"""
    n = len(arr)
    if n < 2:
        return 0.0
    mean_val = np.sum(arr) / n
    sq_sum = 0.0
    for i in range(n):
        d = arr[i] - mean_val
        sq_sum += d * d
    var = sq_sum / n
    return np.sqrt(var)


# ============================================================
# 激光圆形光斑验证函数
# ============================================================
def is_laser_spot(img_np, cx, cy, check_radius=8):
    """
    判断候选点是否为圆形激光光斑（而非矩形角/边缘）。
    原理：激光光斑各方向对称、中心最亮向外衰减；
          矩形角则不对称，亮度沿某条线分布。
    返回: (is_spot: bool, roundness: float)
    """
    h, w = img_np.shape
    if cx < check_radius or cx >= w - check_radius:
        return False, 0.0
    if cy < check_radius or cy >= h - check_radius:
        return False, 0.0

    center_val = float(img_np[cy, cx])
    if center_val <= 0:
        return False, 0.0

    # --- 8方向采样 ---
    directions = [
        (0, -1), (1, -1), (1, 0), (1, 1),
        (0, 1), (-1, 1), (-1, 0), (-1, -1)
    ]
    dists = [check_radius // 2, check_radius, check_radius * 3 // 2]
    if dists[2] <= dists[0]:
        dists = [2, 4, 6]

    # 每个距离环收集8个方向的亮度值
    ring_vals = [[], [], []]

    for dx, dy in directions:
        for di in range(3):
            d = dists[di]
            sx = cx + dx * d
            sy = cy + dy * d
            if 0 <= sx < w and 0 <= sy < h:
                ring_vals[di].append(float(img_np[sy, sx]))

    # --- 对称性评分：同距离各方向亮度应该接近 ---
    symmetry_scores = []
    for vals in ring_vals:
        if len(vals) >= 4:
            arr = np.array(vals)
            m = float(np.sum(arr)) / len(vals)
            if m > 0:
                s = manual_std(arr)
                score = 1.0 - s / (m + 1.0)
                if score < 0.0:
                    score = 0.0
                symmetry_scores.append(score)

    if len(symmetry_scores) == 0:
        symmetry_score = 0.0
    else:
        symmetry_score = sum(symmetry_scores) / len(symmetry_scores)

    # --- 径向衰减检查：中心 > 内环 > 中环 > 外环 ---
    ring_means = []
    for vals in ring_vals:
        if len(vals) > 0:
            ring_means.append(sum(vals) / len(vals))
        else:
            ring_means.append(0.0)

    decay_score = 1.0
    prev = center_val
    for m in ring_means:
        if m > prev:
            decay_score -= 0.3
            if decay_score < 0.0:
                decay_score = 0.0
        prev = m

    # --- 综合圆形度量 ---
    roundness = symmetry_score * decay_score
    if roundness < 0.0:
        roundness = 0.0
    if roundness > 1.0:
        roundness = 1.0

    is_spot = roundness >= laser_spot_roundness
    return is_spot, roundness


# ============================================================
# 主循环
# ============================================================
while True:
    clock.tick()

    # 捕获一帧图像
    img = sensor.snapshot()
    img_np = img.to_numpy_ref()

    # 图像中心坐标
    img_center_x = image_shape[1] // 2   # 640/2 = 320
    img_center_y = image_shape[0] // 2   # 480/2 = 240

    # ========================================================
    # 第一部分：矩形检测（含对角线交点中心点）
    # ========================================================
    rects = cv_lite.grayscale_find_rectangles_with_corners(
        image_shape, img_np,
        canny_thresh1, canny_thresh2,
        approx_epsilon,
        area_min_ratio,
        max_angle_cos,
        gaussian_blur_size
    )

    best_r = None
    best_dist = float('inf')

    # 遍历检测到的矩形，筛选宽高比匹配且最靠近图像中心的
    for i in range(len(rects)):
        r = rects[i]
        w, h = r[2], r[3]
        aspect = max(w, h) / min(w, h) if min(w, h) > 0 else 0
        if abs(aspect - target_aspect) > aspect_tolerance:
            continue

        # 求对角线交点（近似矩形中心）及离图像中心的距离
        c1x, c1y = r[4], r[5]
        c2x, c2y = r[6], r[7]
        c3x, c3y = r[8], r[9]
        c4x, c4y = r[10], r[11]
        denom = (c1x - c3x) * (c2y - c4y) - (c1y - c3y) * (c2x - c4x)
        if denom != 0:
            cx = ((c1x*c3y - c1y*c3x) * (c2x - c4x) - (c1x - c3x) * (c2x*c4y - c2y*c4x)) // denom
            cy = ((c1x*c3y - c1y*c3x) * (c2y - c4y) - (c1y - c3y) * (c2x*c4y - c2y*c4x)) // denom
        else:
            cx = r[0] + r[2] // 2
            cy = r[1] + r[3] // 2

        dist = ((cx - img_center_x)**2 + (cy - img_center_y)**2) ** 0.5
        if dist < best_dist:
            best_dist = dist
            best_r = r

    # 提取最终矩形中心
    rect_cx = 0
    rect_cy = 0
    rect_found = False

    if best_r is not None:
        r = best_r
        c1x, c1y = r[4], r[5]
        c2x, c2y = r[6], r[7]
        c3x, c3y = r[8], r[9]
        c4x, c4y = r[10], r[11]

        denom = (c1x - c3x) * (c2y - c4y) - (c1y - c3y) * (c2x - c4x)
        if denom != 0:
            rect_cx = ((c1x*c3y - c1y*c3x) * (c2x - c4x) - (c1x - c3x) * (c2x*c4y - c2y*c4x)) // denom
            rect_cy = ((c1x*c3y - c1y*c3x) * (c2y - c4y) - (c1y - c3y) * (c2x*c4y - c2y*c4x)) // denom
        else:
            rect_cx = r[0] + r[2] // 2
            rect_cy = r[1] + r[3] // 2

        rect_found = True

        # 绘制矩形中心十字（红色）
        img.draw_cross(rect_cx, rect_cy, color=(255, 0, 0), size=8, thickness=3)

    # ========================================================
    # 第二部分：激光检测（动态ROI跟踪 + 圆形光斑验证）
    # ========================================================
    laser_detected = False
    laser_cx = 0
    laser_cy = 0

    if not laser_initialized:
        # ---------- 阶段1：全局搜索 ----------
        max_val = np.max(img_np)
        if max_val >= laser_threshold:
            max_idx = np.argmax(img_np)
            laser_y = max_idx // image_shape[1]
            laser_x = max_idx % image_shape[1]

            # 圆形光斑验证
            is_spot, roundness = is_laser_spot(img_np, laser_x, laser_y,
                                               check_radius=laser_spot_check_radius)

            if is_spot:
                roi_x1 = max(0, laser_x - laser_roi_size)
                roi_y1 = max(0, laser_y - laser_roi_size)
                roi_x2 = min(image_shape[1], laser_x + laser_roi_size + 1)
                roi_y2 = min(image_shape[0], laser_y + laser_roi_size + 1)

                roi = img_np[roi_y1:roi_y2, roi_x1:roi_x2]
                roi_sum = np.sum(roi)
                if roi_sum > 0:
                    sum_x = 0
                    sum_y = 0
                    for dy in range(roi.shape[0]):
                        for dx in range(roi.shape[1]):
                            val = roi[dy, dx]
                            sum_x += (roi_x1 + dx) * val
                            sum_y += (roi_y1 + dy) * val
                    laser_cx = sum_x // roi_sum
                    laser_cy = sum_y // roi_sum
                else:
                    laser_cx, laser_cy = laser_x, laser_y

                roi_x = laser_cx - tracking_roi_w // 2
                roi_y = laser_cy - tracking_roi_h // 2
                laser_initialized = True
                lost_frames = 0
                laser_detected = True

    else:
        # ---------- 阶段2：ROI跟踪 ----------
        rx1 = max(0, roi_x)
        ry1 = max(0, roi_y)
        rx2 = min(image_shape[1], roi_x + tracking_roi_w)
        ry2 = min(image_shape[0], roi_y + tracking_roi_h)

        if rx2 > rx1 and ry2 > ry1:
            roi_img = img_np[ry1:ry2, rx1:rx2]
            roi_max = np.max(roi_img)

            if roi_max >= laser_threshold:
                roi_max_idx = np.argmax(roi_img)
                local_y = roi_max_idx // roi_img.shape[1]
                local_x = roi_max_idx % roi_img.shape[1]
                laser_x = rx1 + local_x
                laser_y = ry1 + local_y

                qx1 = max(0, laser_x - laser_roi_size)
                qy1 = max(0, laser_y - laser_roi_size)
                qx2 = min(image_shape[1], laser_x + laser_roi_size + 1)
                qy2 = min(image_shape[0], laser_y + laser_roi_size + 1)

                qroi = img_np[qy1:qy2, qx1:qx2]
                qroi_sum = np.sum(qroi)
                if qroi_sum > 0:
                    sum_x = 0
                    sum_y = 0
                    for dy in range(qroi.shape[0]):
                        for dx in range(qroi.shape[1]):
                            val = qroi[dy, dx]
                            sum_x += (qx1 + dx) * val
                            sum_y += (qy1 + dy) * val
                    laser_cx = sum_x // qroi_sum
                    laser_cy = sum_y // qroi_sum
                else:
                    laser_cx, laser_cy = laser_x, laser_y

                roi_x = laser_cx - tracking_roi_w // 2
                roi_y = laser_cy - tracking_roi_h // 2
                lost_frames = 0
                laser_detected = True
            else:
                lost_frames += 1
                if lost_frames >= max_lost_frames:
                    laser_initialized = False
                    lost_frames = 0
        else:
            laser_initialized = False
            lost_frames = 0

    # ========================================================
    # 绘制连线 + 输出位置差（另一点 - 矩形中点）
    # ========================================================
    if rect_found:
        if laser_detected:
            img.draw_line(laser_cx, laser_cy, rect_cx, rect_cy,
                          color=(0, 255, 255), thickness=2)
            offset_x = laser_cx - rect_cx
            offset_y = rect_cy - laser_cy
            print("rect_cx=%d laser_cx=%d offset_x=%d offset_y=%d" %
                  (rect_cx, laser_cx, offset_x, offset_y))
        else:
            img.draw_line(img_center_x, img_center_y, rect_cx, rect_cy,
                          color=(0, 255, 255), thickness=2)
            offset_x = img_center_x - rect_cx
            offset_y = rect_cy - img_center_y
            print("rect_cx=%d center_x=%d offset_x=%d offset_y=%d" %
                  (rect_cx, img_center_x, offset_x, offset_y))

        uart.send("o_x=%d o_y=%d/n"%
                (offset_x, offset_y))
    # 绘制激光标记
    if laser_detected:
        img.draw_cross(laser_cx, laser_cy, color=(255, 255, 255), size=12, thickness=2)
        img.draw_circle(laser_cx, laser_cy, 20, color=(255, 255, 255), thickness=2)
        img.draw_rectangle(
            max(0, roi_x), max(0, roi_y),
            tracking_roi_w, tracking_roi_h,
            color=(0, 255, 0), thickness=1
        )

    # 屏幕中心十字（绿色）
    img.draw_cross(img_center_x, img_center_y,
                   color=(0, 255, 0), size=10, thickness=2)

    # 显示图像
    Display.show_image(img)

    # 垃圾回收
    gc.collect()

# -------------------------------
# 程序退出与资源释放
# -------------------------------
uart.deinit()
sensor.stop()
Display.deinit()
os.exitpoint(os.EXITPOINT_ENABLE_SLEEP)
time.sleep_ms(100)
MediaManager.deinit()
