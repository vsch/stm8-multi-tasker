//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "queues.h"

#ifndef SDCC_TEST_SCHEDULER_H
#define SDCC_TEST_SCHEDULER_H

typedef struct Task {
    struct Task *prev;
    struct Task *next;
    uint16_t ticks;
    void *sp;
    uint8_t priority;
} Task;

// for asm code, offsets to structure's fields
#define TASK_SP 6

extern Task *currentTask;   // current task, in a task this will always be the task's Task structure
extern QNode readyTasks;    // queue of tasks ready to go

extern void InitTasks();
extern void InitTask(Task *task, void *taskSP, uint8_t taskPriority);
extern void Yield();

extern void _IsrYield();
#ifdef USE_PREEMPT
extern void _IsrPreempt();
#endif

#endif //SDCC_TEST_SCHEDULER_H
