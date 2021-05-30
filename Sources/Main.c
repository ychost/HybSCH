#include "./Headers/PreMacro.h"
#include "./Headers/HybSCH.h"
#include "./Headers/Led.h"
#include "./Headers/Button.h"
#include "./Headers/Bee.h"

/**
 * ��ʼ������������
 */
void hsch_init_tasks()
{
	// ��ˮ������
	// hsch_add_task(led_flow, NULL, 0, 1000, SCH_COO_TYPE);
	// �����������
	hsch_add_task(btn_poll_scan, NULL, 0, 10, SCH_COO_TYPE);
}

/**
 * ���ݰ���������0�Ű�������ȥ��������
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
 * ��ʼ���������¼�
 */
void btn_init_listeners()
{
	btn_bind_event(KEY_DOWN, alarm_switch);
	btn_bind_event(KEY_UP, alarm_switch);
}

void main(void)
{
	// 1. ��ʼ������������ʹ�ö�ʱ��2��Ϊʱ��Դ
	hsch_init_timmer2();
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