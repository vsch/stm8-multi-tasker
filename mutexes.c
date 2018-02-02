//
// Created by Vladimir Schneider on 2018-01-30.
//

#include "multitasker.h"

const uint16_t mutexDescriptionSize = sizeof(MutexDescription);

void _InitMutexInX(Mutex *mutex)__naked
{
    (void) mutex;
//    mutex->owner = 0;
//    QInitNode((QNode *) mutex);
// @formatter:off
__asm
    clrw    y
    ldw     (MUTEX_OWNER,x),y
    jp      __InitQNodeInX
__endasm;
// @formatter:on
}

void LockMutex(Mutex *mutex)__naked {
    (void) mutex;
// @formatter:off
__asm
    sim
    ldw     x,(3,sp)  ; get mutex, sp: pch, pcl, ev.h, ev.l

    ldw     y,x
    ldw     y,(MUTEX_OWNER,y)
    jrne    lockmutex.check     ; has owner, see if it is this owner

    ; no owner, this task becomes the owner
    ldw     y,_currentTask
    ldw     (MUTEX_OWNER,x),y
    clr     (MUTEX_LOCKS,x)
    jra     lockmutex.done

lockmutex.check:
    cpw     y,_currentTask
    jreq    lockmutex.owner

    ; x is already the mutex
    ldw     y,#__QNodeLinkTailInXY
    jp      __YieldToXatYStack

lockmutex.owner:
    ; increment count
    inc   (MUTEX_LOCKS,x)

    ; jrne  lockmutex.done
    ; problem now after next lock, the unlock will make mutex think it is unlocked

lockmutex.done:
    rim
    ret

__endasm;
// @formatter:on
}

void UnlockMutex(Mutex *mutex)__naked {
    (void) mutex;
// @formatter:off
__asm
    push cc
    sim
    ldw     x,(3,sp)  ; get mutex sp: pch, pcl, ev.h, ev.l
    ldw     y,x
    ldw     y,(MUTEX_OWNER,y)
    jreq    unlock.done      ; no owner
    cpw     y,_currentTask
    jrne    unlock.done      ; not the owner

    ; decrement lock count and if zero release it
    dec     (MUTEX_LOCKS,x)
    jrne    unlock.done ; not yet

    clrw    y
    ldw     (MUTEX_OWNER,x),y ; clear owner

    ; if there is a process waiting we will move it to ready queue and give it ownership
    ldw     y,x
    cpw     y,(QHEAD,x)
    jreq    unlock.done ;  no waiters

    ; transfer head to ready queue
    ldw     y,(QHEAD,y)
    ldw     (MUTEX_OWNER,x),y   ; give it ownership

    ldw     x,#_readyTasks
    call    __QNodeLinkTailInXY

unlock.done:
    pop     cc
    ret
__endasm;
// @formatter:on
}

