//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "queues.h"

#ifndef MULTI_TASKER_SCHEDULER_H
#define MULTI_TASKER_SCHEDULER_H

typedef struct Task {
    struct Task *prev;
    struct Task *next;
#ifdef OPT_TICK_TIMER
    uint16_t ticks;
#endif
    void *sp;
#ifdef OPT_PRIORITY
    uint8_t priority;
#endif
} Task;

// for asm code, offsets to structure's fields

#ifdef OPT_TICK_TIMER
#define TASK_TICKS 4
#define TASK_SP 6
#else
#define TASK_SP 4
#endif
#ifdef OPT_PRIORITY
#define TASK_PRIORITY 8
#endif

extern Task *currentTask;   // current task, in a task this will always be the task's Task structure
extern QNode readyTasks;    // queue of tasks ready to go

extern void InitTasks();

#ifdef OPT_PRIORITY
extern void InitTask(Task *task, void *taskSP, uint8_t taskPriority);
#else
extern void InitTask(Task *task, void *taskSP);
#endif
extern void Yield();

extern void _IsrYieldJmp();

#ifdef OPT_PRESERVE_GLOBAL_STATE
// Global State Save/Restore requires two user functions to implement pushing/poping global state variables
// the functions should be implemented in assembler and they should pop the return address first then do a jp to that
// address after pushing/poping global state to the stack
extern void SaveGlobalState();
extern void RestoreGlobalState();
#endif

#ifdef OPT_PREEMPT
extern void _IsrPreemptTail();
extern void _IsrPreemptHead();
#endif

#endif //MULTI_TASKER_SCHEDULER_H
