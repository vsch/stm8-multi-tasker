//
// Created by Vladimir Schneider on 2018-01-28.
//

#ifndef SDCC_TEST_SIGNALS_H
#define SDCC_TEST_SIGNALS_H

#include "tasks.h"

typedef struct Signal
{
    struct Task *prev;
    struct Task *next;
} Signal;

extern void InitSignal(Signal *signal);
extern void RaiseSignal(Signal *signal);
extern void WaitSignal(Signal *signal);

#endif //SDCC_TEST_SIGNALS_H
