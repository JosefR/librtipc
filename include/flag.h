/**
 * @file flag.h
 * @author Josef Raschen <josef@raschen.org>
 * 
 * shared memory based binary flag
 */

#ifndef _FLAG_H_
#define _FLAG_H_


/**
 * shared-memory data of flag
 */
typedef struct flag_data {
	volatile int lock;
	volatile int init;
	volatile unsigned char f;
} flag_data_t;


/** 
 * descriptor for flag
 */
typedef struct flag_d{
	int objID;
	flag_data_t *f; /* shared data */
} flag_d_t;


/** 
 * get (and create if neccessrary) the flag
 *
 * @param fd flag descriptor
 * @return 0 on success, -1 on fail
 */
int flag_get(flag_d_t *fd, int objID);


/** 
 * set the flag
 * 
 * @param fd flag descriptor
 */
void flag_set(flag_d_t *fd);


/** 
 * reset the flag
 * 
 * @param fd flag descriptor
 */
void flag_reset(flag_d_t *fd);


/** 
 * get the value of the flag
 * 
 * @param fd flag descriptor
 */
unsigned char flag_getval(flag_d_t *fd);


/** 
 * spin until flag has value val
 * 
 * @param fd flag descriptor
 * @param val the funktion will spin on the flag until it has the value named by val
 */
void flag_spin(flag_d_t *fd, unsigned char val);


/**
 * deattach the shared memory segment
 *
 * @param fd flag descriptor
 * @return 0 on success, -1 on fail
 */
int flag_release(flag_d_t *fd);


#endif
