//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "semaphores.h"

void InitSema(Sema *sema, uint8_t initialCount)__naked
{
    (void)sema;
    (void)initialCount;
//    sema->count = initialCount;
//    InitQNode((QNode *) sema);
// @formatter:off
__asm
    ld      a,(3,sp) ; initialCount
    ldw     x,(4,sp) ; sema
    ld      (SEMA_COUNT,x),a
    jp      __InitQNodeInX
__endasm;
// @formatter:on
}

void AcquireSema(Sema *sema)__naked {
    (void) sema;
// @formatter:off
__asm
    push cc
    rim
    ldw     x,(4,sp)  ; get sema

    ld      a,(SEMA_COUNT,x)
    jrne    acquire.done

    pop     cc
    popw    y ; get return address
    callf   acquire.far
acquire.far:
    ; here the stack looks like the following was done: push pcl, push pch, push pce
    ldw    (1,sp),y ; overwrite return with callers address

    ; simulate an interrupt stack
    pushw y
    pushw x
    push a
    push cc
    rim

    ; put this one in the queue
    ldw     y,#__QNodeLinkTailInXY
    jp      __IsrYieldToXatY

acquire.done:
    dec     a
    ld      (SEMA_COUNT,x),a
    pop     cc
    ret

__endasm;
// @formatter:on
}

void ReleaseSema(Sema *sema)__naked {
    (void) sema;
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
    call    __QNodeLinkTailInXY
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

