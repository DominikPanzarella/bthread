#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bthread.h"
#include "tmutex.h"
#include "tsemaphore.h"

#define N 5
#define LEFT(i) (i+N-1) % N
#define RIGHT(i) (i+1) % N
#define THINKING 0
#define HUNGRY 1
#define EATING 2

int state[N];
bthread_mutex_t mutex;
bthread_sem_t philosopher_semaphore[N];

void* philosopher (void* arg);
void take_forks(int i);
void put_forks(int i);
void test(int i);
void think(int i);
void eat(int i);

void think(int i)
{
    bthread_printf("Philosopher %d is thinking...\n", i);
    bthread_sleep(200);
}

void eat(int i)
{
    bthread_printf("Philosopher %d is eating...\n", i);
    bthread_sleep(300);
}

void* philosopher (void* arg)
{
    int i;
    i = (intptr_t) arg;
    while(1) {
        think(i);
        take_forks(i);
        eat(i);
        put_forks(i);
    }
    bthread_printf("\tPhilosopher %d dead\n", i);
}

void take_forks(int i)
{
    bthread_mutex_lock(&mutex);
    state[i] = HUNGRY;
    bthread_printf("\tPhilosopher %d is hungry\n", i);
    test(i);
    bthread_mutex_unlock(&mutex);
    bthread_sem_wait(&philosopher_semaphore[i]);
}

void put_forks(int i)
{
    bthread_mutex_lock(&mutex);
    state[i] = THINKING;
    test(LEFT(i));
    test(RIGHT(i));
    bthread_mutex_unlock(&mutex);
}

void test(int i)
{
    if (state[i] == HUNGRY
        && state[LEFT(i)] != EATING
        && state[RIGHT(i)] != EATING) {
        state[i] = EATING;
        bthread_printf("\tPhilosopher %d got forks\n", i);
        bthread_sem_post(&philosopher_semaphore[i]);
    }
}

int main(int argc, char *argv[])
{
    int j;
    for (j=0; j<N; j++) {
        bthread_sem_init(&philosopher_semaphore[j], 0, 0);
        state[j] = THINKING;
    }
    bthread_mutex_init(&mutex, NULL);

    bthread_t philosophers[N];
    int i;
    for (i=0; i<N; i++) {
        bthread_create(&philosophers[i], NULL, philosopher, (void*) (intptr_t) i);
    }

    for (i=0; i<N; i++) {
        bthread_join(philosophers[i], NULL);
    }

    for (j=0; j<N; j++) {
        bthread_sem_destroy(&philosopher_semaphore[j]);
    }

    printf("Exiting main\n");
    return 0;
}
