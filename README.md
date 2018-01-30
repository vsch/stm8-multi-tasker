# STM8-Multi-Tasker

## Preemptive/Cooperative Round Robin Scheduler for STM8

Stability: **Not Ready** **Work in Progress**

Status: **Broken** **Do Not Use Yet**

Version: **0.0**

A tiny multi-tasker to allow threads with Stack context switching to be used on the STM8 micro
processor.

Minimal functionality for cooperative multi tasking, no global state preservation: less than 256
bytes of code and 8 bytes of data, with additional per task data needed: 6 + task's stack size.

Minimal timer functionality of WaitTicks(uint16 ticks) for task suspension until number of clock
ticks, will add: 261 bytes of code and 8 bytes of data. It will also increase per task use to 8
\+ task's stack size.

With All features enabled will bite off 34 bytes of data and a bit under 1k of code + per task
use to 8 + task's stack size.

Implemented with [SDCC](http://sdcc.sourceforge.net) in mixed C and assembler. Using JetBrains
CLion environment. (TODO: Add Instructions on how to set it up to work reasonably well).

Most of the core is written in assembler to reduce size. The SDCC compiler is not very efficient
and I prefer to take time to optimize reusable code. C can then be used for specific projects
with less concern for space.

#### NOTE

Any non re-entrant functions that are called with interrupts enabled will need to be protected
from having one task's call interrupted and then another task making the call from another
context. There are two ways of handling this:

* Guard these functions with a Mutex to prevent another task from entering until the last task
  leaves.

* If the location where the function/library stores its temp variables or global state is known
  and extra stack space available, then you can use the global state preserving option
  `OPT_PRESERVE_GLOBAL_STATE`, write two functions to save state to stack and restore state from
  stack. These will be called whenever there is a task switch. Use sparingly because the extra
  space on the stack will need to be allocated for every task and done on every task switch.

Assembly routines are for default compilation with caller saving registers it wants preserved.

This library disables interrupts while manipulating structures. This will increased interrupt
latency.

| Module     | Functionality Provided                 | Data (bytes) | Code (bytes) | Per Instance Overhead (bytes)  |
|:-----------|:---------------------------------------|-------------:|-------------:|:-------------------------------|
| Queues     | Circular linked lists                  |            0 |          121 | per QNode or Queue: 4          |
| Tasks      | Cooperative/Preemptive multi-tasking   |            8 |          186 | per Task: 6  + task stack size |
| Timers     | Timed wait, tick, millisec and seconds |           26 |          438 | per Task: 2                    |
| Semaphores | Resource lock Acquire/Release          |            0 |           74 | per Semaphore: 5               |
| Events     | Multi task event synchronization       |            0 |           49 | per Event: 4                   |
| Mutexes    | Multi task lock synchronization        |            0 |          107 | per Mutex: 7                   |
| **All**    | All options loaded                     |       **34** |      **970** |                                |

## Usage Examples

* [ ] Cooperative
* [ ] Simple Timer
* [ ] Periodic Timer, adjusted
* [ ] Mutex around lib calls
* [ ] Preserve Global State
* [ ] Events and interrupts

## Features

* Circular linked list queues and nodes. Queues and nodes are interchangeable. An empty queue is
  one linked to itself, just like an unlinked node. Head of queue is the next pointer, tail is
  the previous pointer.

  * `void QInit(QNode *node)` - initializes the node/queue in X to unlinked/empty
  * `void QUnlink(QNode *node)` - unlink node
  * `uint8_t QTest(QNode *node)` - test node/queue for being empty, returns 0 if empty
  * `void QNodeLinkPrev(QNode *node, QNode *other)` - link other before node
  * `void QNodeLinkNext(QNode *node, QNode *other)` - link other after node
  * `void QNodeLinkTail(QNode *queue, QNode *node)` - same as link previous, links node to tail
    of queue
  * `void QNodeLinkHead(QNode *queue, QNode *node)` - same as link next, links node to head of
    queue

* Scheduler, implements round robin cooperative scheduling or optionally with preemptive
  capability when a task exceeds maximum time slice, configured in timer ticks.

  Each process has 9 bytes of overhead + its stack area size. Since each process is a QNode it
  can be waiting for at most one Timer, Event or Semaphone, while waiting it is linked into the
  corresponding queue.

  * `void InitTasks()` - initialize scheduler data structures. Do this on reset before enabling
    interrupts or adding any processes to the scheduler.
  * `void InitTask(Task *task, void *taskSP, uint8_t taskPriority)` - initialize task data
    structure and queue it for execution.
  * `void Yield()` - switches out the context of the current task and resumes the next ready
    task.

* Semaphores implement waiting on limited resources. Each starts off with an initial count.
  Every acquisition will decrement the count, release will increment the count and allow any
  tasks waiting to acquire the resource to resume.

  * `void InitSema(Seam *sema, uint8_t initialCount)` - initialize semaphore and set avail
    resources
  * `void AcquireSema(Seam *sema)` - get a single count of the resource, or suspend until one
    becomes available
  * `void ReleaseSema(Seam *sema)` - return a single count of the resource. A task waiting for
    the resource will be scheduled to resume.

* Events implement multiple tasks waiting on event, with all tasks resumed when the event is
  raised.

  * `void InitEvent(Event= *event)` - initialize event
  * `void WaitEvent(Event= *event)` - suspend until the next raise event is called by another
    task
  * `void EventEvent(Event= *event)` - event event and schedule all waiting tasks.

* Timers module is optional, it implements timed waits and schedules a process to run when its
  requested delay has run out.

  * `void InitTimers()` - initialize timers module, call before setting up or enabling timer
    interrupts
  * `void WaitTicks(uint16_t ticks)` - wait for up to 65535 timer ticks before resuming.
  * `void WaitTicksAdj(uint16_t ticks)` - wait for up to 65535 timer ticks before resuming,
    actual ticks will be reduced by difference of (last task resume from timed wait) and (tick
    time of this call).
  * `void _TickTimerIsr()` - timer tick interrupt handler
  * `void WaitMillis(uint16_t millis)` - wait for up to 65535 ms, 65.535 secs
  * `void WaitSecs(uint16_t secs)` - wait for up to 65535 s, 18hrs 12min 15sec before
    rescheduling

* Mutexes same as a single count Semaphore but allows the owner task to call lock multiple times
  without suspending. Unlock must be called equal number of times that lock was called to fully
  relinquish the mutex. Max lock count is 256, after which there is a rollover.
