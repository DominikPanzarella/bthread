#ifndef __BTHREAD_H__
#define __BTHREAD_H__

// Types
typedef void *(*bthread_routine) (void *);
typedef unsigned long int bthread_t;
// BThread scheduling routine (must update scheduler->current_item!)
typedef void (*bthread_scheduling_routine)();

typedef struct {
    // FUTURE PROOF :)
} bthread_attr_t;

int bthread_create(bthread_t *bthread, const bthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);

int bthread_join(bthread_t bthread, void **retval);

void bthread_yield();

void bthread_exit(void *retval);

int bthread_cancel(bthread_t bthread);

void bthread_testcancel(void);

void bthread_sleep(double ms);

void bthread_set_global_scheduling_routine(bthread_scheduling_routine scheduler);

void bthread_printf(const char* format, ...);

#endif
