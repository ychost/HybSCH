#include "./Headers/Bee.h"

sbit bee_port = P0^0;

/**
 * 警报声响
 */
void bee_alarm_open()
{
    bee_port = 0;
}

/**
 * 关闭警报声
 */
void bee_alarm_close()
{
    bee_port = 1;
}
