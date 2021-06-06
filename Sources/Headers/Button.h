#ifndef __KEY_BUTTON_H_
#define __KEY_BUTTON_H_

#include "./PreMacro.h"

#define KEY_DOWN 2
#define KEY_UP 3

typedef idata struct
{
    // 按键编号
    u8 key_code;
    // KEY_DOWN 还是 KEY_UP
    u8 event_type;
    // 内部索引
    u8 listener_index;
} ButtonEvent;

/**
 * 当有按键发放 key_down 或者 key_up 的时候会进行回调
 * 默认情况下 reentrant 读取的是 idata，所以 ButtonEvent 也是 idata
 * 2021-6-6：c51 的 reentrant 有问题，暂时不用结构体回调
 * @see https://www.keil.com/support/man/docs/c51/c51_le_reentrantfuncs.htm
 */
// typedef void(code *ButtonCallback)(ButtonEvent) reentrant;
typedef void(code *ButtonCallback)(u8, u9);

/**
 * 绑定方法
 * @param event_type KEY_DOWN, KEY_UP
 * @param callback 回调方法
 * @param 移除监听句柄
 * @return 是否绑定成功
 */
bool btn_bind_event(u8 event_type, ButtonCallback callback);

/**
 * 移除按键事件绑定
 * @param event_type KEY_DOWN, KEY_UP
 * @param callback 待移除的回调函数
 * @return 被移除的 listener 个数
 */
u8 btn_unbind_event(u8 event_type, ButtonCallback callback);

/**
 * 扫描按键状态，并触发相关回调
 */
void btn_poll_scan();

#endif
