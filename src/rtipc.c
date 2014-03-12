/*
 * @file rtipc.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include "rtipc.h"


int rtipc_create_handle()
{
	return shmman_create_handle();
}


int rtipc_initialize(int handle)
{
	/* set up shared memory management */
	return shmman_initialize(handle);
}


int rtipc_finalize()
{
	/* clean up shared memory management */
	if(shmman_finalize() != 0)
		return -1;

	return 0;
}
