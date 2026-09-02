#ifndef __CONTROL_H
#define __CONTROL_H

#include "sys.h"
#include "tim.h"

/**************************************************************************
项目功能：STM32F103C8T6 + TB6612FNG + MG310 编码器电机 + 八路 IO 巡线。
说明    ：本文件集中配置电机 PWM、左右补偿、低速/高速模式、巡线参数、
          起点识别和调试开关等参数。
**************************************************************************/

#define PWMA                TIM1->CCR1
#define PWMB                TIM1->CCR4

#define BIN2                PBout(12)
#define BIN1                PBout(13)
#define AIN1                PBout(14)
#define AIN2                PBout(15)

#define PWM_MAX             7199
#define PWM_LIMIT           6900
#define PWM_BASE            LOW_PWM_BASE

/*
 * 左右轮基础 PWM 补偿：用于修正左右电机、驱动通道、轮胎摩擦造成的动力差。
 * 正值表示该侧 PWM 增大，负值表示该侧 PWM 减小。
 */
#define LEFT_PWM_COMPENSATION   0
#define RIGHT_PWM_COMPENSATION  LOW_RIGHT_PWM_COMP

/*
 * 启动增益：只在起步前一小段时间额外加 PWM，用来克服静摩擦。
 * 右轮基础 PWM = PWM_BASE + RIGHT_PWM_COMPENSATION；当前约 3400 - 1800 = 1600。
 * 再通过 PWM_START_MIN/RIGHT_START_BOOST 提高起步和过褶皱时的最低动力。
 */
#define START_BOOST_TIME_MS     300
#define LEFT_START_BOOST        0
#define RIGHT_START_BOOST       700
#define PWM_START_MIN       1500

#define MODE_LOW                0
#define MODE_HIGH               1
#define MODE_KEY_GPIO_PORT      GPIOB
#define MODE_KEY_GPIO_PIN       GPIO_PIN_8
#define MODE_KEY_DEBOUNCE_MS    40
#define MODE_SELECT_TIME_MS     3000    /* 上电后 3 秒内可按 PB8 选择低速/高速。 */
#define LOW_CONTROL_PERIOD_MS   20      /* 低速恢复原来的 20ms，避免 10ms 导致低速也发扭。 */
#define HIGH_CONTROL_PERIOD_MS  10      /* 高速保持 10ms，提高起点和弯道检测频率。 */

/* 低速参数：沿用当前已能摆正的设置，作为约 20s 一圈的稳定调试模式。 */
#define LOW_PWM_BASE            3400
#define LOW_RIGHT_PWM_COMP      -1800
#define LOW_LINE_KP             15.0f
#define LOW_LINE_KD             5.0f
#define LOW_LINE_OUT_LIMIT      900.0f
#define LOW_LOST_ERROR          50

/* 高速参数：目标 12s 内一圈；若仍超过 12s，可继续小步提高 HIGH_PWM_BASE。 */
#define HIGH_PWM_BASE           5600
#define HIGH_RIGHT_PWM_COMP     -2200
#define HIGH_LINE_KP            24.0f
#define HIGH_LINE_KD            4.0f
#define HIGH_LINE_OUT_LIMIT     750.0f
#define HIGH_LOST_ERROR         35
#define HIGH_CURVE_SLOWDOWN_PWM 1300

/* 高速自适应巡线：直线小偏差用小 KP 防蛇形，弯道/丢线用大 KP 和更大限幅找线。 */
#define HIGH_LINE_SMALL_ERR     15
#define HIGH_LINE_LARGE_ERR     25
#define HIGH_LINE_KP_SMALL      12.0f
#define HIGH_LINE_KP_MID        22.0f
#define HIGH_LINE_KP_LARGE      30.0f
#define HIGH_LINE_OUT_SMALL     550.0f
#define HIGH_LINE_OUT_MID       850.0f
#define HIGH_LINE_OUT_LARGE     1300.0f
#define HIGH_CURVE_SLOW_ERR     15

#define RUN_TIME_MS         120000
#define CONTROL_PERIOD_MS   LOW_CONTROL_PERIOD_MS

/*
 * 编码器反馈开关：1 表示启用左右轮编码器同步修正；0 表示只按 PWM 开环运行。
 */
#define USE_ENCODER_FEEDBACK 1

/*
 * 八路 IO 巡线开关与参数。
 * LINE_KP/LINE_KD 用 Track_GetError() 的偏差调节左右轮差速。
 */

/*
 * 超声波避障开关：HC-SR04，TRIG=PB5，ECHO=PA12。
 * 小于 30cm 先降速，小于 18cm 停车；18cm 比 10cm 更适合当前有惯性的小车。
 */
#define USE_ULTRASONIC_OBSTACLE 1
#define USE_CAMERA_OBSTACLE_AVOID 1
#define AVOID_START_DISTANCE_CM    35U   /* 由 28cm 提前到 35cm，给横向避让留距离。 */
#define AVOID_STOP_HOLD_MS        250U
#define AVOID_CONFIRM_COUNT       2U     /* 超声波距离+摄像头确认连续成立次数，成立后才真正进入避障。 */

/* 避障动作参数：按“梯形绕障”执行：转出 -> 斜向离开 -> 回正 -> 平行通过 ->
 * 转回第二条腰角度 -> 沿固定角度找线 -> 低速摆正。 */
#define AVOID_PIVOT_PWM           2300   /* 转出/转回阶段外侧轮 PWM。 */
#define AVOID_PIVOT_REVERSE_PWM   800    /* 转出/转回阶段内侧轮反转 PWM。 */
#define AVOID_ALIGN_PWM           2800   /* 车头回正阶段外侧轮 PWM；右轮有缩放，略高一点用于抵消。 */
#define AVOID_ALIGN_INNER_PWM     1200   /* 车头回正阶段内侧轮前进 PWM：ALIGN 改为前进差速，不再反转。 */
#define AVOID_FORWARD_PWM         3100   /* 平行越过障碍物时直行 PWM。 */

/* 避障动作右轮平衡：避障不走 RIGHT_PWM_COMPENSATION，因此单独缩放右轮 PWM。
 * 100 表示不缩放；小于 100 减小右轮动力。MIN_ABS 用于防止缩放后 PWM 太小带不动。
 */
#define AVOID_RIGHT_PWM_SCALE_X100 85
#define AVOID_RIGHT_PWM_MIN_ABS    800

#define AVOID_RETURN_TIMEOUT_MS   1800   /* 第 5 步沿固定角度找线的最长时间。 */
#define AVOID_LINE_HOLD_COUNT     2

/*
 * 左/右绕障独立修正。
 * BACK_TURN 必须大于 PIVOT：FIND 现在是固定角度直行，不再像旧 REJOIN
 * 那样通过差速继续补角，所以第二腰角度需要由 BACK 单独承担。
 * 屏幕 5 时车身仍接近平行：继续加大 BACK_TURN_SCALE_X100；
 * 若八路同时压线接近垂直：调小 BACK_TURN_SCALE_X100。
 * 向左绕实测更容易回不到轨道，因此默认转角、离开距离和找线时间更大。
 * 若向左绕仍找不到线：优先增大 AVOID_LEFT_BACK_TURN_SCALE_X100，
 * 或增大 AVOID_LEFT_RETURN_TIMEOUT_MS。
 * 若向左绕距离障碍太近：增大 AVOID_LEFT_EXIT_SCALE_X100。
 */
#define AVOID_RIGHT_PIVOT_TURN_SCALE_X100 100
#define AVOID_RIGHT_ALIGN_TURN_SCALE_X100 100
#define AVOID_RIGHT_BACK_TURN_SCALE_X100  400
#define AVOID_RIGHT_EXIT_SCALE_X100       100
#define AVOID_RIGHT_PASS_SCALE_X100       100
#define AVOID_RIGHT_RETURN_TIMEOUT_MS     AVOID_RETURN_TIMEOUT_MS

#define AVOID_LEFT_PIVOT_TURN_SCALE_X100  100
#define AVOID_LEFT_ALIGN_TURN_SCALE_X100  100
#define AVOID_LEFT_BACK_TURN_SCALE_X100   400
#define AVOID_LEFT_EXIT_SCALE_X100        115
#define AVOID_LEFT_PASS_SCALE_X100        100
#define AVOID_LEFT_RETURN_TIMEOUT_MS      2500U

/* 避障转向闭环：PIVOT/ALIGN/BACK 按左右编码器差值控制角度；超时兜底避免卡死。 */
#define AVOID_LARGE_TURN_TARGET      150
#define AVOID_MID_TURN_TARGET        130
#define AVOID_SMALL_TURN_TARGET      110
#define AVOID_TURN_TIMEOUT_MS        1500U
#define AVOID_TURN_CONTROL_MS        10U

/*
 * 转出后的斜向离开距离：PIVOT 只负责转角，EXIT 让车沿转出后的斜方向前进一段，形成梯形第一条腰。
 * 数值越大，离开原轨道越远；若距离不够，优先加大 AVOID_xxx_EXIT_ENCODER_TARGET。
 */
#define AVOID_EXIT_PWM               2600
#define AVOID_LARGE_EXIT_ENCODER_TARGET  820
#define AVOID_MID_EXIT_ENCODER_TARGET    710
#define AVOID_SMALL_EXIT_ENCODER_TARGET  600
#define AVOID_EXIT_TIMEOUT_MS        2000U

/*
 * 回到轨道的第二条腰：镜像第一条腰(PIVOT+EXIT)。
 * 这里“BACK_TURN”不是小幅朝轨道方向微调，而是第五步即将沿它直行的
 * 那条斜腰角度。FIND 不会再差速补角，所以 BACK_TURN 必须与第一条腰
 * PIVOT 同量级甚至略大，否则屏幕 5 时车身仍会接近平行轨道。
 * 默认右绕 150%、左绕 170% 就是用于保证足够斜角；
 * 若显示 5 仍接近平行则继续加大，若八路同时压线则调小。
 * BACK_SEARCH_PWM 与第一条腰 EXIT 使用同一套对称 PWM，
 * 不再用差速“画弧”，避免回线过程中车头角度越来越大、变成垂直压线。
 */
#define AVOID_BACK_SEARCH_PWM         AVOID_EXIT_PWM
#define AVOID_FIND_MAX_ENCODER_TARGET 30000   /* 只作安全上限；正常由 RETURN_TIMEOUT_MS 结束。 */

/*
 * 找到黑线后的低速摆正阶段：在避障内用小 PD 让黑线回到 X4/X5 中缝，
 * 连续 AVOID_REALIGN_HOLD_COUNT 次稳定后才算避障真正完成。
 */
#define AVOID_REALIGN_BASE_PWM        2400
#define AVOID_REALIGN_KP              8.0f
#define AVOID_REALIGN_KD              4.0f
#define AVOID_REALIGN_OUT_LIMIT       700.0f
#define AVOID_REALIGN_CENTER_ERROR    8
#define AVOID_REALIGN_CENTER_MASK     ((1U << 3) | (1U << 4))   /* X4/X5。 */
#define AVOID_REALIGN_HOLD_COUNT      4
#define AVOID_REALIGN_TIMEOUT_MS      1800U
#define AVOID_REALIGN_LOST_ERROR      25
/* 根据 OpenMV 障碍物图像大小/位置动态调整避障动作。 */
#define AVOID_IMAGE_CENTER_X      80     /* QQVGA 宽 160，中心 x=80。 */
#define AVOID_CENTER_NEAR_PX      35     /* cx 离中心小于该值，认为比较靠近车头中心线。 */
#define AVOID_SIZE_LARGE_WIDTH_PX 45     /* blob 宽度达到该值，按大障碍处理。 */
#define AVOID_SIZE_MID_WIDTH_PX   28     /* blob 宽度达到该值，按中障碍处理。 */
#define AVOID_LARGE_PASS_MS       1800   /* 大障碍平行直行通过距离。 */
#define AVOID_MID_PASS_MS         1630    /* 中障碍平行直行通过距离，按大障碍约 87% 缩放。 */
#define AVOID_SMALL_PASS_MS       1450     /* 小障碍平行直行通过距离，按大障碍约 73% 缩放。 */
#define ULTRASONIC_SLOW_PWM_LIMIT 3600
#define USE_LINE_TRACKING   1
#define LINE_KP             LOW_LINE_KP
#define LINE_KD             LOW_LINE_KD
#define LINE_OUT_LIMIT      LOW_LINE_OUT_LIMIT
#define LINE_ERROR_OFFSET   6       /* 车身全程偏左时加正值；偏右时改小或改成负值。 */
#define LINE_LOST_ERROR      LOW_LOST_ERROR
#define LAP_STOP_ENABLE     1
#define LAP_MARKER_MIN_COUNT 5       /* 高速下放宽起点识别：至少 5 路检测到黑线。 */
#define LAP_MARKER_REQUIRED_MASK 0x00 /* 0 表示不强制指定探头，避免斜着进起点时漏判。 */
#define LAP_MARKER_HOLD_COUNT  1     /* 高速经过起点时间很短，命中 1 次即停车。 */
#define LAP_LEAVE_TIME_MS   500     /* 离开起点标记持续时间，防止刚起步误判。 */
#define LAP_MIN_RUN_TIME_MS 3000     /* 最短运行 3s 后允许停车；避免跑得快时因 10s 门槛错过起点。 */
#define LAP_MARKER_AFTER_LOSS_MS 0  /* 不再因丢线屏蔽起点判断，避免高速回起点前丢线后漏停。 */

/* 重复运行测试开关：1 表示停车后间隔一段时间自动再次跑圈；0 表示跑完一圈后保持停车。 */
#define REPEAT_RUN_TEST     0

/*
 * A/B 电机通道自检开关：用于单独测试左、右电机驱动通道是否正常。
 * 正常比赛/跑圈时保持 0。
 */
#define MOTOR_CHANNEL_SELF_TEST 0
#define MOTOR_SELF_TEST_PWM     5000

/*
 * PA8 PWM 输出诊断开关：用于排查 PWMA/TIM1_CH1 输出问题。
 * 正常比赛/跑圈时保持 0。
 */
#define PA8_GPIO_OUTPUT_TEST 0
/*
 * PB9 外接运行指示 LED：运行开始后闪烁，用于确认程序进入跑圈流程。
 */
#define RUN_LED_PIN_PB9        PBout(9)
#define RUN_LED_BLINK_TIME_MS  150

#define LEFT_ENCODER_TIM    3
#define RIGHT_ENCODER_TIM   4

#define LEFT_MOTOR_DIR      1
#define RIGHT_MOTOR_DIR     1
#define LEFT_ENCODER_DIR    -1
#define RIGHT_ENCODER_DIR   -1

#define STRAIGHT_KP         1.0f
#define STRAIGHT_KD         6.0f
#define STRAIGHT_OUT_LIMIT  800.0f

uint32_t LineTracking_Run_OneLap(void);
void Mode_Key_Init(void);
void Mode_Select_Before_Run(void);
void Set_Pwm(int motor_a, int motor_b);
void Motor_Stop(void);
void Motor_Channel_Self_Test(void);
void PA8_GPIO_Output_Test(void);

int myabs(int a);
int limit(int input, int min, int max);

#endif

