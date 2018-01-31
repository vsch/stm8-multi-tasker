//
// Created by Vladimir Schneider on 2018-01-30.
//

#include "mutexes.h"

void InitMutex(Mutex *mutex)__naked
{
    (void) mutex;
//    mutex->owner = 0;
//    InitQNode((QNode *) mutex);
// @formatter:off
__asm
    ldw     x,(3,sp) ; mutex
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
    push cc
    rim
    ldw     x,(4,sp)  ; get mutex

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

    ; put this one in the queue
    pop     cc
    popw    y ; get return address
    callf   lockmutex.far
lockmutex.far:
    ; here the stack looks like the following was done: push pcl, push pch, push pce
    ldw    (1,sp),y ; overwrite return with callers address

    ; simulate an interrupt stack
    pushw y
    pushw x
    push a
    push cc
    rim
    ; x is already the mutex
    ldw     y,#__QNodeLinkTailInXY
    jp      __IsrYieldToXatY

lockmutex.owner:
    ; increment count
    inc   (MUTEX_LOCKS,x)

    ; jrne  lockmutex.done
    ; bad condition, mutex lock overflow, hope for the best

lockmutex.done:
    pop     cc
    ret

__endasm;
// @formatter:on
}

void UnlockMutex(Mutex *mutex)__naked {
    (void) mutex;
// @formatter:off
__asm
    push cc
    rim
    ldw     x,(4,sp)  ; get mutex
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

