#include "tcondition.h"
#include "bthread_private.h"

int bthread_cond_init(bthread_cond_t* c, const bthread_condattr_t *attr)
{
    if (c == NULL) {
        c = (bthread_cond_t*) malloc(sizeof(bthread_cond_t));
    }
    c->waiting_list = NULL;
    return 0;
}

int bthread_cond_destroy(bthread_cond_t* c)
{
    assert(tqueue_size(c->waiting_list) == 0);
    return 0;
}

int bthread_cond_wait(bthread_cond_t* c, bthread_mutex_t* mutex)
{
    bthread_block_timer_signal();
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    volatile __bthread_private* bthread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    bthread->state = __BTHREAD_BLOCKED;
    trace("(CONDBLOCKED) %lu %p\n", bthread->tid, c);
    tqueue_enqueue(&c->waiting_list, bthread);
    if (mutex != NULL) {
        assert(mutex->owner != NULL);
        assert(mutex->owner == (__bthread_private*) tqueue_get_data(bthread_get_scheduler()->current_item));
        __bthread_private* unlock = tqueue_pop(&mutex->waiting_list);
        trace("(MUTRELEASE) %lu %p\n", ((__bthread_private*)mutex->owner)->tid, mutex);
        if (unlock != NULL) {
            mutex->owner = unlock;
            unlock->state = __BTHREAD_READY;
            trace("(READY) %lu\n", ((__bthread_private*)mutex->owner)->tid);
			trace("(MUTACQUIRE) %lu %p\n", ((__bthread_private*)mutex->owner)->tid, mutex);
        } else {
            mutex->owner = NULL;
        }
    }
    while(bthread->state != __BTHREAD_READY) {
        bthread_yield();
    };
    bthread_mutex_lock(mutex);
    return 0;
}

int bthread_cond_signal(bthread_cond_t* c)
{
    bthread_block_timer_signal();
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    volatile __bthread_private* bthread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    trace("(CONDSIGNAL) %lu %p\n", bthread->tid, c);
    __bthread_private* unlock = tqueue_pop(&c->waiting_list);
    if (unlock != NULL) {
		trace("(READY) %lu\n", unlock->tid);
        unlock->state = __BTHREAD_READY;
        bthread_yield();
        return 0;
    }
    return 0;
}

int bthread_cond_broadcast(bthread_cond_t* c)
{
    bthread_block_timer_signal();
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    volatile __bthread_private* bthread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    trace("(CONDBROADCAST) %lu %p\n", bthread->tid, c);    
    while(tqueue_size(c->waiting_list) > 0) {
        __bthread_private* unlock = tqueue_pop(&c->waiting_list);
        if (unlock != NULL) {
			trace("(READY) %lu\n", unlock->tid);
            unlock->state = __BTHREAD_READY;
        }
    }
    bthread_yield();
    return 0;
}
