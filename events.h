//
// Created by Vladimir Schneider on 2018-01-28.
//

#ifndef MULTI_TASKER_EVENTS_H
#define MULTI_TASKER_EVENTS_H

#include "stdint.h"
#include "multitasker.h"

typedef struct Event
{
    struct Task *prev;
    struct Task *next;
} Event;

#pragma callee_saves InitEvents, SignalEvent, WaitEvent

extern void SignalEvent(Event *event);
extern void WaitEvent(Event *event);

#ifdef OPT_PRIORITY_EVENTS
#pragma callee_saves SignalPriortyEvent

extern void SignalPriorityEvent(Event *event);
#endif // OPT_PRIORITY_EVENTS


#endif //MULTI_TASKER_EVENTS_H
