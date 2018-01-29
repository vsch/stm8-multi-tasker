//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "semaphores.h"

void InitSema(Sema *sema, uint8_t initialCount) {
    InitQNode((QNode *) sema);
    sema->count = initialCount;
}

void AcquireSema(Sema *sema) __naked {
    (void)sema;
// @formatter:off
__asm
    pushw y
    pushw x
    push a
    push cc
    rim
    ldw     y,(9,sp)  ; get sema

    ld      a,(SEMA_COUNT,y)
    jrne    acquire.done

    ; release this one
    ldw     x,_currentTask
    pushw   y
    pushw   x
    call    _QNodeLinkTail
    addw    sp,#4  ; drop params

    clr    _currentTask
    clr    _currentTask+1

    jp      __IsrYield

acquire.done:
    dec     a
    ld      (SEMA_COUNT,y),a
    iret

__endasm;
// @formatter:on
}

void ReleaseSema(Sema *sema) __naked {
    (void)sema;
// @formatter:off
__asm
    push cc
    rim
    ldw     y,(4,sp)  ; get sema

    ; if there is a process waiting we will move it to ready and not adjust count
    ldw     x,y
    cpw     y,(QHEAD,x)
    jreq    release.decr ;  nope, increment and done

    ldw     y,(QHEAD,y)
    ldw     x,#_readyTasks
    pushw   x
    pushw   y
    call    _QNodeLinkTail
    jra     release.done

release.decr:
    ld      a,(SEMA_COUNT,y)
    inc     a
    ld      (SEMA_COUNT,y),a
release.done:
    pop     cc
    ret
__endasm;
// @formatter:on
}

