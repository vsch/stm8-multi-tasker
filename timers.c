//
// Created by Vladimir Schneider on 2018-01-28.
//
#include "multitasker.h"

#ifdef OPT_TIMERS

#ifdef OPT_TICK_TIMER

Timer mainTimer;
uint16_t tickTime; // incrementing tick counter
#ifdef OPT_MILLI_TIMER
Timer milliTimer;
uint16_t milliPrescaler; // decremented every mainTick, on zero does milliTick

#ifdef OPT_SEC_TIMER
uint16_t secPrescaler; // decrement every milliTick, on zero do secTick
#endif

#ifdef OPT_PREEMPT
Timer secTimer;
uint16_t taskSliceTicks; // ticks left for this task
#endif

#ifdef OPT_MILLI_TIMER
#ifdef OPT_SEC_TIMER
#define TIMER_COUNT 3
#else
#define TIMER_COUNT 2
#endif
#else
#define TIMER_COUNT 1
#endif
#endif
#endif

const uint16_t timerSize = sizeof(Timer);

/*
 * Call this after reset and before staring any interrupts
 *
 * Interrupts should be disabled.
 */
void InitTimers()__naked
{
// @formatter:off
__asm
    clrw     y
    ldw     x,#_mainTimer
    ldw     (TIMER_TICK, x), y
    ld      a,#TIMER_COUNT
init.loop:
    call    __InitQNodeInX
    addw    x,_timerSize
    dec     a
    jrne    init.loop
__endasm;
// @formatter:on

#ifdef OPT_MILLI_TIMER
// @formatter:off
__asm
    ldw     x,#_milliTimer
    ldw     y,#MAIN_TICKS_PER_MILLI
    ldw     (TIMER_TICK,x),y
__endasm;
// @formatter:on

#ifdef OPT_SEC_TIMER
// @formatter:off
__asm
    ldw     x,#_secTimer
    ldw     y,#1000
    ldw     (TIMER_TICK, x),y
__endasm;
// @formatter:on
#endif
#endif

__asm
    ret
__endasm;
}


/*
 *  Add a given timer node to the main timer queue
 *
 *  Each node stores number of ticks from previous node (or main timer if first node)
 *
 *  When the main timer's tick count goes to 0 on the next TimerIsr, all leading nodes with tick count of 0 get scheduled.
 *
 *  These parameters are the same as for Queing Functions, So this function is passed to YieldToXatY to do the task switch
 *  X - timer
 *  Y - task
 */
void _AddTaskYtoTimerX()__naked
{
//    Task *other;
//    if (timer->ticks > node->ticks) {
//        node->ticks -= timer->ticks;
//        other = (Task *) timer;
//        do {
//            other = other->next;
//            if (other == (Task *) timer) {
//                // reached the end of waiting list, we go at the end
//                QNodeLinkTail((QNode *) timer, (QNode *) node);
//                break;
//            }
//            if (node->ticks <= other->ticks) {
//                other->ticks -= node->ticks;
//                QNodeLinkPrev((QNode *) other, (QNode *) node);
//                break;
//            }
//            node->ticks -= other->ticks;
//        } while (1);
//    } else {
//        // if there is no next this will be overwritten by the line afterwards
//        timer->next->ticks += node->ticks;
//        timer->ticks = node->ticks;
//        node->ticks = 0;
//
//        // link node at head of timer
//        QNodeLinkHead((QNode *) timer, (QNode *) node);
//    }
// @formatter:off
__asm
    pushw   x
    ; test high byte
    ld      a,(TASK_TICKS,y)
    cp      a,(TIMER_TICK,x)
    jrc     add.inside  ; timer->ticks > node->ticks
    jrne    add.here    ; timer->ticks < node->ticks

    ; test low byte
    ld      a,(TASK_TICKS+1,y)
    cp      a,(TIMER_TICK+1,x)
    jrc     add.inside  ; timer->ticks > node->ticks

add.here:
    ; here: timer->ticks <= task->ticks
    ldw     x,(QNEXT,x) ; x = timer->next
    ld      a,(TASK_TICKS+1,x)
    add     a,(TASK_TICKS+1,y)
    ld      (TASK_TICKS+1,x),a
    ld      a,(TASK_TICKS,x)
    adc     a,(TASK_TICKS,y)
    ld      (TASK_TICKS,x),a ; timer->next->ticks += node->ticks;

    popw     x ; x = timer
    ld      a,(TASK_TICKS,y)
    ld      (TASK_TICKS,x),a
    ld      a,(TASK_TICKS+1,y)
    ld      (TASK_TICKS+1,x),a ; timer->ticks = node->ticks;

    clr     (TASK_TICKS, y)
    clr     (TASK_TICKS+1, y)  ; node->ticks = 0

    ; x = timer, y = node
    jp      __QNodeLinkHeadInXY

add.inside:
    ; timer->ticks > node->ticks
//        node->ticks -= timer->ticks;
//        other = (Task *) timer;
//        do {
//            other = other->next;
//            if (other == (Task *) timer) {
//                // reached the end of waiting list, we go at the end
//                QNodeLinkTail((QNode *) timer, (QNode *) node);
//                break;
//            }
//            if (node->ticks <= other->ticks) {
//                other->ticks -= node->ticks;
//                QNodeLinkPrev((QNode *) other, (QNode *) node);
//                break;
//            }
//            node->ticks -= other->ticks;
//        } while (1);

    ld      a,(TASK_TICKS+1,y)
    sub     a,(TIMER_TICK+1,x)
    ld      (TASK_TICKS+1,y),a
    ld      a,(TASK_TICKS,y)
    sbc     a,(TIMER_TICK,x)
    ld      (TASK_TICKS,y),a ; node->ticks -= timer->ticks

    ; x = timer, y = node
add.do:
    ; x = other, y = node
    ldw     x,(QNEXT,x)  ; x = other->next

    ; x = other->next, y = node
    cpw     x,(1,sp) ; timer
    jrne    add.head
    ; reached end of queue

    popw     x      ; x = timer, y = node
    jp      __QNodeLinkTailInXY   ; link at end and return

add.head:
    ; x = other, y = node
//            if (node->ticks <= other->ticks) {
//                other->ticks -= node->ticks;
//                QNodeLinkPrev((QNode *) other, (QNode *) node);
//                break;
//            }
//            node->ticks -= other->ticks;
    ld      a,(TASK_TICKS,x)
    cp      a,(TASK_TICKS,y) ;
    jrc     add.nothere ; node->ticks > other->ticks
    jrne    add.foundspot ; node->ticks < other->ticks

    ; test low byte
    ld      a,(TASK_TICKS+1,x)
    cp      a,(TASK_TICKS+1,y)
    jrc     add.nothere ; node->ticks > other->ticks

    ; found spot
add.foundspot:
    ld     a,(TASK_TICKS+1,x)
    sub    a,(TASK_TICKS+1,y)
    ld     (TASK_TICKS+1,x),a
    ld     a,(TASK_TICKS,x)
    sbc    a,(TASK_TICKS,y)
    ld     (TASK_TICKS,x),a ; other->ticks -= node->ticks
    addw    sp,#2 ; drop timer
    jp      __QNodeLinkPrevInXY ; link node before other

add.nothere:
    ld     a,(TASK_TICKS+1,y)
    sub    a,(TASK_TICKS+1,x)
    ld     (TASK_TICKS+1,y),a
    ld     a,(TASK_TICKS,x)
    sbc    a,(TASK_TICKS,y)
    ld     (TASK_TICKS,x),a ; node->ticks -= other->ticks

    ; x = other, y = node
    jra     add.do

__endasm;
// @formatter:on
}

/*
 * Do timer tick for timer X, queing function in Y
 *
 *
 */
void _DoTimerTickInXY()__naked
{
//    // decrement ticks and if 0 schedule all nodes whose ticks are 0
//    if (timer->ticks) {
//        if (!--timer->ticks) {
//            Task *task = (Task *) timer;
//
//            do {
//                task = task->next;
//                if (task == (Task *) timer) break;
//                if (task->ticks) break;
//
//                // schedule this one
//                task->ticks = tickTime; // save it for possible comp on fast clock and short periodic waits
//                qFunc(&readyTasks, (QNode *) task);
//            } while (1);
//        }
//    }
// @formatter:off
__asm
    ldw     __queueFunction,y ; save func in memory
    ldw     y,x
    ldw     y,(TIMER_TICK,y)
    jreq    dotimer.done

    decw    y
    ldw     (TIMER_TICK,x),y    ; --timer->ticks
    jrne    dotimer.done        ; not zero

    pushw   x ; save timer

dotimer.do:
    ldw     x,(QNEXT,x) ; task = task->next;
    cpw     x,(1,sp)  ; timer
    jreq    dotimer.while ; task == timer

    ldw     y,x
    ldw     y,(TASK_TICKS,y)
    jrne    dotimer.while  ; task->ticks != 0
__endasm;

#ifdef OPT_ADJUSTABLE_TICKS
__asm
    ldw     y,_tickTime
    ldw     (TASK_TICKS,x),y ; task->ticks = tickTime
__endasm;
#endif

__asm
    exgw    x,y
    ldw     x,#_readyTasks ; x = readyTasks, y = task
    call    [__queueFunction]
    jra     dotimer.do

dotimer.while:
    addw    sp,#2   ; drop saved timer

dotimer.done:
    ret
__endasm;
// @formatter:on
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

#ifdef OPT_MILLI_TIMER
__asm
    ldw     x,_milliPrescaler
    decw    x
    jrne    tick.milli_save

    ; milli ran out, do milli tick
    ldw     x,_milliTimer
    ldw     y,_QNodeLinkHead
    call    __DoTimerTickInXY
    ldw     x,#MAIN_TICKS_PER_MILLI
tick.milli_save:
    ldw     _milliPrescaler,x
__endasm;
#endif

#ifdef OPT_SEC_TIMER
__asm
    ldw     x,_secPrescaler
    decw    x
    jrne    tick.sec_save

    ; sec ran out, do sec tick
    ldw     x,_secTimer
    ldw     y,_QNodeLinkTail
    call    __DoTimerTickInXY
    ldw     x,#1000
tick.sec_save:
    ldw     _secPrescaler,x
__endasm;
#endif

__asm
    ; now do mainTimer
    ldw     x,#_mainTimer
    ldw     y,_QNodeLinkHead
    call    __DoTimerTickInXY
tick.done:
__endasm;

#ifdef OPT_PREEMPT
__asm
    ; now we see if we preempt the current task
    ; dec task timeslice
    ldw     x,_taskSliceTicks
    decw    x
    ldw     _taskSliceTicks,x
    jreq    tick.preempt
    iret

tick.preempt:
    ; pre-empt the current task
    jp      __IsrYieldToTail
__endasm;
#else
__asm
    iret
__endasm;
#endif

// @formatter:on
}

#ifdef OPT_ADJUSTABLE_TICKS
// reduce the number of ticks by diff between currentTask->tick
void WaitTicksAdj(uint16_t ticks)__naked
{
    (void) ticks;
// @formatter:off
__asm
    sim
    ldw     y,(3,sp)  ; get passed parameter ticks sp: pch, pcl, ev.h, ev.l
    ldw     x,_currentTask
    ldw     x,(TIMER_TICK,x)
    subw    x,_tickTime
    ; see if rolled over
    negw    x  ; x = tickTime - currentTask->tics
    jrpl    waitadj.notrolled ; did not rollover
    ; rolled over, we need to add 65536, ie. negw again
    negw    x

waitadj.notrolled:
    ; need to subtract it from y and if y is less then make it 0
    ld     a,yl
    sub    a,(4,sp)  ; ticks.l
    ld     yl,a
    ld     a,yh
    sbc    a,(3,sp)
    ld     yh,a
    jrnc   waitadj.done ; no carry, was big enough

    clrw    y   ; no wait, delayed too long
waitadj.done:
    ldw     (3,sp),y ; overwrite with adjusted tick count

    ; fallthrough to next function, assume function order is fixed by source
    ;jra     _WaitTicks ; do the usual thing
__endasm;
// @formatter:on
}
#endif

void WaitTicks(uint16_t ticks)__naked
{
    (void) ticks;
// @formatter:off
__asm
    ldw     x,#_mainTimer

; X - timer
; Y - ticks
waitcommon:
    sim
    ldw     __taskQueue,x  ; save timer
    ldw     __queueFunction,y ; save ticks
    ldw     y,(3,sp)  ; get passed parameter ticks, sp: pch, pcl, ev.h, ev.l
    tnzw    y
    jrne    wait.1

    ; already 0, yield
    jp     _Yield

wait.1:
    ldw     y,__queueFunction
    ldw     x,_currentTask
    ldw     (TIMER_TICK,x),y    ; set timer ticks
    ldw     x,__taskQueue ; x is timer
    ldw     y,#__AddTaskYtoTimerX
    jp      __YieldToXatYStack

__endasm;
// @formatter:on
}

#ifdef OPT_MILLI_TIMER
void WaitMillis(uint16_t ticks)__naked
{
    (void) ticks;
// @formatter:off
__asm
    ldw     x,#_milliTimer
    jra     waitcommon
__endasm;
// @formatter:on
}

#ifdef OPT_SEC_TIMER
void WaitSecs(uint16_t ticks)__naked
{
    (void) ticks;
// @formatter:off
__asm
    ldw     x,#_secTimer
    jra     waitcommon
__endasm;
// @formatter:on
}
#endif
#endif

#endif
