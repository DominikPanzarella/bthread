#include <stdio.h>
#include <stdlib.h>
#include "bthread.h"
#include "tmutex.h"
#include "tsemaphore.h"

#define N 10

int stock = 0;
bthread_mutex_t stock_mutex;
bthread_sem_t items_in_stock;
bthread_sem_t space_in_stock;

void* producer(void* arg)
{
    for(;;) {
        bthread_sem_wait(&space_in_stock);
        bthread_mutex_lock(&stock_mutex);
        if (stock < N) {
            /* Produce an item */
            stock = stock + 1;
            bthread_printf("Producer: now there are %d items\n", stock);
        } else {
            bthread_printf("Producer: stock is full\n");
        }
        bthread_mutex_unlock(&stock_mutex);
        bthread_sem_post(&items_in_stock);
    }
}

void* consumer(void* arg)
{
    for(;;) {
        bthread_sem_wait(&items_in_stock);
        bthread_mutex_lock(&stock_mutex);
        if (stock > 0) {
            /* Consume an item */
            stock = stock - 1;
            bthread_printf("Consumer: now there are %d items\n", stock);
        } else {
            bthread_printf("Consumer: stock is empty\n");
        }
        bthread_mutex_unlock(&stock_mutex);
        bthread_sem_post(&space_in_stock);
    }

}

int main(int argc, char *argv[])
{
    bthread_mutex_init(&stock_mutex, NULL);
    bthread_sem_init(&items_in_stock, 0, 0);
    bthread_sem_init(&space_in_stock, 0, N);

    bthread_t producer1, consumer1, consumer2;

    bthread_create(&producer1, NULL, producer, NULL);
    bthread_create(&consumer1, NULL, consumer, NULL);
    bthread_create(&consumer2, NULL, consumer, NULL);

    bthread_join(producer1, NULL);
    bthread_join(consumer1, NULL);
    bthread_join(consumer2, NULL);

    bthread_sem_destroy(&items_in_stock);
    bthread_sem_destroy(&space_in_stock);
    bthread_mutex_destroy(&stock_mutex);
    printf("Exiting main\n");
    return 0;
}





