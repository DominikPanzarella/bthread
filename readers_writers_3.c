#include <stdio.h>
#include <stdlib.h>
#include "bthread.h"
#include "tsemaphore.h"

bthread_sem_t arrival_order;
bthread_sem_t readers_count_sem;
bthread_sem_t db_sem;
int readers_count = 0;
int writers_count = 0;

void read_data()
{
    bthread_printf("Reading data\n");
}

void process_data()
{
    bthread_printf("Processing data\n");
}

void* reader(void* arg)
{
    for(;;) {
        bthread_sem_wait(&arrival_order);
        bthread_sem_wait(&readers_count_sem);
        readers_count += 1;
        if (readers_count == 1) {
            bthread_sem_wait(&db_sem);
        }
        bthread_printf("There are %d readers\n", readers_count);
        bthread_sem_post(&readers_count_sem);
        bthread_sem_post(&arrival_order);

        read_data();
        bthread_sem_wait(&readers_count_sem);
        readers_count -= 1;
        bthread_printf("There are %d readers\n", readers_count);
        if (readers_count == 0) {
            bthread_sem_post(&db_sem);
        }
        bthread_sem_post(&readers_count_sem);
        process_data();
    }
}

void produce_data()
{
    bthread_printf("Produce data\n");
}

void write_data()
{
    bthread_printf("Writing data\n");
    bthread_sleep(2000);
}

void* writer(void* arg)
{
    for(;;) {
        produce_data();

        bthread_sem_wait(&arrival_order);
        bthread_sem_wait(&db_sem);
        bthread_sem_post(&arrival_order);
        write_data();
        bthread_sem_post(&db_sem);

        bthread_sleep(1000);
    }
}

int main(int argc, char *argv[])
{
    bthread_sem_init(&db_sem, 0, 1);
    bthread_sem_init(&readers_count_sem, 0, 1);
    bthread_sem_init(&arrival_order, 0, 1);

    bthread_t readers[5];
    bthread_t writers[5];

    int i;
    for (i=0; i<5; i++) {
        bthread_create(&readers[i], NULL, reader, NULL);
        bthread_create(&writers[i], NULL, writer, NULL);
    }

    for (i=0; i<5; i++) {
        bthread_join(readers[i], NULL);
        bthread_join(writers[i], NULL);
    }

    bthread_sem_destroy(&arrival_order);
    bthread_sem_destroy(&readers_count_sem);
    bthread_sem_destroy(&db_sem);

    return 0;
}

