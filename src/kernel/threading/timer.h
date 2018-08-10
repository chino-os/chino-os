#pragma once
#include <cstdint>

#define CLOCK_SECOND 100

typedef struct
{
	/* 开始时间 */
	uint16_t start;
	/* 时间间隔 */
	uint16_t interval;
}timer_typedef;

/* 外部函数声明 ***************************************************************/
void timer_config(void);
uint16_t clock_time(void);
void timer_set(timer_typedef* ptimer, uint16_t interval);
void timer_reset(timer_typedef* ptimer);
int8_t timer_expired(timer_typedef* ptimer);