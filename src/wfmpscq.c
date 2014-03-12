/**
 * @file wfmpscq.c
 * @author Josef Raschen <josef@raschen.org>
 *
 */

#include "wfmpscq.h"

#include <stdio.h>
#include <string.h>
#include "shmman.h"
#include "atomic.h"
#include "hwfunctions.h"

#define MAXID 0xffffffff

#define UINT32MAX 0xffffffff


int wfmpscq_get(wfmpscq_d_t *qd, int objID, int numWriter, int writerID, enum wfmpscq_deqstrategy dqstrat, int nodeDataSize, int numSlots) 
{
	void *ptr;
	int size, i;
	wfmpscq_queue_d_t *q;

	qd->objID = objID;
	qd->writerID = writerID;
	qd->numWriter = numWriter;
	qd->dqstrat = dqstrat;
	qd->nodeDataSize = nodeDataSize;
	qd->numSlots = numSlots;

	/* get/create shared memory segment */
	
	size = sizeof(wfmpscq_head_t) 
		+ qd->numWriter * sizeof(wfmpscq_queue_d_t)
		+ qd->numWriter * qd->numSlots * sizeof(wfmpscq_slot_t) 
		+ qd->numWriter * qd->numSlots * qd->nodeDataSize;
	
	if (shmman_get_shmseg(qd->objID, size, &ptr) == -1) {
		perror("wfmpscq: shmman_get_shmseg() failed\n");
		return -1;
	}


	/* set up descriptor */
	qd->mem = (wfmpscq_head_t *)ptr;
	qd->queues = (wfmpscq_queue_d_t *)((unsigned long)ptr + sizeof(wfmpscq_head_t));
	qd->slots = (wfmpscq_slot_t *)((unsigned long)(qd->queues) + qd->numWriter * sizeof(wfmpscq_queue_d_t));
	qd->data = (wfmpscq_slot_t *)((unsigned long)(qd->slots) + qd->numWriter * qd->numSlots * sizeof(wfmpscq_slot_t));

	/* get lock */
	while (!atomic_cas32(&(qd->mem->lock), 0, 1))
		hwfunctions_nop();

	/* initialize if necessary */
	if (qd->mem->init == 0) {
		qd->mem->id_cnt = 0;

		/* set up writer-queues */
		q = (wfmpscq_queue_d_t*)((unsigned long)ptr + sizeof(wfmpscq_head_t));
		for (i = 0; i < qd->numWriter; i++) {
			q->head = 0;
			q->tail = 0;
			q++;
		}
		
		qd->mem->init = 1;
	}
		
	/* release lock */
	qd->mem->lock = 0;

	return 0;	
}


int wfmpscq_enqueue(wfmpscq_d_t *qd, void *data)
{
	unsigned int id;
	volatile int head, tail, tail_next, slot_idx;
	wfmpscq_slot_t *slot_ptr;
	void *slot_data_ptr;

	head = qd->queues[qd->writerID].head;
	tail = qd->queues[qd->writerID].tail;

	/* check if queue is full */
	tail_next = (tail + 1) % qd->numSlots;
	if (tail_next == head)
		return -1;

	/* get new id */
	id = atomic_faa64u(&(qd->mem->id_cnt), 1);
	
	slot_idx = tail;

	/* copy data to slot */
	slot_ptr = qd->slots + (qd->writerID * qd->numSlots + slot_idx);
	slot_data_ptr = (void *)((unsigned long)(qd->data) + qd->writerID * qd->numSlots * qd->nodeDataSize + slot_idx * qd->nodeDataSize); 
	hwfunctions_memcpy(slot_data_ptr, data, qd->nodeDataSize);
	slot_ptr->id = id;

	hwfunctions_membarrier();

	/* set new tail */
	qd->queues[qd->writerID].tail = (tail + 1) % qd->numSlots;

	return 0;
}


int wfmpscq_dequeue(wfmpscq_d_t *qd, void *data)
{
	int i, head, tail, queue_id, slot_idx;
	unsigned int min_id, id, msg_id;
	void *slot_data_ptr;

	/* determine the queue (variable queue_id) from which the next message should be read */

	if (qd->dqstrat == DQSTR_IDABSOLUTE) {
		/* find the queue which head has the minumum id */
		min_id = MAXID;
		queue_id = -1;
		for (i = 0; i < qd->numWriter; i++) {
			head = qd->queues[i].head;
			tail = qd->queues[i].tail;
			id = (qd->slots + (i * qd->numSlots + head))->id;
			/* queue is not empty and we found new minimum? */
			if (head != tail && id < min_id) {
				/* save new minimum id and the queue */
				min_id = id;
				queue_id = i;
			}
		}
		msg_id = min_id;
	} else if (qd->dqstrat == DQSTR_ROUNDROBIN) {
		static int lastqueue = 0;
		queue_id = -1;
		for (i = 0; i < qd->numWriter; i++) {
			lastqueue = (lastqueue + 1) % qd->numWriter;
			head = qd->queues[lastqueue].head;
			tail = qd->queues[lastqueue].tail;
			
			if (head != tail) {
				queue_id = lastqueue;
				break;
			}	
		}

		msg_id = (qd->slots + (queue_id * qd->numSlots + head))->id;
	} else {
		/* undefined */
		msg_id = -1;
		return -1;
	}
	
	/* all queues are empty? */
	if (queue_id == -1)
		return -1; 

	/* now read the element */
	head = qd->queues[queue_id].head;
	slot_idx = head;

	slot_data_ptr = (void *)((unsigned long)(qd->data) + (queue_id * qd->numSlots * qd->nodeDataSize) + (slot_idx * qd->nodeDataSize)); 
	hwfunctions_memcpy(data, slot_data_ptr, qd->nodeDataSize);

	/* update head */
	qd->queues[queue_id].head = (head + 1) % (qd->numSlots);
	
	return msg_id;
}


int wfmpscq_release(wfmpscq_d_t *qd)
{
	/* deattach shared memory */
        if (shmman_release_shmseg(qd->objID) == -1) {
                perror("shmman_release_shmseg failed\n");
                return -1;
        }

        return 0;
}
 
