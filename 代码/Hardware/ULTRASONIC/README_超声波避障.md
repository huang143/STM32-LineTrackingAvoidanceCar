# HC-SR04 超声波避障接入说明

## 接线

- HC-SR04 VCC -> 5V
- HC-SR04 GND -> STM32 GND
- HC-SR04 TRIG -> PB5
- HC-SR04 ECHO -> PA12

注意：HC-SR04 的 ECHO 常见为 5V 电平；若你的 STM32 板子没有做电平兼容，建议给 ECHO 加分压或电平转换后再接 PA12。

## 停车距离

本工程设置：

- 30cm 内进入慢速区；
- 18cm 内停车；
- 25cm 以上解除停车状态。

没有采用 10cm 的原因：当前小车有电机惯性、控制周期和超声波测量周期，10cm 对高速模式偏近，容易来不及停。18cm 既接近“十厘米左右”的要求，又比 10cm 更安全；30cm 先降速可以减小急停冲击。

## 参考依据

参考 `HC-SR04参考代码.zip` 中 STM32F103C8T6 示例的测距原理：

1. TRIG 输出大于 10us 的高电平触发测距；
2. 等待 ECHO 高电平；
3. 记录 ECHO 高电平持续时间；
4. 使用 `距离(cm) = 时间(us) / 58` 换算距离。

本工程没有照搬参考代码中 TIM3 中断计时的方法，因为现有工程 TIM3/TIM4 已用于编码器，改为使用 Cortex-M3 DWT 周期计数做微秒计时。

## 相关文件

- `Hardware/ULTRASONIC/ultrasonic.c`
- `Hardware/ULTRASONIC/ultrasonic.h`
- `Core/Src/main.c`：调用 `Ultrasonic_Init()`。
- `BALANCE/CONTROL/control.c`：在控制循环中定期 `Ultrasonic_Update()`，近距离停车。
- `BALANCE/CONTROL/control.h`：超声波避障开关和慢速 PWM 限幅。
