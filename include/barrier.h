/**
 * @file barrier.h
 * @author Josef Raschen <josef@raschen.org>
 * 
 * shared memory based barrier
 */

#ifndef _BARRIER_H_
#define _BARRIER_H_


/**
 * shared-memory barrier-data 
 */
typedef struct barrier_data {
	volatile int lock;
	volatile int init;
	volatile int cnt;
	volatile int fallen;
} barrier_data_t;


/** 
 * descriptor for barrier 
 */
typedef struct barrier_d{
	int objID;
	barrier_data_t *shm;
	int numUser;
} barrier_d_t;


/** 
 * get (and create if neccessary) the barrier 
 *
 * @param bd barier descriptor; value of users has to be set before calling
 * @param objID unique ID for object
 * @param numUser the numer of users od the barrier
 * @return 0 on success, -1 on fail
 */
int barrier_get(barrier_d_t *bd, int objID, int numUser);


/** 
 * busy waiting at barrier
 * 
 * @param bd barier descriptor
 */
void barrier_wait(barrier_d_t *bd);


/**
 * deattache the shared memory segment
 *
 * @param bd barrier descriptor
 * @return 0 on success, -1 on fail
 */
int barrier_release(barrier_d_t *bd);


#endif
