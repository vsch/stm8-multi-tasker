//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "multitasker.h"

Task *currentTask = 0;  // current task
QList readyTasks;       // queue of ready tasks

#ifdef OPT_TIMERS
#ifdef OPT_PREEMPT
extern uint16_t taskSliceTicks; // ticks left for this task
#endif
extern void InitTimers();
#endif

void (*_queueFunction)(); // option passed to vary which function is called to queue
QList *_taskQueue;    // option passed to vary which queue the task is moved

const uint16_t taskDescriptionSize = sizeof(TaskDescription);

void _InitTaskDescInX() __naked
{
// @formatter:off
__asm
    ; x - task description
    ldw     y,x

    ldw     x,(TD_PC,x)       ; get PC
    ldw     __queueFunction,x   ; save entry code
    ldw     x,y

    ldw     x,(TD_SP,x)  ; SP of task
    ; offset to end of stack
    pushw   x   ; save SP of task
    ldw     x,y ; x = task description
    ldw     x,(TD_STACK_SIZE,x)  ; x = tasks->stackSize
    addw    x,(1,sp)             ; add SP of task
    addw    sp,#2                ; drop SP of task

    ; x is SP pointer of task
    ; y is task description
__endasm;

#ifdef OPT_PRIORITY
__asm
    ld      a,(TD_PRIORITY,y)   ; get task description priority
__endasm;
#endif // OPT_PRIORITY

__asm
    ldw     y,(TD_TASK,y) ; get task structure
__endasm;

#ifdef OPT_PRIORITY
__asm
    ld      (TASK_PRIORITY,y), a ; init priority
__endasm;
#endif // OPT_PRIORITY

__asm
    ; need to simulate interrupt stack
    ldw     __taskQueue,y      ; save task
    ldw     y,sp    ; save real sp

    ldw     sp,x    ; get the task stack
    callf   init.next
init.next:
    ; here the stack looks like the following was done: push pcl, push pch, push pce
    ldw    x,__queueFunction    ; get entry PC
    ldw    (2,sp),x ; overwrite return with task address

    clr     a
    clrw    x
    ; simulate an interrupt stack
    pushw   x ; y
    pushw   x
    push    a
    ld      a,#TASK_INITIAL_CC
    push    a ; cc

    ldw     x,sp
    ldw     sp,y        ; restore stack
    ldw     y,__taskQueue
    ; now save this stack
    ldw     (TASK_SP,y),x  ; init stack pointer
__endasm;

#ifdef OPT_TIMERS
__asm
    clrw    x
    ldw     (TASK_TICKS, y), x ; init ticks
__endasm;
#endif // OPT_PRIORITY

__asm
    ; y - task
    ldw     x,y
    call    __InitQNodeInX

    ldw     x,#_readyTasks
    jp    __QNodeLinkNextInXY   ; add to ready tasks
__endasm;
// @formatter:on
}

/*
 * Jmp to this once the stack is as after an interrupt
 * All ready to yield,
 */

void _IsrYieldToTail()__naked
{
// @formatter:off
__asm
    sim
    ldw     x,_readyTasks
    ldw     y,#__QNodeLinkTailInXY
    jra   __IsrYieldToXatY
__endasm;
// @formatter:on
}

/*
 * Push all registers to simulate interrupt and yield to tail
 *
 * All ready to yield,
 */

void Yield()__naked
{
// @formatter:off
__asm
    sim
    ldw     x,#_readyTasks
    ldw     y,#__QNodeLinkTailInXY
    ; fallthrough to next function, assume function order is fixed by source
    ; jra   __YieldToXatYStack
__endasm;
// @formatter:on
}

/*
 * Will push all registers to simulate interrupt stack!!!
 *
 * must be jumped to
 * X : is the queue to put the task into after preempting
 * Y : is function to queue the task should be __QNodeLinkTailInXY or __QNodeLinkHeadInXY
 * sp[2] - is return address of caller, it will be used to simulate interrupt stack
 *
 * overwrites
 * _queueFunction
 * _taskQueue
 */

void _YieldToXatYStack()__naked
{
// @formatter:off
__asm
    sim
    ldw     __taskQueue,x ; save queue
    ldw     x,_currentTask
    jreq    yield.skipstack    ; no task, dont need stack it will be discarded

    popw    x ; get return address
    callf   yield.far
yield.far:
    ; here the stack looks like the following was done: push pcl, push pch, push pce
    ldw    (2,sp),x ; overwrite return with callers address

    ; simulate an interrupt stack
    pushw y
    pushw x
    push a
    push cc

yield.skipstack:
    ldw     x,__taskQueue ; restore the queue
    ; fallthrough to next function, assume function order is fixed by source
    ; jra   __IsrYieldToXatY
__endasm;
// @formatter:on
}

/*
 * Assumes all registers have been pushed to the stack as per interrupt
 *
 * must be jumped to
 * X : is the queue to put the task into after preempting
 * Y : is function to queue the task should be __QNodeLinkTailInXY or __QNodeLinkHeadInXY
 *
 * overwrites
 * _queueFunction
 * _taskQueue
 */

void _IsrYieldToXatY()__naked {
// @formatter:off
__asm
    ldw     __taskQueue,x
    ldw     __queueFunction,y

    ldw     x,_currentTask
    jreq    yield.next ; no current proc, just schedule next
__endasm;

#ifdef OPT_PRESERVE_GLOBAL_STATE
__asm
    call    _SaveGlobalState
    ldw     y,_currentTask
__endasm;
#endif

__asm
    ldw     x,sp
    ldw     (TASK_SP,y),x ; save context

    ; put task in queue
    ldw     x,[__taskQueue]
    call    [__queueFunction]
    ; after the call do not assume x/y content, could be another function

yield.next:
    ldw     x,#_readyTasks
    ldw     y,x
    cpw     y,(QHEAD,x)
    jreq    yield.halt

    ldw     x,(QHEAD,x)
    ldw     _currentTask,x
    call    __QNodeUnlinkInX

    ldw     x,(TASK_SP,x)
    ldw     sp,x   ; restore task context
__endasm;

#ifdef OPT_PRESERVE_GLOBAL_STATE
__asm
    call    _RestoreGlobalState
__endasm;
#endif

#ifdef OPT_PREEMPT
__asm
    ; reset the time slice ticks
    ldw     x, #TASK_SLICE_TICKS
    ldw     _taskSliceTicks,x
__endasm;
#endif

__asm
    ; enable interrupts for resumed task
    ld   a,(1,sp)
    and  a,#CC_CLEAR_FLAGS
    or   a,#CC_SET_FLAGS
    ld   (1,sp),a

    iret

yield.halt:
    wfi
    jra     yield.next     ; check if any available to run now

__endasm;
// @formatter:on
}

