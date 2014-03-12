#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
#include <rtipc.h>
#include <sys/io.h>

/* architecture specific */
#include <arch.h>
#ifdef ARCH_X86
#include "rdtsc.h"
#endif 
#ifdef ARCH_ARM
#include "perfcounters.h"
#endif

#define SHMID_BARRIER 0
#define SHMID_SENSORBUFFER 1
#define BUFFER_SIZE 1024
#define NUM_BUFFER 5
#define NUM_ITERATION 100100

#define HISTOGRAM_STEP 5000
#define HISTOGRAM_SIZE 10

int main()
{
	int i, pID, handle, cpu;
	struct barrier_d b;
	sensorbuffer_d_t sb;
	long long it = 0, cnt;
	unsigned long long t1, t2, dt, t_min, t_max, t_sum;
	void *data;
	int histogram[HISTOGRAM_SIZE];
	int last_msgid;

	data = malloc(sizeof(BUFFER_SIZE));

	for (i = 0; i < HISTOGRAM_SIZE; i++) 
		histogram[i] = 0;

	handle = rtipc_create_handle();
	
	/* fork one process */
	if (fork() == 0) { 
		pID = 1; /* child */
		cpu = 1; 
	} else {
		pID = 0;
		cpu = 0;
	}

#ifdef ARCH_X86
	/* change previledge level */
	iopl(3);
#endif
	osfunctions_mLockAll();

	if (osfunctions_setMaxPriority() == -1)
		printf("cannot set priority\n");
			
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
	}


	t_max = 0;
	t_min = 0xffffffffffffffff;
	t_sum = 0;

	barrier_wait(&b);

	if (pID == 0) {
		cnt = 0;	
		while (cnt < NUM_ITERATION) {
			*((int *)data) = i;
#ifdef ARCH_X86
			t1 = read_tsc();
#endif
			sensorbuffer_update(&sb, data);
#ifdef ARCH_X86
			t2 = read_tsc();
#endif
			if (cnt >= 100) {
				it++;
				dt = t2 - t1;
				if(dt > t_max)
					t_max = dt;
				if(dt < t_min)
					t_min = dt;
				t_sum += dt;
				histogram[(((dt / HISTOGRAM_STEP) < HISTOGRAM_SIZE) && (dt > 0)) ? (dt / HISTOGRAM_STEP) : (HISTOGRAM_SIZE - 1)]++;
			}
			/*printf("wrote %d\n", *((int *)data));*/
			cnt++;
		}
	} else {
		int msgid = 0, read;
		cnt = 0;
		last_msgid = 0;
		while (cnt < NUM_ITERATION) {
			read = sensorbuffer_read(&sb, data);
			if (read == -1)  /* retry if no buffer was written yet */
				continue;
/*			printf("read buffer: %d\n", *((int *)data));*/
			if ((msgid = *((int *)data)) < last_msgid)
				printf("Read old message!\n");
			else
				last_msgid = msgid;
			cnt++;
		}
	}

	barrier_wait(&b);

	if (pID == 0 && it > 0) {
		printf("Proc %d: min: %llu, max: %llu, avrg: %llu, num interation: %lld\n", pID, t_min, t_max, t_sum / it, it);
		printf("Proc %d: histogram \n range (cycles) \t\t abs. frequency\n", pID);
		for (i = 0; i < HISTOGRAM_SIZE - 1; i++)
			printf("%d \t <= # < \t %d :\t %d\n", i * HISTOGRAM_STEP, (i + 1) * HISTOGRAM_STEP, histogram[i]);
		printf("%d \t <= # \t\t\t :\t %d\n", (HISTOGRAM_SIZE - 1) * HISTOGRAM_STEP, histogram[HISTOGRAM_SIZE - 1]);
	}

	if (sensorbuffer_release(&sb) == -1)
		printf("Proc %d: sensorbuffer_release() failed\n", pID);
	if (barrier_release(&b) == -1)
		printf("Proc %d: barrier_release() failed\n", pID);
		
	/* stop librtipc */
	if (pID == 0)
		rtipc_finalize();
		
	printf("Proc %d: finished\n", pID);

	return 0;
}
