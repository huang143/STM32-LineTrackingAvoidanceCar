#include "ultrasonic.h"

/*
 * 参考 C:\Users\huangsw\Desktop\小车\巡线+避障\HC-SR04参考代码.zip 中
 * “超声波测距（stm32f103c8t6）”示例：
 * - TRIG 输出 10us 以上高电平触发；
 * - 等待 ECHO 高电平持续时间；
 * - 距离 = 高电平时间 * 声速 / 2，常用换算为 cm = us / 58。
 *
 * 本工程是 STM32 HAL 工程，且 TIM3/TIM4 已用于编码器，所以没有照搬参考代码
 * 中“TIM3 每 10us 中断计时”的做法，改为 Cortex-M3 DWT 周期计数实现微秒计时。
 */

#define ULTRASONIC_TRIGGER_US            12U
#define ULTRASONIC_WAIT_RISING_TIMEOUT_US 3000U
#define ULTRASONIC_ECHO_HIGH_TIMEOUT_US  15000U
#define ULTRASONIC_MIN_VALID_CM          2U
#define ULTRASONIC_MAX_VALID_CM          250U
#define ULTRASONIC_UPDATE_PERIOD_MS       60U
#define ULTRASONIC_INVALID_MAX_COUNT      3U

static uint16_t g_ultrasonic_distance_cm = 0;
static u8 g_ultrasonic_valid = 0;
static u8 g_ultrasonic_stop = 0;

static void Ultrasonic_DwtInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t Ultrasonic_GetUs(void)
{
    return DWT->CYCCNT / (SystemCoreClock / 1000000U);
}

static void Ultrasonic_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);

    while ((uint32_t)(DWT->CYCCNT - start) < ticks) {
        ;
    }
}

static u8 Ultrasonic_WaitEcho(GPIO_PinState target_level, uint32_t timeout_us)
{
    uint32_t start_us = Ultrasonic_GetUs();

    while (HAL_GPIO_ReadPin(ULTRASONIC_ECHO_GPIO_PORT, ULTRASONIC_ECHO_GPIO_PIN) != target_level) {
        if ((uint32_t)(Ultrasonic_GetUs() - start_us) >= timeout_us) {
            return 0;
        }
    }

    return 1;
}

void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    Ultrasonic_DwtInit();

    HAL_GPIO_WritePin(ULTRASONIC_TRIG_GPIO_PORT, ULTRASONIC_TRIG_GPIO_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = ULTRASONIC_TRIG_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ULTRASONIC_TRIG_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ULTRASONIC_ECHO_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ULTRASONIC_ECHO_GPIO_PORT, &GPIO_InitStruct);

    g_ultrasonic_distance_cm = 0;
    g_ultrasonic_valid = 0;
    g_ultrasonic_stop = 0;
}

uint16_t Ultrasonic_ReadDistanceCm(void)
{
    uint32_t echo_start_us;
    uint32_t echo_width_us;
    uint32_t distance_cm;

    /* 确保 TRIG 先保持低电平，再输出大于 10us 的触发脉冲。 */
    HAL_GPIO_WritePin(ULTRASONIC_TRIG_GPIO_PORT, ULTRASONIC_TRIG_GPIO_PIN, GPIO_PIN_RESET);
    Ultrasonic_DelayUs(3);
    HAL_GPIO_WritePin(ULTRASONIC_TRIG_GPIO_PORT, ULTRASONIC_TRIG_GPIO_PIN, GPIO_PIN_SET);
    Ultrasonic_DelayUs(ULTRASONIC_TRIGGER_US);
    HAL_GPIO_WritePin(ULTRASONIC_TRIG_GPIO_PORT, ULTRASONIC_TRIG_GPIO_PIN, GPIO_PIN_RESET);

    if (!Ultrasonic_WaitEcho(GPIO_PIN_SET, ULTRASONIC_WAIT_RISING_TIMEOUT_US)) {
        return 0;
    }

    echo_start_us = Ultrasonic_GetUs();

    if (!Ultrasonic_WaitEcho(GPIO_PIN_RESET, ULTRASONIC_ECHO_HIGH_TIMEOUT_US)) {
        return 0;
    }

    echo_width_us = Ultrasonic_GetUs() - echo_start_us;

    /* HC-SR04 常用换算：距离(cm) = ECHO 高电平时间(us) / 58。 */
    distance_cm = (echo_width_us + 29U) / 58U;

    if (distance_cm < ULTRASONIC_MIN_VALID_CM || distance_cm > ULTRASONIC_MAX_VALID_CM) {
        return 0;
    }

    return (uint16_t)distance_cm;
}

void Ultrasonic_Update(void)
{
    static uint32_t last_update_ms = 0;
    static u8 update_inited = 0;
    static u8 invalid_count = 0;
    uint32_t now_ms = HAL_GetTick();
    uint16_t distance_cm;

    /* HC-SR04 不适合跟随 10/20ms 巡线周期高频触发，单独限制为约 60ms 测一次。 */
    if (update_inited && ((uint32_t)(now_ms - last_update_ms) < ULTRASONIC_UPDATE_PERIOD_MS)) {
        return;
    }
    update_inited = 1;
    last_update_ms = now_ms;

    distance_cm = Ultrasonic_ReadDistanceCm();

    if (distance_cm == 0) {
        if (invalid_count < 255) {
            invalid_count++;
        }

        /* 偶发 1~2 次失败时保留上一次有效距离，连续失败后才显示 NO DATA。 */
        if (invalid_count >= ULTRASONIC_INVALID_MAX_COUNT) {
            g_ultrasonic_valid = 0;
            g_ultrasonic_stop = 0;
        }
        return;
    }

    invalid_count = 0;
    g_ultrasonic_distance_cm = distance_cm;
    g_ultrasonic_valid = 1;

    if (distance_cm <= ULTRASONIC_STOP_DISTANCE_CM) {
        g_ultrasonic_stop = 1;
    } else if (distance_cm >= ULTRASONIC_CLEAR_DISTANCE_CM) {
        g_ultrasonic_stop = 0;
    }
}

uint16_t Ultrasonic_GetDistanceCm(void)
{
    return g_ultrasonic_distance_cm;
}

u8 Ultrasonic_IsDistanceValid(void)
{
    return g_ultrasonic_valid;
}

u8 Ultrasonic_IsObstacleStop(void)
{
    return g_ultrasonic_stop;
}

u8 Ultrasonic_ShouldSlowDown(void)
{
    return (g_ultrasonic_valid &&
            (g_ultrasonic_distance_cm > ULTRASONIC_STOP_DISTANCE_CM) &&
            (g_ultrasonic_distance_cm <= ULTRASONIC_SLOW_DISTANCE_CM)) ? 1 : 0;
}
