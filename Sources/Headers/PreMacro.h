#ifndef __PREMACRO_H_
#define __PREMACRO_H_

#ifndef DEBUG
#define DEBUG
#endif

#include <at89x52.h>

typedef unsigned char u8;
typedef char byte;
typedef unsigned int u16;
typedef unsigned long u32;
typedef bit bool;
typedef void(code *Action)(void xdata *);

#ifndef true
#define true 1
#define false !true
#endif

#define NULL 0
#define NULL_STR "\0"
#define EXTERNAL0_ITRP 0
#define TIMMER0_ITRP 1
#define EXTERNAL1_ITRP 2
#define TIMMER1_ITRP 3
#define UART_ITRP 4
#define TIMMER2_ITRP 5

#define OSC_FREQ (12000000UL)
#define OSC_PER_INST (12)

#define RETURN_NORMAL (bit)0
#define RETURN_ERROR (bit)1

#ifdef DEBUG
void printf_init(void);
#endif

bool string_contains(char *s1, char *s2);
u8 string_len(char *str);
u8 string_append(char *str, char *append);
u8 string_clear(char *str);

void hard_delay_ms(const u16 t);
void soft_delay_ms(const u16 t);
// 合作式调度器 Cooperation
#define SCH_COO_TYPE 1
// 抢占式调度器 Preemptive
#define SCH_PRE_TYPE 0
#endif
