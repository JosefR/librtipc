/**
 * @file spscq.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include "spscq.h"

#include <stdio.h>
#include <string.h>
#include "shmman.h"
#include "hwfunctions.h"
#include "atomic.h"


int spscq_get(spscq_d_t *qd, int objID, int slotSize, int numSlots) 
{
	void *ptr;

	qd->objID = objID;
	qd->slotSize = slotSize;
	qd->numSlots = numSlots;
	
        /* get shared memory segment for queue */
        if (shmman_get_shmseg(qd->objID, sizeof(spscq_head_t) + qd->numSlots * qd->slotSize, &ptr) == -1) {
                perror("shmman_get_shmseg() failed\n");
                return -1;
        }

	/* set pointer to shared memory segment */
	qd->q = (spscq_head_t *)ptr;
	
	/* get lock */
        while (!atomic_cas32(&(qd->q->lock), 0, 1)) {
                hwfunctions_nop();
	}
         
        /* initialize if we are the first user of flag */
        if (qd->q->init == 0) {

		/* set deault values */
		qd->q->head = 0;
		qd->q->next_slot = 0;
	
		qd->q->init = 1;	
	}

	/* release lock */
	qd->q->lock = 0;

	return 0;	
}


int spscq_enqueue(spscq_d_t *qd, void *data)
{
	void *slot;

	/* check if queue is full */
	if ((qd->q->next_slot + 1) % qd->numSlots == qd->q->head) {
		return -1;
	}
	
	/* get pointer to the next slot and copy the data */
	slot = (void *)((unsigned long)(qd->q) + sizeof(spscq_head_t) + (qd->q->next_slot * (qd->slotSize)));
        hwfunctions_memcpy(slot, data, qd->slotSize);

	/* make sure next_slot is not updated before memcpy was finished */
	hwfunctions_membarrier();

	/* set the next free slot */
        qd->q->next_slot = (qd->q->next_slot + 1) % qd->numSlots;
		
	return 0;
}


int spscq_dequeue(spscq_d_t *qd, void *data)
{
	void *slot;

        /* check if queue is empty */
        if (qd->q->head == qd->q->next_slot) {
                return -1;
	}

	/* get pointer to the slot and copy data to buffer*/
        slot = (void *)((long int)(qd->q) + sizeof(spscq_head_t) + ((qd->q->head) * (qd->slotSize)));
        hwfunctions_memcpy(data, slot, qd->slotSize);

	/* make sure head is not updated before memcpy was finished */
	hwfunctions_membarrier();

	/* set new head */
        qd->q->head = (qd->q->head + 1) % qd->numSlots;

	return 0;
}


int spscq_release(spscq_d_t *qd)
{
	if (shmman_release_shmseg(qd->objID) == -1) {
                perror("shmman_release_shmseg() failed\n");
                return -1;
        }

	return 0;
}


