#include "./Headers/PreMacro.h"
#include "./Headers/Led.h"

// P_10 --> P_17 依次点亮
#define UP 0
// P_17 --> P_10 依次点亮
#define DOWN !UP

void led_flow()
{
    static direct = UP;
    // 上下流水，注意初始化 0xFF 和 0x8F 的位移操作
    P1 = (direct == UP) ? (P1 << 1 | (P1 == 0xFF ? 0x00 : 0x01)) : (P1 >> 1 | (P1 == 0x8F ? 0x00 : 0x80));
    // 流水灯掉头
    direct = ((P1 == 0x7F && direct == UP) || (P1 == 0xFE && direct == DOWN)) ? !direct : direct;
}
