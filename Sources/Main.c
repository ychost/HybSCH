#include "./Headers/PreMacro.h"
#include "./Headers/HybSCH.h"
#include "./Headers/Led.h"
#include "./Headers/Button.h"
#include "./Headers/Bee.h"

/**
 * 初始化调度器任务
 */
void hsch_init_tasks()
{
	// 流水灯任务
	// hsch_add_task(led_flow, NULL, 0, 1000, SCH_COO_TYPE);
	// 按键检查任务
	hsch_add_task(btn_poll_scan, NULL, 0, 10, SCH_COO_TYPE);
}

/**
 * 根据按键报警，0号按键按下去蜂鸣器响
 */
// void alarm_switch(u8 event_type, u8 key_code)
void alarm_switch(ButtonEvent event)
{
	if (event.key_code != 0)
	{
		return;
	}
	if (event.event_type == KEY_DOWN)
	{
		bee_alarm_open();
	}
	if (event.event_type == KEY_UP)
	{
		bee_alarm_close();
	}
}

/**
 * 初始化按键绑定事件
 */
void btn_init_listeners()
{
	btn_bind_event(KEY_DOWN, alarm_switch);
	btn_bind_event(KEY_UP, alarm_switch);
}

void main(void)
{
	// 1. 初始化调度器，并使用定时器2作为时钟源
	hsch_init_timmer2();
	// 2. 初始化调度任务
	hsch_init_tasks();
	// 3. 初始化按键绑定
	btn_init_listeners();
	// 4. 启动调度器
	hsch_start();
	while (true)
	{
		// 5. 派遣执行任务
		hsch_dispatch_tasks();
	}
}