//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "multitasker.h"

const uint16_t eventDescriptionSize = sizeof(EventDescription);

void InitEvent(Event *event) __naked {
    (void)event;
    __SYNONYM_FOR(QInitNode)
}

/*
 * Wait for next raise event call
 */
void WaitEvent(Event *event) __naked {
    (void)event;
// @formatter:off
__asm
    sim
    ldw     x,(3,sp)  ; get event sp: pch, pcl, ev.h, ev.l
    ; put this one in the queue
    ldw     y,#__QNodeLinkTailInXY
    jp      __YieldToXatYStack
__endasm;
// @formatter:on
}

/*
 * Signal event and release all waiting tasks
 *
 * tasks are added to the head of the ready queue in FIFO order of their waiting calls
 */
void SignalEvent(Event *event) __naked {
    (void)event;
// @formatter:off
__asm
    ldw     y,#__QNodeLinkTailInXY
signal_event:
    push    cc
    sim

    ldw     x,(4,sp)  ; get event: sp: cc, pch, pcl, ev.h, ev.l
    sim
    ldw     __taskQueue,x
    ldw     __queueFunction,y
raise.loop:
    ldw     x,__taskQueue
    ldw     y,x
    cpw     y,(QTAIL,x)
    jreq    release.done ;  no one is waiting

    ldw     y,(QTAIL,y)
    ldw     x,#_readyTasks
    call    [__queueFunction] ; push to start of queue so gets faster rescheduling
    jra     raise.loop

release.done:
    pop     cc
    ret
__endasm;
// @formatter:on
}

#ifdef OPT_PRIORITY_EVENTS
/*
 * Signal event and release all waiting tasks to head of queue
 *
 * tasks are added to the head of the ready queue in LIFO order of their waiting calls
 */
void SignalPriorityEvent(Event *event)__naked {
    (void) event;
// @formatter:off
    __asm
    ldw     y,#__QNodeLinkHeadInXY
    jra     signal_event
__endasm;
// @formatter:on
}
#endif // OPT_PRIORITY_EVENTS

