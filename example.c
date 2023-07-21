/*
 * WHAT THIS EXAMPLE DOES
 *
 * We create a pool of 16 threads and then add 40 tasks to the pool
 * (20 task1 functions and 20 task2 functions). task1 and task2
 * simply print which thread is running them.
 *
 * As soon as we add the tasks to the pool, the threads will run
 * them. It can happen that you see a single thread running all the
 * tasks (highly unlikely). It is up the OS to decide which thread
 * will run what. So it is not an error of the thread pool but
 * rather a decision of the OS.
 */

#ifdef __linux__
# ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
#endif
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "thpool.h"

typedef struct
{
  int id;
} workload;

void
task(void* arg)
{
  static __thread clock_t start;
  static __thread clock_t end;
  static __thread clock_t used;

  start = time(NULL);
  sleep(1+rand()%10);
  end = time(NULL);
  used = (end-start);
  fprintf(stdout, "\r<- Thread %u finished on item #%d in %d seconds.\r\n",
	 (unsigned)(intptr_t)pthread_self(),
	 (int)(intptr_t)arg, used);
}

 void
 task_via_struct(void* arg)
 {
   workload wl = *((workload*)arg);
   fprintf(stdout, "\rThread %u on item #%d\r\n",
	  (unsigned)(intptr_t)pthread_self(),
	  (int)(intptr_t)wl.id);
 }

int
main(void)
{

#define NUM_THREADS 12
#define NUM_TASKS   128

  srand(getpid() ^ time(NULL));
  fprintf(stdout, "\rMaking threadpool with %u threads\r\n", NUM_THREADS);
  threadpool thpool = thpool_init(NUM_THREADS);

  fprintf(stdout, "\rAdding %u tasks to threadpool\r\n", NUM_TASKS);
  for (int i = 1; i <= NUM_TASKS-1; i++) {
    usleep(1+rand()%2500000);
    thpool_add_work(thpool, task, (void*)(uintptr_t)i);
    fprintf(stdout, "\r-> Dispatched item %d\r\n", i);
  };

  //thpool_wait(thpool);

  fprintf(stdout, "\rAdding %u tasks to threadpool using struct\r\n", NUM_TASKS);
  workload* instructions[NUM_TASKS + 1];
  for (int i = 1; i <= NUM_TASKS + 1; i++) {
    instructions[i] = malloc(sizeof(workload));
    if (!instructions[i])
	abort();
    instructions[i]->id = i;
    thpool_add_work(thpool, task_via_struct, instructions[i]);
  };

  fprintf(stdout, "\r**** Now we wait ***\r\n");
  thpool_wait(thpool);

  fprintf(stdout, "\rKilling threadpool\r\n");
  thpool_destroy(thpool);

  for (int i = 1; i <= NUM_TASKS + 1; i++) {
    free(instructions[i]);
  }
  return 0;
}
