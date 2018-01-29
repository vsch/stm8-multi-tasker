//
// Created by Vladimir Schneider on 2018-01-28.
//

#ifndef SDCC_TEST_HELPERS_H
#define SDCC_TEST_HELPERS_H

#define USE_MILLI_TIMERS
#define USE_SEC_TIMERS  // sec timer needs milli timer
#define USE_PREEMPT     // needs timers module, tasks have will have max ticks pre time slice before they are switched, otherwise cooperative, except for timed waits

#define MAIN_TICKS_PER_MILLI    10     // how many main timer tick's per millisecond, assume 100us resolution for tick timer
// task slice milli * MAIN_TICKS_PER_MILLI (defined in timers.h) should be less than 65536
#define TASK_SLICE_MILLI        20      // 20ms per task max before it is pre-empted

#define  __naked __naked
#define  __interrupt __interrupt
#define __SAVE_DISABLE_INT  __asm__("push cc\nrim\n");
#define __RESTORE_INT  __asm__("pop cc\n");
#define __RESTORE_INT_RET  __asm__("pop cc\nret\n");
#define __SYNONYM_FOR(func)   __asm__("jp _" #func);

#endif //SDCC_TEST_HELPERS_H
