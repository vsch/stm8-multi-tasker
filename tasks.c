//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "tasks.h"

Task *currentTask;  // current task
QNode readyTasks;       // queue of ready tasks

#ifdef OPT_PREEMPT
extern uint16_t taskSliceTicks; // ticks left for this task
#endif

void *_savedReturn; // return value when we manipulate the stack

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
    ldw     __savedReturn,x  ; save x register
    popw    x ; get return address
    callf   yield.far
yield.far:
    ; here the stack looks like the following was done: push pcl, push pch, push pce
    ldw    (1,sp),x ; overwrite return with callers address
    ldw     x,__savedReturn ; restore x, not strictly needed but nice to have

    ; simulate an interrupt stack
    pushw y
    pushw x
    push a
    push cc
    rim
    jra   __IsrYieldJmp
__endasm;
// @formatter:on
}

#ifdef OPT_PREEMPT
/*
 * causes the current task to be moved to tail of ready queue.
 * current task set to 0
 * The task is not re-scheduled until the next task yield.
 *
 * interrupts disabled, assumes SP+2 is ready for iret
 */
void _IsrPreemptTail()__naked
{
// @formatter:off
__asm
    ; assumes registers already pushed and interrupts disabled
    ldw     x,_currentTask
    jrne    preempt.task
    ret

preempt.task:
    ; now pop return address and call save state, if needed
    popw     y
    ldw     __savedReturn,y ; save our return address
__endasm;

#ifdef OPT_PRESERVE_GLOBAL_STATE
__asm
    call    _RestoreGlobalState
__endasm;
#endif

__asm
    ; clear current task
    clrw    y
    ldw     _currentTask,y

    ldw     y,sp
    ldw     (TASK_SP,x),y ; save task context

    ldw     y,#_readyTasks
    exgw    x,y
    call    __QNodeLinkTailInXY
    jp      [__savedReturn]

__endasm;

// @formatter:on
}

/*
 * causes the current task to be moved to head of ready queue.
 * current task set to 0
 * The task is not re-scheduled until the next task yield.
 *
 * interrupts disabled, assumes SP+2 is ready for iret
 */
void _IsrPreemptHead()__naked
{
// @formatter:off
__asm
    ; assumes registers already pushed and interrupts disabled
    ldw     x,_currentTask
    jrne    preempth.task
    ret

preempth.task:
    ; now pop return address and call save state, if needed
    popw     y
    ldw     __savedReturn,y ; save our return address
__endasm;

#ifdef OPT_PRESERVE_GLOBAL_STATE
__asm
    call    _RestoreGlobalState
__endasm;
#endif

__asm
    ; clear current task
    clrw    y
    ldw     _currentTask,y

    ldw     y,sp
    ldw     (TASK_SP,x),y ; save task context

    ldw     y,#_readyTasks
    exgw    x,y
    call    __QNodeLinkHeadInXY
    jp      [__savedReturn]

__endasm;

// @formatter:on
}
#endif

// assumes all registers pushed as per interrupt and interrupts are disabled
// must be jumped to with:
// X : Queue to which to append the current task and then do a _IsrYieldJmp
void _IsrYieldToXTail()__naked {
// @formatter:off
__asm
    ; wait for raise event call
    ldw     y,_currentTask
    call    __QNodeLinkTailInXY

    clr    _currentTask
    clr    _currentTask+1

    jra     __IsrYieldJmp
__endasm;
// @formatter:on
}

// assumes all registers pushed as per interrupt and interrupts are disabled
// must be jumped to
void _IsrYieldJmp()__naked {
// @formatter:off
__asm
    ; yield to next available process, if none is available then returns
    ; assumes registers already pushed and interrupts disabled
    ldw     x,_currentTask
    jreq    yield.next ; no current proc, just schedule next
__endasm;

#ifdef OPT_PRESERVE_GLOBAL_STATE
__asm
    call    _SaveGlobalState
    ldw     x,_currentTask      ; restore x
__endasm;
#endif

__asm
    ldw     y,sp
    ldw     (TASK_SP,x),y ; save context

    exgw    x,y
    ldw     x,#_readyTasks
    call    __QNodeLinkTailInXY

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

