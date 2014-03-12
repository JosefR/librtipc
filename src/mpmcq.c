/**
 * @file mpmcq.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include "mpmcq.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shmman.h"
#include "hwfunctions.h"
#include "atomic.h"

#define UNDEFINED_NODE (-1)
#define NUM_SHAREDIDX (2)

int *RList = NULL; /* local retire list */
static int RCnt = 0;
int *TmpList = NULL; /* helper list for retire function */


int getFreeSlot(mpmcq_d_t *qd)
{
	int cnt;
	void *s;
	static int i = 0;

	cnt = 0;
	while (cnt < qd->numSlots) {
		s = (void *)((unsigned long)(qd->slots) + i * (sizeof(mpmcq_slot_head_t) + qd->nodeDataSize));
		if (atomic_cas32(&(((mpmcq_slot_head_t *)s)->used), 0, 1))
			return i;			
		i = (i + 1) % qd->numSlots;
		cnt++;
	}
	
	return -1;
}


int mpmcq_get(mpmcq_d_t *qd, int objID, int userID, int nodeDataSize, int numSlots, int maxUser) 
{
	int i;
	void *ptr;
	int *idx;
	unsigned long s;

	qd->objID = objID;
	qd->userID = userID;
	qd->nodeDataSize = nodeDataSize;
	qd->numSlots = numSlots;
	qd->maxUser = maxUser;

        /* get shared memory segment for queue */
        if (shmman_get_shmseg(qd->objID, sizeof(mpmcq_head_t) + NUM_SHAREDIDX * qd->maxUser * sizeof(int) + qd->numSlots * (qd->nodeDataSize + sizeof(mpmcq_slot_head_t)), &ptr) == -1) {
                perror("shmman_get_shmseg() failed\n");
                return -1;
        }

	/* set pointer to shared memory segment */
	qd->q = (mpmcq_head_t *)ptr;
	qd->sharedIdxs = (int *)((unsigned long)ptr + sizeof(mpmcq_head_t));
	qd->slots = (void *)((unsigned long)ptr + sizeof(mpmcq_head_t) + NUM_SHAREDIDX * qd->maxUser * sizeof(int));
	
	/* get lock */
        while (!atomic_cas32(&(qd->q->lock), 0, 1)) {
                hwfunctions_nop();
	}
         
        /* initialize if we are the first user of queue */
        if (qd->q->init == 0) {

		/* init shared indexes */
		idx = (int *)(qd->sharedIdxs);
		for (i = 0; i < (NUM_SHAREDIDX * qd->maxUser); i++) {
			*idx = UNDEFINED_NODE;
			idx++;
		}

		/* init slots */
		s = (unsigned long)(qd->slots);
		((mpmcq_slot_head_t *)s)->used = 1; /* dummy */
		((mpmcq_slot_head_t *)s)->next = UNDEFINED_NODE; /* dummy */
		for (i = 1; i < qd->numSlots; i++) {
			s += qd->nodeDataSize + sizeof(mpmcq_slot_head_t);
			((mpmcq_slot_head_t *)s)->used = 0;
			((mpmcq_slot_head_t *)s)->next = UNDEFINED_NODE;
		}

		/* init shared head/tail */
		qd->q->head = 0;
		qd->q->tail = 0;
	
		qd->q->init = 1;	
	}

	/* init local data if necessary */
	if (qd->init == 0) {
		RList = (int *)malloc(qd->numSlots * sizeof(int));
		RCnt = 0;
		TmpList = (int *)malloc(qd->numSlots * sizeof(int));
		qd->init = 1;
	}

	/* release lock */
	qd->q->lock = 0;

	return 0;	
}


void TryRetire(mpmcq_d_t *qd)
{
	int i, k, tmpcnt;
	mpmcq_slot_head_t *node_ptr;

	/* copy retire list */
	for (i = 0; i < RCnt; i++)
		TmpList[i] = RList[i];
	tmpcnt = RCnt;

	/* delete nodes from RList if they do not exist in any sharedIdx */
	RCnt = 0;
	for (i = 0; i < tmpcnt; i++) {
		for (k = 0; k < (NUM_SHAREDIDX * qd->maxUser); k++) {
			if ((qd->sharedIdxs)[k] == TmpList[i]) {
				RList[RCnt++] = TmpList[i];
				break;
			}
		}
		if (k == (NUM_SHAREDIDX * qd->maxUser)) {
			/* delete */
			node_ptr = (void *)((unsigned long)(qd->slots) + TmpList[i] * (sizeof(mpmcq_slot_head_t) + qd->nodeDataSize));
			node_ptr->next = UNDEFINED_NODE;
			node_ptr->used = 0; /* allow reuse of slot */
		}
	}
}


void RetireNode(mpmcq_d_t *qd, int nodeIdx)
{
	/* add node to retire list */
	RList[RCnt++] = nodeIdx;

	TryRetire(qd);
}


int mpmcq_enqueue(mpmcq_d_t *qd, void *data)
{
	int node_idx, t_idx, t_next;
	mpmcq_slot_head_t  *node_ptr, *t_ptr;
	int *shared_idx;

	/* try to free unused slots */
	TryRetire(qd);

	/* try to get a free slot for the new node */
	if ((node_idx = getFreeSlot(qd)) == -1)
		return -1;

	/* set up new node */
	node_ptr = (void *)((unsigned long)(qd->slots) + node_idx * (sizeof(mpmcq_slot_head_t) + qd->nodeDataSize));
	hwfunctions_memcpy((void *)((unsigned long)node_ptr + sizeof(mpmcq_slot_head_t)), data, qd->nodeDataSize);
	node_ptr->next = UNDEFINED_NODE;
	
	/* set pointer to shared node index */
	shared_idx = (int *)(qd->sharedIdxs + NUM_SHAREDIDX * qd->userID);

	/* now enqueue */
	while(1) {
		/* read tail */
		t_idx = qd->q->tail;

		/* set shared index to protect the node from reuse until this was allowed explicitely */
		*(shared_idx) = t_idx;
		
		/* make sure the index saved in shared_idx was a valid index of tail at some time beween the previous line and now */
		if(qd->q->tail != t_idx)
			continue;

		/* read the next-index of the node we assume as tail */
		t_ptr = (mpmcq_slot_head_t *)((unsigned long)(qd->slots) + t_idx * (sizeof(mpmcq_slot_head_t) + qd->nodeDataSize));
		t_next = t_ptr->next;
		
		/* if tail (or what we assume to be tail) is not pointing to last node in list, 
		 * try to update tail and start over 
		 */
		if(t_next != UNDEFINED_NODE) {
			atomic_cas32(&(qd->q->tail), t_idx, t_next);
			continue;
		}

		/* try to append the new node */
		if(atomic_cas32(&(t_ptr->next), UNDEFINED_NODE, node_idx))
			break;
	}

	/* try to update tail; if not successfull, the next enqueue will do this for us */
	atomic_cas32(&(qd->q->tail), t_idx, node_idx);

	return 0;
}


int mpmcq_dequeue(mpmcq_d_t *qd, void *data)
{
	int h_idx, t_idx, h_next_idx;
	mpmcq_slot_head_t *h_ptr, *h_next_ptr;
	int *shared_idx0, *shared_idx1;
	
	/* set pointer to shared node indexes */
	shared_idx0 = (int *)(qd->sharedIdxs + (NUM_SHAREDIDX * qd->userID));
	shared_idx1 = (int *)(qd->sharedIdxs + (NUM_SHAREDIDX * qd->userID + 1));

	/* try to free unsused slots */
	TryRetire(qd);

	while (1) {
		/* get current head */
		h_idx = qd->q->head;

		/* save head in first shared idx to protect the node */
		*shared_idx0 = h_idx;
		
		/* make sure the saved index really was a valid head when it was saved */
		if (qd->q->head != h_idx)
			continue;

		/* get tail */
		t_idx = qd->q->tail;

		/* get the index of next node from what we assume is head */
		h_ptr = (void *)((unsigned long)(qd->slots) + h_idx * (sizeof(mpmcq_slot_head_t) + qd->nodeDataSize));
		h_next_idx = h_ptr->next;

		/* save next in shared pointer */
		*shared_idx1 = h_next_idx;

		/* make sure the index saved in shared_idx1 is read from a valid head */
		if (qd->q->head != h_idx)
			continue;
		
		/* check if there is at least one node we can read */
		if (h_next_idx == UNDEFINED_NODE)
			return -1;
		
		/* try to correct tail if it seems to point to the dummy node */
		if (h_idx == t_idx) {
			atomic_cas32(&(qd->q->tail), t_idx, h_next_idx);
			continue;
		}
			
		/* try to update head */
		if (atomic_cas32(&(qd->q->head), h_idx, h_next_idx))
			break;
	}

	/* read data from next (safe befaues node wont be reused until RetireNode has been called */
	h_next_ptr = (mpmcq_slot_head_t *)((unsigned long)(qd->slots) + h_next_idx * (sizeof(mpmcq_slot_head_t) + qd->nodeDataSize));
	hwfunctions_memcpy(data, (void *)((unsigned long)h_next_ptr + sizeof(mpmcq_slot_head_t)), qd->nodeDataSize);

	/* add old head node to retire list and try to delete retired nodes */
	RetireNode(qd, h_idx);

	return 0;
}


int mpmcq_release(mpmcq_d_t *qd)
{
	if (shmman_release_shmseg(qd->objID) == -1) {
                perror("shmman_release_shmseg() failed\n");
                return -1;
        }

	return 0;
}

