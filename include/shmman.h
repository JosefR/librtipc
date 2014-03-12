/**
 * @file shmman.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * Functions to manage shared memory segments.
 */

#ifndef __SHMMAN_H__
#define __SHMMAN_H__


/**
 * create a handle
 *
 * @return handle
 */
int shmman_create_handle();

/**
 * write infromation from shared shmseg table to stdout
 */
void shmman_printShmsegTable();

/**
 * initialize internal data structures to manage the shared memory
 *
 * this function has to be called once to set up the shared memory manager
 *
 * @return 0 on success, -1 on fail
 */
int shmman_initialize();


/**
 * set up descriptor of existing shared memory segment 
 *
 * @param shmid id of shared memory segment
 * @param ptr pointer to shared memory segment; will be set
 * @return 0 on success, -1 on fail
 */
int shmman_get_shmseg(int shmid, int size, void **ptr);


/** 
 * release a shared memory segment
 *
 * @param shmid id of shared memory segment
 * @return 0 on success, -1 on fail
 */
int shmman_release_shmseg(int shmid);


/**
 * disconnect from the shared memory manager
 * 
 * @param sd descriptor of shared memory segment
 * @return 0 on success, -1 on fail
 */
int shmman_disconnect();


/**
 * clean up internal data structures used to manage the shared memory
 * 
 * @return 0 on success, -1 on fail
 */
int shmman_finalize(); 


#endif /*_SHMMAN_H_*/

