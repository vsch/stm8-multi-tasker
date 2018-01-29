cmake_minimum_required(VERSION 3.9)
project(stm8-multi-tasker C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_SYSTEM_NAME Generic)

add_executable(stm8-multi-tasker main.c queues.h queues.c helpers.h tasks.h tasks.c timers.c timers.h semaphores.c semaphores.h signals.c signals.h)
#include_directories("/usr/local/share/sdcc/include")
