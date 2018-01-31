//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "tasks.h"

Task *currentTask;  // current task
QNode readyTasks;       // queue of ready tasks

#ifdef OPT_PREEMPT
extern uint16_t taskSliceTicks; // ticks left for this task
#endif

void *_queueFunction; // option passed to vary function called
void *_taskQueue;    // option passed to vary which queue the task is moved

void InitTasks() __naked
{
//    InitQNode(&readyTasks);
//    currentTask = 0;

    // @formatter:off
    __asm
    clrw    x
    ldw     _currentTask,x
    ldw     x,#_readyTasks
    jp      __InitQNodeInX
    __endasm;
    // @formatter:on
}

#ifdef OPT_PRIORITY
void InitTask(Task *task, void *taskSP, uint8_t taskPriority)__naked
{
    (void)task;
    (void)taskSP;
    (void)taskPriority;
//    task->sp = taskSP;
//    task->ticks = 0;
//    task->priority = taskPriority;
//    InitQNode((QNode *) task);
// @formatter:off
    __asm
    ld      a,(1,sp) ; priority
    ldw     y,(3,sp) ; taskSP
    ldw     x,(5,sp) ; task
            ldw     (TASK_SP,x),y
    ld      (TASK_PRIORITY,x),a
    jp      __InitQNodeInX
    __endasm;
// @formatter:on
}
#else
void InitTask(Task *task, void *taskSP)__naked
{
    (void)task;
    (void)taskSP;
//    task->sp = taskSP;
//    task->ticks = 0;
//    task->priority = taskPriority;
//    InitQNode((QNode *) task);
// @formatter:off
    __asm
    ldw     y,(2,sp) ; taskSP
    ldw     x,(4,sp) ; task
    ldw     (TASK_SP,x),y
    jp      __InitQNodeInX
    __endasm;
// @formatter:on
}
#endif

void Yield()__naked
{
// @formatter:off
__asm
    push    cc
    rim
    callf   yield.far    ; now we simulate an ISR stack
    pop     cc           ; task will be resumed here
    ret

yield.far:
    ; simulate an interrupt stack
    pushw y
    pushw x
    push a
    push cc
    rim

    ; fallthrough to next function, assume function order is fixed by source
    ; jra   __YieldToTail
__endasm;
// @formatter:on
}

/*
 * Jmp to this once the stack is as after an interrupt
 * All ready to yield,
 */
void _YieldToTail()__naked
{
// @formatter:off
__asm
    ldw     x,#_currentTask
    ldw     y,#__QNodeLinkTailInXY

    ; fallthrough to next function, assume function order is fixed by source
    ; jra   __IsrYieldToXatY
__endasm;
// @formatter:on
}

/* assumes all registers pushed as per interrupt and interrupts are disabled
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
    iret

yield.halt:
    wfi
    jra     yield.next     ; check if any available to run now

    __endasm;
// @formatter:on
}

