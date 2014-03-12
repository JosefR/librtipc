#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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
#include "perfcounters.h"
#endif

#define SHMID_BARRIER 0


#define NUM_PROC 2
#define NUM_ITERATION 100
#define NUM_MESSAGES 50000
#define NUM_SLOTS 100
#define SLOT_SIZE 16


struct message {
	volatile int pid;
	volatile long long cnt;
};

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
	int i, c, pID = 0, handle, sizeQShm;
	struct barrier_d b;
	spscq_d_t q[NUM_PROC];
	long long cnt, enqueue_fails, dequeue_fails;
	unsigned long long t1, t2, dt, t_min, t_max, t_sum;
	int qIDenq, qIDdeq;
	struct message msg;

#ifdef ARCH_X86
	/* change previledge level */
	iopl(3);
#endif

#ifdef ARCH_ARM
	apm_init();
#endif

	/* get rtipc handle */
	handle = rtipc_create_handle();

	sizeQShm = sizeof(spscq_head_t) + NUM_SLOTS * SLOT_SIZE;

	/* set up childs */
	for (i = 0; i < NUM_PROC - 1; i++) {
		if ((c = fork()) == 0) {
			pID = i + 1;
			break;
		}
	}
	
	/* determine, which queue to enqueue and which to dequeue*/
	qIDenq = pID;
	qIDdeq = (pID + 1) % NUM_PROC;

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

	/* connect to the queue to enqueue */
	if (spscq_get(&(q[qIDenq]), SHMID_BARRIER + 1 + qIDenq, SLOT_SIZE, NUM_SLOTS) == -1) {
		printf("Proc %d: spscq_get() failed\n", pID);
		return 0;
	}

	/* connect to the queue to dequeue from */
	if (spscq_get(&(q[qIDdeq]), SHMID_BARRIER + 1 + qIDdeq, SLOT_SIZE, NUM_SLOTS) == -1) {
		printf("Proc %d: spscq_get() failed\n", pID);
		return 0;
	}

	printf("Proc %d: running\n", pID);

	barrier_wait(&b);

	hwfunctions_cli();

	enqueue_fails = 0;
	dequeue_fails = 0;
	t_max = 0;
	t_min = 0xffffffffffffffff;
	t_sum = 0;
	for (i = 0; i < NUM_ITERATION; i++) {
#ifdef ARCH_ARM
		apm_stopCCNTR();
		if (apm_fetchCCNTRovfl() == 1) {
			printf("cycle counter overflow\n");
                        apm_resetCCNTRovfl();
		}
		apm_resetCCNTR();
		apm_startCCNTR();
#endif
		cnt = 0;
		while (cnt < NUM_MESSAGES) {
			if (pID == 0) {
				msg.cnt = cnt;
/*				printf("send %d (%d)\n", msg.cnt, qIDenq);*/
#ifdef ARCH_X86
				t1 = read_tsc();
#endif
#ifdef ARCH_ARM
				t1 = apm_fetchCCNTR();
#endif
				while (spscq_enqueue(&(q[qIDenq]), &msg) != 0)
					enqueue_fails++;
				msg.cnt = -1;
				while (spscq_dequeue(&(q[qIDdeq]), &msg) != 0)
					dequeue_fails++;
#ifdef ARCH_X86
				t2 = read_tsc();
#endif
#ifdef ARCH_ARM
				t2 = apm_fetchCCNTR();
#endif
				if (msg.cnt != cnt)
					printf("ERROR: received wrong message %lld (expected %lld)\n", msg.cnt, cnt);	 
/*				printf("received %d (%d)\n", msg.cnt, qIDdeq);*/
				if (pID == 0) {
					if (i > 0) {
						dt = t2 - t1;
						if (dt > t_max)
							t_max = dt;
						if (dt < t_min)
							t_min = dt;
						t_sum += dt;
					}				
				}
			} else {			
				msg.cnt = -1;
				while(spscq_dequeue(&(q[qIDdeq]), &msg) != 0)
					dequeue_fails++;
/*				printf("send %d (%d->%d)\n", msg.cnt, qIDdeq,  qIDenq);*/
				while(spscq_enqueue(&(q[qIDenq]), &msg) != 0)
					enqueue_fails++;
			}

			cnt++;
		}
	}
	hwfunctions_sti();

#ifdef ARCH_ARM
	apm_stopCCNTR();
#endif

	if (pID == 0) {
		printf("t_min: %lld\nt_max: %lld\naverage: %lld\n\n", t_min, t_max, (long long)((float)t_sum / ((NUM_ITERATION - 1) * NUM_MESSAGES )));
	}


	barrier_wait(&b);
	
	barrier_release(&b);
	spscq_release(&q[qIDenq]);
	spscq_release(&q[qIDdeq]);

	if (pID == 0)
		rtipc_finalize();
	
	printf("Proc %d: finished\n", pID);
	
	return 0;
}
