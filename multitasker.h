//
// Created by Vladimir Schneider on 2018-01-28.
//

#ifndef MULTI_TASKER_H
#define MULTI_TASKER_H

#include "stdint.h"

// module options
#define OPT_QUEUES
#define OPT_TASKS
#define OPT_TIMERS
#define OPT_SEMAPHORES
#define OPT_EVENTS
#define OPT_MUTEXES

#ifdef OPT_QUEUES

#ifdef OPT_TASKS
// Global State Save/Restore requires two user functions to implement pushing/poping global state variables
// the functions should be implemented in assembler and they should pop the return address first then do a return to that
// address after pushing/poping global state to the stack
#define OPT_PRESERVE_GLOBAL_STATE
#define OPT_PREEMPT     // needs timers module, tasks have will have max ticks pre time slice before they are switched, otherwise cooperative, except for timed waits
//#define OPT_PRIORITY  // not implemented

#ifdef OPT_TIMERS

#define OPT_TICK_TIMER  //need this to reduce size of task overhead
#define OPT_ADJUSTABLE_TICKS // define the wait ticks that adjusts for scheduling delay, needs tick timer
#define OPT_MILLI_TIMER // use millisecond waits, needs tick timer
#define OPT_SEC_TIMER  // sec timer needs milli timer
#define MAIN_TICKS_PER_MILLI    10     // how many main timer tick's per millisecond, default 100us resolution for tick timer
#define TASK_SLICE_TICKS        50    // clock period, 5ms per task time slice before it is preempted

#endif // OPT_TIMERS

#ifdef OPT_SEMAPHORES
// no options
#endif // OPT_SEMAPHORES

#ifdef OPT_EVENTS
// no options
#define OPT_PRIORITY_EVENTS
#endif // OPT_EVENTS

#ifdef OPT_MUTEXES
// no options
#endif // OPT_MUTEXES

#endif // OPT_TASKS

// CLion helpers
#define  __naked __naked
#define  __interrupt __interrupt
#define  __critical __critical
#define  __code __code

#define __SAVE_AND_DISABLE_INTS     __asm__("push cc\nsim\n");
#define __RESTORE_INTS              __asm__("pop cc\n");
#define __SYNONYM_FOR(func)         __asm__("jp _" #func);     // do a transfer, define function as __naked then add this in the body

#include "queues.h"

// should be defined const so it goes into the __code segment
typedef struct QueueDescription {
    QNode *const node;
} QueueDescription;

// for asm code for QueueDescription
#define QD_NODE 0

#ifdef OPT_TASKS
#include "tasks.h"

#ifdef OPT_TIMERS
#include "timers.h"
#endif // OPT_TIMERS

// should be defined const so it goes into the __code segment
typedef struct TaskDescription {
    Task *const task;
    uint8_t *const sp;
    const uint16_t stackSize;
    const void (*const pc)();  // far address
#ifdef OPT_PRIORITY
    const uint16_t priority;
#endif // OPT_PRIORITY_EVENTS
} TaskDescription;

// for asm code
#define TD_TASK 0
#define TD_SP 2
#define TD_STACK_SIZE 4
#define TD_PC 6

#ifdef OPT_SEMAPHORES
#include "semaphores.h"

// should be defined const so it goes into the __code segment
typedef struct SemaDescription {
    Sema *const sema;
    uint8_t const initialCount;
} SemaDescription;

// for asm code for TaskDescription
#define SD_SEMA 0
#define SD_INIT 2

#endif // OPT_SEMAPHORES

#ifdef OPT_EVENTS
#include "events.h"

// should be defined const so it goes into the __code segment
typedef struct EventDescription {
    Event *const event;
    uint8_t const initialCount;
} EventDescription;

// for asm code
#define ED_EVENT 0
#define ED_INIT 2

#endif // OPT_EVENTS

#ifdef OPT_MUTEXES
#include "mutexes.h"
// should be defined const so it goes into the __code segment
typedef struct MutexDescription {
    Mutex *const mutex;
} MutexDescription;

// for asm code
#define MD_MUTEX 0

#endif // OPT_MUTEXES

// should be defined const so it goes into the __code segment
typedef struct Descriptions {
    const QueueDescription *const queues;
    const uint16_t queuesCount;
#ifdef OPT_TASKS
    const TaskDescription *const tasks;
    const uint16_t tasksCount;
#ifdef OPT_SEMAPHORES
    const SemaDescription *const semas;
    const uint16_t semasCount;
#endif // OPT_SEMAPHORES
#ifdef OPT_EVENTS
    const EventDescription *const events;
    const uint16_t eventsCount;
#endif // OPT_EVENTS
#ifdef OPT_MUTEXES
    const MutexDescription *const mutexes;
    const uint16_t mutexesCount;
#endif // OPT_MUTEXES
#endif // OPT_TASKS
} Descriptions;

#define descriptionCount(instance)        (sizeof(instance)/sizeof(*(instance)))

// for asm code
#define MTD_QUEUES 0
#define MTD_QUEUES_COUNT 2
#define MTD_TASKS 4
#define MTD_TASKS_COUNT 6
#define MTD_SEMAS 8
#define MTD_SEMAS_COUNT 10
#define MTD_EVENTS 12
#define MTD_EVENTS_COUNT 14
#define MTD_MUTEXES 16
#define MTD_MUTEXES_COUNT 18

extern void InitMultiTasker(const Descriptions *descriptions);

#endif // OPT_TASKS

#endif // OPT_QUEUES


#endif // MULTI_TASKER_H
