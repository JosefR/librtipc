/**
 * measure enqueue latency for spscq
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <unistd.h>
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
#include "perfcounters.h"
#endif

#define SHMID_BARRIER 0
#define SHMID_ENQBARRIER 2
#define SHMID_QUEUE 2

#define ENQUEUER_CPU 0 
#define DEQUEUER_CPU 1

#define NUM_ITERATION 1002 // 2 warm up iteration to make sure all memory pages are present
#define NUM_MESSAGES 10010
#define NUM_SLOTS 10020
#define SLOT_DATA_SIZE 128

#define HISTOGRAM_STEP 400
#define HISTOGRAM_SIZE 31


struct message {
	volatile int pid;
	volatile long long cnt;
};

void wasteSomeTime(unsigned int num) 
{
	unsigned int i, k;
	for (i = 0; i < num; i++)
		for (k = 0; k < 1000; k++)
			hwfunctions_nop();
}


int main()
{
	int i, c, pID = 0, handle;
	struct barrier_d b;
	spscq_d_t q;
	long long cnt, d, enqueue_fails, dequeue_fails;
	unsigned long long t1, t2, dt, t_min, t_max, t_sum;
	int histogram[HISTOGRAM_SIZE];
	void *data;

	data = malloc(SLOT_DATA_SIZE);
	for (i = 0; i < HISTOGRAM_SIZE; i++)
		histogram[i] = 0;

#ifdef ARCH_X86
	/* change previledge level */
	iopl(3);
#endif

#ifdef ARCH_ARM
	apm_init(); /* init arm perfcounter */
#endif

	handle = rtipc_create_handle();

	if ((c = fork()) == 0) {
		pID = 1;
	}
	
	/* prevent swapping */
	osfunctions_mLockAll();

	if (pID == 0) {
		if (osfunctions_movetocpu(ENQUEUER_CPU) == -1)
			printf("Proc %d: cannot move to cpu %d\n", pID, ENQUEUER_CPU);
	} else {
		if (osfunctions_movetocpu(DEQUEUER_CPU) == -1)
			printf("Proc %d: cannot move to cpu %d\n", pID, DEQUEUER_CPU);
	}
		

	if (osfunctions_setMaxPriority() == -1)
		printf("Proc %d: cannot set priority\n", pID);

	/* init librtipc */
	if (rtipc_initialize(handle) == -1) {
		printf("Proc %d: rtipc_initialize() failed\n", pID);
		return 0;
	}
	
	/* get barrier */
	if (barrier_get(&b, SHMID_BARRIER, 2) == -1) {
		printf("Proc %d: barrier_get() failed\n", pID);
		return 0;
	}

	/* connect to the queue */
	if (spscq_get(&q, SHMID_QUEUE, SLOT_DATA_SIZE, NUM_SLOTS) == -1) {
		printf("Proc %d: spscq_get() failed\n", pID);
		return 0;
	}

	
	printf("Proc %d: running, pid: %d\n", pID, getpid());

	barrier_wait(&b);


	t_max = 0;
	t_min = 0xffffffffffffffff;
	t_sum = 0;

	
	hwfunctions_cli();
	d = 0;
	if (pID == 0) {
		enqueue_fails = 0;
		for (i = 0; i < NUM_ITERATION; i++) {
			barrier_wait(&b);
#ifdef ARCH_ARM
			apm_resetCCNTR();
			apm_startCCNTR();
#endif
			cnt = 0;
			while (cnt < NUM_MESSAGES) {
				((struct message *)data)->pid = pID;
				((struct message *)data)->cnt = cnt;
#ifdef ARCH_X86
				t1 = read_tsc();
#endif
#ifdef ARCH_ARM
				t1 = apm_fetchCCNTR();
#endif
				while (spscq_enqueue(&q, data) != 0)
					enqueue_fails++;
#ifdef ARCH_X86
				t2 = read_tsc();
#endif
#ifdef ARCH_ARM
				t2 = apm_fetchCCNTR();
#endif
				if (i > 1 && cnt >= 10) {
					d++;
					dt = t2 - t1;
					if (dt > t_max)
						t_max = dt;
					if (dt < t_min)
						t_min = dt;
					t_sum += dt;
					histogram[(((dt / HISTOGRAM_STEP) < HISTOGRAM_SIZE) && (dt > 0)) ? (dt / HISTOGRAM_STEP) : (HISTOGRAM_SIZE - 1)]++;
				}				
				cnt++;
			}
#ifdef ARCH_ARM
			apm_stopCCNTR();
			if (apm_fetchCCNTRovfl() == 1) {
				printf("cycle counter overflow\n");
				apm_resetCCNTRovfl();
			}
#endif
		}
	} else {
		dequeue_fails = 0;
		for (i = 0; i < NUM_ITERATION; i++) {
			barrier_wait(&b);
#ifdef ARCH_ARM
			apm_resetCCNTR();
			apm_startCCNTR();
#endif
			cnt = 0;
			while (cnt < NUM_MESSAGES) {
				((struct message *)data)->pid = pID;
				((struct message *)data)->cnt = cnt;
#ifdef ARCH_X86
				t1 = read_tsc();
#endif
#ifdef ARCH_ARM
				t1 = apm_fetchCCNTR();
#endif
				while (spscq_dequeue(&q, data) != 0)
					dequeue_fails++;
#ifdef ARCH_X86
				t2 = read_tsc();
#endif
#ifdef ARCH_ARM
				t2 = apm_fetchCCNTR();
#endif
				if (i > 1 && cnt >= 10) {
					d++;
					dt = t2 - t1;
					if (dt > t_max)
						t_max = dt;
					if (dt < t_min)
						t_min = dt;
					t_sum += dt;
					histogram[(((dt / HISTOGRAM_STEP) < HISTOGRAM_SIZE) && (dt > 0)) ? (dt / HISTOGRAM_STEP) : (HISTOGRAM_SIZE - 1)]++;
				}				
				cnt++;
			}
#ifdef ARCH_ARM
			apm_stopCCNTR();
			if (apm_fetchCCNTRovfl() == 1) {
				printf("cycle counter overflow\n");
				apm_resetCCNTRovfl();
			}
#endif
		}
		
	}
	hwfunctions_sti();

	barrier_wait(&b);

	if (pID == 0) {
		printf("Proc %d (enqueue): interations: %lld enqueue fails: %lld t_min: %lld t_max: %lld average %lld\n", pID, d, enqueue_fails, t_min, t_max, t_sum / d);
		printf("\nenqueue histogram \n range (cycles) \t\t abs. frequency\n");
		for (i = 0; i < HISTOGRAM_SIZE - 1; i++)
			printf("%d \t <= # < \t %d :\t %d\n", i * HISTOGRAM_STEP, (i + 1) * HISTOGRAM_STEP, histogram[i]);
		printf("%d \t <= # \t\t :\t %d\n", (HISTOGRAM_SIZE - 1) * HISTOGRAM_STEP, histogram[HISTOGRAM_SIZE - 1]);
	} else {
		printf("Proc %d (dequeue): iterations: %lld dequeue fails: %lld t_min: %lld t_max: %lld average %lld\n", pID, d, dequeue_fails, t_min, t_max, t_sum / d);
	}
	

	barrier_wait(&b);
	
	barrier_release(&b);
	spscq_release(&q);

	if (pID == 0)
		rtipc_finalize();

	printf("Proc %d: finished, pid: %d\n", pID, getpid());
	
	return 0;
}
