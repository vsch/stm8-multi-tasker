# STM8-Multi-Tasker
## Preemptive/Cooperative with Round Robin Scheduler

Stability: **Work in Progress**

Status: **Not Ready**

Version: **0.0**

A tiny multi-tasker to allow threads with Stack context switching to be used on the STM8 micro
processor. With All features enabled will take 30 bytes of data and 1k of code.

Implemented with [SDCC](http://sdcc.sourceforge.net) in mixed C and assembler. Using JetBrains
CLion environment. (TODO: Add Instructions on how to set it up to work reasonably well).

**NOTE**: You will need to wrap any calls to non re-entrant libraries with a resource lock so
that no other task will be able to enter the library if there is a task switch. Create a
semaphore by declaring `Semaphore sema;`. Initialize it in the initialization routine via
`InitSema(&sema)`. Before entering a library that need guarding `AcquireSema(&sema);` upon
return `ReleaseSema(&sema)`.

Assembly routines are for default compilation with caller saving registers it wants preserved.

This library disables interrupts while manipulating structures. This will increased interrupt
latency but does make it thread safe.


| Module     | Functionality Provided                 | Data (bytes) | Code (bytes) | Per Instance Overhead (bytes)  |
|:-----------|:---------------------------------------|-------------:|-------------:|:-------------------------------|
| Queues     | Circular linked lists                  |            0 |          168 | per QNode or Queue: 4          |
| Tasks      | Cooperative multi-tasking              |            4 |           89 | per Task: 9  + task stack size |
|            | Preemptive (needs tick timer)          |            6 |          141 |                                |
| Timers     | Timed wait, tick, millisec and seconds |           24 |          505 |                                |
|            | Timed wait, tick and millisec only     |           16 |          447 |                                |
|            | Timed wait, tick only                  |            8 |          389 |                                |
| Semaphores | Resource lock Acquire/Release          |            0 |           87 | per Semaphore: 5               |
| Signals    | Multi task event synchronization       |            0 |           61 | per Signal: 4                  |
| **All**    | All options loaded                     |       **30** |      **980** |                                |

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
  can be waiting for at most one Timer, Signal or Semaphone, while waiting it is linked into the
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

* Signals implement multiple tasks waiting on event, with all tasks resumed when the signal is
  raised.

  * `void InitSignal(Signal= *signal)` - initialize signal
  * `void WaitSignal(Signal= *signal)` - suspend until the next raise signal is called by
    another task
  * `void RaiseSignal(Signal= *signal)` - raise signal and schedule all waiting tasks.

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

