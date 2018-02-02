# STM8-Multi-Tasker

## Cooperative/Preemptive Round Robin Scheduler for STM8

Stability: **Not Ready** **In Development**

Status: **Broken** **Do Not Use Yet**

Version: **0.0**

A tiny scheduler to allow threading with stack based context switching to be used on the STM8
micro processor. Implemented with [SDCC] in mixed C and assembler. Using CMake with
[JetBrains CLion IDE]. (TODO: Add Instructions on how to set it up to work reasonably well).

The core is written in assembler to reduce size. The [SDCC] compiler does not produce very
efficient code for the STM8. I opted out to optimize reusable code, then I can use C for specific
projects with less concern for space.

<!--
Minimal cooperative multi tasking is less than 500 bytes of code and 8 bytes of data, with
additional per task data penalty of 6 bytes + task's stack size.

With tick timer functionality to allow timed suspensions, less than 700 bytes of code and 16
bytes of data. It will also increase per task data use to 8 bytes + task's stack size.
 -->

With All features enabled, clocks in just over 1k of code, 34 bytes data + per task data use of
8 bytes + task's stack size.

#### Multi Threading Caveats

Any non re-entrant functions that are called with interrupts enabled will need to be protected
from having one task's call interrupted and then another task making the call from another
context. There are two ways of handling this:

* Guard these functions with a Mutex to prevent other tasks from entering until the last task
  leaves.

* If the location where the function/library stores its temp variables or global state is known
  and extra stack space available, then you can use the global state preserving option
  `OPT_PRESERVE_GLOBAL_STATE`, write two functions to save state to stack and restore state from
  stack. These will be called whenever there is a task switch. Use sparingly because the extra
  space on the stack will need to be allocated for every task and done on every task switch.

* Use a mix of above methods as needed.

* Use only cooperative scheduling and don't try to use shared, unlocked resources from ISRs

#### Library Limitations

* all assembly routines are written for default SDCC compilation of caller saving registers.
* Interrupts are disabled while in library calls. This will increased interrupt response
  latency.

On the plus side, I was using this scheduler design on a Z80 running at a whopping 1MHz. Having
a real context switcher to simplify logic and implementation was well worth the overhead.

## Usage Examples

* [ ] Cooperative
* [ ] Simple Timer
* [ ] Periodic Timer, adjusted
* [ ] Mutex around lib calls
* [ ] Preserve Global State
* [ ] Events and interrupts

## Features

* Circular linked list queues and nodes. Lists and nodes are interchangeable. An empty queue is
  one linked to itself, just like an unlinked node. Head of queue is the next pointer, tail is
  the previous pointer.

  * Initialize node
  * Unlink node
  * Link after next/previous node or head/tail of a list. Will unlink the node before linking it
    into a new list.
  * Test if node is unlinked or list is empty.

  All elements in the library are QLists or QNodes.

* Scheduler, implements round robin cooperative scheduling or optionally with preemptive
  capability when a task exceeds maximum time slice in timer ticks.

  * Yield to relinquish CPU, or on time slice max (if preemptive option is enabled)
  * Global state save/restore via user provided functions to push/pop global state on task
    switch

* Timers implement timed waits to resume a task after delay.

  * WaitTicks - suspend for up to 65535 timer ticks before resuming, tick period defined by
    timer ISR calls.
  * WaitTicksAdj - suspend for up to 65535 timer ticks before resuming. Adjusts requested ticks
    to compensate for scheduling delay incurred during last resume from a WaitTicks or
    WaitTicksAdj.
  * WaitMillis - suspend for up to 65535 ms, 65.535 secs
  * WaitSecs - suspend for up to 65535 s, 18hrs 12min 15sec before rescheduling

* Semaphores implement time allocation of limited resources. Semaphore starts off with initial
  count of available resources. Every acquisition decrements the count by one, release
  increments it by one. A task trying to acquire a semaphore will suspend until at least one
  resource is free.

  * Initialize semaphore passing in initial resource count
  * Acquire semaphore - get single count of resource or suspend until available
  * Release semaphore - release single count of resource

* Events implement multiple tasks waiting on event, with all tasks resumed when the event is
  raised.

  * Wait for event - suspend until the next event signal
  * Signal an event - resume all tasks waiting for the event

* Mutexes same as a single count semaphore but allows the owner task to call lock multiple times
  without suspending. Unlock must be called equal number of times that lock was called to fully
  relinquish the mutex. Max lock count is 256.

  * Lock mutex - get ownership of mutex, increment its lock count or suspend until mutex is
    free.
  * Unlock mutex - decrement lock count, if becomes unlocked then relinquish ownership.
    Ownership will be given to first task suspended by lock mutex request.

## Per Module Resource Requirements

| Module         | Functionality Provided                 | Data (bytes) | Code (bytes) | Per Instance Overhead (bytes)  |
|:---------------|:---------------------------------------|-------------:|-------------:|:-------------------------------|
| Initialization | required                               |            4 |          121 |                                |
| Queues         | Circular linked lists                  |            0 |          131 | per Node or List: 4            |
| Tasks          | Cooperative/Preemptive multi-tasking   |           10 |          186 | per Task: 6  + task stack size |
| Timers         | Timed wait, tick, millisec and seconds |           24 |          378 | per Task: 2                    |
| Semaphores     | Resource lock Acquire/Release          |            0 |           55 | per Semaphore: 5               |
| Events         | Multi task event synchronization       |            0 |           60 | per Event: 4                   |
| Mutexes        | Multi task lock synchronization        |            0 |          121 | per Mutex: 7                   |
| **All**        | All options enabled                    |       **38** |     **1052** |                                |

[JetBrains CLion IDE]: https://www.jetbrains.com/clion/?fromMenu
[SDCC]: http://sdcc.sourceforge.net
