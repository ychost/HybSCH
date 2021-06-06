#define SCH_TASK_TIMER1
#include "./Headers/PreMacro.h"
#include "./Headers/HybSCH.h"
#include "./Headers/Led.h"
#include "./Headers/Button.h"
#include "./Headers/Bee.h"
#include "./Headers/Esp8266Wifi.h"
#include <stdio.h>
#include <stdlib.h>

extern u16 random_seeds;

/**
 * 根据按键报警，0号按键按下去蜂鸣器响
 */
void alarm_switch(u8 event_type, u8 key_code)
{
	if (event_type == KEY_DOWN)
	{
		bee_alarm_open();
	}
	if (event_type == KEY_UP)
	{
		bee_alarm_close();
	}
}

/**
 * 模拟采集，并上传数据
 */
void mock_data_post(u8 event_type, u8 key_code)
{

	WifiPost xdata mock_post;
	char xdata mock_value_str[4];
	char xdata mock_name_str[5] = "btn_";
	int xdata mock_value = 0;
	mock_name_str[4] = '0' + key_code;
	// 0-key_code*100 的随机数
	mock_value = random_seeds % ((key_code + 1) * 100);
	sprintf(mock_value_str, "%d", mock_value);

	mock_post.host = WIFI_HTTP_INFLUXDB_HOST;
	mock_post.path = WIFI_HTTP_INFLUXDB_WRITE_PATH;
	mock_post.content_type = NULL_STR;
	mock_post.body = NULL_STR;
	mock_post.influx_db_measure_name = mock_name_str;
	mock_post.influx_db_measure_value = mock_value_str;
	wifi_exec_post(&mock_post);
}

/**
 * 初始化按键绑定事件
 */
void btn_init_listeners()
{
	// btn_bind_event(KEY_DOWN, alarm_switch);
	// btn_bind_event(KEY_UP, alarm_switch);
	btn_bind_event(KEY_UP, mock_data_post);
}

/**
 * 初始化调度器任务
 */
void hsch_init_tasks()
{
	// 流水灯任务
	// hsch_add_task(led_flow, NULL, 0, 1000, SCH_COO_TYPE);
	// 按键检查任务
	// hsch_add_task(btn_poll_scan, NULL, 0, 10, SCH_COO_TYPE);
	// WIFI 首次初始化配置，仅需调用一次，后面都是持久化进去的
	// hsch_add_task(wifi_first_init, NULL, 5000, 0, SCH_COO_TYPE);
	// WIFI 连接 TCP 服务器，延迟 10 秒启动
	// WifiPost xdata post;
	// post.host = WIFI_HTTP_INFLUXDB_HOST;
	// post.path = WIFI_HTTP_INFLUXDB_WRITE_PATH;
	// post.body = NULL_STR;
	// post.content_type = NULL_STR;
	// post.influx_db_measure_name = "p2p";
	// post.influx_db_measure_value = "11.1";
	// 第 10 秒连接 TCP 服务器
	hsch_add_task(wifi_connect_tcp_server, WIFI_CMD_TCP_SERVER, 10000, 0, SCH_COO_TYPE);
	hsch_add_task(btn_poll_scan, NULL, 25000, 10, SCH_COO_TYPE);
	// 第 25 秒写入数据
	// hsch_add_task(wifi_exec_post, &post, 25000, 0, SCH_COO_TYPE);
}

void main(void)
{
	// 初始化 wifi
	wifi_init_timer2();
	// 1. 初始化调度器，并使用定时器1作为时钟源
	hsch_init_timmer1();
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