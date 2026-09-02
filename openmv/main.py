# 巡线+避障 OpenMV 主程序
# 方法：底部 ROI 估计黑线宽度，上方多层 ROI 同时检测“异常变宽的暗色块”和橙色块；
#       通过 UART3 输出障碍物有效性、位置、尺寸和粗略距离，STM32 结合 HC-SR04 距离执行避障。
# 串口协议：$O,valid,zone,cx,cy,w,h,area,dist_cm,band,line_w,exp_w,type,fps#
# ===== 模块导入 =====
# sensor/image/time：OpenMV 摄像头、图像处理和计时模块。
import sensor, image, time
# UART：OpenMV 与 STM32 通信使用的串口类。
from pyb import UART
# ===== 串口通信参数 =====
# 这里完成串口部分的基础配置：使用 UART3，波特率 115200。
UART_ID = 3
UART_BAUD = 115200
DEBUG_PRINT_USB = True
# ===== 图像尺寸与中心线参数 =====
# QQVGA 图像大小为 160x120，CENTER_X 是画面水平中心，用于判断目标在左/中/右。
IMG_W = 160
IMG_H = 120
CENTER_X = IMG_W // 2
# 黑线真实宽度的放大参数，当前程序主要依赖图像中的线宽估计。
BLACK_LINE_REAL_WIDTH_X10 = 18
# ===== 颜色阈值配置 =====
# BLACK_THRESHOLDS：黑色/暗色阈值，用于识别巡线黑线，也用于识别黑色障碍物。
BLACK_THRESHOLDS = [
	(0, 35, -25, 25, -25, 25),
]
# ORANGE_THRESHOLDS：橙色阈值，用于识别橙色障碍物。
ORANGE_THRESHOLDS = [
	(30, 100, 10, 70, 10, 80),
]
# ===== ROI 区域配置 =====
# LINE_ROI：画面底部巡线检测区域，用来估计当前黑线宽度。
LINE_ROI = (0, 92, IMG_W, 24)
# OBSTACLE_BANDS：画面上方/中部多层障碍物检测区域。
OBSTACLE_BANDS = [
	(0, 66, IMG_W, 12),
	(0, 80, IMG_W, 10),
]
# ===== 目标位置与检测阈值 =====
# CENTER_X_MIN/CENTER_X_MAX：判断障碍物在左侧、中间、右侧的分界范围。
CENTER_X_MIN = 55
CENTER_X_MAX = 105
# 底部黑线检测的最小像素数和面积，过滤太小的噪声。
LINE_MIN_PIXELS = 15
LINE_MIN_AREA = 15
# 黑色障碍物检测阈值：像素、面积、宽高、填充率等，用于判断黑色块是否有效。
DARK_BAND_MIN_PIXELS = 12
DARK_BAND_MIN_AREA = 12
# 判断大小/宽度的参数：暗色目标宽度必须明显大于同高度处预计黑线宽度，避免把普通黑线误判为障碍物。
OBSTACLE_WIDTH_RATIO_X10 = 15
OBSTACLE_EXTRA_WIDTH_PX = 2
MIN_OBSTACLE_WIDTH_PX = 8
DARK_MIN_FILL_X100 = 30
DARK_MIN_HEIGHT_PX = 5
DARK_MAX_WIDTH_PX = 100
# 橙色障碍物检测阈值：像素、面积、宽高、填充率等，用于判断彩色块是否有效。
COLOR_BAND_MIN_PIXELS = 20
COLOR_BAND_MIN_AREA = 20
COLOR_MIN_WIDTH_PX = 8
COLOR_MIN_HEIGHT_PX = 5
COLOR_MIN_AREA_PX = 35
COLOR_MIN_FILL_X100 = 25
COLOR_MAX_WIDTH_PX = 110
# ===== 发送周期与镜头校正 =====
# SEND_PERIOD_MS：串口数据发送周期；ENABLE_LENS_CORR：是否启用镜头畸变校正。
SEND_PERIOD_MS = 50
ENABLE_LENS_CORR = False
LENS_CORR_STRENGTH = 1.8
# ===== 摄像头初始化 =====
# 设置 RGB565、QQVGA，关闭自动增益和自动白平衡，让颜色阈值识别更稳定。
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=1500)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
# 初始化串口对象，后面通过 uart.write() 将识别结果发送给 STM32。
uart = UART(UART_ID, UART_BAUD)
clock = time.clock()
last_send = time.ticks_ms()
# last_line_w：保存上一帧检测到的底部黑线宽度，当前帧没找到线时作为备用值。
last_line_w = 10
# 根据目标中心点 cx 判断目标区域：-1=左侧，0=中间，1=右侧。
def calc_zone(cx):
	if cx < CENTER_X_MIN:
		return -1
	if cx > CENTER_X_MAX:
		return 1
	return 0
# 根据目标在画面中的 y 坐标粗略估计距离：越靠近画面底部，通常表示越近。
def estimate_distance_by_y(cy):
	if cy >= 88:
		return 20
	if cy >= 74:
		return 28
	if cy >= 60:
		return 35
	if cy >= 46:
		return 45
	if cy >= 32:
		return 55
	return 70
# 根据 y 坐标给出线宽缩放比例，用于透视关系下估计不同高度处黑线应有宽度。
def line_width_scale_x100_by_y(y):
	if y >= 82:
		return 85
	if y >= 68:
		return 72
	if y >= 54:
		return 58
	if y >= 40:
		return 45
	if y >= 26:
		return 35
	return 28
# 结合底部实际检测到的黑线宽度，计算指定 y 位置处的预计黑线宽度。
def expected_line_width_at_y(y, bottom_line_w):
	scale = line_width_scale_x100_by_y(y)
	exp_w = (bottom_line_w * scale + 50) // 100
	return max(exp_w, 3)
# 在底部 LINE_ROI 中识别黑色巡线，并更新/返回当前黑线宽度。
def find_bottom_line_width(img):
	global last_line_w
	blobs = img.find_blobs(
		BLACK_THRESHOLDS,
		roi=LINE_ROI,
		pixels_threshold=LINE_MIN_PIXELS,
		area_threshold=LINE_MIN_AREA,
		merge=True,
		margin=2
	)
	best = None
	best_score = -1
	for b in blobs:
		center_penalty = abs(b.cx() - CENTER_X)
		if b.w() > 40:
			continue
		score = b.pixels() - center_penalty * 2
		if score > best_score:
			best = b
			best_score = score
	if best:
		last_line_w = best.w()
		img.draw_rectangle(best.rect(), color=(0, 255, 0))
		img.draw_cross(best.cx(), best.cy(), color=(0, 255, 0))
	return last_line_w
# 生成无障碍物时的默认结果，保证串口发送的数据字段完整。
def empty_result():
	return {
		'valid': 0,
		'zone': 9,
		'cx': 0,
		'cy': 0,
		'w': 0,
		'h': 0,
		'area': 0,
		'dist_cm': 0,
		'band': -1,
		'exp_w': 0,
		'type': 0,
		'rect': None,
		'score': 0
	}
# 将识别到的 blob 封装成统一结果，包括位置、大小、面积、距离、类型和评分。
def make_result(blob, band_idx, exp_w, obs_type, score):
	return {
		'valid': 1,
		'zone': calc_zone(blob.cx()),
		'cx': blob.cx(),
		'cy': blob.cy(),
		'w': blob.w(),
		'h': blob.h(),
		'area': blob.pixels(),
		'dist_cm': estimate_distance_by_y(blob.cy()),
		'band': band_idx,
		'exp_w': exp_w,
		'type': obs_type,
		'rect': blob.rect(),
		'score': score
	}
# 计算色块填充率：像素数 / 外接矩形面积，用于判断目标是否足够实心。
def blob_fill_x100(blob):
	rect_area = blob.w() * blob.h()
	if rect_area <= 0:
		return 0
	return (blob.pixels() * 100) // rect_area
# 识别黑色障碍物：在指定 ROI 中找暗色块，并通过宽度、高度、填充率判断它是否不是普通巡线。
def detect_dark_obstacle(img, roi, band_idx, bottom_line_w):
	best = None
	blobs = img.find_blobs(
		BLACK_THRESHOLDS,
		roi=roi,
		pixels_threshold=DARK_BAND_MIN_PIXELS,
		area_threshold=DARK_BAND_MIN_AREA,
		merge=True,
		margin=3
	)
	for b in blobs:
		exp_w = expected_line_width_at_y(b.cy(), bottom_line_w)
		width_th = (exp_w * OBSTACLE_WIDTH_RATIO_X10 + 9) // 10
		width_th = max(width_th, exp_w + OBSTACLE_EXTRA_WIDTH_PX, MIN_OBSTACLE_WIDTH_PX)
		fill = blob_fill_x100(b)
		if (b.w() >= width_th and
			b.h() >= DARK_MIN_HEIGHT_PX and
			b.w() <= DARK_MAX_WIDTH_PX and
			fill >= DARK_MIN_FILL_X100):
			center_bonus = max(0, 80 - abs(b.cx() - CENTER_X))
			score = b.w() * 5 + b.pixels() + fill * 2 + center_bonus
			cand = make_result(b, band_idx, exp_w, 1, score)
			if best is None or cand['score'] > best['score']:
				best = cand
	return best
# 识别彩色障碍物：按传入颜色阈值查找色块，并通过大小、面积和填充率筛选有效目标。
def detect_color_obstacle(img, roi, band_idx, thresholds, obs_type):
	best = None
	blobs = img.find_blobs(
		thresholds,
		roi=roi,
		pixels_threshold=COLOR_BAND_MIN_PIXELS,
		area_threshold=COLOR_BAND_MIN_AREA,
		merge=True,
		margin=3
	)
	for b in blobs:
		fill = blob_fill_x100(b)
		if (b.w() >= COLOR_MIN_WIDTH_PX and
			b.h() >= COLOR_MIN_HEIGHT_PX and
			b.pixels() >= COLOR_MIN_AREA_PX and
			b.w() <= COLOR_MAX_WIDTH_PX and
			fill >= COLOR_MIN_FILL_X100):
			center_bonus = max(0, 80 - abs(b.cx() - CENTER_X))
			score = b.w() * 4 + b.h() * 3 + b.pixels() + fill + center_bonus
			cand = make_result(b, band_idx, 0, obs_type, score)
			if best is None or cand['score'] > best['score']:
				best = cand
	return best
# 在两个候选障碍物之间选择评分更高的一个。
def choose_better(a, b):
	if a is None:
		return b
	if b is None:
		return a
	return b if b['score'] > a['score'] else a
# 综合障碍物检测：遍历多层 ROI，同时检测黑色障碍物和橙色障碍物，返回最可信目标。
def detect_obstacle(img, bottom_line_w):
	best = None
	for idx, roi in enumerate(OBSTACLE_BANDS):
		img.draw_rectangle(roi, color=(255, 255, 0))
		dark = detect_dark_obstacle(img, roi, idx, bottom_line_w)
		orange = detect_color_obstacle(img, roi, idx, ORANGE_THRESHOLDS, 3)
		best = choose_better(best, dark)
		best = choose_better(best, orange)
	if best:
		if best['type'] == 1:
			color = (255, 0, 0)
		else:
			color = (255, 0, 255)
		img.draw_rectangle(best['rect'], color=color)
		img.draw_cross(best['cx'], best['cy'], color=color)
		return best
	return empty_result()
# ===== 主循环 =====
# 每帧完成：拍照 -> 可选镜头校正 -> 底部黑线识别 -> 障碍物识别 -> 按周期串口发送结果。
while True:
	clock.tick()
	img = sensor.snapshot()
	if ENABLE_LENS_CORR:
		img.lens_corr(LENS_CORR_STRENGTH)
	# 识别底部黑线，得到当前黑线宽度，后续用于判断暗色目标是不是异常变宽。
	bottom_line_w = find_bottom_line_width(img)
	# 识别障碍物，包括黑色宽块和橙色块，并返回目标位置、大小、类型等信息。
	obs = detect_obstacle(img, bottom_line_w)
	img.draw_rectangle(LINE_ROI, color=(0, 255, 255))
	now = time.ticks_ms()
	# 串口发送部分：到达发送周期后，将识别结果打包为协议字符串发送给 STM32。
	if time.ticks_diff(now, last_send) >= SEND_PERIOD_MS:
		last_send = now
		msg = "$O,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d#" % (
			obs['valid'],
			obs['zone'],
			obs['cx'],
			obs['cy'],
			obs['w'],
			obs['h'],
			obs['area'],
			obs['dist_cm'],
			obs['band'],
			bottom_line_w,
			obs['exp_w'],
			obs['type'],
			int(clock.fps())
		)
		uart.write(msg + "\n")
		if DEBUG_PRINT_USB:
			print(msg)

