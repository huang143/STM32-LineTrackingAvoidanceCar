#ifndef __CAMERA_H
#define __CAMERA_H

#include "sys.h"
#include "usart.h"

/**************************************************************************
OpenMV 摄像头串口避障数据
接线：OpenMV TX -> STM32 PA10(USART1_RX)，OpenMV RX -> STM32 PA9(USART1_TX，可选)，GND 共地。
当前 OpenMV 协议：
$O,valid,zone,cx,cy,w,h,area,dist_cm,band,line_w,exp_w,type,fps#

type: 0=无障碍，1=黑色/暗色，3=橙色/彩色。白色/亮色 type=2 已停用。
**************************************************************************/

typedef struct
{
    u8 valid;          /* 1=OpenMV 判断有障碍，0=无障碍 */
    int zone;          /* -1=障碍偏左，0=居中/正前方，1=偏右，9=无障碍 */
    int cx;            /* 障碍 blob 中心 x */
    int cy;            /* 障碍 blob 中心 y */
    int w;             /* 障碍 blob 像素宽 */
    int h;             /* 障碍 blob 像素高 */
    long area;         /* 障碍 blob 像素面积 */
    int dist_cm;       /* OpenMV 根据 cy 粗略估算的距离；精确停车仍以超声波为准 */
    int band;          /* 命中的扫描带，-1=无障碍 */
    int line_w;        /* 底部黑线像素宽度，仅调试 */
    int exp_w;         /* 该 y 位置预计正常黑线宽度，仅调试 */
    int type;          /* 0=无障碍，1=黑/暗，3=橙/彩 */
    int fps;           /* OpenMV 帧率，仅调试 */
    u32 last_update_ms;
    u8 new_frame;
} camera_data_t;

/* OpenMV 已经做过过滤，STM32 这里只做很低的面积兜底，避免 5cm 小障碍被过滤掉。 */
#define CAMERA_OBS_AREA_MIN          30L
#define CAMERA_FRAME_TIMEOUT_MS      400U
#define CAMERA_CONFIRM_FRAMES        2U

extern camera_data_t g_camera_data;
extern u8 Camera_RxByte;

void Camera_Init(void);
void Camera_UartRxCpltCallback(UART_HandleTypeDef *huart);
void Camera_InputByte(u8 ch);
u8 Camera_IsOnline(void);
u8 Camera_IsObstacleConfirmed(void);
u8 Camera_IsLineVisible(void);
const camera_data_t *Camera_GetData(void);

#endif

