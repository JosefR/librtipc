/**
 * @file flag.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include "flag.h"
#include "shmman.h"
#include "atomic.h"
#include "hwfunctions.h"
#include <stdio.h> 


int flag_get(flag_d_t *fd, int objID)
{
	void *ptr;

	fd->objID = objID;

	if (shmman_get_shmseg(fd->objID, sizeof(flag_data_t), &ptr) == -1) {
		perror("shmman_get_shmseg() failed\n");
		return -1;
	}
	
	/* set pointer to shared memory */
	fd->f = (flag_data_t *)ptr;

	/* get lock */
	while (!atomic_cas32(&(fd->f->lock), 0, 1))
		hwfunctions_nop();

	/* initialize if we are the first user of flag */
	if (fd->f->init == 0) {
		fd->f->f = 0;
		
		fd->f->init = 1;
	}
	
	/* release lock */
	fd->f->lock = 0;
		
	return 0;
}


void flag_set(flag_d_t *fd)
{
	fd->f->f = 1;
}


void flag_reset(flag_d_t *fd)
{
	fd->f->f = 0;
}


unsigned char flag_getval(flag_d_t *fd)
{
	return fd->f->f;
}


void flag_spin(flag_d_t *fd, unsigned char val)
{
	while (fd->f->f != val) {
		hwfunctions_nop();
		hwfunctions_nop();
		hwfunctions_nop();
		hwfunctions_nop();
		hwfunctions_nop();
	}
}


int flag_release(flag_d_t *fd)
{
	if (shmman_release_shmseg(fd->objID) == -1) {
		perror("flag: shmman_release_shmseg() failed\n"); 
		return -1;
	}

	return 0;
}

