#include "bthread_private.h"
#include "thelper.h"
#include <stdarg.h>
#include <stdlib.h> // for printf


static void bthread_setup_timer();
static int bthread_check_if_zombie(bthread_t bthread, void **retval);
static TQueue bthread_get_queue_at(bthread_t bthread);

#define save_context(CONTEXT) sigsetjmp(CONTEXT, 1)

#define restore_context(CONTEXT) siglongjmp(CONTEXT, 1)

static TQueue bthread_get_queue_at(bthread_t bthread)
{
	volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
	volatile TQueue item = scheduler->queue;
	if (item == NULL) {
		return NULL;
	}
	do {
		__bthread_private* t = tqueue_get_data(item);
		if (t->tid == bthread) {
			return item;
		} else {
			item = tqueue_at_offset(item, 1);
		}
	} while (item != scheduler->queue);
	return NULL;
}

static void bthread_setup_timer()
{
    static bool initialized = false;

    if (!initialized) {
        // Note: Here we cannot use timer_create because it causes
        // a race condition in sigprocmask
        signal(SIGVTALRM, (void (*)()) bthread_yield);
        struct itimerval time;
        time.it_interval.tv_sec = 0;
        time.it_interval.tv_usec = QUANTUM_USEC;
        time.it_value.tv_sec = 0;
        time.it_value.tv_usec = QUANTUM_USEC;
        initialized = true;
        setitimer(ITIMER_VIRTUAL, &time, NULL);
    }
}

__bthread_scheduler_private* bthread_get_scheduler()
{
    static __bthread_scheduler_private* scheduler;
    if (scheduler == NULL) {
        scheduler = (__bthread_scheduler_private*)
                    malloc(sizeof(__bthread_scheduler_private));
        sigemptyset(&scheduler->timer_signal_mask);
        sigaddset(&scheduler->timer_signal_mask, SIGVTALRM);
        scheduler->queue = NULL;
        scheduler->scheduling_routine = NULL;
        scheduler->current_item = NULL;
        scheduler->current_tid = 0;
    }
    return scheduler;
}

int bthread_create(bthread_t *bthread, const bthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg)
{
	bthread_block_timer_signal();
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    volatile __bthread_private* bthread_priv = (__bthread_private*) malloc(sizeof(__bthread_private));
    bthread_priv->body = start_routine;
    bthread_priv->arg = arg;
    if (attr != NULL) {
        bthread_priv->attr = *attr;
    }
    bthread_priv->stack = NULL;
    bthread_priv->cancel_req = 0;
    bthread_priv->wake_up_time = 0;
    bthread_priv->tid = ++scheduler->current_tid;
    tqueue_enqueue(&scheduler->queue, bthread_priv);
    bthread_priv->state = __BTHREAD_READY;
    *bthread = bthread_priv->tid;
    trace("(CREATE) %lu\n", bthread_priv->tid);
    return 0;
}

int bthread_check_if_zombie(bthread_t bthread, void **retval)
{
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    if (!scheduler->queue) {
		return 1;
	} else {
		__bthread_private* thread = (__bthread_private*) tqueue_get_data(scheduler->current_item);		
		if (thread->tid == bthread && thread->state == __BTHREAD_ZOMBIE) {
			if (retval != NULL) {
				*retval = thread->retval;
			}
			trace("(DESTROY) %lu %p\n", thread->tid, thread->retval);
			TQueue head = bthread_get_queue_at(bthread);
			free(thread->stack);
			free(thread);
			tqueue_pop(&head);
			scheduler->queue = scheduler->current_item = head;
			return 1;
		} else {
			return 0;
		}
	}
}

int bthread_join(bthread_t bthread, void **retval)
{
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    scheduler->current_item = scheduler->queue;
	save_context(scheduler->context);
	trace("(INSCHEDULER)\n");
    if (bthread_check_if_zombie(bthread, retval)) return 0;
	bthread_setup_timer(); 
	__bthread_private* tp;
	trace("(RESCHEDULING)\n");
	do {
		if (scheduler->scheduling_routine != NULL) {
			scheduler->scheduling_routine();
		} else {
			scheduler->current_item = tqueue_at_offset(scheduler->current_item, 1);
		}
		tp = (__bthread_private*) tqueue_get_data(scheduler->current_item);
		double tms = get_current_time_millis();
		if (tp->state == __BTHREAD_SLEEPING) {
			if (tp->wake_up_time < tms) {
				trace("(READY) %lu\n", tp->tid);
				tp->state = __BTHREAD_READY;
			}
		}
	} while (tp->state != __BTHREAD_READY);
	trace("(SCHEDULING) %lu\n", tp->tid);
	// Restore context or setup stack and perform first call
	//trace("(SCHEDULE) %lu\n", tp->tid);
        if (tp->stack) {
			restore_context(tp->context);
        } else {
            tp->stack = (char*) malloc(sizeof(char) * STACK_SIZE);
            unsigned long target = (unsigned long) (tp->stack + STACK_SIZE - 1);
            #if __APPLE__ || __arm__ || __aarch64__
                        // Align the stack at 16 bytes
                            target -= (target % 16);
            #endif
            #if __aarch64__
                        // We need to store the value of tp in a register, and then restore it after
                            // changing the stack pointer
                            asm __volatile__("mov x1, %0" :: "r"(tp) : "x1");
                            asm __volatile__("mov sp, %0" :: "r"((uintptr_t) target) : "x1");
                            asm __volatile__("mov %0, x1" : "=r"(tp) );
            #elif __arm__
                        asm __volatile__("mov %%sp, %0" :: "r"((uintptr_t) target));
            #elif __x86_64__
                        asm __volatile__("movq %0, %%rsp" :: "r"((uintptr_t) target));
            #else
                        asm __volatile__("movl %0, %%esp" :: "r"((uintptr_t) target));
            #endif
		bthread_unblock_timer_signal();
		bthread_exit(tp->body(tp->arg));
		return -1; // This can't be
	}
}

void bthread_yield()
{
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    volatile __bthread_private* thread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    if (save_context(thread->context) == 0) {
		trace("(YIELD) %lu\n", thread->tid);
        restore_context(scheduler->context);
    }
    bthread_unblock_timer_signal();
}

void bthread_exit(void *retval)
{
	bthread_block_timer_signal();
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    volatile  __bthread_private* thread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    thread->retval = retval;
    thread->state = __BTHREAD_ZOMBIE;
    trace("(EXIT) %lu %p\n", thread->tid, retval);
    bthread_yield();
}

int bthread_cancel(bthread_t bthread)
{
    TQueue q = bthread_get_queue_at(bthread);
    if (q != NULL) {
		atomic_trace("(CANCEL) %lu\n", bthread);
		((__bthread_private*) tqueue_get_data(q))->cancel_req = 1;
		return 0;
	} else {
		return -1;
	}
}

void bthread_testcancel(void)
{
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    volatile __bthread_private* thread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    if (thread->cancel_req) {
		atomic_trace("(TEST_CANCEL) %lu\n", thread->tid);
        bthread_exit((void*) -1);
    }
}

void bthread_sleep(double ms)
{
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    volatile __bthread_private* thread = ((__bthread_private*) tqueue_get_data(scheduler->current_item));
    double tms = get_current_time_millis();
    thread->wake_up_time = tms + ms;
    thread->state = __BTHREAD_SLEEPING;
    trace("(SLEEPING) %lu\n", thread->tid);
    bthread_unblock_timer_signal();
    while(thread->state != __BTHREAD_READY) {
        bthread_yield();
    };
}

void bthread_set_global_scheduling_routine(bthread_scheduling_routine scheduling)
{
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    scheduler->scheduling_routine = scheduling;
}

void bthread_block_timer_signal()
{
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    sigprocmask(SIG_BLOCK, &scheduler->timer_signal_mask, NULL);
}

void bthread_unblock_timer_signal()
{
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    sigprocmask(SIG_UNBLOCK, &scheduler->timer_signal_mask, NULL);
}

void bthread_printf(const char* format, ...)
{
    bthread_block_timer_signal();
va_list va;
    va_start(va, format);

    int result = vsnprintf(NULL, 0, format, va);

    // Error checking
    if (result < 0)
        return;

    // Here result is the number of bytes we need to allocate (excluding terminator)
    char *string = malloc(result + 1);

    // Now do the actual formatting
    vsnprintf(string, result + 1, format, va); 
    printf(string);
    free(string);
    bthread_unblock_timer_signal();
}
