//
// Created by Vladimir Schneider on 2018-02-01.
//

#include "multitasker.h"

extern const uint16_t queueDescriptionSize;
extern const uint16_t taskDescriptionSize;
extern const uint16_t semaDescriptionSize;
extern const uint16_t eventDescriptionSize;
extern const uint16_t mutexDescriptionSize;

extern void _InitTaskDescInX();
extern void _InitSemaInXA();
extern void _InitMutexX();

// all init descriptions are of this form
typedef struct _InitDescription {
    void *data;
    const uint16_t size; // description structure size, only low byte is used, int16 to keep structure size same as InitDescription
} _InitDescription;

const uint16_t _initDescriptionSize = sizeof(_InitDescription);

void (*_initFunc)(); // temp init function
uint16_t _initSize; // temp init size

/*
 * NOTE: description, count order for various options shold be in the same order
 * as the calls to InitFunc here because it is assumed and the pointer to next description
 * is advanced by __InitFunc to reduce code size
 */
void InitMultiTasker(Descriptions *descriptions) __naked
{
    (void)descriptions;
// @formatter:off
#ifdef OPT_QUEUES
__asm
    ldw   x,#initqnode
    ldw   y,_queueDescriptionSize
    callr  initFunc
__endasm;
#endif // OPT_QUEUES

#ifdef OPT_TASKS
__asm
    clrw    x
    ldw     _currentTask,x
    ldw     x,#_readyTasks
    call    __InitQNodeInX

    ldw   x,#initqtask
    ldw   y,_taskDescriptionSize
    callr  initFunc
__endasm;
#endif

#ifdef OPT_SEMAPHORES
__asm
    ldw   x,#initqsema
    ldw   y,_semaDescriptionSize
    callr  initFunc
__endasm;
#endif

#ifdef OPT_EVENTS
__asm
    ldw   x,#initqnode
    ldw   y,_eventDescriptionSize
    callr  initFunc
__endasm;
#endif

#ifdef OPT_MUTEXES
__asm
    ldw   x,#initqmutex
    ldw   y,_mutexDescriptionSize
    callr  initFunc
__endasm;
#endif

    // leave as last to do a jp to init timers
#ifdef OPT_TASKS
#ifdef OPT_TIMERS
__asm
    jp      _InitTimers
__endasm;
#endif
#endif

__asm
initqnode:
    ldw   x,(x) ; get queue pointer
    jp    __InitQNodeInX

initqsema:
    ld    a,(SD_INIT,x)
    ldw   x,(x) ; get queue pointer
    jp   __InitSemaInXA

initqtask:
    jp    __InitTaskDescInX

initqmutex:
    ldw     x,(x)
    jp  __InitMutexInX

// X - pointer to function to call for every instance of description
// Y - description structure size
// uses description from previous call on the stack
// increments description pointer on stack by _initDescriptionSize

initFunc:
    ldw     __initFunc,x   ; save func pointer to use
    ldw     __initSize,y   ; save structure size
    ldw     y,(5,sp) ; get description pointer followd by count sp: pch, pcl, ev.h, ev.l, cn.h, cn.l
    ldw     x,y
    ldw     x,(0,x) ; get pointer to desription

    ldw     y,(2,y) ; get count
    jreq    init.done

init.loop:
    pushw   y ; save count
    pushw   x ; push description argument
    call    [__initFunc]
    popw    x
    addw    x,__initSize

    popw    y ; get count
    decw    y
    jrne    init.loop

init.done:
    ldw     x,(5,sp) ; get description pointer followd by count sp: pch, pcl, ev.h, ev.l, cn.h, cn.l
    addw    x,__initDescriptionSize
    ldw     (5,sp),x ; advance it by 4
    ret

__endasm;

#if 0
// @formatter:off
__asm
    ldw     x,#__initDescriptions
    ldw     __initDescription,x  ; initialize pointer to information
    ldw     x,__initDescriptionCount
    ld      a,(1,x) ; get low byte of count
    jreq    initm.done;

initm.loop:
    ldw     x,__initDescription
    ldw     y,x
    ldw     x,(INIT_FUNC,x)
    ldw     y,(INIT_PCOUNT,y)
    push    a
    call    __InitFunc
    pop     a
    dec     a
    jrne   initm.loop

initm.done:
    ret
__endasm;
// @formatter:on
#endif
}
