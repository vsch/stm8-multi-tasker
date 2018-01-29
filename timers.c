//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "timers.h"

uint16_t tickTime; // incrementing tick counter

Timer mainTimer;

#ifdef USE_MILLI_TIMERS
uint16_t milliPrescaler; // decremented every mainTick, on zero does milliTick
Timer milliTimer;

#ifdef USE_SEC_TIMERS
uint16_t secPrescaler; // decrement every milliTick, on zero do secTick
Timer secTimer;
#endif
#endif

#ifdef USE_PREEMPT
uint16_t taskSliceTicks; // ticks left for this task
#endif

/*
 *  Add a given timer node to the main timer queue
 *
 *  Each node stores number of ticks from previous node (or main timer if first node)
 *
 *  When the main timer's tick count goes to 0 on the next TimerIsr, all leading nodes with tick count of 0 get scheduled.
 */
void _AddTimerTask(Timer *timer, Task *node)
{
    Task *other;
    if (timer->ticks > node->ticks) {
        node->ticks -= timer->ticks;
        other = (Task *) timer;
        do {
            other = other->next;
            if (other == (Task *) timer) {
                QNodeLinkTail((QNode *) timer, (QNode *) node);
                break;
            } else if (node->ticks <= other->ticks) {
                other->ticks -= node->ticks;
                QNodeLinkPrev((QNode *) other, (QNode *) node);
                break;
            }
            node->ticks -= other->ticks;
        } while (1);
    } else {
        timer->ticks = node->ticks;
        timer->next->ticks += node->ticks;
        node->ticks = 0;

        // link node at head of timer
        QNodeLinkHead((QNode *) timer, (QNode *) node);
    }
}

void _InitTimer(Timer *timer, uint16_t tick)
{
    timer->ticks = tick;
    InitQNode((QNode *) timer);
}

/*
 * Call this after reset and before staring any interrupts
 *
 * Interrupts should be disabled.
 */
void InitTimers()
{
    _InitTimer(&mainTimer, 0);
#ifdef USE_MILLI_TIMERS
    _InitTimer(&milliTimer, 0);
    milliPrescaler = MAIN_TICKS_PER_MILLI;
    (void) milliPrescaler;
#ifdef USE_SEC_TIMERS
    _InitTimer(&secTimer, 0);
    secPrescaler = 1000;
    (void) secPrescaler;
#endif
#endif

#ifdef USE_PREEMPT
    taskSliceTicks = TASK_SLICE_MILLI * MAIN_TICKS_PER_MILLI; // decrement every milliTick, on zero do secTick
#endif
}

/*
 * Handles timer tick, decrementing and if 0 scheduling tasks
 */
typedef void (*QFunc)(QNode *, QNode *);

void DoTimerTick(Timer *timer, QFunc qFunc)
{
    // decrement ticks and if 0 schedule all nodes whose ticks are 0
    if (timer->ticks) {
        if (!--timer->ticks) {
            Task *timer1 = (Task *) timer;

            do {
                timer1 = timer1->next;
                if (timer1 == (Task *) timer) break;
                if (timer1->ticks) break;

                // schedule this one
                timer1->ticks = tickTime; // save it for possible comp on fast clock and short periodic waits
                qFunc(&readyTasks, (QNode *) timer1);
            } while (1);
        }
    }
}

/*
 * Main Timer ISR
 *
 * Tasks whose timeout is done will be pushed on the task ready queue as follows:
 *
 * MainTimer : to head of queue
 * MilliTimer : to head of queue, after the tasks pushed by main timer (if in same timer tick)
 * SecTimer: to tail of queue
 */
void _TickTimerIsr(void)__naked __interrupt
{
// @formatter:off
__asm
    ; bump up the tick count
    ldw     x,_tickTime
    incw    x
    ldw     _tickTime,x
__endasm;

#ifdef USE_PREEMPT
__asm
    ; dec task timeslice
    ldw     x,_taskSliceTicks
    decw    x
    ldw     _taskSliceTicks,x
    jrne    tick.no_preempt

    ; pre-empt the current task
    call    __IsrPreempt
tick.no_preempt:
__endasm;
#endif

#ifdef USE_MILLI_TIMERS
__asm
    ldw     x,_milliPrescaler
    decw    x
    jrne    tick.milli_save

    ; milli ran out, do milli tick
    ldw     x,_milliTimer
    pushw   x
    ldw     x,_QNodeLinkHead
    pushw   x
    call    _DoTimerTick
    addw    sp,#4
    ldw     x,#MAIN_TICKS_PER_MILLI
tick.milli_save:
    ldw     _milliPrescaler,x
__endasm;

#ifdef USE_SEC_TIMERS
__asm

    ldw     x,_secPrescaler
    decw    x
    jrne    tick.sec_save

    ; sec ran out, do sec tick
    ldw     x,_secTimer
    pushw   x
    ldw     x,_QNodeLinkTail
    pushw   x
    call    _DoTimerTick
    addw    sp,#4
    ldw     x,#1000
tick.sec_save:
    ldw     _secPrescaler,x
__endasm;

#endif
#endif

__asm

    ; now do mainTimer
    ldw     x,#_mainTimer
    pushw   x
    ldw     x,_QNodeLinkHead
    pushw   x
    call    _DoTimerTick
    addw    sp,#4
tick.done:
    iret
__endasm;
// @formatter:on
}

void _WaitCommon()__naked {
// @formatter:off
    // all registers pushed on stack ready for _IsrYield call
    // X is timer
    // Y is ticks
__asm
    tnzw    y
    jrne    wait.1

    ; already 0, yield
    jp     __IsrYield

wait.1:
    pushw   x
    ldw     x,_currentTask
    ldw     (TIMER_TICK,x),y    ; set timer ticks

    ; timer already on stack
    pushw   x   ; push task as if it was timer node
    call    __AddTimerTask
    addw    sp,#4  ; drop params

    clr    _currentTask
    clr    _currentTask+1

    jp      __IsrYield
__endasm;
// @formatter:on
}

void WaitTicks(uint16_t ticks)__naked
{
    (void) ticks;
// @formatter:off
__asm
    pushw y
    pushw x
    push a
    push cc
    rim
    ldw     y,(9,sp)  ; get passed parameter ticks
    ldw     x,#_mainTimer
    jp      __WaitCommon
__endasm;
// @formatter:on
}

// reduce the number of ticks by diff between currentTask->tick
void WaitTicksAdj(uint16_t ticks)__naked
{
    (void) ticks;
// @formatter:off
__asm
    pushw y
    pushw x
    push a
    push cc
    rim
    ldw     y,(9,sp)  ; get passed parameter ticks
    ldw     x,#_currentTask
    ldw     x,(TIMER_TICK,x)
    subw    x,_tickTime
    ; see if rolled over
    negw    x  ; x = tickTime - currentTask->tics
    jrpl    waitadj.notrolled ; did not rollover
    ; rolled over, we need to add 65536, ie. negw again
    negw    x

waitadj.notrolled:
    pushw   x
    ; need to subtract it from y and if y is less then make it 0
    ld     a,yl
    sub    a,(1,sp)
    ld     yl,a
    ld     a,yh
    sbc    a,(2,sp)
    ld     yh,a
    jrnc   waitadj.done ; no carry, was big enough

    clrw    y   ; no wait, delayed too long
waitadj.done:
    popw    x
    ldw     x,#_mainTimer
    jp      __WaitCommon
__endasm;
// @formatter:on
}

#ifdef USE_MILLI_TIMERS
void WaitMillis(uint16_t millis)__naked
{
    (void) millis;
// @formatter:off
__asm
    pushw y
    pushw x
    push a
    push cc
    rim
    ldw     y,(9,sp)  ; get passed parameter ticks
    ldw     x,#_milliTimer
    jp      __WaitCommon
__endasm;
// @formatter:on
}

#ifdef USE_SEC_TIMERS
void WaitSecs(uint16_t secs)__naked
{
    (void) secs;
// @formatter:off
__asm
    pushw y
    pushw x
    push a
    push cc
    rim
    ldw     y,(9,sp)  ; get passed parameter ticks
    ldw     x,#_secTimer
    jp      __WaitCommon
__endasm;
// @formatter:on
}
#endif
#endif

