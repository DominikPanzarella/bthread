#include "tbarrier.h"
#include "bthread_private.h"
#include <assert.h>


int bthread_barrier_init(bthread_barrier_t* b,
                         const bthread_barrierattr_t* attr,
                         unsigned count)
{
    assert(count > 0);
    if (b == NULL) {
        b = (bthread_barrier_t*) malloc(sizeof(bthread_barrier_t));
    }
    b->barrier_size = count;
    b->count = 0;
    b->waiting_list = NULL;
    return 0;
}

int bthread_barrier_destroy(bthread_barrier_t* b)
{
    assert(b->count == 0);
    assert(tqueue_size(b->waiting_list) == 0);
    return 0;
}

int bthread_barrier_wait(bthread_barrier_t* b)
{
    bthread_block_timer_signal();
    b->count++;
    __bthread_scheduler_private* s = bthread_get_scheduler();
    __bthread_private* bt = (__bthread_private*) tqueue_get_data(s->current_item);
    trace("(BARRIERWAIT) %lu %p %u %u\n", bt->tid, b, b->count, b->barrier_size);
    if (b->count == b->barrier_size) {
		trace("(BARRIERUNLOCK) %p %u %u\n", b, b->count, b->barrier_size);
        while(tqueue_size(b->waiting_list) > 0) {
            __bthread_private* unlock = tqueue_pop(&b->waiting_list);
            if (unlock != NULL) {
				trace("(READY) %lu\n", unlock->tid);
                unlock->state = __BTHREAD_READY;
            }
            b->count = 0;
        }
        bthread_unblock_timer_signal();
    } else {
        __bthread_scheduler_private* scheduler = bthread_get_scheduler();
        __bthread_private* bthread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
        trace("(BARRIERBLOCKED) %lu %p %u %u\n", bthread->tid, b, b->count, b->barrier_size);
        bthread->state = __BTHREAD_BLOCKED;
        tqueue_enqueue(&b->waiting_list, bthread);
        while(bthread->state != __BTHREAD_READY) {
            bthread_yield();
        };
    }
    return 0;
}

