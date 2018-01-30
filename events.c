//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "events.h"

void InitEvent(Event *event) __naked {
    (void)event;
    __SYNONYM_FOR(InitQNode)
}

/*
 * Wait for next raise event call
 */
void WaitEvent(Event *event) __naked {
    (void)event;
// @formatter:off
__asm
    popw    x ; get return address
    callf   waitevent.far
waitevent.far:
    ; here the stack looks like the following was done: push pcl, push pch, push pce
    ldw    (1,sp),x ; overwrite return with callers address

    ; simulate an interrupt stack
    pushw y
    pushw x
    push a
    push cc
    rim
    ldw     x,(10,sp)  ; get event
    ; put this one in the queue
    jp      __IsrYieldToXTail
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
    push cc
    rim
raise.loop:
    ldw     x,(4,sp)  ; get event
    ldw     y,x
    cpw     y,(QTAIL,x)
    jreq    release.done ;  no one is waiting

    ldw     y,(QTAIL,y) ; get last so when linked at head will result in FIFO order
    ldw     x,#_readyTasks
    pushw   x
    pushw   y
    call    _QNodeLinkHead ; push to start of queue so gets faster rescheduling
    addw    sp,#4  ; drop prev task
    jra     raise.loop

release.done:
    pop     cc
    ret
__endasm;
// @formatter:on
}

