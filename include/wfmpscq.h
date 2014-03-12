/**
 * @file wfmpscq.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * multi-producer single-consumer queue
 * 
 * new implmentation: table based implementation
 */

#ifndef _WFMPSCQ_H_
#define _WFMPSCQ_H_


/**
 * struct for slot description
 */
typedef struct wfmpscq_slot {
        volatile unsigned long long id; /* the message id */
} wfmpscq_slot_t;


/**
 * shared data of all queues
 */
typedef struct wfmpscq_head {
	volatile int lock;
	volatile int init;
	volatile unsigned long long id_cnt; /* 64 bit counter for message ids */
} wfmpscq_head_t;


/**
 * descriptor of a single queue 
 */
typedef struct wfmpscq_queue_d {
	volatile int head; /* index of first slot of list */
        volatile int tail; /* index if last slot of list */
} wfmpscq_queue_d_t;


/**
 * enum for defining the strategy for dequeuing elements from the queues
 * DQSTR_IDABSOLUTE: the head of all queues with the lowest ID will be dequeued
 *    should only be used, if total number of messages in system will never exeed
 *    0xffffffff
 * DQSTR_ROUNDROBIN: An element (if any) from queue A will be dequeued. The next 
 *    queue from which an element will be dequeued will be queue 
 *    (A + 1) % NUM_QUEUES
 */
enum wfmpscq_deqstrategy {
	DQSTR_IDABSOLUTE, 
	DQSTR_ROUNDROBIN
};


/**
 * descriptor of the object
 */
typedef struct wfmpscq_d {
	int objID; 
	wfmpscq_head_t *mem; /* pointer to the shared memory */
	wfmpscq_queue_d_t *queues; /* pointer to the writer queue descriptors */
	wfmpscq_slot_t *slots; /* pointer to the writer queue slots */
	void *data; /* pointer to the writer queue data */
	int writerID; 
	int numWriter; /* number of producer */
	enum wfmpscq_deqstrategy dqstrat; /* the dequeue strategy */
	int nodeDataSize; /* size of a slot in byte */
	int numSlots; /* number of available slots per queue */
} wfmpscq_d_t;


/**
 * create/get the queue
 *
 * @param qd descriptor of queue
 * @param idmethod 
 * @return 0 on success, -1 on fail
 */
int wfmpscq_get(wfmpscq_d_t *qd, int objID, int writerID, int numWriter, enum wfmpscq_deqstrategy dqstrat, int nodeDataSize, int numSlots);


/**
 * enqueue new data to the queue
 *
 * @param qd descriptor of the queue
 * @param data the data that should be enqueued
 * @return 0 on success, -1 if no free slot is available in queue
 */
int wfmpscq_enqueue(wfmpscq_d_t *qd, void *data);


/**
 * dequeue data from the queue
 *
 * @param qd descriptor of the queue
 * @param data pointer to a buffer in which the data from queue should be copied
 * @return 0 on success, -1 if queue is empty
 */
int wfmpscq_dequeue(wfmpscq_d_t *qd, void *data);


/**
 * deattach the shared memory segment
 *
 * @param qd descriptor of the queue
 * @return 0 on success, -1 on fail
 */
int wfmpscq_release(wfmpscq_d_t *qd);


#endif
