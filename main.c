#include "stdint.h"
#include "multitasker.h"

#define CLK_DIVR    (*(volatile uint8_t *)0x50c6)
#define CLK_PCKENR1    (*(volatile uint8_t *)0x50c7)

#define TIM1_CR1    (*(volatile uint8_t *)0x5250)
#define TIM1_CNTRH    (*(volatile uint8_t *)0x525e)
#define TIM1_CNTRL    (*(volatile uint8_t *)0x525f)
#define TIM1_PSCRH    (*(volatile uint8_t *)0x5260)
#define TIM1_PSCRL    (*(volatile uint8_t *)0x5261)

#define PB_ODR    (*(volatile uint8_t *)0x5005)
#define PB_DDR    (*(volatile uint8_t *)0x5007)
#define PB_CR1    (*(volatile uint8_t *)0x5008)

#define PIN_NUM 4

unsigned int clock(void) {
    unsigned char h = TIM1_CNTRH;
    unsigned char l = TIM1_CNTRL;
    return ((unsigned int) (h) << 8 | l);
}

Mutex mutex;
QNode timer;
Sema sema;
Event isrEvent;

extern void syncedToISR();
uint8_t syncedToISR_SP[64]; // syncedISR stack of 64 bytes
Task syncToISR_Task;

uint8_t ledBlink_SP[64];
Task ledBlink_Task;

/*
 * An example of a task waiting for an interrupt without
 * polling a port or having the work done in the ISR
 *
 * This one will schedule the waiting task at the head of the list
 * and preempt the currently running taks
 */

void timerISR()__interrupt(0)
{
// @formatter:off
__asm
    jp  __TickTimerIsr
__endasm;
// @formatter:on
}

void eventISR()__interrupt(1)
{
    SignalPriorityEvent(&isrEvent);
    // can yield right away to preempt running task, and schedule the released tasks by the priority event
    YieldFromISR();
}

void syncedToISR() {
    for (;;) {
        // do some work
        WaitEvent(&isrEvent);
        // do some more
    }
}

void ledBlink() {
    for (;;) {
        PB_ODR = clock() % 1024 < 256 ? 0x20 : 0;
        Yield();
    }
}

const QueueDescription queueDescription[] = {
        {.node = &timer,},
};

const TaskDescription taskDescription[] = {
        {.task = &syncToISR_Task, .pc = &syncedToISR, .sp = syncedToISR_SP, .stackSize = sizeof(syncedToISR_SP)},
        {.task= &ledBlink_Task, .pc = &ledBlink, .sp = ledBlink_SP, .stackSize = sizeof(ledBlink_SP)},
};

const SemaDescription semaDescription[] = {
        {.sema = &sema, .initialCount = 1,},
};

const EventDescription eventDescription[] = {
        {.event   = &isrEvent,},
};

const MutexDescription mutexDescription[] = {
        {.mutex = &mutex,},
};

const Descriptions descriptions = {
        .queues = queueDescription, .queuesCount = descriptionCount(queueDescription),
        .tasks = taskDescription, .tasksCount = descriptionCount(taskDescription),
        .semas = semaDescription, .semasCount = descriptionCount(semaDescription),
        .events = eventDescription, .eventsCount = descriptionCount(eventDescription),
        .mutexes = mutexDescription, .mutexesCount = descriptionCount(mutexDescription),
};

void main(void) {
    // init and queue up tasks
    InitMultiTasker(&descriptions);

    TIM1_CR1 ^= (1 << PIN_NUM);

    CLK_DIVR = 0x00; // Set the frequency to 16 MHz

    // Configure timer
    // 1000 ticks per second
    TIM1_PSCRH = 0x3e;
    TIM1_PSCRL = 0x80;

    // Enable timer
    TIM1_CR1 = 0x01;

    PB_DDR = 0x20;
    PB_CR1 = 0x20;

    Yield(); // start scheduler, never returns since there is not currently active task.
}

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
void SaveGlobalState()__naked {
// @formatter:off
    __asm
    popw    x
    ldw __globalStateReturn, x
    ld a, _globalVar1
    push a
    ldw x, _globalVar2
    pushw x
    jp[__globalStateReturn]
    __endasm;
// @formatter:on
}

void RestoreGlobalState()__naked {
// @formatter:off
    __asm
    popw    x
    ldw __globalStateReturn, x
    pop a
    ld _globalVar1, a
    popw x
    ldw _globalVar2, x
    jp[__globalStateReturn]
    __endasm;
// @formatter:on
}

#endif

