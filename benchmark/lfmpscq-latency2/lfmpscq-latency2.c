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
#endif

#define SHMID_BARRIER 0
#define SHMID_ENQBARRIER 1
#define SHMID_QUEUE 2


#define NUM_PROC 3
#define NUM_ITERATION 1000
#define NUM_MESSAGES 1010
#define NUM_SLOTS 10000
#define NODE_DATA_SIZE 128


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

void writeResults(unsigned long long *maxLat, unsigned long long *minLat, unsigned long long *sumLat, 
	int *cntLat, unsigned long long t_min, unsigned long long t_max, const char *filename)
{
	int i;
	FILE *f;
	char pathname[50] = {"/tmp/"};
	
	strcat(pathname, filename);

	if ((f = fopen(pathname, "w+")) == NULL) {
		printf("cannot open file %s\n\r", filename);
		return;
	}

	for (i = 0; i < NUM_SLOTS; i++) {
		
	}
	
	fclose(f);	
}


int main()
{
	int i, c, pID, handle;
	struct barrier_d b, eb;
	lfmpscq_d_t q;
	long long cnt, d, enqueue_fails, dequeue_fails;
	unsigned long long t1, t2, dt, t_min, t_max, t_sum;
	void *data;

	data = malloc(NODE_DATA_SIZE);

#ifdef ARCH_X86
	/* change previledge level */
	iopl(3);
#endif

	/* get rtipc handle */
	handle = rtipc_create_handle();

	/* set up childs */
	for (i = 0; i < NUM_PROC - 1; i++) {
		if ((c = fork()) == 0) {
			pID = i + 1;
			break;
		} else {
			pID = 0;
		}
	}
	

	/* prevent swapping */
	osfunctions_mLockAll();

	if (osfunctions_movetocpu(pID) == -1)
		printf("Proc %d: cannot move to cpu %d\n", pID, pID);

	if (osfunctions_setMaxPriority() == -1)
		printf("Proc %d: cannot set priority\n", pID);

	/* init librtipc */
	if (rtipc_initialize(handle) == -1) {
		printf("Proc %d: rtipc_initialize() failed\n", pID);
		return 0;
	}
	
	/* get barrier */
	if (barrier_get(&b, SHMID_BARRIER, NUM_PROC) == -1) {
		printf("Proc %d: barrier_get() failed\n", pID);
		return 0;
	}
	if (pID != 0) {
		if (barrier_get(&eb, SHMID_ENQBARRIER, NUM_PROC - 1) == -1) {
			printf("Proc %d: barrier_get() failed\n", pID);
			return 0;
		}
	}

	/* connect to the queue */
	if (lfmpscq_get(&q, SHMID_QUEUE, pID, NODE_DATA_SIZE, NUM_SLOTS, NUM_PROC, 1) == -1) {
		printf("Proc %d: lfmpscq_get() failed\n", pID);
		return 0;
	}

	
	printf("Proc %d: running, pid: %d\n", pID, getpid());

	barrier_wait(&b);


	t_max = 0;
	t_min = 0xffffffffffffffff;
	t_sum = 0;

	
#ifdef ARCH_X86
	hwfunctions_cli();
#endif
	if (pID != 0) {
		d = 0;
		enqueue_fails = 0;
		for (i = 0; i < NUM_ITERATION; i++) {
			barrier_wait(&b);
			cnt = 0;
			while (cnt < NUM_MESSAGES) {
				((struct message *)data)->pid = pID;
				((struct message *)data)->cnt = cnt;
			/*	hwfunctions_invalidateCacheRegion(q.mem, sizeQshm);*/
				barrier_wait(&eb);
#ifdef ARCH_X86
				t1 = read_tsc();
#endif
				while (lfmpscq_enqueue(&q, data) != 0)
					enqueue_fails++;
#ifdef ARCH_X86
				t2 = read_tsc();
#endif
				if (cnt >= 10) {
					d++;
					dt = t2 - t1;
					if (dt > t_max)
						t_max = dt;
					if (dt < t_min)
						t_min = dt;
					t_sum += dt;
				}				
				cnt++;
			}
		}
	} else {
		d = 0;
		dequeue_fails = 0;
		for (i = 0; i < NUM_ITERATION; i++) {
			barrier_wait(&b);
			cnt = 0;
			while (cnt < NUM_MESSAGES * (NUM_PROC - 1)) {
				((struct message *)data)->pid = pID;
				((struct message *)data)->cnt = cnt;
#ifdef ARCH_X86
				t1 = read_tsc();
#endif
				while (lfmpscq_dequeue(&q, data) != 0)
					dequeue_fails++;
#ifdef ARCH_X86
				t2 = read_tsc();
#endif
			/*	printf("received: %d:%lld\n", ((struct message *)data)->pid, ((struct message *)data)->cnt);*/
				if (cnt >= 10) {
					d++;
					dt = t2 - t1;
					if (dt > t_max)
						t_max = dt;
					if (dt < t_min)
						t_min = dt;
					t_sum += dt;
				}				
				cnt++;
			}
		}
		
	}
#ifdef ARCH_X86
	hwfunctions_sti();
#endif

	barrier_wait(&b);

	if (pID == 0) {
		printf("Proc %d (dequeue): interations: %lld dequeue fails: %lld t_min: %lld t_max: %lld average %lld\n", pID, d, dequeue_fails, t_min, t_max, t_sum / d);
	} else {
		printf("Proc %d (enqueue): iterations: %lld enqueue fails: %lld t_min: %lld t_max: %lld average %lld\n", pID, d, enqueue_fails, t_min, t_max, t_sum / d);
	}
	

	barrier_wait(&b);
	
	barrier_release(&b);
	lfmpscq_release(&q);

	if (pID != 0)
		barrier_release(&eb);

	if (pID == 0)
		rtipc_finalize();

	printf("Proc %d: finished, pid: %d\n", pID, getpid());
	
	return 0;
}
