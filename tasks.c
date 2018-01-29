//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "tasks.h"

Task *currentTask;  // current task
QNode readyTasks;       // queue of ready tasks

#ifdef USE_PREEMPT
extern uint16_t taskSliceTicks; // ticks left for this task
#endif

void InitTasks()
{
    InitQNode(&readyTasks);
    currentTask = 0;
}

void InitTask(Task *task, void *taskSP, uint8_t taskPriority)
{
    InitQNode((QNode *) task);
    task->sp = taskSP;
    task->ticks = 0;
    task->priority = taskPriority;
}

void Yield()__naked
{
// @formatter:off
__asm
    pushw y
    pushw x
    push a
    push cc
    rim
    jp   __IsrYield
__endasm;
// @formatter:on
}

#ifdef USE_PREEMPT

/*
 * causes the current task to be moved to end of ready queue
 * current task set to 0
 *
 * New task is not scheduled until _IsrYield is called
 *
 * interrupts disabled, SP ready for iret
 */
void _IsrPreempt() __naked
{
// @formatter:off
__asm
    ; assumes registers already pushed and interrupts disabled
    ldw     x,_currentTask
    jreq    preempt.done ; no current proc

    ldw     y,sp
    ldw     (TASK_SP,x),y ; save context

    clrw    y
    ldw     _currentTask,y ; clear current task

    ldw     y,#_readyTasks
    pushw   y
    pushw   x
    call    _QNodeLinkTail
    addw    sp,#4
preempt.done:
    ret
__endasm;

// @formatter:on
}
#endif

// assumes all registers pushed as per interrupt and interrupts are disabled
void _IsrYield()__naked {
// @formatter:off
    __asm
    ; yield to next available process, if none is available then returns
    ; assumes registers already pushed and interrupts disabled
    ldw     x,_currentTask
    jreq    yield.next ; no current proc, just schedule next

    ldw     y,sp
    ldw     (TASK_SP,x),y ; save context

    ldw     y,#_readyTasks
    pushw   y
    pushw   x
    call    _QNodeLinkTail
    addw    sp,#4

yield.next:
    ldw     x,#_readyTasks
    ldw     y,x
    cpw     y,(QHEAD,x)
    jreq    yield.halt

    ldw     x,(QHEAD,x)
    ldw     _currentTask,x
    ldw     x,(TASK_SP,x)
    ldw     sp,x   ; restore tasks context
__endasm;

#ifdef USE_PREEMPT

__asm
    ; reset the time slice ticks
    ldw     x, #TASK_SLICE_MILLI*MAIN_TICKS_PER_MILLI
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

