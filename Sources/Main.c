#include "./Headers/PreMacro.h"
#include "./Headers/HybSCH.h"

// P_10 --> P_17 ���ε���
#define UP 0
// P_17 --> P_10 ���ε���
#define DOWN !UP

void led_flow(void)
{
	static direct = UP;
	// ������ˮ��ע���ʼ�� 0xFF �� 0x8F ��λ�Ʋ���
	P1 = (direct == UP) ? (P1 << 1 | (P1 == 0xFF ? 0x00 : 0x01)) : (P1 >> 1 | (P1 == 0x8F ? 0x00 : 0x80));
	// ��ˮ�Ƶ�ͷ
	direct = ((P1 == 0x7F && direct == UP) || (P1 == 0xFE && direct == DOWN)) ? !direct : direct;
}

void main(void)
{
	// ��ʼ��������
	hsch_init_timmer2();
	// ׷��������ˮ��
	hsch_add_task(led_flow, 0, 1000, SCH_COO_TYPE);
	// ����������
	hsch_start();
	while (true)
	{
		// ��ǲ����
		hsch_dispatch_tasks();
	}
}