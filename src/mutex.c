/**
 * @file mutex.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include "mutex.h"
#include "shmman.h"
#include "atomic.h"
#include "hwfunctions.h"

#include <stdio.h> 


int mutex_get(mutex_d_t *md, int objID)
{
	void *ptr;

	md->objID = objID;

	if (shmman_get_shmseg(md->objID, sizeof(mutex_data_t), &ptr) == -1) {
		perror("mutex: shmman_get_shmseg() failed\n");
		return -1;
	}
	
	/* set pointer to shared memory */
	md->m = (mutex_data_t *)ptr;

	/* get lock */
	while (!atomic_cas32(&(md->m->lock), 0, 1))
		hwfunctions_nop();

	/* initialize if we are the first user of mutex */
	if (md->m->init == 0) {
		md->m->mutex = 0;
		
		md->m->init = 1;
	}
	
	/* release lock */
	md->m->lock = 0;
		
	return 0;
}


void mutex_lock(mutex_d_t *md)
{
	/* spin until we get the mutex */
	while(!atomic_cas32(&(md->m->mutex), 0, 1))
		hwfunctions_nop();
}


int mutex_trylock(mutex_d_t *md)
{
	if(atomic_cas32(&(md->m->mutex), 0, 1))
		return 0;
	else
		return -1;
}


void mutex_unlock(mutex_d_t *md)
{
	md->m->mutex = 0;
}


int mutex_release(mutex_d_t *md)
{
	if (shmman_release_shmseg(md->objID) == -1) {
		perror("mutex: shmman_release_shmseg() failed\n"); 
		return -1;
	}

	return 0;
}

