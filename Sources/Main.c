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
 * ���ݰ���������0�Ű�������ȥ��������
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
 * ģ��ɼ������ϴ�����
 */
void mock_data_post(u8 event_type, u8 key_code)
{

	WifiPost xdata mock_post;
	char xdata mock_value_str[4];
	char xdata mock_name_str[5] = "btn_";
	int xdata mock_value = 0;
	mock_name_str[4] = '0' + key_code;
	// 0-key_code*100 �������
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
 * ��ʼ���������¼�
 */
void btn_init_listeners()
{
	// btn_bind_event(KEY_DOWN, alarm_switch);
	// btn_bind_event(KEY_UP, alarm_switch);
	btn_bind_event(KEY_UP, mock_data_post);
}

/**
 * ��ʼ������������
 */
void hsch_init_tasks()
{
	// ��ˮ������
	// hsch_add_task(led_flow, NULL, 0, 1000, SCH_COO_TYPE);
	// �����������
	// hsch_add_task(btn_poll_scan, NULL, 0, 10, SCH_COO_TYPE);
	// WIFI �״γ�ʼ�����ã��������һ�Σ����涼�ǳ־û���ȥ��
	// hsch_add_task(wifi_first_init, NULL, 5000, 0, SCH_COO_TYPE);
	// WIFI ���� TCP ���������ӳ� 10 ������
	// WifiPost xdata post;
	// post.host = WIFI_HTTP_INFLUXDB_HOST;
	// post.path = WIFI_HTTP_INFLUXDB_WRITE_PATH;
	// post.body = NULL_STR;
	// post.content_type = NULL_STR;
	// post.influx_db_measure_name = "p2p";
	// post.influx_db_measure_value = "11.1";
	// �� 10 ������ TCP ������
	hsch_add_task(wifi_connect_tcp_server, WIFI_CMD_TCP_SERVER, 10000, 0, SCH_COO_TYPE);
	hsch_add_task(btn_poll_scan, NULL, 25000, 10, SCH_COO_TYPE);
	// �� 25 ��д������
	// hsch_add_task(wifi_exec_post, &post, 25000, 0, SCH_COO_TYPE);
}

void main(void)
{
	// ��ʼ�� wifi
	wifi_init_timer2();
	// 1. ��ʼ������������ʹ�ö�ʱ��1��Ϊʱ��Դ
	hsch_init_timmer1();
	// 2. ��ʼ����������
	hsch_init_tasks();
	// 3. ��ʼ��������
	btn_init_listeners();
	// 4. ����������
	hsch_start();
	while (true)
	{
		// 5. ��ǲִ������
		hsch_dispatch_tasks();
	}
}