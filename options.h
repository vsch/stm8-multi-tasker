//
// Created by Vladimir Schneider on 2018-01-28.
//

#ifndef MULTI_TASKER_OPTIONS_H
#define MULTI_TASKER_OPTIONS_H


// Global State Save/Restore requires two user functions to implement pushing/poping global state variables
// the functions should be implemented in assembler and they should pop the return address first then do a return to that
// address after pushing/poping global state to the stack
//#define OPT_PRESERVE_GLOBAL_STATE
#define OPT_TICK_TIMER  //need this to reduce size of task overhead
//#define OPT_ADJUSTABLE_TICKS // define the wait ticks that adjusts for scheduling delay, needs tick timer
//#define OPT_MILLI_TIMER // use millisecond waits, needs tick timer
//#define OPT_SEC_TIMER  // sec timer needs milli timer
//#define OPT_PREEMPT     // needs timers module, tasks have will have max ticks pre time slice before they are switched, otherwise cooperative, except for timed waits
//#define OPT_PRIORITY  // not implemented

#define MAIN_TICKS_PER_MILLI    10     // how many main timer tick's per millisecond, default 100us resolution for tick timer
// task slice milli * MAIN_TICKS_PER_MILLI (defined in timers.h) should be less than 65536
#define TASK_SLICE_TICKS        50    // 5ms per task time slice before it is pre-empted

#define  __naked __naked
#define  __interrupt __interrupt
#define __SAVE_AND_DISABLE_INTS     __asm__("push cc\nrim\n");
#define __RESTORE_INTS              __asm__("pop cc\n");
#define __SYNONYM_FOR(func)         __asm__("jp _" #func);     // do a transfer, define function as __naked then add this in the body

#endif //MULTI_TASKER_OPTIONS_H
