//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "signals.h"

void InitSignal(Signal *signal) __naked {
    (void)signal;
    __SYNONYM_FOR(InitQNode)
}

/*
 * Wait for next raise signal call
 */
void WaitSignal(Signal *signal) __naked {
    (void)signal;
// @formatter:off
__asm
    pushw y
    pushw x
    push a
    push cc
    rim
    ldw     y,(9,sp)  ; get signal

    ; wait for raise signal call
    ldw     x,_currentTask
    pushw   y ; signal
    pushw   x ; task
    call    _QNodeLinkTail
    addw    sp,#4  ; drop params

    clr    _currentTask
    clr    _currentTask+1

    jp      __IsrYield

__endasm;
// @formatter:on
}

/*
 * Raise signal and release all waiting tasks
 *
 * tasks are added to the head of the ready queue in FIFO order of their waiting calls
 */
void RaiseSignal(Signal *signal) __naked {
    (void)signal;
// @formatter:off
__asm
    push cc
    rim
raise.loop:
    ldw     x,(4,sp)  ; get signal
    ldw     y,x
    cpw     y,(QTAIL,x)
    jreq    release.done ;  no one is waiting

    ldw     y,(QTAIL,y) ; get last so when linked at head will result in FIFO order
    ldw     x,#_readyTasks
    pushw   x
    pushw   y
    call    _QNodeLinkHead
    addw    sp,#4  ; drop prev task
    jra     raise.loop

release.done:
    pop     cc
    ret
__endasm;
// @formatter:on
}

void _dummy()
{
}
