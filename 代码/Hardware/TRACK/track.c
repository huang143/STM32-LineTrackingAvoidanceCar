#include "track.h"

/* 权重从左到右：负数表示黑线在左侧，正数表示黑线在右侧。 */
static const int g_track_weight[8] = {-35, -25, -15, -5, 5, 15, 25, 35};
static int g_last_error = 0;
static u8 g_last_active_count = 0;

static u8 Track_ReadOne(GPIO_TypeDef *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? 1 : 0;
}

void Track_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    g_last_error = 0;
    g_last_active_count = 0;
}

/**************************************************************************
函数功能：读取 X1~X8 原始电平
返回值  ：bit0~bit7 对应 X1~X8，1 表示高电平，0 表示低电平
**************************************************************************/
u8 Track_ReadRaw(void)
{
    u8 raw = 0;

    raw |= Track_ReadOne(GPIOA, GPIO_PIN_0) << 0;
    raw |= Track_ReadOne(GPIOA, GPIO_PIN_1) << 1;
    raw |= Track_ReadOne(GPIOA, GPIO_PIN_2) << 2;
    raw |= Track_ReadOne(GPIOA, GPIO_PIN_3) << 3;
    raw |= Track_ReadOne(GPIOA, GPIO_PIN_4) << 4;
    raw |= Track_ReadOne(GPIOA, GPIO_PIN_5) << 5;
    raw |= Track_ReadOne(GPIOB, GPIO_PIN_0) << 6;
    raw |= Track_ReadOne(GPIOB, GPIO_PIN_1) << 7;

    return raw;
}

/**************************************************************************
函数功能：读取黑线命中状态
返回值  ：bit0~bit7 对应 X1~X8，1 表示该探头检测到黑线
依据    ：示例 app_irtracking.c 中大量以 xN == 0 表示检测到黑线，
          所以默认 TRACK_BLACK_LEVEL = 0。
**************************************************************************/
u8 Track_ReadBlackMask(void)
{
    u8 i;
    u8 raw;
    u8 mask = 0;

    raw = Track_ReadRaw();

    for (i = 0; i < 8; i++) {
        u8 level = (raw >> i) & 0x01;

        if (level == TRACK_BLACK_LEVEL) {
            mask |= (1 << i);
        }
    }

    return mask;
}

u8 Track_GetActiveCount(void)
{
    return g_last_active_count;
}

int Track_GetLastError(void)
{
    return g_last_error;
}

/**************************************************************************
函数功能：计算巡线偏差
返回值  ：负数=黑线偏左；0=居中；正数=黑线偏右
逻辑    ：对命中黑线的探头做加权平均。
          X1~X8 从左到右权重为 -35、-25、-15、-5、5、15、25、35。当前机械安装下，L4/L5 同时压线时视为居中。
          多个探头同时检测到黑线时，取平均可以比逐项 if 更平滑。
**************************************************************************/
int Track_GetError(void)
{
    u8 i;
    u8 mask;
    int sum = 0;
    int count = 0;

    mask = Track_ReadBlackMask();

    for (i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            sum += g_track_weight[i];
            count++;
        }
    }

    g_last_active_count = (u8)count;

    if (count == 0) {
        return g_last_error;
    }

    if (count >= 7) {
        g_last_error = 0;
        return g_last_error;
    }

    g_last_error = sum / count;

    return g_last_error;
}



