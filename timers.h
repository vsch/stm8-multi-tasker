//
// Created by Vladimir Schneider on 2018-01-28.
//
#include "tasks.h"

#ifndef MULTI_TASKER_TIMERS_H
#define MULTI_TASKER_TIMERS_H

typedef struct Timer {
    Task *prev;
    Task *next;
    uint16_t ticks;
} Timer;

// for asm code, offsets to structure's fields
#define TIMER_TICK 4

extern uint16_t TickTime();
extern void InitTimersF();
extern void WaitTicks(uint16_t ticks);

#ifdef OPT_ADJUSTABLE_TICKS
extern void WaitTicksAdj(uint16_t ticks);
#endif

#ifdef OPT_MILLI_TIMER
extern void WaitMillis(uint16_t millis);
#ifdef OPT_MILLI_TIMER
extern void WaitSecs(uint16_t secs);
#endif
#endif

extern void _TickTimerIsr()__interrupt;

#endif //MULTI_TASKER_TIMERS_H
