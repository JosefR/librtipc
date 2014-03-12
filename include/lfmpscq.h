/**
 * @file lfmpscq.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * lock free multi-enqueuer single-dequeuer fifo queue
 *
 * implementation based on: 
 * Maged M. Michael: "Hazard Pointers: Safe Memory Reclamation for Lock-Free Objects"
 * 
 */

#ifndef _LFMPSCQ_H_
#define _LFMPSCQ_H_


/**
 * shared data of the queue
 */
typedef struct lfmpscq_head {
	volatile int lock;
	volatile int init;
	volatile int head; 
	volatile int tail; 
} lfmpscq_head_t;


/**
 * head for each slot
 */
typedef struct lfmpscq_slot_head {
	volatile int used;
	volatile int next;
} lfmpscq_slot_head_t;


/**
 * descriptor of the queue
 */
typedef struct lfmpscq_d {
	int objID; 
	volatile lfmpscq_head_t *q; /* pointer to the shared memory */
	volatile int *sharedIdxs; /* the shared indexes, one for each user; single-writer multi-reader (-> hazard pointer)*/
	volatile void *slots; 
	int userID;
	int nodeDataSize; /* size of a message in byte */
	int numSlots; /* number of max possible nodes */
	int maxUser; /* the maximum number of user accessing the queue */
} lfmpscq_d_t;


/**
 * create/get the queue
 *
 * @param qd descriptor of queue
 * @param init set to 1 if initialization of local data structures necessary, 0 otherwise
 * @return 0 on success, -1 on fail
 */
int lfmpscq_get(lfmpscq_d_t *qd, int objID, int userID, int nodeDataSize, int numSlots, int maxUser, int init);


/**
 * enqueue new data to the queue
 *
 * @param qd descriptor of the queue
 * @param data the data that should be enqueued
 * @return 0 on success, -1 on fail
 */
int lfmpscq_enqueue(lfmpscq_d_t *qd, void *data);


/**
 * dequeue data from the queue
 *
 * @param qd descriptor of the queue
 * @param pointer to a buffer in which the data from the queue should be copied
 * @return 0 on success, -1 on fail
 */
int lfmpscq_dequeue(lfmpscq_d_t *qd, void *data);


/**
 * deattach shared memory of the queue
 *
 * @param qd descriptor of queue
 * @return 0 on success, -1 on fail
 */
int lfmpscq_release(lfmpscq_d_t *qd);

	
#endif
