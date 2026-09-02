#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "sys.h"

/**************************************************************************
HC-SR04 超声波测距模块
接线：TRIG -> PB5，ECHO -> PA12，VCC -> 5V，GND -> GND。
距离单位：cm。返回 0 表示本次测量无效或超时。
**************************************************************************/

#define ULTRASONIC_TRIG_GPIO_PORT        GPIOB
#define ULTRASONIC_TRIG_GPIO_PIN         GPIO_PIN_5
#define ULTRASONIC_ECHO_GPIO_PORT        GPIOA
#define ULTRASONIC_ECHO_GPIO_PIN         GPIO_PIN_12

/* 停车阈值：18cm。10cm 对高速小车偏近，18cm 给传感器周期和电机惯性留余量。 */
#define ULTRASONIC_STOP_DISTANCE_CM      18U
/* 解除阈值做成 25cm，形成滞回，防止距离在 18cm 附近抖动造成反复启停。 */
#define ULTRASONIC_CLEAR_DISTANCE_CM     25U
/* 慢速区：小于 30cm 先降速，接近 18cm 再停车，减小急停冲击。 */
#define ULTRASONIC_SLOW_DISTANCE_CM      30U

void Ultrasonic_Init(void);
uint16_t Ultrasonic_ReadDistanceCm(void);
void Ultrasonic_Update(void);
uint16_t Ultrasonic_GetDistanceCm(void);
u8 Ultrasonic_IsDistanceValid(void);
u8 Ultrasonic_IsObstacleStop(void);
u8 Ultrasonic_ShouldSlowDown(void);

#endif
