#include "./Headers/PreMacro.h"

/**
 * KEIL调试打印输出，使用了定时器1，作为串口的时钟源
 */
#ifdef DEBUG
void printf_init(void)
{
	SCON = 0x52;
	PCON &= 0x7f;
	TMOD |= 0x20;
	TH1 = 0xfD;
	TL1 = 0xfD;
	TR1 = 1;
	TI = 1;
	ES = 0;
}
#endif

/**
 * 硬件延时,T0作为该延时的时钟源
 * @param t 延时的毫秒数
 */
void hard_delay_ms(const u16 t)
{
	u16 ms;
	TMOD &= 0xf0;
	TMOD |= 0x01;
	ET0 = 0;
	for (ms = 0; ms < t; ++ms)
	{
		TH0 = 0XFC;
		TL0 = 0X18;
		TF0 = 0;
		TR0 = 1;
		while (TF0 == 0)
			;
		TR0 = 0;
	}
}

/**
 * 软件延时
 * @param t 延时的毫秒数
 */
void soft_delay_ms(const u16 t)
{
	u8 x, y;
	for (x = 0; x < t; ++x)
	{
		for (y = 0; y < 120; ++y)
			;
	}
}