/**
 * demo application for barrier object
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <rtipc.h>


#define SHMID_SHMSEG 0
#define SHMID_BARRIER 1

#define SHMSEG_SIZE 1024

int main()
{
	int c, handle;
	struct barrier_d b;
	void *mem;


	handle = rtipc_create_handle();

	if ((c = fork()) == 0) {
		/* child */

		if (osfunctions_movetocpu(1) == -1)
			printf("CHILD: cannot move to cpu %d\n", 1);
		
		/* initialize librtipc */
		rtipc_initialize(handle);

		/* get the shared memory segment */
		if (shmman_get_shmseg(SHMID_SHMSEG, SHMSEG_SIZE, &mem) == -1) {
			printf("shmman_get_shmseg() failed\n");
			return 0;
		}

		/* get barrier */
		if (barrier_get(&b, SHMID_BARRIER, 2) == -1) {
			printf("barrier_get() failed\n");
			return 0;
		}

		/* sync */
		barrier_wait(&b);
		printf("CHILD: passed barrier 1\n");

		/* sync */
		barrier_wait(&b);
		printf("CHILD: passed barrier 2\n");

		sleep(1);

		/* sync */
		barrier_wait(&b);
		printf("CHILD: passed barrier 3\n");

		/* write something to shared mem */
		*((int*)mem + 1) = 27;

		/* sync */
		barrier_wait(&b);
		
		/* release the shared memory segment */
		shmman_release_shmseg(SHMID_SHMSEG);		

		/* release barrier */
		barrier_release(&b);		
	}
	else {
		/* parent */
	
		if (osfunctions_movetocpu(0) == -1)
			printf("PARENT: cannot move to cpu %d\n", 0);
		
		/* initilize librtipc */
		rtipc_initialize(handle);

		/* get the shared memory segment */
		if (shmman_get_shmseg(SHMID_SHMSEG, SHMSEG_SIZE, &mem) == -1) {
			printf("PARENT: shmman_get_shmseg() failed\n");
			return 0;
		}

		/* get barrier */
		if (barrier_get(&b, SHMID_BARRIER, 2) == -1) {
			printf("PARENT: barrier_get() failed\n");
			return 0;
		}

		sleep(1);
	
		printf("cnt: %d\n", b.shm->cnt);

		/* sync */
		barrier_wait(&b);
		printf("PARENT: passed barrier 1\n");

		sleep(1);

		/* sync */
		barrier_wait(&b);
		printf("PARENT: passed barrier 2\n");

		/* sync */
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
