#include <stdio.h>
#include <stdlib.h>
#include "bthread.h"
#include "tmutex.h"
#include "tsemaphore.h"
#include <stdbool.h>

#define CHAIRS 3

typedef enum BarberState { WORKING, SLEEPING } BarberState;
BarberState state = SLEEPING;
int customers_count = 0;
bthread_mutex_t customers_count_state_mutex;
bthread_sem_t customers_semaphore;
bthread_sem_t barber_semaphore;

void do_hairs()
{
    bthread_printf("Cutting... cutting... cutting...\n");
}

void get_hairdo()
{
    bthread_printf(":)\n");
}

void* barber(void* arg)
{
    for(;;) {
        bthread_mutex_lock(&customers_count_state_mutex);
        if (customers_count > 0) {
            state = WORKING;
            bthread_sem_post(&customers_semaphore);
            customers_count -= 1;
            bthread_mutex_unlock(&customers_count_state_mutex);
            do_hairs();
        } else {
            state = SLEEPING;
            bthread_mutex_unlock(&customers_count_state_mutex);
            bthread_printf("%s - Going to sleep\n", (char*) arg);
            bthread_sem_wait(&barber_semaphore);
            bthread_printf("%s - Yaaws!\n", (char*) arg);
        }
    }
}

void* customer(void* arg)
{
    for(;;) {
        bthread_mutex_lock(&customers_count_state_mutex);
        if (state == SLEEPING) {
            bthread_sem_post(&barber_semaphore);
            bthread_printf("%s - Wake up barber!\n", (char*) arg);
        }
        if (customers_count < CHAIRS) {
            customers_count += 1;
            bthread_mutex_unlock(&customers_count_state_mutex);
            bthread_sem_wait(&customers_semaphore);
            get_hairdo();
        } else {
            bthread_mutex_unlock(&customers_count_state_mutex);
            bthread_printf("%s - No chairs, I'm going home...\n", (char*) arg);
        }
        bthread_sleep(1500);
    }
}

int main(int argc, char *argv[])
{
    bthread_t b, c1, c2;

    bthread_mutex_init(&customers_count_state_mutex, NULL);
    bthread_sem_init(&customers_semaphore, 0, 0);
    bthread_sem_init(&barber_semaphore, 0, 0);

    bthread_create(&b, NULL, &barber, (void*) "Figaro");
    bthread_create(&c1, NULL, &customer, (void*) "Count Almaviva");
    bthread_create(&c2, NULL, &customer, (void*) "Rosina");

    bthread_join(b, NULL);
    bthread_join(c1, NULL);
    bthread_join(c2, NULL);

    bthread_sem_destroy(&barber_semaphore);
    bthread_sem_destroy(&customers_semaphore);
    bthread_mutex_destroy(&customers_count_state_mutex);
    printf("Exiting main\n");

    return 0;
}





