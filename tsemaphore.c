#include "tsemaphore.h"
#include "bthread_private.h"

int bthread_sem_init(bthread_sem_t* m, int pshared, int value)
{
    if (m == NULL) {
        m = (bthread_sem_t*) malloc(sizeof(bthread_sem_t));
    }
    m->value = value;
    m->waiting_list = NULL;
    return 0;
}

int bthread_sem_destroy(bthread_sem_t* m)
{
    assert(m->value >= 0);
    assert(tqueue_size(m->waiting_list) == 0);
    return 0;
}

int bthread_sem_wait(bthread_sem_t* m)
{
    bthread_block_timer_signal();
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* bthread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    trace("(SEMWAIT) %lu %p %d\n", bthread->tid, m, m->value);
    if (m->value > 0) {
		m->value--;
		trace("(SEMACQUIRE) %lu %p %d\n", bthread->tid, m, m->value);
        bthread_unblock_timer_signal();
    } else {
		trace("(SEMBLOCKED) %lu\n", bthread->tid);
        bthread->state = __BTHREAD_BLOCKED;
        tqueue_enqueue(&m->waiting_list, bthread);
        while(bthread->state != __BTHREAD_READY) {
            bthread_yield();
        };
    }
    return 0;
}

int bthread_sem_post(bthread_sem_t* m)
{
    bthread_block_timer_signal();
    __bthread_private* unlock = tqueue_pop(&m->waiting_list);
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* bthread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    trace("(SEMPOST) %lu %p %d\n", bthread->tid, m, m->value);
    m->value++;
    if (unlock != NULL) {
        unlock->state = __BTHREAD_READY;
        trace("(READY) %lu\n", unlock->tid);
        m->value--;
        trace("(SEMACQUIRE) %lu %p %d\n", unlock->tid, m, m->value);
        bthread_yield();
        return 0;
    }
    bthread_unblock_timer_signal();
    return 0;
}


