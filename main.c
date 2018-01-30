#include "stdint.h"
#include "tasks.h"
#include "timers.h"
#include "semaphores.h"
#include "events.h"
#include "mutexes.h"

#define CLK_DIVR	(*(volatile uint8_t *)0x50c6)
#define CLK_PCKENR1	(*(volatile uint8_t *)0x50c7)

#define TIM1_CR1	(*(volatile uint8_t *)0x5250)
#define TIM1_CNTRH	(*(volatile uint8_t *)0x525e)
#define TIM1_CNTRL	(*(volatile uint8_t *)0x525f)
#define TIM1_PSCRH	(*(volatile uint8_t *)0x5260)
#define TIM1_PSCRL	(*(volatile uint8_t *)0x5261)

#define PB_ODR	(*(volatile uint8_t *)0x5005)
#define PB_DDR	(*(volatile uint8_t *)0x5007)
#define PB_CR1	(*(volatile uint8_t *)0x5008)

#define PIN_NUM 4

unsigned int clock(void)
{
    unsigned char h = TIM1_CNTRH;
    unsigned char l = TIM1_CNTRL;
    return((unsigned int)(h) << 8 | l);
}

QNode timer;

#ifdef OPT_PRESERVE_GLOBAL_STATE

uint8_t globalVar1;
uint16_t globalVar2;

uint16_t _globalStateReturn;
/*
 * SaveGlobalState/RestoreGlobalState examples
 *
 * this is the simplest form which is easy to create and hard to maintain,
 * any way to get state to from SP is fine
 *
 * If you have a block of memory to be saved it will be more efficient to copy the
 * whole block to the stack instead pushing/popping one byte/word at a time
 */
void SaveGlobalState() __naked {
// @formatter:off
__asm
    popw    x
    ldw     __globalStateReturn,x

    ; now just push global vars to stack
    ld      a,_globalVar1
    push    a
    ldw     x,_globalVar2
    pushw   x

    jp     [__globalStateReturn] ; do a return
__endasm;
// @formatter:on
}

void RestoreGlobalState() __naked {
// @formatter:off
__asm
    popw    x
    ldw     __globalStateReturn,x

    ; now just pop global vars from stack
    pop     a
    ld      _globalVar1,a
    popw    x
    ldw     _globalVar2,x

    jp     [__globalStateReturn] ; do a return
__endasm;
// @formatter:on
}

#endif

Mutex mutex;

void main(void)
{
    mutex.owner = 0;
    mutex.prev = (Task *)&mutex;
    mutex.next = (Task *)&mutex;

    InitMutex(&mutex);

    InitQNode(&timer);

    if (QNodeTest(&timer))
    {
        TIM1_CR1 ^= (1 << PIN_NUM);
    }

    CLK_DIVR = 0x00; // Set the frequency to 16 MHz

    // Configure timer
    // 1000 ticks per second
    TIM1_PSCRH = 0x3e;
    TIM1_PSCRL = 0x80;
    // Enable timer
    TIM1_CR1 = 0x01;

    PB_DDR = 0x20;
    PB_CR1 = 0x20;

    for(;;)
        PB_ODR = clock() % 1024 < 256 ? 0x20 : 0;
}

