//
// Created by Vladimir Schneider on 2018-01-28.
//

#include "multitasker.h"

const uint16_t semaDescriptionSize = sizeof(SemaDescription);

/*
 * X - Sema *sema,
 * A - uint8_t initialCount
*/
void _InitSemaInXA()__naked
{
//    sema->count = initialCount;
//    QInitNode((QNode *) sema);
// @formatter:off
__asm
    ld      (SEMA_COUNT,x),a
    jp      __InitQNodeInX
__endasm;
// @formatter:on
}

void AcquireSema(Sema *sema)__naked {
    (void) sema;
// @formatter:off
__asm
    sim
    ldw     x,(3,sp)  ; get sema sp: pc.h, pc.l, n.h, n.l

    ld      a,(SEMA_COUNT,x)
    jrne    acquire.done

    ; put this one in the queue
    ldw     y,#__QNodeLinkTailInXY
    jp      __YieldToXatYStack

acquire.done:
    dec     a
    ld      (SEMA_COUNT,x),a
    rim
    ret

__endasm;
// @formatter:on
}

void ReleaseSema(Sema *sema)__naked {
    (void) sema;
// @formatter:off
__asm
    push cc
    sim
    ldw     y,(3,sp)  ; get sema sp: pc.h, pc.l, n.h, n.l

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

