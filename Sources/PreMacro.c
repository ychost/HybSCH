#include "./Headers/PreMacro.h"

u16 soft_delay_count = 0;
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

/**
 * 重写 putbyte
 */
byte putbyte(byte ch)
{
	SBUF = ch;
	while (TI == 0)
		;
	TI = 0;
	return ch;
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
	soft_delay_count = t;
	while (soft_delay_count != 0)
		;
}

/**
 * 清空字符串
 * @param str 待清空的字符串
 * @return 清空的长度
 */
u8 string_clear(char *str)
{
	u8 str_len = 0;
	while (*str != NULL)
	{
		str[str_len] = NULL;
		str_len += 1;
		str += 1;
	}
	return str_len;
}

/**
 * 往 str 里面追加 append
 * @param str 原始字符串
 * @param append 追加字符串
 * @return str 追加了之后的长度
 */
u8 string_append(char *str, char *append)
{
	u8 str_len = string_len(str);
	while (*append != NULL)
	{
		str[str_len] = *append;
		append += 1;
		str_len += 1;
	}
	return str_len;
}

/**
 * 类似 strlen，但不想引入 string.h
 */
u8 string_len(char *str)
{
	u8 i = 0;
	char *str_ptr = str;
	while (*str_ptr != NULL)
	{
		i += 1;
		str_ptr += 1;
	}
	return i;
}

/**
 * s1 是否包含 s2
 */
bool string_contains(char *s1, char *s2)
{
	char *s1_ptr = s1;
	char *s2_ptr = s2;
	if (string_len(s1_ptr) < string_len(s2_ptr))
	{
		return false;
	}
	while (*s1_ptr != NULL && *s2_ptr != NULL)
	{
		if (*s1_ptr == *s2_ptr)
		{
			s2_ptr += 1;
		}
		else
		{
			// 不等就重新置位
			s2_ptr = s2;
		}
		s1_ptr += 1;
	}
	return *s2_ptr == NULL ? true : false;
}