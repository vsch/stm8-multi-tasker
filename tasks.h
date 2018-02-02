//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "stdint.h"

#ifndef MULTI_TASKER_TASKS_H
#define MULTI_TASKER_TASKS_H

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

#ifdef OPT_PRIORITY
#define TD_PRIORITY 8
#endif // OPT_PRIORITY

#define TASK_INITIAL_CC         0x20    // interrupts enabled, all else 0
#define CC_CLEAR_FLAGS  0xf7  // clear I0 for resuming tasks
#define CC_SET_FLAGS    0x20  // set I1 for resuming tasks

extern Task *currentTask;   // current task, in a task this will always be the task's Task structure
extern QNode readyTasks;    // queue of tasks ready to go

#pragma callee_saves InitTasks, Yield, _IsrYieldJmp

extern void Yield();        // if there is no current active task then never returns to caller

#define YieldFromISR()      __asm__("ldw x,#_readyTasks\nldw y,#__QNodeLinkNextInXY\njp __IsrYieldToXatY")

#ifdef OPT_PRESERVE_GLOBAL_STATE
// Global State Save/Restore requires two user functions to implement pushing/poping global state variables
// the functions should be implemented in assembler and they should pop the return address first then do a jp to that
// address after pushing/poping global state to the stack
#pragma callee_saves SaveGlobalState, RestoreGlobalState

extern void SaveGlobalState();
extern void RestoreGlobalState();
#endif

#endif //MULTI_TASKER_TASKS_H
