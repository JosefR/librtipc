/**
 * @file barrier.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include "barrier.h"
#include "shmman.h"
#include "hwfunctions.h"
#include "atomic.h"
#include <stdio.h> 


int barrier_get(barrier_d_t *bd, int objID, int numUser)
{
	void *ptr;

	bd->objID = objID;
	bd->numUser = numUser;

	/* create shared memory segment for barrier */
	if (shmman_get_shmseg(bd->objID, sizeof(barrier_data_t), &ptr) == -1) {
		perror("barrier: shmman_get_shmseg() failed\n");
		return -1;
	}

	/* set pointer to shared memory */
        bd->shm = (barrier_data_t *)ptr;

        /* get lock */
        while (!atomic_cas32(&(bd->shm->lock), 0, 1))
                hwfunctions_nop();

        /* initialize if we are the first user of flag */
        if (bd->shm->init == 0) {
                bd->shm->cnt = 0;
                bd->shm->fallen = 0;

                bd->shm->init = 1;
        }

        /* release lock */
        bd->shm->lock = 0;
	
	return 0;
}


void barrier_wait(barrier_d_t *bd)
{
	/* wait until all users have left the barrier and the last one has set up the barrierer*/
	while (bd->shm->fallen == 1) 
		hwfunctions_nop(); 

	/* add new user waiting at barrier */
	atomic_faa32(&(bd->shm->cnt), 1);

	while ((bd->shm->cnt != bd->numUser) && (bd->shm->fallen == 0))	{
	}

	bd->shm->fallen = 1; /* allow all waiting users to pass barrier */

	/* decrement cnt; if last user leaves the barrier: reset the barrier and allow new processes to enter the barrier */
	if (atomic_faa32(&(bd->shm->cnt), -1) == 1) {
		bd->shm->fallen = 0;
	}
}


int barrier_release(barrier_d_t *bd)
{
	if (shmman_release_shmseg(bd->objID) == -1) {
		perror("barrier: shmman_release_shmseg() failed\n"); 
		return -1;
	}
	
	bd->shm = NULL;

	return 0;
}

