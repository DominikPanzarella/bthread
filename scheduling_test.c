#include "bthread.h"
#include "schedulers.h"

bthread_t alpha, beta, omega, ctrl;
unsigned long alpha_counter, beta_counter, omega_counter;

void* alpha_fn (void* args)
{
	task_set_priority(1);
	for(;;) {
		alpha_counter++;
		bthread_testcancel();
	}
	return NULL;
}

void* beta_fn (void* args)
{
	task_set_priority(10);
	for(;;) {
		beta_counter++;
		bthread_testcancel();
	}
	return NULL;
}

void* omega_fn (void* args)
{
	task_set_priority(100);
	for(;;) {
		omega_counter++;
		bthread_testcancel();
	}
	return NULL;
}

void* ctrl_fn (void* args)
{
	task_set_priority(10000);
	bthread_sleep(4000);
	bthread_cancel(alpha);
	bthread_cancel(beta);
	bthread_cancel(omega);
	return NULL;
}

int main(int argc, char *argv[])
{
	//bthread_set_global_scheduling_routine(random_scheduler);
	//bthread_set_global_scheduling_routine(lottery_scheduler);
	bthread_set_global_scheduling_routine(priority_scheduler);

	bthread_create(&alpha, NULL, &alpha_fn, NULL);
	bthread_create(&beta, NULL, &beta_fn, NULL);
	bthread_create(&omega, NULL, &omega_fn, NULL);
	bthread_create(&ctrl, NULL, &ctrl_fn, NULL);
	bthread_join(ctrl, NULL);
	printf("Joined controlling thread\n");
	bthread_join(alpha, NULL);
	printf("Joined alpha thread\n");
	bthread_join(beta, NULL);
	printf("Joined beta thread\n");
	bthread_join(omega, NULL);
	printf("Joined omega thread\n");

	printf("Alpha: %lu, Beta: %lu, Omega: %lu\n", alpha_counter, beta_counter, omega_counter);
	return 0;
}




  
