#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "thelper.h"
#include "schedulers.h"

void priority_scheduler()
{
    double tms;
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* tp = tqueue_get_data(scheduler->current_item);
    /* if the bthread is still ready, consume a quantum */
    if (tp->state == __BTHREAD_READY) {
        tp->quantums = tp->quantums - 1;
        if (tp->quantums > 0) return;
    }
    /* ... not ready, or quantums expired */
    for(;;) {
        tms = get_current_time_millis();
        /* Get next bthread, round robin */
        scheduler->current_item = tqueue_at_offset(scheduler->current_item, 1);
        tp = (__bthread_private*) tqueue_get_data(scheduler->current_item);
        if (tp->state == __BTHREAD_SLEEPING) {
            if (tp->wake_up_time < tms) {
                tp->state = __BTHREAD_READY;
            }
        }
        if (tp->state == __BTHREAD_READY) {
            tp->quantums = tp->priority;
            break;
        }
    }
}

#define LOTTERY_TICKETS 1000

void lottery_scheduler()
{
    int tickets[LOTTERY_TICKETS];
    double tickets_count;
    int index, t_tickets, count, bthread_num;

    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* tp = tqueue_get_data(scheduler->current_item);

    TQueue tq;
    double tms;

    do {
        tickets_count = 0;
        memset(&tickets, -1, LOTTERY_TICKETS * sizeof(int));

        while(tickets_count == 0) {
            tq = scheduler->queue;
            do {
                tp = (__bthread_private*) tqueue_get_data(tq);
                tms = get_current_time_millis();
                if (tp->state == __BTHREAD_SLEEPING) {
                    if (tp->wake_up_time < tms) {
                        tp->state = __BTHREAD_READY;
                    }
                }
                if (tp->state == __BTHREAD_READY) {
                    tickets_count += (tp->priority + 1) * 100;
                }
                tq = tqueue_at_offset(tq, 1);

            } while(tq != scheduler->queue);
        }

        index = 0;

        tq = scheduler->queue;
        bthread_num = 0;
        do {
            tp = tqueue_get_data(tq);
            if (tp->state == __BTHREAD_READY) {
                t_tickets = ((tp->priority + 1) * 100) * LOTTERY_TICKETS / tickets_count;
                for (count = 0; count < t_tickets; count++, index++) {
                    tickets[index] = bthread_num;
                }
            }
            bthread_num++;
            tq = tqueue_at_offset(tq, 1);
        } while(tq != scheduler->queue);
        srand(time(NULL));
        int choosen_bthread;
        choosen_bthread = tickets[rand() % LOTTERY_TICKETS];
        if (choosen_bthread == -1) continue;
        scheduler->current_item = tqueue_at_offset(scheduler->queue, choosen_bthread);
    } while(((__bthread_private*) tqueue_get_data(scheduler->current_item))->state != __BTHREAD_READY);

    ((__bthread_private*) tqueue_get_data(scheduler->current_item))->quantums = 1;
}

void random_scheduler()
{
    int next;
    double tms;
    int bthreads_count;

    __bthread_scheduler_private* scheduler = bthread_get_scheduler();

    bthreads_count = tqueue_size(scheduler->queue);

    do {
        next = rand() % bthreads_count;
        scheduler->current_item = tqueue_at_offset(scheduler->queue, next);
        tms = get_current_time_millis();
        __bthread_private* current_thread = (__bthread_private*) tqueue_get_data(scheduler->current_item);
        if (current_thread->state == __BTHREAD_SLEEPING) {
            if (current_thread->wake_up_time < tms) {
                current_thread->state = __BTHREAD_READY;
            }
        }
    } while (((__bthread_private*) tqueue_get_data(scheduler->current_item))->state != __BTHREAD_READY);

    ((__bthread_private*) tqueue_get_data(scheduler->current_item))->quantums = 1;
}

void task_set_priority(int priority)
{
    __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* tp = tqueue_get_data(scheduler->current_item);
    tp->priority = priority;
}


