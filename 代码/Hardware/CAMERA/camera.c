#include "camera.h"
#include <stdlib.h>
#include <string.h>

#define CAMERA_PACKET_MAX_LEN 128

camera_data_t g_camera_data;
u8 Camera_RxByte;

static char g_camera_packet[CAMERA_PACKET_MAX_LEN];
static u8 g_camera_rx_active = 0;
static u8 g_camera_rx_index = 0;
static u8 g_camera_confirm_count = 0;

static void Camera_ParsePacket(char *packet)
{
    char *token;
    int field = 0;
    camera_data_t temp = g_camera_data;

    /*
     * 当前 OpenMV 示例：
     * $O,1,0,67,85,24,10,225,28,1,19,16,1,46#
     * 字段：O,valid,zone,cx,cy,w,h,area,dist_cm,band,line_w,exp_w,type,fps
     */
    if (packet[0] != '$' || packet[1] != 'O') {
        return;
    }

    packet[0] = 'O';
    token = strtok(packet, ",#");
    while (token != NULL) {
        switch (field) {
        case 0: /* O */
            break;
        case 1:
            temp.valid = (u8)atoi(token);
            break;
        case 2:
            temp.zone = atoi(token);
            break;
        case 3:
            temp.cx = atoi(token);
            break;
        case 4:
            temp.cy = atoi(token);
            break;
        case 5:
            temp.w = atoi(token);
            break;
        case 6:
            temp.h = atoi(token);
            break;
        case 7:
            temp.area = atol(token);
            break;
        case 8:
            temp.dist_cm = atoi(token);
            break;
        case 9:
            temp.band = atoi(token);
            break;
        case 10:
            temp.line_w = atoi(token);
            break;
        case 11:
            temp.exp_w = atoi(token);
            break;
        case 12:
            temp.type = atoi(token);
            break;
        case 13:
            temp.fps = atoi(token);
            break;
        default:
            break;
        }

        field++;
        token = strtok(NULL, ",#");
    }

    /* 至少要收到 O + 13 个数据字段。 */
    if (field >= 14) {
        temp.last_update_ms = HAL_GetTick();
        temp.new_frame = 1;
        g_camera_data = temp;

        if (g_camera_data.valid && g_camera_data.area >= CAMERA_OBS_AREA_MIN) {
            if (g_camera_confirm_count < 255) {
                g_camera_confirm_count++;
            }
        } else {
            g_camera_confirm_count = 0;
        }
    }
}

void Camera_Init(void)
{
    memset(&g_camera_data, 0, sizeof(g_camera_data));
    g_camera_data.zone = 9;
    g_camera_data.band = -1;
    g_camera_data.type = 0;
    g_camera_confirm_count = 0;
    g_camera_rx_active = 0;
    g_camera_rx_index = 0;
    HAL_UART_Receive_IT(&huart1, &Camera_RxByte, 1);
}

void Camera_InputByte(u8 ch)
{
    if (ch == '$') {
        g_camera_rx_active = 1;
        g_camera_rx_index = 0;
        g_camera_packet[g_camera_rx_index++] = (char)ch;
        return;
    }

    if (!g_camera_rx_active) {
        return;
    }

    if (g_camera_rx_index >= (CAMERA_PACKET_MAX_LEN - 1)) {
        g_camera_rx_active = 0;
        g_camera_rx_index = 0;
        return;
    }

    g_camera_packet[g_camera_rx_index++] = (char)ch;

    if (ch == '#') {
        g_camera_packet[g_camera_rx_index] = '\0';
        g_camera_rx_active = 0;
        g_camera_rx_index = 0;
        Camera_ParsePacket(g_camera_packet);
    }
}

void Camera_UartRxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        Camera_InputByte(Camera_RxByte);
        HAL_UART_Receive_IT(&huart1, &Camera_RxByte, 1);
    }
}

u8 Camera_IsOnline(void)
{
    return ((HAL_GetTick() - g_camera_data.last_update_ms) <= CAMERA_FRAME_TIMEOUT_MS) ? 1 : 0;
}

u8 Camera_IsObstacleConfirmed(void)
{
    if (!Camera_IsOnline()) {
        return 0;
    }

    if (g_camera_confirm_count < CAMERA_CONFIRM_FRAMES) {
        return 0;
    }

    return (g_camera_data.valid && g_camera_data.area >= CAMERA_OBS_AREA_MIN) ? 1 : 0;
}

u8 Camera_IsLineVisible(void)
{
    /* 当前 OpenMV 只做避障，不输出 line_valid，因此这里不再把摄像头作为回线依据。 */
    return 0;
}

const camera_data_t *Camera_GetData(void)
{
    return &g_camera_data;
}

