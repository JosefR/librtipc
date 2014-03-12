/**
 * @file spscq.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * single-producer single-consumer queue
 *
 * the queue is implemented using a ringbuffer
 */

#ifndef _SPSCQ_H_
#define _SPSCQ_H_


/**
 * shared data of the queue
 */
typedef struct spscq_head {
	volatile int lock;
	volatile int init;
	volatile int head; /* first used slot */
        volatile int next_slot; /* next free slot */
/*        volatile int used_slots; * number of slots currently usef */
} spscq_head_t;


/**
 * descriptor of the queue
 */
typedef struct spscq_d {
	int objID; 
	spscq_head_t *q; /* pointer to the shared memory */
	int slotSize; /* site of a message in byte */
	int numSlots; /* number of available slots */
} spscq_d_t;


/**
 * create/get the queue
 *
 * @param qd descriptor of queue
 * @return 0 on success, -1 on fail
 */
int spscq_get(spscq_d_t *qd, int objID, int slotSize, int numSlots);


/**
 * enqueue new data to the queue
 *
 * @param qd descriptor of the queue
 * @param data the data that should be enqueued
 * @return 0 on success, -1 on fail
 */
int spscq_enqueue(spscq_d_t *qd, void *data);


/**
 * dequeue data from the queue
 *
 * @param qd descriptor of the queue
 * @param pointer to a buffer in which the data from the queue should be copied
 * @return 0 on success, -1 on fail
 */
int spscq_dequeue(spscq_d_t *qd, void *data);


/**
 * get the current number of used slots
 *
 * @param qd descriptor of the queue
 * @return number of slots
 */
int spscq_getFillLevel(spscq_d_t *qd);


/**
 * deattach shared memory of the queue
 *
 * @param qd descriptor of queue
 * @return 0 on success, -1 on fail
 */
int spscq_release(spscq_d_t *qd);

	
#endif
