//
// Created by Vladimir Schneider on 2018-01-28.
//
#include "tasks.h"

#ifndef SDCC_TEST_TIMERS_H
#define SDCC_TEST_TIMERS_H

typedef struct Timer
{
    Task *prev;
    Task *next;
    uint16_t ticks;
} Timer;

// for asm code, offsets to structure's fields
#define TIMER_TICK 4

extern uint16_t TickTime();
extern void InitTimers();
extern void WaitTicks(uint16_t ticks);
extern void WaitTicksAdj(uint16_t ticks);
extern void _TickTimerIsr() __interrupt;

#ifdef USE_MILLI_TIMERS
extern void WaitMillis(uint16_t millis);
#ifdef USE_MILLI_TIMERS
extern void WaitSecs(uint16_t secs);
#endif
#endif

// for asm code, offsets to structure's fields
#define TASK_SP 6

#endif //SDCC_TEST_TIMERS_H
