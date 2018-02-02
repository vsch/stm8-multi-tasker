//
// Created by Vladimir Schneider on 2018-01-28.
//
#include "stdint.h"

#ifndef MULTI_TASKER_TIMERS_H
#define MULTI_TASKER_TIMERS_H

typedef struct Timer {
    Task *prev;
    Task *next;
    uint16_t ticks;
} Timer;

// for asm code, offsets to structure's fields
#define TIMER_TICK 4
#pragma callee_saves TickTime, WaitTicks

extern uint16_t TickTime();
extern void WaitTicks(uint16_t ticks);

#ifdef OPT_ADJUSTABLE_TICKS

#pragma callee_saves WaitTicksAdj
extern void WaitTicksAdj(uint16_t ticks);

#endif

#ifdef OPT_MILLI_TIMER

#pragma callee_saves WaitMillis
extern void WaitMillis(uint16_t millis);

#ifdef OPT_MILLI_TIMER
#pragma callee_saves WaitSecs
extern void WaitSecs(uint16_t secs);
#endif
#endif

extern void _TickTimerIsr()__interrupt;

#endif //MULTI_TASKER_TIMERS_H
