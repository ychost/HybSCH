#include "./Headers/PreMacro.h"
#include "./Headers/HybSCH.h"

// P_10 --> P_17 依次点亮
#define UP 0
// P_17 --> P_10 依次点亮
#define DOWN !UP

void led_flow(void)
{
	static direct = UP;
	// 上下流水，注意初始化 0xFF 和 0x8F 的位移操作
	P1 = (direct == UP) ? (P1 << 1 | (P1 == 0xFF ? 0x00 : 0x01)) : (P1 >> 1 | (P1 == 0x8F ? 0x00 : 0x80));
	// 流水灯掉头
	direct = ((P1 == 0x7F && direct == UP) || (P1 == 0xFE && direct == DOWN)) ? !direct : direct;
}

void main(void)
{
	// 初始化调度器
	hsch_init_timmer2();
	// 追加任务，流水灯
	hsch_add_task(led_flow, 0, 1000, SCH_COO_TYPE);
	// 启动调度器
	hsch_start();
	while (true)
	{
		// 派遣任务
		hsch_dispatch_tasks();
	}
}