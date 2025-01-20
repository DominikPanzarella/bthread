#include "tmutex.h"
#include "bthread_private.h"

int bthread_mutex_init(bthread_mutex_t* m, const bthread_mutexattr_t *attr)
{
    if (m == NULL) {
        m = (bthread_mutex_t*) malloc(sizeof(bthread_mutex_t));
    }
    m->owner = NULL;
    m->waiting_list = NULL;
    return 0;
}

int bthread_mutex_destroy(bthread_mutex_t* m)
{
    assert(m->owner == NULL);
    assert(tqueue_size(m->waiting_list) == 0);
    return 0;
}

int bthread_mutex_lock(bthread_mutex_t* m)
{
    bthread_block_timer_signal();
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* bthread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    if (m->owner == NULL) {
		fflush(stderr);
		trace("(MUTACQUIRE) %lu %p\n", bthread->tid, (void*) m);
        m->owner = bthread;
        bthread_unblock_timer_signal();
    } else {
		trace("(MUTBLOCKED) %lu %p\n", bthread->tid, (void*) m);
        bthread->state = __BTHREAD_BLOCKED;
        tqueue_enqueue(&m->waiting_list, bthread);
        while(bthread->state != __BTHREAD_READY) {
            bthread_yield();
        };
        atomic_trace("(MUTACQUIRE) %lu %p\n", bthread->tid, m);
    }
    return 0;
}

int bthread_mutex_trylock(bthread_mutex_t* m)
{
    bthread_block_timer_signal();
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* bthread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    if (m->owner == NULL) {
		trace("(MUTACQUIRE) %lu %p\n", bthread->tid, m);
        m->owner = bthread;
        bthread_unblock_timer_signal();
    } else {
		trace("(MUTFAIL) %lu %p\n", bthread->tid, m);
        bthread_unblock_timer_signal();
        return -1;
    }
    return 0;
}

int bthread_mutex_unlock(bthread_mutex_t* m)
{
    bthread_block_timer_signal();
    assert(m->owner != NULL);
    assert(m->owner == tqueue_get_data(bthread_get_scheduler()->current_item));
    __bthread_private* unlock = tqueue_pop(&m->waiting_list);
    trace("(MUTRELEASE) %lu %p\n", ((__bthread_private*)m->owner)->tid, m);
    if (unlock != NULL) {
        m->owner = unlock;
        trace("(READY) %lu\n", ((__bthread_private*)m->owner)->tid);
        trace("(MUTACQUIRE) %lu %p\n", ((__bthread_private*)m->owner)->tid, m);
        unlock->state = __BTHREAD_READY;
        bthread_yield();
        return 0;
    } else {
        m->owner = NULL;
    }
    bthread_unblock_timer_signal();
    return 0;
}
