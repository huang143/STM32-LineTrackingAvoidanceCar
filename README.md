# 巡线 + 避障智能小车

本项目是在原八路红外巡线小车基础上增加 OpenMV 视觉识别与 HC-SR04 超声波测距，实现“巡线行驶、识别障碍、绕开障碍、重新找线、回到起点停车”的完整流程。

## 当前保留内容

```text
巡线+避障/
├── 代码/                  STM32F103C8T6 主控工程（Keil / STM32CubeMX）
│   ├── Core/              CubeMX 生成的初始化代码
│   ├── Drivers/           STM32 HAL / CMSIS
│   ├── Hardware/          电机外设、巡线、编码器、OLED、OpenMV、超声波等驱动
│   ├── BALANCE/CONTROL/   运动控制、巡线控制、避障状态机
│   ├── SYSTEM/            delay/sys 等基础支持
│   ├── MDK-ARM/           Keil 工程文件
│   └── LineTrackingCar.ioc CubeMX 配置
├── openmv/
│   └── main.py            OpenMV 当前使用脚本
├── docs/
│   ├── 代码架构.md
│   └── 方法说明.md
└── README.md
```

备份文件、探索脚本、OpenMV 旧方案说明、Keil 输出文件和 HC-SR04 参考压缩包已删除；探索过程中的取舍记录整理到了 `docs/方法说明.md`。

## 使用方法

1. 用 Keil 打开 `代码/MDK-ARM/LineTrackingCar.uvprojx`，编译并下载到 STM32F103C8T6。
2. 将 `openmv/main.py` 复制到 OpenMV，确认 UART3 波特率为 115200。
3. 确认接线：
   - OpenMV TX -> STM32 PA10（USART1_RX），GND 共地；
   - HC-SR04 TRIG -> PB5，ECHO -> PA12；
   - TB6612、电机、编码器、OLED、八路巡线模块按工程引脚配置连接。
4. 上电后 3 秒内可按 PB8 切换 LOW/HIGH 模式；随后小车开始巡线。

## 当前算法概览

- **巡线**：STM32 读取八路 IO 巡线模块，按偏差做 PD 差速控制；高速模式下根据偏差大小自适应调整转向增益和限幅。
- **测距安全**：HC-SR04 提供实际距离，35 cm 内且视觉确认后进入绕障；18 cm 内作为安全底线立即停车。
- **视觉确认**：OpenMV 使用底部黑线宽度估计透视下的正常线宽，在上方两层 ROI 检测“明显比黑线更宽的暗色块”，同时检测橙色/彩色障碍。
- **绕障动作**：STM32 按“转出 -> 斜向离开 -> 回正 -> 平行通过 -> 转回找线”的梯形路径执行；转角和离开距离用编码器计数控制，减少只靠延时造成的不稳定。
- **回线与停车**：绕障后通过八路巡线重新找黑线；回到起点宽黑线标记后停车。

## 参数入口

- STM32 主参数：`代码/BALANCE/CONTROL/control.h`
- STM32 避障逻辑：`代码/BALANCE/CONTROL/control.c`
- OpenMV 阈值与 ROI：`openmv/main.py`
- 超声波驱动：`代码/Hardware/ULTRASONIC/ultrasonic.c/.h`
- OpenMV 串口解析：`代码/Hardware/CAMERA/camera.c/.h`

更详细的设计理由见 `docs/方法说明.md`，模块调用关系见 `docs/代码架构.md`。
