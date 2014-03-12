/**
 * simple demo application showing how to use functions of shmman.h
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <rtipc.h>

#define SHMID_SHMSEG 0
#define SHMID_BARRIER 1


int main()
{
	int handle, c, cpu;
	struct barrier_d b;
	void *mem;

	/* create handle */
	handle = rtipc_create_handle();


	if ((c = fork()) == 0) {
		/* child */
		
		cpu = 1;
		if (osfunctions_movetocpu(1) == -1)
			printf("osfunctions_movetocpu(%d) fails\n", cpu);
		else
			printf("osfunctions_movetocpu(%d) moved to cpu %d\n", cpu, cpu);
	
		/* initialize librtipc */
		if (rtipc_initialize(handle) == -1) {
			printf("cannot connect to librtipc\n");
			return 0;
		} else {
			printf("successfully connected to librtipc\n");
		}

		/* get the pointer to the shared memory segment */
		if (shmman_get_shmseg(SHMID_SHMSEG, 1024, &mem) == -1) {
			printf("shmseg_get() failed\n");
			return 0;
		}

		/* get barrier */
		if (barrier_get(&b, SHMID_BARRIER, 2) == -1) {
			printf("barrier_get() failed\n");
			return 0;
		}

		/* sync */
		printf("CHILD: wait at barrier 1\n");
		barrier_wait(&b);
		printf("CHILD: passed barrier 1\n");

		/* sync */
		printf("CHILD: wait at barrier 2\n");
		barrier_wait(&b);
		printf("CHILD: passed barrier 2\n");

		
		printf("CHILD: now sleep(5)\n");
		sleep(5);

		/* sync */
		barrier_wait(&b);
		printf("CHILD: passed barrier 3\n");

		/* write something to shared mem */
		*((int*)mem + 1) = 27;

		/* sync */
		barrier_wait(&b);
		
		/* release the shared memory segment */
		shmman_release_shmseg(SHMID_SHMSEG);	

		/* release the barrier */
		barrier_release(&b);
	}
	else {
		/* parent */

		cpu = 0;	
		if (osfunctions_movetocpu(1) == -1)
			printf("PARENT: osfunctions_movetocpu(%d) fails\n", cpu);
		else
			printf("PARENT: osfunctions_movetocpu(%d) moved to cpu %d\n", cpu, cpu);
	
		
		/* connect to librtipc */
		if (rtipc_initialize(handle) == -1) {
			printf("PARENT: cannot connect to librtipc\n");
			return 0;
		} else {
			printf("PARENT: successfully connected to librtipc\n");
		}

		/* get the shared memeory segment */
		if (shmman_get_shmseg(SHMID_SHMSEG, 1024, &mem) == -1) {
			printf("PARENT: shmman_get_shmseg() failed\n");
			return 0;
		} else {
			printf("PARENT: shmman_get_shmseg() successful\n");
		}

		/* get barrier */
		if (barrier_get(&b, SHMID_BARRIER, 2) == -1) {
			printf("PARENT: barrier_get() failed\n");
			return 0;
		} else {
			printf("PARENT: barrier_get() successful\n");
		}

		printf("PARENT: now sleep(5)\n");
		sleep(5);
	
		printf("cnt: %d\n", b.shm->cnt);

		/* sync */
		barrier_wait(&b);
		printf("PARENT: passed barrier 1\n");

		printf("PARENT: now sleep(5)\n");
		sleep(5);

		/* sync */
		barrier_wait(&b);
		printf("PARENT: passed barrier 2\n");

		/* sync */
		printf("PARENT: wait at barrier 3\n");
		barrier_wait(&b);
		printf("PARENT: passed barrier 3\n");

		/* wait until child wrote to shm */
		while(*((int*)mem+ 1) != 27);

		/* sync */
		barrier_wait(&b);

		/* release the shared memory segment */
		shmman_release_shmseg(SHMID_SHMSEG);		

		/* release barrier */
		barrier_release(&b);

		/* stop librtipc */
		rtipc_finalize();
	}

	return 0;
}
