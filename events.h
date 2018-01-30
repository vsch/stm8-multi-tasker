//
// Created by Vladimir Schneider on 2018-01-28.
//

#ifndef MULTI_TASKER_EVENTS_H
#define MULTI_TASKER_EVENTS_H

#include "tasks.h"

typedef struct Event
{
    struct Task *prev;
    struct Task *next;
} Event;

extern void InitEvent(Event *event);
extern void SignalEvent(Event *event);
extern void WaitEvent(Event *event);

#endif //MULTI_TASKER_EVENTS_H
