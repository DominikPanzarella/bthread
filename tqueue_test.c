#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "tqueue.h"

int main(void)
{
    TQueue t = NULL;

    void* t1;
    void* t2;
    void* t3;
    void* t4;

    // Unsafe but ok for testing purposes
    t1 = (void*) 1;
    t2 = (void*) 2;
    t3 = (void*) 3;
    t4 = (void*) 4;

    assert(tqueue_size(t) == 0);
    assert(tqueue_enqueue(&t, t1) == 0);
    assert(tqueue_size(t) == 1);
    assert(tqueue_enqueue(&t, t2) == 1);
    assert(tqueue_size(t) == 2);
    assert(tqueue_enqueue(&t, t3) == 2);
    assert(tqueue_size(t) == 3);
    assert(tqueue_enqueue(&t, t4) == 3);
    assert(tqueue_size(t) == 4);

    int i;
    for (i=0; i<4; i++) {
        assert(tqueue_get_data(tqueue_at_offset(t, i)) == (void*) (intptr_t) (i+1));
    }

    for (i=1; i<5; i++) {
        assert(tqueue_pop(&t) == (void*) (intptr_t) i);
        assert(tqueue_size(t) == (4-i));
    }

    assert(tqueue_pop(&t) == NULL);

    assert(tqueue_size(t) == 0);
    assert(tqueue_enqueue(&t, t1) == 0);
    assert(tqueue_size(t) == 1);
    assert(tqueue_enqueue(&t, t2) == 1);
    assert(tqueue_size(t) == 2);

    assert(tqueue_pop(&t) == (void*) 1);
    assert(tqueue_size(t) == 1);

    assert(tqueue_enqueue(&t, t1) == 1);
    assert(tqueue_size(t) == 2);

    assert(tqueue_pop(&t) == (void*) 2);
    assert(tqueue_size(t) == 1);
    
    assert(tqueue_at_offset(t, 0) != NULL);
	assert(tqueue_at_offset(t, 1) == t);

    assert(tqueue_pop(&t) == (void*) 1);
    assert(tqueue_size(t) == 0);
    
    printf("All test passed\n");

    return 0;
}
