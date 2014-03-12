#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>

#include <rtipc.h>

#define SHMID_SENSORBUFFER 0
#define SHMID_BARRIER 1
#define BUFFER_SIZE 1024
#define NUM_BUFFER 5

#define NUM_ITERATION 10000


int main()
{
	int i, pID, handle, cpu;
	struct barrier_d b;
	sensorbuffer_d_t sb;
	void *data;

	data = malloc(sizeof(BUFFER_SIZE));

	handle = rtipc_create_handle();
	
	/* fork one process */
	if (fork() == 0) { 
		pID = 1; /* child */
		cpu = 1; 
	} else {
		pID = 0;
		cpu = 0;
	}
			
	/* set CPUs to run on */
	if (osfunctions_movetocpu(cpu) == -1)
		printf("Proc %d: cannot move to CPU %d\n", pID, cpu);

	/* initialize librtipc */
	rtipc_initialize(handle);

	/* get barrier */
	if (barrier_get(&b, SHMID_BARRIER, 2) == -1) {
		printf("Proc %d: barrier_get() failed\n", pID);
		return 0;
	}

	/* get to the buffer */
	if (sensorbuffer_get(&sb, SHMID_SENSORBUFFER, BUFFER_SIZE, NUM_BUFFER) == -1) {
		printf("Proc %d: sensorbuffer_get() failed\n", pID);
		return 0;
	} else {
		printf("Proc %d: sensorbuffer_get() successful\n", pID);
	}

	/* sync */
	barrier_wait(&b);
	
	if (pID == 0) {
		for (i = 0; i < NUM_ITERATION; i++) {
			*((int *)data) = i;
			sensorbuffer_update(&sb, data);
			printf("wrote %d\n", *((int *)data));
			usleep(100000);
		}
	}
	else {
		for (i = 0; i < NUM_ITERATION; i++) {
			while(sensorbuffer_read(&sb, data) == -1); /* repeat until first element was inserted to buffer */
			printf("read buffer: %d\n", *((int *)data));
			usleep(370000);
		}
	}

	/* sync */
	barrier_wait(&b);

	sensorbuffer_release(&sb);
	barrier_release(&b);

	/* stop librtipc */
	if (pID == 0)
		rtipc_finalize();
		
	printf("Proc %d: finished\n", pID);

	return 0;
}
