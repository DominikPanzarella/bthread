#ifndef __BTHREADPRIVATE_H__
#define __BTHREADPRIVATE_H__

#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "bthread.h"
#include "tqueue.h"

#define QUANTUM_USEC 100
#define STACK_SIZE 64000000

typedef enum { __BTHREAD_READY = 0, __BTHREAD_BLOCKED, __BTHREAD_SLEEPING, __BTHREAD_ZOMBIE } bthread_state;

typedef struct {
    bthread_t tid;
    bthread_routine body;
    void* arg;
    bthread_state state;
    bthread_attr_t attr;
    char* stack;
    jmp_buf context;
    void* retval;
    int cancel_req;
    double wake_up_time;
    /* Priority scheduling data */
    int priority;
    int quantums;
    double quantum_start;
    double quantum_use;
} __bthread_private;

typedef struct {
    TQueue queue;
    TQueue current_item;
    jmp_buf context;
    bthread_t current_tid;
    sigset_t timer_signal_mask;
    /* Custom scheduler */
    bthread_scheduling_routine scheduling_routine;
} __bthread_scheduler_private;

__bthread_scheduler_private* bthread_get_scheduler();

void bthread_cleanup();

void bthread_block_timer_signal();

void bthread_unblock_timer_signal();

#ifdef TRACING
#define trace(...) printf(__VA_ARGS__);
#define atomic_trace(...) bthread_block_timer_signal(); printf(__VA_ARGS__); bthread_unblock_timer_signal();
#else
#define trace(...)
#define atomic_trace(...)
#endif


#endif
