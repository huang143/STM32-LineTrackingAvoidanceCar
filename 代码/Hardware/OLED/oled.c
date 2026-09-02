#include "oled.h"
#include "oledfont.h"

/**************************************************************************
0.96寸四针 OLED：GND VCC SCL SDA
本文件使用普通 GPIO 模拟 I2C，依据江科大 4-1 OLED 示例的四针 I2C 写法，
并保留 WHEELTEC 工程原来的 OLED_ShowString/OLED_ShowNumber 坐标接口。
**************************************************************************/

#define OLED_I2C_ADDR  0x78

#define OLED_SCL_PORT  GPIOB
#define OLED_SCL_PIN   GPIO_PIN_10
#define OLED_SDA_PORT  GPIOB
#define OLED_SDA_PIN   GPIO_PIN_11

#define OLED_SCL_HIGH() HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_SET)
#define OLED_SCL_LOW()  HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_RESET)
#define OLED_SDA_HIGH() HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_SET)
#define OLED_SDA_LOW()  HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_RESET)

#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define MAX_CHAR_POSX   122
#define MAX_CHAR_POSY   58

u8 OLED_GRAM[128][8];

static void OLED_I2C_Delay(void)
{
    __NOP();
    __NOP();
    __NOP();
}

static void OLED_I2C_Start(void)
{
    OLED_SDA_HIGH();
    OLED_SCL_HIGH();
    OLED_I2C_Delay();
    OLED_SDA_LOW();
    OLED_I2C_Delay();
    OLED_SCL_LOW();
}

static void OLED_I2C_Stop(void)
{
    OLED_SDA_LOW();
    OLED_SCL_HIGH();
    OLED_I2C_Delay();
    OLED_SDA_HIGH();
    OLED_I2C_Delay();
}

static void OLED_I2C_SendByte(u8 byte)
{
    u8 i;

    for (i = 0; i < 8; i++) {
        if (byte & 0x80) {
            OLED_SDA_HIGH();
        } else {
            OLED_SDA_LOW();
        }

        OLED_I2C_Delay();
        OLED_SCL_HIGH();
        OLED_I2C_Delay();
        OLED_SCL_LOW();
        byte <<= 1;
    }

    /* 第9个时钟为ACK位。本工程不读取ACK，保持与江科大示例一致。 */
    OLED_SDA_HIGH();
    OLED_I2C_Delay();
    OLED_SCL_HIGH();
    OLED_I2C_Delay();
    OLED_SCL_LOW();
}

static void OLED_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = OLED_SCL_PIN | OLED_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    OLED_SCL_HIGH();
    OLED_SDA_HIGH();
}

void OLED_WR_Byte(u8 dat, u8 cmd)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_I2C_ADDR);

    if (cmd == OLED_CMD) {
        OLED_I2C_SendByte(0x00);
    } else {
        OLED_I2C_SendByte(0x40);
    }

    OLED_I2C_SendByte(dat);
    OLED_I2C_Stop();
}

void OLED_Refresh_Gram(void)
{
    u8 i;
    u8 n;

    for (i = 0; i < 8; i++) {
        OLED_WR_Byte(0xB0 + i, OLED_CMD);
        OLED_WR_Byte(0x00, OLED_CMD);
        OLED_WR_Byte(0x10, OLED_CMD);

        for (n = 0; n < 128; n++) {
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
        }
    }
}

void OLED_Display_On(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x14, OLED_CMD);
    OLED_WR_Byte(0xAF, OLED_CMD);
}

void OLED_Display_Off(void)
{
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x10, OLED_CMD);
    OLED_WR_Byte(0xAE, OLED_CMD);
}

void OLED_Clear(void)
{
    u8 i;
    u8 n;

    for (i = 0; i < 8; i++) {
        for (n = 0; n < 128; n++) {
            OLED_GRAM[n][i] = 0x00;
        }
    }

    OLED_Refresh_Gram();
}

void OLED_DrawPoint(u8 x, u8 y, u8 t)
{
    u8 pos;
    u8 bx;
    u8 temp = 0;

    if (x > 127 || y > 63) {
        return;
    }

    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);

    if (t) {
        OLED_GRAM[x][pos] |= temp;
    } else {
        OLED_GRAM[x][pos] &= ~temp;
    }
}

void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size, u8 mode)
{
    u8 temp;
    u8 t;
    u8 t1;
    u8 y0 = y;
    u8 csize;
    u8 chr_index;

    if (x > MAX_CHAR_POSX || y > MAX_CHAR_POSY) {
        return;
    }

    if (size != 12 && size != 16) {
        size = 12;
    }

    chr_index = chr - ' ';
    csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);

    for (t = 0; t < csize; t++) {
        if (size == 12) {
            temp = oled_asc2_1206[chr_index][t];
        } else {
            temp = oled_asc2_1608[chr_index][t];
        }

        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80) {
                OLED_DrawPoint(x, y, mode);
            } else {
                OLED_DrawPoint(x, y, !mode);
            }

            temp <<= 1;
            y++;

            if ((y - y0) == size) {
                y = y0;
                x++;
                break;
            }
        }
    }
}

static u32 OLED_Pow(u8 m, u8 n)
{
    u32 result = 1;

    while (n--) {
        result *= m;
    }

    return result;
}

void OLED_ShowNumber(u8 x, u8 y, u32 num, u8 len, u8 size)
{
    u8 t;
    u8 temp;
    u8 enshow = 0;

    for (t = 0; t < len; t++) {
        temp = (num / OLED_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                OLED_ShowChar(x + (size / 2) * t, y, ' ', size, 1);
                continue;
            } else {
                enshow = 1;
            }
        }

        OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size, 1);
    }
}

void OLED_ShowString(u8 x, u8 y, const u8 *p)
{
    while (*p != '\0') {
        if (x > MAX_CHAR_POSX) {
            x = 0;
            y += 12;
        }

        if (y > MAX_CHAR_POSY) {
            y = 0;
            OLED_Clear();
        }

        OLED_ShowChar(x, y, *p, 12, 1);
        x += 6;
        p++;
    }
}

void OLED_Init(void)
{
    OLED_GPIO_Init();
    HAL_Delay(100);

    OLED_WR_Byte(0xAE, OLED_CMD);
    OLED_WR_Byte(0xD5, OLED_CMD);
    OLED_WR_Byte(80, OLED_CMD);
    OLED_WR_Byte(0xA8, OLED_CMD);
    OLED_WR_Byte(0x3F, OLED_CMD);
    OLED_WR_Byte(0xD3, OLED_CMD);
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0x40, OLED_CMD);
    OLED_WR_Byte(0x8D, OLED_CMD);
    OLED_WR_Byte(0x14, OLED_CMD);
    OLED_WR_Byte(0x20, OLED_CMD);
    OLED_WR_Byte(0x02, OLED_CMD);
    OLED_WR_Byte(0xA1, OLED_CMD);
    OLED_WR_Byte(0xC0, OLED_CMD);
    OLED_WR_Byte(0xDA, OLED_CMD);
    OLED_WR_Byte(0x12, OLED_CMD);
    OLED_WR_Byte(0x81, OLED_CMD);
    OLED_WR_Byte(0xEF, OLED_CMD);
    OLED_WR_Byte(0xD9, OLED_CMD);
    OLED_WR_Byte(0xF1, OLED_CMD);
    OLED_WR_Byte(0xDB, OLED_CMD);
    OLED_WR_Byte(0x30, OLED_CMD);
    OLED_WR_Byte(0xA4, OLED_CMD);
    OLED_WR_Byte(0xA6, OLED_CMD);
    OLED_WR_Byte(0xAF, OLED_CMD);

    OLED_Clear();
}
