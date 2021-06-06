#include "./Headers/PreMacro.h"
#include "./Headers/Led.h"

// P_10 --> P_17 依次点亮
#define UP 0
// P_17 --> P_10 依次点亮
#define DOWN !UP

/**
 * led 流水灯
 */
void led_flow()
{
    static direct = UP;
    // 上下流水，注意初始化 0xFF 和 0x8F 的位移操作
    P1 = (direct == UP) ? (P1 << 1 | (P1 == 0xFF ? 0x00 : 0x01)) : (P1 >> 1 | (P1 == 0x8F ? 0x00 : 0x80));
    // 流水灯掉头
    direct = ((P1 == 0x7F && direct == UP) || (P1 == 0xFE && direct == DOWN)) ? !direct : direct;
}

/**
 * 点亮一颗 LED
 * @param led_index led 索引，0-7
 */
bool led_light_one(u8 led_index)
{
    u8 led_state = 0xFE;
    u8 i = 0;
    if (led_index > 7)
    {
        return false;
    }
    // 左移补 1
    for (; i < led_index; i++)
    {
        led_state <<= 1;
        led_state |= 0x01;
    }
    P1 &= led_state;
    return true;
}