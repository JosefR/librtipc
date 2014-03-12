/**
 * @file mutex.h
 * @author Josef Raschen <josef@raschen.org>
 * 
 * shared memory mutex
 */

#ifndef _MUTEX_H_
#define _MUTEX_H_


/**
 * shared-memory of mutex
 */
typedef struct mutex_data {
	int lock;
	int init;
	int mutex;
} mutex_data_t;


/** 
 * descriptor of mutex
 */
typedef struct mutex_d{
	int objID;
	mutex_data_t *m; /* shared data */
} mutex_d_t;


/** 
 * get/create mutex
 *
 * @param md descriptor of mutex
 * @return 0 on success, -1 on fail
 */
int mutex_get(mutex_d_t *md, int objID);


/** 
 * lock mutex, spin if necessary
 * 
 * @param md descriptor of mutex
 */
void mutex_lock(mutex_d_t *md);


/** 
 * try to lock the mutex, do NOT spin
 * 
 * @param md descriptor of mutex
 * @return 0 on success, -1 on fail
 */
int mutex_trylock(mutex_d_t *md);


/** 
 * unlock the mutex
 * 
 * @param md descriptor of mutex
 */
void mutex_unlock(mutex_d_t *md);


/**
 * release the object
 *
 * @param md descriptor of mutex
 * @return 0 on success, -1 on fail
 */
int mutex_release(mutex_d_t *md);


#endif
