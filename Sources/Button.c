#include "./Headers/PreMacro.h"
#include "./Headers/HybSCH.h"
#include "./Headers/Button.h"

// 最多绑定多少个事件
#ifndef BUTTON_EVENT_MAX
#define BUTTON_EVENT_MAX 5
#endif

// 两种状态的电平
#define KEY_UP_VOL 1
#define KEY_DOWN_VOL 0

// btn_poll_scan 按下消抖
#define KEY_TRIGGER_TIMES 3

// 按键所做的 IO 口
// 如果有多个 IO，请扩展 get_button_vol
#ifndef BUTTON_PORTS
#define BUTTON_PORTS P1
#endif

// 一个多少个按键对应 key_code = 1 这里代表的时候 P1^1
#ifndef KEY_CODE_MAX
#define KEY_CODE_MAX 3
#endif

typedef idata struct
{
    // KEY_DOWN, KEY_UP
    u8 event_type;
    // 按键回调
    ButtonCallback callback;
} ButtonListener;

/**
 * 注册的回调集合
 */
ButtonListener listeners[BUTTON_EVENT_MAX];

/**
 * 按键消抖缓存，即 scan 了多少次
 */
u8 idata btn_press_buff[KEY_CODE_MAX];

/**
 * 绑定方法
 * @param event_type KEY_DOWN, KEY_UP
 * @param callback 回调方法
 * @return 是否绑定成功
 */
bool btn_bind_event(u8 event_type, ButtonCallback callback)
{
    u8 index = 0;
    for (; index < BUTTON_EVENT_MAX; index += 1)
    {
        if (listeners[index].event_type == NULL)
        {
            break;
        }
    }
    if (index >= BUTTON_EVENT_MAX)
    {
        return false;
    }
    listeners[index].event_type = event_type;
    listeners[index].callback = callback;
    return true;
}

/**
 * 移除按键事件绑定
 * @param event_type KEY_DOWN, KEY_UP
 * @param callback 待移除的回调函数
 * @return 被移除的 listener 个数
 */
u8 btn_unbind_event(u8 event_type, ButtonCallback callback)
{
    u8 count = 0;
    u8 index = 0;
    for (; index < BUTTON_EVENT_MAX; index += 1)
    {
        if (listeners[index].event_type == event_type && listeners[index].callback == callback)
        {
            listeners[index].event_type = NULL;
            listeners[index].callback = NULL;
            count += 1;
        }
    }
    return count;
}

/**
 * 获取某个按键的电平
 * @param key_code 按键索引
 * @return KEY_UP, KEY_DOWN
 */
u8 get_button_vol(u8 key_code)
{
    if (key_code >= KEY_CODE_MAX)
    {
        return KEY_UP_VOL;
    }
    return (BUTTON_PORTS >> key_code) & 0x01 == KEY_UP_VOL ? KEY_UP_VOL : KEY_DOWN_VOL;
}

/**
 * 触发所有监听 event_type 的回调
 */
void btn_raise_event(u8 event_type, u8 key_code)
{
    u8 i = 0;
    for (; i < BUTTON_EVENT_MAX; i++)
    {
        if (listeners[i].event_type == event_type && listeners[i].callback != NULL)
        {
            listeners[i].callback(event_type, key_code);
        }
    }
}

/**
 * 扫描按键状态，并触发相关回调，建议 10ms 调用一次
 */
void btn_poll_scan()
{
    u8 idata key_code = 0;
    for (; key_code < KEY_CODE_MAX; key_code += 1)
    {
        if (get_button_vol(key_code) == KEY_DOWN_VOL)
        {
            // 防止加法溢出，即用户按下去不松手的情况
            if (btn_press_buff[key_code] < 250)
            {
                btn_press_buff[key_code] += 1;
            }
            if (btn_press_buff[key_code] == KEY_TRIGGER_TIMES)
            {
                btn_raise_event(KEY_DOWN, key_code);
            }
        }
        else
        {
            if (btn_press_buff[key_code] > KEY_TRIGGER_TIMES)
            {
                btn_raise_event(KEY_UP, key_code);
            }
            btn_press_buff[key_code] = 0;
        }
    }
}