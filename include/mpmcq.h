/**
 * @file mpmcq.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * lock free multi-enqueuer multi-dequeuer fifo queue
 *
 * implementation based on: 
 * Maged M. Michael: "Hazard Pointers: Safe Memory Reclamation for Lock-Free Objects"
 * 
 */

#ifndef _MPMCQ_H_
#define _MPMCQ_H_


/**
 * shared data of the queue
 */
typedef struct mpmcq_head {
	volatile int lock;
	volatile int init;
	volatile int head; 
	volatile int tail; 
} mpmcq_head_t;


/**
 * head for each slot
 */
typedef struct mpmcq_slot_head {
	volatile int used;
	volatile int next;
	volatile int id;
} mpmcq_slot_head_t;


/**
 * descriptor of the queue
 */
typedef struct mpmcq_d {
	int objID; 
	int init;
	volatile mpmcq_head_t *q; /* pointer to the shared memory */
	volatile int *sharedIdxs; /* the shared indexes, two for each user; single-writer multi-reader (-> hazard pointer)*/
	volatile void *slots; 
	int userID;
	int nodeDataSize; /* size of a message in byte */
	int numSlots; /* number of max possible nodes */
	int maxUser; /* the maximum number of user accessing the queue */
} mpmcq_d_t;


/**
 * create/get the queue
 *
 * @param qd descriptor of queue
 * @return 0 on success, -1 on fail
 */
int mpmcq_get(mpmcq_d_t *qd, int objID, int userID, int nodeDataSize, int numSlots, int maxUser);


/**
 * enqueue new data to the queue
 *
 * @param qd descriptor of the queue
 * @param data the data that should be enqueued
 * @return 0 on success, -1 on fail
 */
int mpmcq_enqueue(mpmcq_d_t *qd, void *data);


/**
 * dequeue data from the queue
 *
 * @param qd descriptor of the queue
 * @param pointer to a buffer in which the data from the queue should be copied
 * @return 0 on success, -1 on fail
 */
int mpmcq_dequeue(mpmcq_d_t *qd, void *data);


/**
 * deattach shared memory of the queue
 *
 * @param qd descriptor of queue
 * @return 0 on success, -1 on fail
 */
int mpmcq_release(mpmcq_d_t *qd);

	
#endif
