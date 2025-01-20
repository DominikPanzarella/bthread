##CFLAGS = -ffixed-r8 -ffixed-r9 -ffixed-r10 -ffixed-r11
CFLAGS = 
CC = gcc -O0 -g -Wall -std=gnu11 $(CFLAGS) -Wno-discarded-qualifiers
CC_OBJ = $(CC) -c $(CFLAGS)
CORE_OBJ = tqueue.o schedulers.o thelper.o bthread.o tmutex.o tbarrier.o tsemaphore.o tcondition.o
LIBS = -lm
IPC_SOURCES = tmutex.c tbarrier.c tcondition.c tsemaphore.c
IPC_HEADERS = tmutex.h tbarrier.h tcondition.h tsemaphore.h

all: bthread.o bthread_test tqueue_test sleeping_barber philosophers philosophers_try readers_writers_3 readers_writers_1 readers_writers_2 producer_consumer producer_consumer_cond matrix_cube scheduling_test

tqueue.o: tqueue.c tqueue.h
	$(CC_OBJ) $< $(LIBS)
	
ipc.o: $(IPC_SOURCES) $(IPC_HEADERS)
	$(CC_OBJ) $(IPC_SOURCES) $(LIBS)
	
schedulers.o: schedulers.c schedulers.h bthread.h bthread_private.h bthread.o
	$(CC_OBJ) $< $(LIBS)	
	
thelper.o: thelper.c thelper.h
	$(CC_OBJ) $< $(LIBS)	
	
bthread.o: bthread.c bthread.h tqueue.o
	$(CC_OBJ) $< $(LIBS)

bthread_test: bthread_test.c $(CORE_OBJ)
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 
	
philosophers: philosophers.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 
	
philosophers_try: philosophers_try.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 	
	
producer_consumer: producer_consumer.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 
	
producer_consumer_cond: producer_consumer_cond.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 
	
readers_writers_1: readers_writers_1.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 
	
readers_writers_2: readers_writers_2.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 
	
readers_writers_3: readers_writers_3.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 
	
sleeping_barber: sleeping_barber.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ) 
	
matrix_cube: matrix_cube.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ)	

scheduling_test: scheduling_test.c $(CORE_OBJ) 
	$(CC) -o $@ $< $(LIBS) $(CORE_OBJ)	

tqueue_test: tqueue_test.c tqueue.o
	$(CC) -o $@ $(LIBS) $^ 
	
style:
	astyle -A8 *.c
	astyle -A8 *.h
	rm -f *.orig
	
clean:
	rm -rf bthread_test tqueue_test sleeping_barber philosophers philosophers_try readers_writers_3 producer_consumer producer_consumer_cond readers_writers_1 readers_writers_2 matrix_cube scheduling_test philo
	rm -rf *.o
	
	
