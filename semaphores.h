//
// Created by Vladimir Schneider on 2018-01-28.
//

#ifndef MULTI_TASKER_SEMAPHORES_H
#define MULTI_TASKER_SEMAPHORES_H

#include "stdint.h"

typedef struct Sema {
    struct Task *prev;
    struct Task *next;
    uint8_t count;
} Sema;

// for asm code, offsets to structure's fields
#define SEMA_COUNT 4

#pragma callee_saves InitSema, AcquireSema, ReleaseSema

extern void InitSema(Sema *sema, uint8_t initialCount);
extern void AcquireSema(Sema *sema);
extern void ReleaseSema(Sema *sema);

#endif //MULTI_TASKER_SEMAPHORES_H
