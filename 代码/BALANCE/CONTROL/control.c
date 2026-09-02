#include "control.h"
#include "encoder.h"
#include "track.h"
#include "oled.h"
#include "ultrasonic.h"
#include "camera.h"

static u8 g_run_mode = MODE_LOW;
static int Start_Compensate(int pwm);

static const char *Control_GetModeName(void)
{
    return (g_run_mode == MODE_HIGH) ? "HIGH" : "LOW";
}

static int Control_GetBasePwm(void)
{
    return (g_run_mode == MODE_HIGH) ? HIGH_PWM_BASE : LOW_PWM_BASE;
}

static int Control_GetRightCompensation(void)
{
    return (g_run_mode == MODE_HIGH) ? HIGH_RIGHT_PWM_COMP : LOW_RIGHT_PWM_COMP;
}

static float Control_GetLineKp(void)
{
    return (g_run_mode == MODE_HIGH) ? HIGH_LINE_KP : LOW_LINE_KP;
}

static float Control_GetLineKd(void)
{
    return (g_run_mode == MODE_HIGH) ? HIGH_LINE_KD : LOW_LINE_KD;
}

static float Control_GetLineOutLimit(void)
{
    return (g_run_mode == MODE_HIGH) ? HIGH_LINE_OUT_LIMIT : LOW_LINE_OUT_LIMIT;
}

static int Control_GetLostError(void)
{
    return (g_run_mode == MODE_HIGH) ? HIGH_LOST_ERROR : LOW_LOST_ERROR;
}

static int Control_GetCurveSlowdownPwm(void)
{
    return (g_run_mode == MODE_HIGH) ? HIGH_CURVE_SLOWDOWN_PWM : 0;
}

static uint32_t Control_GetControlPeriodMs(void)
{
    return (g_run_mode == MODE_HIGH) ? HIGH_CONTROL_PERIOD_MS : LOW_CONTROL_PERIOD_MS;
}


#if USE_CAMERA_OBSTACLE_AVOID
/* 避障动作段返回值：0=完成，1=找到线提前结束，2=超时。 */
#define AVOID_MOVE_DONE        0
#define AVOID_MOVE_LINE_FOUND  1
#define AVOID_MOVE_TIMEOUT     2

static void Control_OLED_ShowSignedNumber(u8 x, u8 y, int value, u8 len);
static int g_avoid_debug_left_total = 0;
static int g_avoid_debug_right_total = 0;
static u8 g_avoid_success = 0;

static int Control_ApplyAvoidRightBalance(int pwm)
{
    int sign = 1;
    int abs_pwm;

    if (pwm == 0) {
        return 0;
    }

    if (pwm < 0) {
        sign = -1;
        abs_pwm = -pwm;
    } else {
        abs_pwm = pwm;
    }

    abs_pwm = (abs_pwm * AVOID_RIGHT_PWM_SCALE_X100) / 100;
    if (abs_pwm > 0 && abs_pwm < AVOID_RIGHT_PWM_MIN_ABS) {
        abs_pwm = AVOID_RIGHT_PWM_MIN_ABS;
    }

    return sign * abs_pwm;
}

static void Control_DriveByDirection(int left_pwm, int right_pwm)
{
    /*
     * 避障动作不走正常巡线的 right_pwm_comp，因此这里单独做右轮平衡。
     * 目的：减小“向左绕很大、向右绕很小”的不对称。
     */
    right_pwm = Control_ApplyAvoidRightBalance(right_pwm);
    Set_Pwm(Start_Compensate(left_pwm), Start_Compensate(right_pwm));
}

static int Control_ScaleInt(int value, int scale_x100)
{
    if (value >= 0) {
        return (value * scale_x100 + 50) / 100;
    }

    return -((-value * scale_x100 + 50) / 100);
}

static uint32_t Control_ScaleTimeMs(uint32_t value, int scale_x100)
{
    return (uint32_t)(((uint32_t)value * (uint32_t)scale_x100 + 50U) / 100U);
}

/*
 * 根据 OpenMV 输出的障碍物横向位置和像素宽度分档：
 * 返回 3=大障碍，2=中障碍，1=小障碍。
 * 这个分档同时用于 OLED 显示和实际避障动作，方便调参时对应观察。
 */
static u8 Control_GetAvoidLevel(const camera_data_t *cam)
{
    int obstacle_left;
    int obstacle_right;
    int center_offset_abs;

    if (cam == 0 || cam->valid == 0) {
        return 0;
    }

    obstacle_left = cam->cx - cam->w / 2;
    obstacle_right = cam->cx + cam->w / 2;
    center_offset_abs = cam->cx - AVOID_IMAGE_CENTER_X;
    if (center_offset_abs < 0) {
        center_offset_abs = -center_offset_abs;
    }

    if ((obstacle_left <= AVOID_IMAGE_CENTER_X && obstacle_right >= AVOID_IMAGE_CENTER_X) ||
        cam->w >= AVOID_SIZE_LARGE_WIDTH_PX) {
        return 3;
    }

    if ((center_offset_abs < AVOID_CENTER_NEAR_PX) || cam->w >= AVOID_SIZE_MID_WIDTH_PX) {
        return 2;
    }

    return 1;
}


static void Control_OLED_ShowAvoidStage(u8 stage, const char *name, u8 level)
{
    OLED_ShowString(0, 48, "                ");
    OLED_ShowString(0, 48, "AVD:");
    OLED_ShowNumber(30, 48, stage, 1, 12);
    OLED_ShowString(42, 48, (const u8 *)name);
    OLED_ShowString(84, 48, "G:");
    if (level >= 3) {
        OLED_ShowString(102, 48, "L");
    } else if (level == 2) {
        OLED_ShowString(102, 48, "M");
    } else if (level == 1) {
        OLED_ShowString(102, 48, "S");
    } else {
        OLED_ShowString(102, 48, "?");
    }
    OLED_Refresh_Gram();
}

static void Control_AbortAvoidance(u8 level)
{
    /* 临时调试：超时时显示左右编码器累计值，用于判断 PIVOT/前进是否正常。 */
    OLED_ShowString(0, 32, "TMO L:      ");
    Control_OLED_ShowSignedNumber(48, 32, g_avoid_debug_left_total, 4);
    OLED_ShowString(0, 48, "R:          ");
    Control_OLED_ShowSignedNumber(16, 48, g_avoid_debug_right_total, 4);
    OLED_Refresh_Gram();
    Motor_Stop();
    HAL_Delay(60);
}

static u8 Control_WaitMsWithLineCheck(uint32_t wait_ms, u8 stop_when_line_found)
{
    uint32_t start = HAL_GetTick();
    u8 line_hold_count = 0;

    while ((HAL_GetTick() - start) < wait_ms) {
        u8 line_seen = (Track_GetActiveCount() > 0) || (Track_ReadBlackMask() != 0) || Camera_IsLineVisible();

        if (stop_when_line_found && line_seen) {
            if (++line_hold_count >= AVOID_LINE_HOLD_COUNT) {
                return 1;
            }
        } else {
            line_hold_count = 0;
        }

        HAL_Delay(10);
    }

    return 0;
}


static void Control_ResetAvoidEncoder(void)
{
    TIM3->CNT = 0;
    TIM4->CNT = 0;
}

static u8 Control_RunAvoidEncoderTurn(int left_pwm,
                                      int right_pwm,
                                      int turn_target,
                                      u8 stop_when_line_found)
{
    uint32_t start = HAL_GetTick();
    int left_total = 0;
    int right_total = 0;
    u8 line_hold_count = 0;

    Control_ResetAvoidEncoder();
    Control_DriveByDirection(left_pwm, right_pwm);

    while ((HAL_GetTick() - start) < AVOID_TURN_TIMEOUT_MS) {
        int left_step;
        int right_step;
        int turn_error;
        u8 line_seen;

        HAL_Delay(AVOID_TURN_CONTROL_MS);

        left_step = LEFT_ENCODER_DIR * Read_Encoder(LEFT_ENCODER_TIM);
        right_step = RIGHT_ENCODER_DIR * Read_Encoder(RIGHT_ENCODER_TIM);
        left_total += left_step;
        right_total += right_step;

        /* 差速车车头角度与左右轮路程差成正比，取绝对值即可同时适配左/右转。 */
        turn_error = right_total - left_total;
        if (turn_error < 0) {
            turn_error = -turn_error;
        }

        if (turn_error >= turn_target) {
            return 0;
        }

        line_seen = (Track_ReadBlackMask() != 0) || Camera_IsLineVisible();
        if (stop_when_line_found && line_seen) {
            if (++line_hold_count >= AVOID_LINE_HOLD_COUNT) {
                return 1;
            }
        } else {
            line_hold_count = 0;
        }
    }

    g_avoid_debug_left_total = left_total;
    g_avoid_debug_right_total = right_total;
    return AVOID_MOVE_TIMEOUT;
}
static u8 Control_RunAvoidEncoderForward(int left_pwm,
                                         int right_pwm,
                                         int distance_target,
                                         uint32_t timeout_ms,
                                         u8 stop_when_line_found)
{
    uint32_t start = HAL_GetTick();
    int left_total = 0;
    int right_total = 0;
    u8 line_hold_count = 0;

    Control_ResetAvoidEncoder();
    Control_DriveByDirection(left_pwm, right_pwm);

    while ((HAL_GetTick() - start) < timeout_ms) {
        int left_step;
        int right_step;
        int left_abs;
        int right_abs;
        int distance_done;
        u8 line_seen;

        HAL_Delay(AVOID_TURN_CONTROL_MS);

        left_step = LEFT_ENCODER_DIR * Read_Encoder(LEFT_ENCODER_TIM);
        right_step = RIGHT_ENCODER_DIR * Read_Encoder(RIGHT_ENCODER_TIM);
        left_total += left_step;
        right_total += right_step;

        left_abs = left_total;
        if (left_abs < 0) {
            left_abs = -left_abs;
        }
        right_abs = right_total;
        if (right_abs < 0) {
            right_abs = -right_abs;
        }

        distance_done = (left_abs + right_abs) / 2;
        if (distance_done >= distance_target) {
            return 0;
        }

        line_seen = (Track_ReadBlackMask() != 0) || Camera_IsLineVisible();
        if (stop_when_line_found && line_seen) {
            if (++line_hold_count >= AVOID_LINE_HOLD_COUNT) {
                return 1;
            }
        } else {
            line_hold_count = 0;
        }
    }

    g_avoid_debug_left_total = left_total;
    g_avoid_debug_right_total = right_total;
    return AVOID_MOVE_TIMEOUT;
}

static u8 Control_AvoidCountTrackBits(u8 mask)
{
    u8 count = 0;

    while (mask != 0) {
        count += (u8)(mask & 1U);
        mask = (u8)(mask >> 1);
    }

    return count;
}

/*
 * 找到黑线后的摆正阶段。
 * 这里刻意放在避障内而不是直接交回主巡线：主巡线带着起点宽线停车、
 * 超声波再次触发避障等完整状态机，从“斜着刚压到线”的状态进入很容易还没
 * 摆正就被打断。避障内用低速小 PD 把黑线引导回 X4/X5 并稳定保持，
 * 之后主巡线接手时线已经在中缝附近，姿态也基本正了。
 */
static u8 Control_RunAvoidLineRealign(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    int line_error = 0;
    int line_last_error = 0;
    u8 center_hold_count = 0;

    while ((HAL_GetTick() - start) < timeout_ms) {
        u8 mask;
        u8 active_count;
        float correction;
        int left_pwm;
        int right_pwm;

        HAL_Delay(AVOID_TURN_CONTROL_MS);

        mask = Track_ReadBlackMask();
        active_count = Control_AvoidCountTrackBits(mask);

        if (active_count > 0 && active_count < 7) {
            line_error = Track_GetError();
            line_error += LINE_ERROR_OFFSET;

            /*
             * 不能只凭 Track_GetError()==0 判断居中：斜着压宽线时八路全亮
             * 会被巡线函数当成 0，所以这里要求 X4/X5 至少一路命中，且偏差小，
             * 连续几拍稳定才算摆正完成。
             */
            if ((mask & AVOID_REALIGN_CENTER_MASK) != 0 &&
                myabs(line_error) <= AVOID_REALIGN_CENTER_ERROR) {
                if (++center_hold_count >= AVOID_REALIGN_HOLD_COUNT) {
                    return AVOID_MOVE_DONE;
                }
            } else {
                center_hold_count = 0;
            }
        } else {
            center_hold_count = 0;

            if (active_count >= 7) {
                /* 八路几乎同时压线说明回线角度过大，已接近垂直轨道，
                 * 此时没有可用的左右偏差，不能把它当“居中”，直接按超时停车。 */
                return AVOID_MOVE_TIMEOUT;
            }

            if (active_count == 0) {
                /* 摆正中短暂丢线：沿用上一次偏差方向继续找，避免停在中途。 */
                if (line_error < 0) {
                    line_error = -AVOID_REALIGN_LOST_ERROR;
                } else if (line_error > 0) {
                    line_error = AVOID_REALIGN_LOST_ERROR;
                }
            }
        }

        correction = AVOID_REALIGN_KP * (float)line_error +
                     AVOID_REALIGN_KD * (float)(line_error - line_last_error);
        line_last_error = line_error;

        if (correction > AVOID_REALIGN_OUT_LIMIT) {
            correction = AVOID_REALIGN_OUT_LIMIT;
        }
        if (correction < -AVOID_REALIGN_OUT_LIMIT) {
            correction = -AVOID_REALIGN_OUT_LIMIT;
        }

        /* 符号与主巡线一致：线偏左时左轮慢、右轮快，车头向左回线。 */
        left_pwm = AVOID_REALIGN_BASE_PWM + (int)correction;
        right_pwm = AVOID_REALIGN_BASE_PWM - (int)correction;
        Control_DriveByDirection(left_pwm, right_pwm);
    }

    return AVOID_MOVE_TIMEOUT;
}

static void Control_RunObstacleAvoidance(const camera_data_t *cam)
{
    int avoid_right;
    u8 avoid_level;
    int turn_target;
    int pivot_turn_target;
    int align_turn_target;
    int back_turn_target;
    int exit_target;
    uint32_t pass_time_ms;
    uint32_t return_timeout_ms;
    u8 back_line_found;
    u8 search_result;

    /* 只有 REALGN 成功后才允许恢复巡线；任何超时都会保持失败状态。 */
    g_avoid_success = 0;

    /*
     * 避障方向：
     * - zone=-1：障碍偏左，向右绕；
     * - zone=0 ：障碍正前方，默认向右绕；
     * - zone=1 ：障碍偏右，向左绕。
     *
     * 梯形绕障：根据 OpenMV 输出的 cx/w 分档设置不同“梯形宽度/长度”：
     * - 转出/回正/转回角：由 AVOID_xxx_TURN_TARGET 编码器差值闭环决定；
     * - 斜向离开距离：由 AVOID_xxx_EXIT_ENCODER_TARGET 决定；
     * - 平行通过距离：由 AVOID_xxx_PASS_MS 决定；
     * - 第二条腰：BACK 转角后沿固定角度直行找线，再用 REALGN 摆正。
     */
    avoid_right = (cam->zone <= 0) ? 1 : 0;

    avoid_level = Control_GetAvoidLevel(cam);
    if (avoid_level >= 3) {
        turn_target = AVOID_LARGE_TURN_TARGET;
        exit_target = AVOID_LARGE_EXIT_ENCODER_TARGET;
        pass_time_ms = AVOID_LARGE_PASS_MS;
    } else if (avoid_level == 2) {
        turn_target = AVOID_MID_TURN_TARGET;
        exit_target = AVOID_MID_EXIT_ENCODER_TARGET;
        pass_time_ms = AVOID_MID_PASS_MS;
    } else {
        turn_target = AVOID_SMALL_TURN_TARGET;
        exit_target = AVOID_SMALL_EXIT_ENCODER_TARGET;
        pass_time_ms = AVOID_SMALL_PASS_MS;
    }

    if (avoid_right) {
        pivot_turn_target = Control_ScaleInt(turn_target, AVOID_RIGHT_PIVOT_TURN_SCALE_X100);
        align_turn_target = Control_ScaleInt(turn_target, AVOID_RIGHT_ALIGN_TURN_SCALE_X100);
        back_turn_target = Control_ScaleInt(turn_target, AVOID_RIGHT_BACK_TURN_SCALE_X100);
        exit_target = Control_ScaleInt(exit_target, AVOID_RIGHT_EXIT_SCALE_X100);
        pass_time_ms = Control_ScaleTimeMs(pass_time_ms, AVOID_RIGHT_PASS_SCALE_X100);
        return_timeout_ms = AVOID_RIGHT_RETURN_TIMEOUT_MS;
    } else {
        pivot_turn_target = Control_ScaleInt(turn_target, AVOID_LEFT_PIVOT_TURN_SCALE_X100);
        align_turn_target = Control_ScaleInt(turn_target, AVOID_LEFT_ALIGN_TURN_SCALE_X100);
        back_turn_target = Control_ScaleInt(turn_target, AVOID_LEFT_BACK_TURN_SCALE_X100);
        exit_target = Control_ScaleInt(exit_target, AVOID_LEFT_EXIT_SCALE_X100);
        pass_time_ms = Control_ScaleTimeMs(pass_time_ms, AVOID_LEFT_PASS_SCALE_X100);
        return_timeout_ms = AVOID_LEFT_RETURN_TIMEOUT_MS;
    }

    Motor_Stop();
    HAL_Delay(80);

    /* 1) 转出：按编码器左右轮差值闭环控制转角，避免固定时间导致每次角度不同。 */
    Control_OLED_ShowAvoidStage(1, "PIVOT", avoid_level);
    if (avoid_right) {
        if (Control_RunAvoidEncoderTurn(AVOID_PIVOT_PWM, -AVOID_PIVOT_REVERSE_PWM,
                                        pivot_turn_target, 0) == AVOID_MOVE_TIMEOUT) {
            Control_AbortAvoidance(avoid_level);
            return;
        }
    } else {
        if (Control_RunAvoidEncoderTurn(-AVOID_PIVOT_REVERSE_PWM, AVOID_PIVOT_PWM,
                                        pivot_turn_target, 0) == AVOID_MOVE_TIMEOUT) {
            Control_AbortAvoidance(avoid_level);
            return;
        }
    }

    /* 1B) 斜向离开：PIVOT 只改变车头角度，这一步沿当前斜方向前进，真正走出横向距离。 */
    Control_OLED_ShowAvoidStage(1, "EXIT", avoid_level);
    if (Control_RunAvoidEncoderForward(AVOID_EXIT_PWM, AVOID_EXIT_PWM,
                                       exit_target, AVOID_EXIT_TIMEOUT_MS, 0) == AVOID_MOVE_TIMEOUT) {
        Control_AbortAvoidance(avoid_level);
        return;
    }

    /* 2) 前进差速回正：同样按编码器差值闭环，目标与转出一致，让车头重新平行轨道。 */
    Control_OLED_ShowAvoidStage(2, "ALIGN", avoid_level);
    if (avoid_right) {
        if (Control_RunAvoidEncoderTurn(AVOID_ALIGN_INNER_PWM, AVOID_ALIGN_PWM,
                                        align_turn_target, 0) == AVOID_MOVE_TIMEOUT) {
            Control_AbortAvoidance(avoid_level);
            return;
        }
    } else {
        if (Control_RunAvoidEncoderTurn(AVOID_ALIGN_PWM, AVOID_ALIGN_INNER_PWM,
                                        align_turn_target, 0) == AVOID_MOVE_TIMEOUT) {
            Control_AbortAvoidance(avoid_level);
            return;
        }
    }

    /* 3) 平行直行通过障碍物侧边。 */
    Control_OLED_ShowAvoidStage(3, "PASS", avoid_level);
    Control_DriveByDirection(AVOID_FORWARD_PWM, AVOID_FORWARD_PWM);
    Control_WaitMsWithLineCheck(pass_time_ms, 0);

    /*
     * 4) 与第 1 步相反方向转回轨道，只转到梯形第二条腰的角度。
     * 这里故意不用差速“画弧”：继续画弧会让车头在前进过程中越转越多，
     * 角度一大就变成斜向垂直压线，八路会同时看见黑线并误判成居中/起点。
     * BACK 转向过程中允许提前看到黑线：如果线已经在车底经过却仍转完整圈，
     * 第五步固定角度直行时很容易继续朝远离轨道的方向走。
     */
    Control_OLED_ShowAvoidStage(4, "BACK", avoid_level);
    if (avoid_right) {
        back_line_found = Control_RunAvoidEncoderTurn(-AVOID_PIVOT_REVERSE_PWM,
                                                      AVOID_PIVOT_PWM,
                                                      back_turn_target,
                                                      1);
        if (back_line_found == AVOID_MOVE_TIMEOUT) {
            Control_AbortAvoidance(avoid_level);
            return;
        }
    } else {
        back_line_found = Control_RunAvoidEncoderTurn(AVOID_PIVOT_PWM,
                                                      -AVOID_PIVOT_REVERSE_PWM,
                                                      back_turn_target,
                                                      1);
        if (back_line_found == AVOID_MOVE_TIMEOUT) {
            Control_AbortAvoidance(avoid_level);
            return;
        }
    }

    if (back_line_found == AVOID_MOVE_DONE) {
        /*
         * 5) BACK 转到目标角度仍未看到黑线时，保持这个固定角度直行找线。
         * 左右 PWM 对称，书写和第一条腰的 EXIT 一样：先转角、再沿当前方向走。
         * 右绕结束后车头已经朝左前方，左绕结束后已经朝右前方，
         * 所以这里不需要再切换内外轮，直到巡线模块连续识别到黑线。
         */
        Control_OLED_ShowAvoidStage(5, "FIND", avoid_level);
        search_result = Control_RunAvoidEncoderForward(AVOID_BACK_SEARCH_PWM,
                                                       AVOID_BACK_SEARCH_PWM,
                                                       AVOID_FIND_MAX_ENCODER_TARGET,
                                                       return_timeout_ms,
                                                       1);
        if (search_result != AVOID_MOVE_LINE_FOUND) {
            Control_AbortAvoidance(avoid_level);
            return;
        }
    }

    /*
     * 6) 巡线已经看到黑线，但车身还是斜着压线，必须先把线摆回 X4/X5。
     * 完成这个阶段后避障才算结束，主巡线接手时不需要再处理大的初始偏差。
     */
    Control_OLED_ShowAvoidStage(6, "REALGN", avoid_level);
    if (Control_RunAvoidLineRealign(AVOID_REALIGN_TIMEOUT_MS) != AVOID_MOVE_DONE) {
        Control_AbortAvoidance(avoid_level);
        return;
    }

    Motor_Stop();
    HAL_Delay(60);
    g_avoid_success = 1;
}
#endif


void Mode_Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = MODE_KEY_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MODE_KEY_GPIO_PORT, &GPIO_InitStruct);
}

/* PB8 按键：另一端接 GND，低电平为按下；带 40ms 防抖，只在按下沿切换一次。 */
static u8 Mode_Key_Update(void)
{
    static u8 stable_level = 1;
    static u8 last_read_level = 1;
    static uint32_t last_change_time = 0;
    u8 now_level;

    now_level = (HAL_GPIO_ReadPin(MODE_KEY_GPIO_PORT, MODE_KEY_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1;

    if (now_level != last_read_level) {
        last_read_level = now_level;
        last_change_time = HAL_GetTick();
    }

    if ((HAL_GetTick() - last_change_time) >= MODE_KEY_DEBOUNCE_MS) {
        if (stable_level != last_read_level) {
            stable_level = last_read_level;
            if (stable_level == 0) {
                g_run_mode = (g_run_mode == MODE_LOW) ? MODE_HIGH : MODE_LOW;
                return 1;
            }
        }
    }

    return 0;
}

static void Control_OLED_ShowModeSelect(uint32_t remain_ms)
{
    OLED_ShowString(0, 0,  "Select Mode     ");
    OLED_ShowString(0, 16, "PB8->GND Change ");
    OLED_ShowString(0, 32, "MODE:           ");
    OLED_ShowString(48, 32, (const u8 *)Control_GetModeName());
    OLED_ShowString(0, 48, "Start in:       ");
    OLED_ShowNumber(72, 48, remain_ms / 1000 + 1, 1, 12);
    OLED_ShowString(88, 48, "s");
    OLED_Refresh_Gram();
}

void Mode_Select_Before_Run(void)
{
    uint32_t start_time;
    uint32_t last_oled_time;
    uint32_t now;

    start_time = HAL_GetTick();
    last_oled_time = start_time - 500;

    while ((HAL_GetTick() - start_time) < MODE_SELECT_TIME_MS) {
        now = HAL_GetTick();
        if (Mode_Key_Update() || ((now - last_oled_time) >= 200)) {
            last_oled_time = now;
            Control_OLED_ShowModeSelect(MODE_SELECT_TIME_MS - (now - start_time));
        }
    }
}

/**************************************************************************
函数功能：绝对值函数
入口参数：int
返回  值：unsigned int
**************************************************************************/
int myabs(int a)
{
    int temp;

    if (a < 0) {
        temp = -a;
    } else {
        temp = a;
    }

    return temp;
}

/**************************************************************************
函数功能：限制函数
入口参数：输入值、最小值、最大值
返回  值：限幅后的数值
**************************************************************************/
int limit(int input, int min, int max)
{
    if (input < min) {
        input = min;
    }

    if (input > max) {
        input = max;
    }

    return input;
}

/**************************************************************************
函数功能：小幅 PWM 死区补偿
入口参数：PWM
返回  值：补偿后的 PWM
说明    ：MG310 减速电机低占空比可能无法克服静摩擦。
**************************************************************************/
static int Start_Compensate(int pwm)
{
    if (pwm > 0 && pwm < PWM_START_MIN) {
        return PWM_START_MIN;
    }

    if (pwm < 0 && pwm > -PWM_START_MIN) {
        return -PWM_START_MIN;
    }

    return pwm;
}

/**************************************************************************
函数功能：赋值给 PWM 寄存器，并控制 TB6612 方向
入口参数：左轮 PWM、右轮 PWM
返回  值：无
依据    ：接线说明.txt 中 PWMA=PA8，PWMB=PA11，AIN1/AIN2=PB14/PB15，
          BIN1/BIN2=PB13/PB12；与 WHEELTEC HAL 参考工程 control.h 一致。
**************************************************************************/
void Set_Pwm(int motor_a, int motor_b)
{
    motor_a = limit(motor_a * LEFT_MOTOR_DIR, -PWM_LIMIT, PWM_LIMIT);
    motor_b = limit(motor_b * RIGHT_MOTOR_DIR, -PWM_LIMIT, PWM_LIMIT);

    if (motor_a > 0) {
        AIN1 = 0;
        AIN2 = 1;
    } else if (motor_a < 0) {
        AIN1 = 1;
        AIN2 = 0;
    } else {
        AIN1 = 0;
        AIN2 = 0;
    }

    if (motor_b > 0) {
        BIN1 = 0;
        BIN2 = 1;
    } else if (motor_b < 0) {
        BIN1 = 1;
        BIN2 = 0;
    } else {
        BIN1 = 0;
        BIN2 = 0;
    }

    PWMA = myabs(motor_a);
    PWMB = myabs(motor_b);
}

/**************************************************************************
函数功能：停车
入口参数：无
返回  值：无
**************************************************************************/
void Motor_Stop(void)
{
    Set_Pwm(0, 0);
}


/**************************************************************************
函数功能：OLED 显示有符号速度值
入口参数：x/y 坐标、速度值、显示长度
返回  值：无
说明    ：这里的速度是编码器在 CONTROL_PERIOD_MS 内的计数，不是 m/s。
**************************************************************************/
static void Control_OLED_ShowSignedNumber(u8 x, u8 y, int value, u8 len)
{
    if (value < 0) {
        OLED_ShowString(x, y, "-");
        OLED_ShowNumber(x + 6, y, (u32)(-value), len, 12);
    } else {
        OLED_ShowString(x, y, "+");
        OLED_ShowNumber(x + 6, y, (u32)value, len, 12);
    }
}

/**************************************************************************
函数功能：OLED 显示小车状态和编码器速度
入口参数：状态字符串、左/右编码器速度
返回  值：无
**************************************************************************/
static void Control_OLED_ShowTime(u8 x, u8 y, uint32_t elapsed_ms)
{
    u32 sec = elapsed_ms / 1000;
    u32 centisecond = (elapsed_ms % 1000) / 10;

    if (sec > 99) {
        sec = 99;
    }

    /* 手动逐位显示，保证始终是 xx.xx 秒，而不是省略前导 0。 */
    OLED_ShowNumber(x, y, sec / 10, 1, 12);
    OLED_ShowNumber(x + 6, y, sec % 10, 1, 12);
    OLED_ShowString(x + 12, y, ".");
    OLED_ShowNumber(x + 20, y, centisecond / 10, 1, 12);
    OLED_ShowNumber(x + 26, y, centisecond % 10, 1, 12);
    OLED_ShowString(x + 36, y, "s");
}

static void Control_OLED_ShowObstacleInfo(void)
{
#if USE_ULTRASONIC_OBSTACLE
    OLED_ShowString(0, 32, "US:");
    if (Ultrasonic_IsDistanceValid()) {
        if (Ultrasonic_IsObstacleStop()) {
            OLED_ShowString(18, 32, "OBS ");
        } else if (Ultrasonic_ShouldSlowDown()) {
            OLED_ShowString(18, 32, "NEAR");
        } else {
            OLED_ShowString(18, 32, "CLR ");
        }
        OLED_ShowString(48, 32, "D:");
        OLED_ShowNumber(66, 32, Ultrasonic_GetDistanceCm(), 3, 12);
        OLED_ShowString(90, 32, "cm");
    } else {
        OLED_ShowString(18, 32, "NO DATA");
    }
#else
    OLED_ShowString(0, 32, "US:OFF");
#endif

#if USE_CAMERA_OBSTACLE_AVOID
    {
        const camera_data_t *cam;

        OLED_ShowString(0, 48, "                ");
        OLED_ShowString(0, 48, "CAM:");
        if (!Camera_IsOnline()) {
            OLED_ShowString(30, 48, "OFF");
        } else if (Camera_IsObstacleConfirmed()) {
            cam = Camera_GetData();
            /* 只显示障碍物大小分档，避免方位 L/R/F 和大小 L/M/S 同时出现造成误解。 */
            OLED_ShowString(30, 48, "G:");
            switch (Control_GetAvoidLevel(cam)) {
            case 3:
                OLED_ShowString(48, 48, "L");
                break;
            case 2:
                OLED_ShowString(48, 48, "M");
                break;
            case 1:
                OLED_ShowString(48, 48, "S");
                break;
            default:
                OLED_ShowString(48, 48, "?");
                break;
            }
        } else {
            OLED_ShowString(30, 48, "NO OBS");
        }
    }
#else
    OLED_ShowString(0, 48, "CAM:OFF");
#endif
}

static void Control_OLED_ShowStatus(const char *state,
                                    uint32_t elapsed_ms,
                                    int left_speed,
                                    int right_speed,
                                    int left_pwm,
                                    int right_pwm,
                                    int line_error,
                                    u8 line_active_count,
                                    u8 line_mask)
{
    (void)left_speed;
    (void)right_speed;
    (void)left_pwm;
    (void)right_pwm;
    (void)line_active_count;
    (void)line_mask;

    /* 不在刷新时 OLED_Clear()，避免屏幕整屏闪烁；每行先用空格覆盖旧内容。 */
    OLED_ShowString(0, 0,  "                ");
    OLED_ShowString(0, 16, "                ");
    OLED_ShowString(0, 32, "                ");
    OLED_ShowString(0, 48, "                ");

    /* 第 1 行：电机状态 + 运行/一圈时间。 */
    OLED_ShowString(0, 0, "M:");
    OLED_ShowString(16, 0, (const u8 *)state);
    OLED_ShowString(56, 0, "T:");
    Control_OLED_ShowTime(72, 0, elapsed_ms);

    /* 第 2 行：巡线偏差。E<0 表示线在左侧，E>0 表示线在右侧。 */
    OLED_ShowString(0, 16, "E:");
    Control_OLED_ShowSignedNumber(16, 16, line_error, 3);
    OLED_ShowString(56, 16, "L<0 R>0");

    /* 第 3/4 行：超声波与摄像头避障状态。 */
    Control_OLED_ShowObstacleInfo();

    OLED_Refresh_Gram();
}

#if USE_LINE_TRACKING && LAP_STOP_ENABLE
/**************************************************************************
函数功能：判断是否压到起点宽黑线标记
说明    ：起点标志已加长，现在到达起点时 L1~L8 可以全亮。
          因此停车规则改为：8 路全亮才认为到达起点。
          这样比 23456/34567 更可靠，也能避免转弯斜压多路时误停。
**************************************************************************/
static u8 Control_CountBits(u8 value)
{
    u8 i;
    u8 count = 0;

    for (i = 0; i < 8; i++) {
        if (value & (1 << i)) {
            count++;
        }
    }

    return count;
}

static u8 Control_IsLapStartMarker(u8 mask)
{
    /*
     * 起点是宽黑线，高速斜着压过时不一定刚好 X2~X7 或 X1~X8 同时全中。
     * 所以这里改为：只要黑线命中数量足够多，就认为是起点标志。
     * 若 LAP_MARKER_REQUIRED_MASK 不为 0，再额外要求指定探头命中。
     */
    if (Control_CountBits(mask) < LAP_MARKER_MIN_COUNT) {
        return 0;
    }

#if LAP_MARKER_REQUIRED_MASK
    if ((mask & LAP_MARKER_REQUIRED_MASK) != LAP_MARKER_REQUIRED_MASK) {
        return 0;
    }
#endif

    return 1;
}
#endif
/**************************************************************************
函数功能：直行/巡线运行；启用 LAP_STOP_ENABLE 时回到起点停止
入口参数：无
返回  值：无
说明    ：默认按接线说明当前阶段，不使用编码器，固定 PWM 直走。
          若 USE_ENCODER_FEEDBACK 改为 1，则用编码器计数做左右轮同步修正。
**************************************************************************/
uint32_t LineTracking_Run_OneLap(void)
{
    uint32_t start_time;
    uint32_t last_time;
    uint32_t last_oled_time;
    uint32_t last_led_time;
    u8 run_led_step;
    uint32_t elapsed_time;
    int base_pwm;
    int right_pwm_comp;
    int lost_error;
    int curve_slowdown_pwm;
    int current_base_pwm;
    uint32_t control_period_ms;
    float line_kp;
    float line_kd;
    float line_out_limit;
    int display_line_error;
    u8 display_line_active_count;
    u8 display_line_mask;
    const char *stop_state;
#if USE_ULTRASONIC_OBSTACLE
    uint16_t display_obstacle_distance_cm;
#endif

#if USE_ENCODER_FEEDBACK
    int left_speed;
    int right_speed;
    int left_total;
    int right_total;
    int distance_error;
    int speed_error;
    int left_pwm;
    int right_pwm;
    float correction;
#endif

#if USE_LINE_TRACKING
    int line_error;
    int line_last_error;
    u8 line_active_count;
    float line_correction;
#endif

#if USE_CAMERA_OBSTACLE_AVOID
    u8 avoid_confirm_count;
#endif

#if USE_LINE_TRACKING && LAP_STOP_ENABLE
    u8 lap_left_start;
    u8 lap_marker_count;
    uint32_t lap_marker_clear_time;
#endif

#if USE_ENCODER_FEEDBACK
    TIM3->CNT = 0;
    TIM4->CNT = 0;
    left_total = 0;
    right_total = 0;
#endif

    start_time = HAL_GetTick();
    last_time = start_time;
    last_oled_time = start_time;
    last_led_time = start_time;
    run_led_step = 0;
    RUN_LED_PIN_PB9 = 0;
    elapsed_time = 0;
    display_line_error = 0;
    display_line_active_count = 0;
    display_line_mask = 0;
    stop_state = "STOP";
#if USE_ULTRASONIC_OBSTACLE
    display_obstacle_distance_cm = 0;
    (void)display_obstacle_distance_cm;
#endif
    base_pwm = Control_GetBasePwm();
    right_pwm_comp = Control_GetRightCompensation();
    lost_error = Control_GetLostError();
    curve_slowdown_pwm = Control_GetCurveSlowdownPwm();
    current_base_pwm = base_pwm;
    control_period_ms = Control_GetControlPeriodMs();
    line_kp = Control_GetLineKp();
    line_kd = Control_GetLineKd();
    line_out_limit = Control_GetLineOutLimit();

#if USE_LINE_TRACKING
    line_last_error = 0;
#endif
#if USE_CAMERA_OBSTACLE_AVOID
    avoid_confirm_count = 0;
#endif

#if USE_LINE_TRACKING && LAP_STOP_ENABLE
    lap_left_start = 0;
    lap_marker_count = 0;
    lap_marker_clear_time = start_time;
#endif

    Set_Pwm(base_pwm + LEFT_PWM_COMPENSATION + LEFT_START_BOOST,
            base_pwm + right_pwm_comp + RIGHT_START_BOOST);
    Control_OLED_ShowStatus("RUN", 0, 0, 0,
            base_pwm + LEFT_PWM_COMPENSATION + LEFT_START_BOOST,
            base_pwm + right_pwm_comp + RIGHT_START_BOOST,
            0, 0, Track_ReadBlackMask());

    while ((HAL_GetTick() - start_time) < RUN_TIME_MS) {
        if (Mode_Key_Update()) {
            base_pwm = Control_GetBasePwm();
            right_pwm_comp = Control_GetRightCompensation();
            lost_error = Control_GetLostError();
            curve_slowdown_pwm = Control_GetCurveSlowdownPwm();
            control_period_ms = Control_GetControlPeriodMs();
            line_kp = Control_GetLineKp();
            line_kd = Control_GetLineKd();
            line_out_limit = Control_GetLineOutLimit();
        }

        /* 小车运行开始后，PB9 外接绿色 LED 非阻塞闪烁两次。 */
        if (run_led_step < 4 && (HAL_GetTick() - last_led_time) >= RUN_LED_BLINK_TIME_MS) {
            last_led_time = HAL_GetTick();
            RUN_LED_PIN_PB9 = !RUN_LED_PIN_PB9;
            run_led_step++;
        }

#if USE_ENCODER_FEEDBACK
        if ((HAL_GetTick() - last_time) >= control_period_ms) {
            last_time += control_period_ms;

#if USE_ULTRASONIC_OBSTACLE
            Ultrasonic_Update();
            if (Ultrasonic_IsDistanceValid()) {
                display_obstacle_distance_cm = Ultrasonic_GetDistanceCm();
            }

#if USE_CAMERA_OBSTACLE_AVOID
            /*
             * 摄像头只负责确认和判断障碍物方位，超声波仍然是距离安全依据。
             * 为避免到 18cm 才开始绕障太晚，摄像头确认后在 28cm 左右先刹车，再执行绕障。
             */
            if (Ultrasonic_IsDistanceValid() &&
                (Ultrasonic_GetDistanceCm() <= AVOID_START_DISTANCE_CM) &&
                Camera_IsObstacleConfirmed()) {
                if (avoid_confirm_count < 255) {
                    avoid_confirm_count++;
                }
            } else {
                avoid_confirm_count = 0;
            }

            if (avoid_confirm_count >= AVOID_CONFIRM_COUNT) {
                avoid_confirm_count = 0;
                stop_state = "AVOID";
                Motor_Stop();
                elapsed_time = HAL_GetTick() - start_time;
                Control_OLED_ShowStatus("AVOID", elapsed_time,
                                        0, 0, 0, 0,
                                        display_line_error,
                                        display_line_active_count,
                                        display_line_mask);
                HAL_Delay(AVOID_STOP_HOLD_MS);
                Control_RunObstacleAvoidance(Camera_GetData());
                if (g_avoid_success == 0) {
                    stop_state = "FAIL";
                    Motor_Stop();
                    elapsed_time = HAL_GetTick() - start_time;
                    Control_OLED_ShowStatus("FAIL", elapsed_time,
                                            0, 0, 0, 0,
                                            display_line_error,
                                            display_line_active_count,
                                            display_line_mask);
                    break;
                }
                Ultrasonic_Update();
                line_last_error = Track_GetLastError();
                last_time = HAL_GetTick();
                continue;
            }
#endif

            if (Ultrasonic_IsObstacleStop()) {
                /* 安全底线：只要超声波进入 18cm 停车区，即使摄像头没确认也立即停车。 */
                stop_state = "OBS";
                Motor_Stop();
                elapsed_time = HAL_GetTick() - start_time;
                Control_OLED_ShowStatus("OBS", elapsed_time,
                                        0, 0, 0, 0,
                                        display_line_error,
                                        display_line_active_count,
                                        display_line_mask);
                break;
            }
#endif

            left_speed = LEFT_ENCODER_DIR * Read_Encoder(LEFT_ENCODER_TIM);
            right_speed = RIGHT_ENCODER_DIR * Read_Encoder(RIGHT_ENCODER_TIM);

            left_total += left_speed;
            right_total += right_speed;

            distance_error = left_total - right_total;
            speed_error = left_speed - right_speed;
            correction = STRAIGHT_KP * distance_error + STRAIGHT_KD * speed_error;

            if (correction > STRAIGHT_OUT_LIMIT) {
                correction = STRAIGHT_OUT_LIMIT;
            }

            if (correction < -STRAIGHT_OUT_LIMIT) {
                correction = -STRAIGHT_OUT_LIMIT;
            }

            current_base_pwm = base_pwm;
            left_pwm = current_base_pwm + LEFT_PWM_COMPENSATION - (int)correction;
            right_pwm = current_base_pwm + right_pwm_comp + (int)correction;
#if USE_ULTRASONIC_OBSTACLE
            if (Ultrasonic_ShouldSlowDown() && current_base_pwm > ULTRASONIC_SLOW_PWM_LIMIT) {
                current_base_pwm = ULTRASONIC_SLOW_PWM_LIMIT;
                left_pwm = current_base_pwm + LEFT_PWM_COMPENSATION - (int)correction;
                right_pwm = current_base_pwm + right_pwm_comp + (int)correction;
            }
#endif

#if USE_LINE_TRACKING
            /*
             * 1. 读取八路巡线偏差：
             *    line_error < 0：黑线偏左；line_error > 0：黑线偏右；0：居中。
             * 2. Track_GetActiveCount() 是本次有几路看到黑线。
             *    如果 active=0，说明转弯处/褶皱处传感器完全丢线。
             */
            line_error = Track_GetError();
            line_active_count = Track_GetActiveCount();
            if (line_active_count > 0 && line_active_count < 7) {
                line_error += LINE_ERROR_OFFSET;
            }

            /*
             * 高速模式自适应：
             * - 直线小偏差：降低 KP 和限幅，解决蛇形；
             * - 弯道大偏差/丢线：提高 KP 和限幅，减少长时间找不到线。
             * 低速模式不进入这里，保持原来的低速手感。
             */
            if (g_run_mode == MODE_HIGH) {
                int abs_line_error = myabs(line_error);
                if (line_active_count == 0 || abs_line_error > HIGH_LINE_LARGE_ERR) {
                    line_kp = HIGH_LINE_KP_LARGE;
                    line_out_limit = HIGH_LINE_OUT_LARGE;
                } else if (abs_line_error > HIGH_LINE_SMALL_ERR) {
                    line_kp = HIGH_LINE_KP_MID;
                    line_out_limit = HIGH_LINE_OUT_MID;
                } else {
                    line_kp = HIGH_LINE_KP_SMALL;
                    line_out_limit = HIGH_LINE_OUT_SMALL;
                }
            }

            display_line_error = line_error;
            display_line_active_count = line_active_count;
            display_line_mask = Track_ReadBlackMask();

#if LAP_STOP_ENABLE
            /*
             * 起点停车判断要放在丢线处理前：
             * 高速回起点前常会短暂丢线，如果先进入丢线屏蔽窗口，可能刚好把起点宽黑线屏蔽掉。
             */
            if (Control_IsLapStartMarker(display_line_mask)) {
                if (lap_marker_count < 255) {
                    lap_marker_count++;
                }

                if (lap_left_start &&
                    (lap_marker_count >= LAP_MARKER_HOLD_COUNT) &&
                    ((HAL_GetTick() - start_time) >= LAP_MIN_RUN_TIME_MS)) {
                    break;
                }

                if (!lap_left_start) {
                    line_error = 0;
                    line_last_error = 0;
                    line_correction = 0.0f;
                    lap_marker_clear_time = HAL_GetTick();
                }
            } else {
                lap_marker_count = 0;
                if (!lap_left_start &&
                    ((HAL_GetTick() - lap_marker_clear_time) >= LAP_LEAVE_TIME_MS)) {
                    lap_left_start = 1;
                }
            }
#endif

            /* 高速模式下：大偏差或丢线时自动降速，直线高速、弯道稳一点。 */
            if ((line_active_count == 0) || (myabs(line_error) > HIGH_CURVE_SLOW_ERR)) {
                current_base_pwm = base_pwm - curve_slowdown_pwm;
                if (current_base_pwm < PWM_START_MIN) {
                    current_base_pwm = PWM_START_MIN;
                }
                left_pwm = current_base_pwm + LEFT_PWM_COMPENSATION - (int)correction;
                right_pwm = current_base_pwm + right_pwm_comp + (int)correction;
#if USE_ULTRASONIC_OBSTACLE
                /* 超声波慢速区再次限速，避免高速模式接近障碍物时急停冲击过大。 */
                if (Ultrasonic_ShouldSlowDown() && current_base_pwm > ULTRASONIC_SLOW_PWM_LIMIT) {
                    current_base_pwm = ULTRASONIC_SLOW_PWM_LIMIT;
                    left_pwm = current_base_pwm + LEFT_PWM_COMPENSATION - (int)correction;
                    right_pwm = current_base_pwm + right_pwm_comp + (int)correction;
                }
#endif
            }

            if (line_active_count == 0) {
                /*
                 * 完全丢线处理：
                 * - 不把偏差清零，否则车会直着冲出去；
                 * - 按丢线前的方向继续加大转向，主动把线找回来；
                 * - 起点判断已提前执行；这里不再屏蔽起点检测，避免高速漏停。
                 */
                if (line_error < 0 || line_last_error < 0) {
                    line_error = -lost_error;
                } else if (line_error > 0 || line_last_error > 0) {
                    line_error = lost_error;
                } else {
                    line_error = 0;
                }
#if LAP_STOP_ENABLE
                lap_marker_count = 0;
#endif
            }

            /*
             * 左右调节核心：
             * - LINE_KP 是主要转向力度，越大越会快速向线修正；
             * - LINE_KD 抑制变化太快造成的摆动；
             * - line_correction 最后会变成左右轮 PWM 的差速量。
             */
            line_correction = line_kp * (float)line_error +
                              line_kd * (float)(line_error - line_last_error);
            line_last_error = line_error;

            if (line_correction > line_out_limit) {
                line_correction = line_out_limit;
            }

            if (line_correction < -line_out_limit) {
                line_correction = -line_out_limit;
            }

            /*
             * 把巡线修正量叠加到左右轮：
             * - line_error < 0 时 line_correction 为负：左轮 PWM 变小、右轮 PWM 变大，车向左回线；
             * - line_error > 0 时 line_correction 为正：左轮 PWM 变大、右轮 PWM 变小，车向右回线。
             */
            left_pwm += (int)line_correction;
            right_pwm -= (int)line_correction;
#endif

            if ((HAL_GetTick() - start_time) < START_BOOST_TIME_MS) {
                left_pwm += LEFT_START_BOOST;
                right_pwm += RIGHT_START_BOOST;
            }

            left_pwm = Start_Compensate(left_pwm);
            right_pwm = Start_Compensate(right_pwm);

            Set_Pwm(left_pwm, right_pwm);

            if ((HAL_GetTick() - last_oled_time) >= 500) {
                last_oled_time = HAL_GetTick();
                elapsed_time = HAL_GetTick() - start_time;
                Control_OLED_ShowStatus("RUN", elapsed_time,
                                        left_speed, right_speed,
                                        left_pwm, right_pwm,
                                        display_line_error,
                                        display_line_active_count,
                                        display_line_mask);
            }
        }
#else
        (void)last_time;
#endif
    }

    /*
     * 运行循环结束后统一停车：
     * - 可能是跑完一圈 break 出来；
     * - 也可能是 RUN_TIME_MS 安全超时；
     * Motor_Stop() 会把左右 PWM 置 0，并关闭方向输出。
     */
    elapsed_time = HAL_GetTick() - start_time;
    Motor_Stop();
    RUN_LED_PIN_PB9 = 0;
    Control_OLED_ShowStatus(stop_state, elapsed_time,
                            0, 0, 0, 0,
                            display_line_error,
                            display_line_active_count,
                            display_line_mask);

    return elapsed_time;
}

/**************************************************************************
函数功能：A/B 电机通道分步自检
入口参数：无
返回  值：无
依据    ：TB6612 要让 AO1/AO2 或 BO1/BO2 有输出，必须同时满足：
          1. 对应 PWM 输入有效；2. 对应 IN1/IN2 给出方向；3. STBY 为高。
          因此本函数先单独输出 A 通道，再单独输出 B 通道，用来判断
          PA8/PWMA/AIN1/AIN2/AO1/AO2 这一整条链路是否工作。
**************************************************************************/
void Motor_Channel_Self_Test(void)
{
    /* A 通道单独测试：左侧 AO1/AO2 应转动 3 秒。 */
    Set_Pwm(MOTOR_SELF_TEST_PWM, 0);
    HAL_Delay(3000);
    Motor_Stop();
    HAL_Delay(1000);

    /* B 通道单独测试：右侧 BO1/BO2 应转动 3 秒。 */
    Set_Pwm(0, MOTOR_SELF_TEST_PWM);
    HAL_Delay(3000);
    Motor_Stop();
    HAL_Delay(1000);
}

/**************************************************************************
函数功能：PA8 普通 GPIO 高电平输出诊断
入口参数：无
返回  值：无
依据    ：TB6612 的 PWM 输入脚可以接 PWM，也可以在测试时直接给高电平。
          高电平相当于 100% 占空比。若 PA8 拉高 + AIN1/AIN2 方向正确时
          A 通道仍不转，则问题不再是 TIM1_CH1 PWM 配置，而应检查 PA8 引脚、
          PA8 到 PWMA 的线、面包板接触、TB6612 A 通道或 AO1/AO2。
**************************************************************************/
void PA8_GPIO_Output_Test(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 暂时把 PA8 从 TIM1_CH1 复用输出改为普通推挽输出。 */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* A 通道方向：与 Set_Pwm(正值, 0) 保持一致。 */
    AIN1 = 0;
    AIN2 = 1;
    BIN1 = 0;
    BIN2 = 0;

    /* PA8/PWMA 直接高电平，等价于 PWM 100% 占空比。 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    PWMB = 0;
    HAL_Delay(3000);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    Motor_Stop();
    HAL_Delay(1000);

    /* 恢复 PA8/PA11 为 TIM1 复用推挽输出，避免影响后续 PWM 测试。 */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC4E;
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->CR1  |= TIM_CR1_CEN;
}

