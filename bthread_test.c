#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "bthread.h"

#ifdef VERBOSE
#define debug(...) \
    bthread_block_timer_signal(); \
	printf(__VA_ARGS__); \
	bthread_unblock_timer_signal();
#else
#define debug(...)
#endif

void* bthread1(void* arg);
void* bthread2(void* arg);
void* bthread3(void* arg);
void* bthread4(void* arg);


int a, b, c;
bthread_t t1, t2, t3, t4[100];


void* bthread1(void* arg)
{
    bthread_printf("BThread1 Sleeping...\n");
    bthread_sleep(5000);
    bthread_printf("BThread1 Waking Up!\n");
    for(a=0; a<10; a++) {
        debug("BThread1: %s %d\n", (const char*) arg, a);
    }
    for(; a<100; a++) {
        debug("BThread1: %s %d\n", (const char*) arg, a);
    }
    bthread_printf("BThread1 Creating another thread\n");
    for (int k=0; k<100; ++k) {
		bthread_create(&t4[k], NULL, &bthread4, (void*) "Delta");
	}
    bthread_printf("BThread1 exiting\n");
    return (void*)  13;
}

void* bthread2(void* arg)
{
    for(b=0; b<10000; b++) {
        debug("BThread2: %s %d\n", (const char*) arg, b);
        bthread_yield();
    }
    bthread_printf("BThread2 cancelling\n");
    bthread_testcancel();
    for(; b<10000; b++) {
        debug("BThread2: %s %d\n", (const char*) arg, b);
    }
    bthread_printf("BThread2 exiting\n");
    return (void*) 17;
}

void* bthread3(void* arg)
{
    for(c=0; c<1000; c++) {
        debug("BThread3: %s %d\n", (const char*) arg, c);
    }
    bthread_printf("BThread3 exiting\n");
    bthread_cancel(t2);
    bthread_exit((void*)42);
    for(c=0; c<1000; c++) {
        bthread_printf("BThread3: %s %d\n", (const char*) arg, c);
    }
    return (void*)  77;
}

void* bthread4(void* arg)
{
    for(volatile int d=0; d<20; d++) {
        write(0,".", 1);
    }
    bthread_printf("BThread4 exiting\n");
    return (void*)  77;
}


int main(int argc, char *argv[])
{
    int *r1, *r2, *r3;
    uintptr_t rv1, rv2, rv3;
    
    for (volatile int i=0; i<500; ++i) {
		printf("Creating Alpha thread...\n");
		bthread_create(&t1, NULL, &bthread1, (void*) "Alpha");
		printf("Creating Beta thread...\n");
		bthread_create(&t2, NULL, &bthread2, (void*) "Beta");
		printf("Creating Gamma thread...\n");
		bthread_create(&t3, NULL, &bthread3, (void*) "Gamma");

		printf("Joining BThread 1\n");
		bthread_join(t1, (void **)(&r1));
		printf("Joining BThread 2\n");
		bthread_join(t2, (void **)(&r2));
		printf("Joining BThread 3\n");
		bthread_join(t3, (void **)(&r3));
		printf("Joining BThread 4\n");
		for (int k=0; k<100; ++k) {
			bthread_join(t4[k], NULL);
		}

		rv1 = (uintptr_t)r1;
		rv2 = (uintptr_t)r2;
		rv3 = (uintptr_t)r3;

		printf("End of run: %ld=%d,%d  %ld=%d,%d  %ld=%d,%d\n", t1, a, (int) rv1, t2, b, (int) rv2, t3, c, (int) rv3);
	}	
    
    return 0;
}
