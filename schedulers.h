#ifndef __SCHEDULERS_H__
#define __SCHEDULERS_H__

#include "bthread_private.h"

void priority_scheduler();
void task_set_priority(int priority);
void lottery_scheduler();
void random_scheduler();


#endif
