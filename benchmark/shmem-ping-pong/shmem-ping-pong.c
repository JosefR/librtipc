#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>

#include <rtipc.h>

/* architecture specific */
#include <arch.h>
#ifdef ARCH_X86
#include "rdtsc.h"
#endif 

#ifdef ARCH_ARM

#endif

#define SHMID_BARRIER 0
#define SHMID_SHMSEG 1 

#define NUM_ITERATION 10000100
#define HISTOGRAM_STEP 100
#define HISTOGRAM_SIZE 16


int main()
{
	int i, c, pID, handle;
	struct barrier_d b;
	void *mem;
	volatile int *ptr;
	long long cnt;
	unsigned long long t1, t2, dt, t_min, t_max, t_sum;
	int histogram[HISTOGRAM_SIZE];

#ifdef ARCH_X86
	/* change previledge level */
	iopl(3);
#endif

	handle = rtipc_create_handle();

	if ((c = fork()) == 0) {
		pID = 1;
	} else {
		pID = 0;
	}
			
	osfunctions_mLockAll();

	if (pID == 0) {
		if (osfunctions_movetocpu(0) == -1) 
			printf("PROC %d: cannot move to cpu %d\n", pID, 0);
	} else {
		if (osfunctions_movetocpu(1) == -1) 
			printf("PROC %d: cannot move to cpu %d\n", pID, 1);
	}
		

	if (osfunctions_setMaxPriority() == -1)
		printf("PROC %d: cannot set priority\n", pID);

	if (rtipc_initialize(handle) == -1) {
		printf("PROC %d: rtipc_initialize() failed\n", pID);
		return 0;
	}
	
	if (barrier_get(&b, SHMID_BARRIER, 2) == -1) {
		printf("PROC %d: barrier_get() failed\n", pID);
		return 0;
	}

	/* get the shared memory segment */
	if (shmman_get_shmseg(SHMID_SHMSEG, 1024, &mem)) {
		printf("PROC %d: shmman_getshm_seg() failed\n", pID);
		return 0;
	}
	ptr = (volatile int *)mem;
	*ptr = 0;
	*(ptr + 1) =  0;

	for (i = 0; i < HISTOGRAM_SIZE; i++) 
		histogram[i] = 0;

	t_max = 0;
	t_min = 0xffffffffffffffff;
	t_sum = 0;

	
	barrier_wait(&b);

#ifdef ARCH_X86
	hwfunctions_cli();
#endif


	if (pID == 0) { 
		cnt = 0;
		while(cnt < NUM_ITERATION) {
			*(ptr) =  cnt + 1;
#ifdef ARCH_X86
			t1 = read_tsc();
#endif
			while(*(ptr + 1) != cnt + 1)
				hwfunctions_nop();
#ifdef ARCH_X86
			t2 = read_tsc();
#endif			
			if (cnt >= 100) {
				dt = t2 - t1;
				if(dt > t_max)
					t_max = dt;
				if(dt < t_min)
					t_min = dt;
				t_sum += dt;

				histogram[(((dt / HISTOGRAM_STEP) < HISTOGRAM_SIZE) && (dt > 0)) ? (dt / HISTOGRAM_STEP) : (HISTOGRAM_SIZE - 1)]++;
			}

			cnt++;
		}
	} else {
		cnt = 0;
		while(cnt < NUM_ITERATION) {
			while(*(ptr) != cnt + 1)
				hwfunctions_nop();
			*(ptr + 1) =  cnt + 1;
			cnt++;
		}
	}
		
		
	barrier_wait(&b);

	if (pID == 0) {
		printf("min: %llu, max: %llu, avrg: %llu, num interation: %d\n", t_min, t_max, t_sum / (NUM_ITERATION - 100), NUM_ITERATION - 100);
		printf("\n histogram \n range (cycles) \t\t abs. frequency\n");
		for (i = 0; i < HISTOGRAM_SIZE - 1; i++)
			printf("%d \t <= # < \t %d :\t %d\n", i * HISTOGRAM_STEP, (i + 1) * HISTOGRAM_STEP, histogram[i]);
		printf("%d \t <= # \t\t :\t %d\n", (HISTOGRAM_SIZE - 1) * HISTOGRAM_STEP, histogram[HISTOGRAM_SIZE - 1]);
	}	

#ifdef ARCH_X86
	hwfunctions_sti();
#endif
		
	barrier_release(&b);
	shmman_release_shmseg(SHMID_SHMSEG);

	if (pID == 0)
		rtipc_finalize();


	return 0;
}
