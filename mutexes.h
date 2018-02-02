//
// Created by Vladimir Schneider on 2018-01-28.
//

#ifndef MULTI_TASKER_MUTEXES_H
#define MULTI_TASKER_MUTEXES_H

#include "stdint.h"

typedef struct Mutex {
    struct Task *prev;
    struct Task *next;
    uint16_t owner;
    uint8_t locks;
} Mutex;

// for asm code, offsets to structure's fields
#define MUTEX_OWNER 4
#define MUTEX_LOCKS 6

#pragma callee_saves InitMutex, LockMutex, UnlockMutex

extern void LockMutex(Mutex *mutex);
extern void UnlockMutex(Mutex *mutex);

#endif //MULTI_TASKER_MUTEXES_H
