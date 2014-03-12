/**
 * @file lfmpscq.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include "lfmpscq.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shmman.h"
#include "hwfunctions.h"
#include "atomic.h"

#define UNDEFINED_NODE (-1)

int *RList = NULL; /* local retire list */
int RCnt = 0;
int *TmpList = NULL; /* helper list for retire function */


int getFreeSlot(lfmpscq_d_t *qd)
{
	int cnt;
	void *s;
	static int i = 0;

	cnt = 0;
	while(cnt < qd->numSlots) {
		s = (void *)((unsigned long)(qd->slots) + i * (sizeof(lfmpscq_slot_head_t) + qd->nodeDataSize));
		if (atomic_cas32(&(((lfmpscq_slot_head_t *)s)->used), 0, 1))
			return i;			
		i = (i + 1) % qd->numSlots;
		cnt++;
	}
	
	return -1;
}


int lfmpscq_get(lfmpscq_d_t *qd, int objID, int userID, int nodeDataSize, int numSlots, int maxUser, int init) 
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
        if (shmman_get_shmseg(qd->objID, sizeof(lfmpscq_head_t) + qd->maxUser * sizeof(int) + qd->numSlots * (qd->nodeDataSize + sizeof(lfmpscq_slot_head_t)), &ptr) == -1) {
                perror("shmman_get_shmseg() failed\n");
                return -1;
        }

	/* set pointer to shared memory segment */
	qd->q = (lfmpscq_head_t *)ptr;
	qd->sharedIdxs = (int *)((unsigned long)ptr + sizeof(lfmpscq_head_t));
	qd->slots = (void *)((unsigned long)ptr + sizeof(lfmpscq_head_t) + qd->maxUser * sizeof(int));
	
	/* get lock */
        while (!atomic_cas32(&(qd->q->lock), 0, 1)) {
                hwfunctions_nop();
	}
         
        /* initialize if we are the first user of queue */
        if (qd->q->init == 0) {

		/* init shared indexes */
		idx = (int *)(qd->sharedIdxs);
		for (i = 0; i < qd->maxUser; i++) {
			*idx = UNDEFINED_NODE;
			idx++;
		}

		/* init slots */
		s = (unsigned long)(qd->slots);
		((lfmpscq_slot_head_t *)s)->used = 1; /* dummy */
		((lfmpscq_slot_head_t *)s)->next = UNDEFINED_NODE; /* dummy */
		for (i = 1; i < qd->numSlots; i++) {
			s += qd->nodeDataSize + sizeof(lfmpscq_slot_head_t);
			((lfmpscq_slot_head_t *)s)->used = 0;
			((lfmpscq_slot_head_t *)s)->next = UNDEFINED_NODE;
		}

		/* init shared head/tail */
		qd->q->head = 0;
		qd->q->tail = 0;
	
		qd->q->init = 1;	
	}

	/* init local data if necessary */
	if (init == 1) {
		RList = (int *)malloc(qd->numSlots * sizeof(int));
		TmpList = (int *)malloc(qd->numSlots * sizeof(int));
	}

	/* release lock */
	qd->q->lock = 0;

	return 0;	
}


int lfmpscq_enqueue(lfmpscq_d_t *qd, void *data)
{
	int node_idx, t, t_next;
	lfmpscq_slot_head_t  *node_ptr, *t_ptr;
	int *shared_idx;

	/* try to get a free slot for the new node */
	if ((node_idx = getFreeSlot(qd)) == -1)
		return -1;

	/* set up new node */
	node_ptr = (void *)((unsigned long)(qd->slots) + node_idx * (sizeof(lfmpscq_slot_head_t) + qd->nodeDataSize));
	hwfunctions_memcpy((void *)((unsigned long)node_ptr + sizeof(lfmpscq_slot_head_t)), data, qd->nodeDataSize);
	node_ptr->next = UNDEFINED_NODE;
	
	/* set pointer to shared node index */
	shared_idx = (int *)(qd->sharedIdxs + qd->userID);

	/* now enqueue */
	while(1) {
		/* read tail */
		t = qd->q->tail;

		/* set shared index to protect the node from reuse until this was allowed explicitely */
		*(shared_idx) = t;
		
		/* make sure the index saved in shared_idx was a valid index of tail at some time beween the previous line and now */
		if(qd->q->tail != t)
			continue;

		/* read the next-index of the node we assume as tail */
		t_ptr = (lfmpscq_slot_head_t *)((unsigned long)(qd->slots) + t * (sizeof(lfmpscq_slot_head_t) + qd->nodeDataSize));
		t_next = t_ptr->next;
		
		/* if tail (or what we assume to be tail) is not pointing to last node in list, 
		 * try to update tail and start over 
		 */
		if(t_next != UNDEFINED_NODE) {
			atomic_cas32(&(qd->q->tail), t, t_next);
			continue;
		}

		/* try to append the new node */
		if(atomic_cas32(&(t_ptr->next), UNDEFINED_NODE, node_idx))
			break;
	}

	/* try to update tail; if not successfull, the next enqueue will do this for us */
	atomic_cas32(&(qd->q->tail), t, node_idx);

	return 0;
}


void RetireNode(lfmpscq_d_t *qd, int nodeIdx)
{
	int i, k, tmpcnt;
	lfmpscq_slot_head_t *node_ptr;

	/* add node to retire list */
	RList[RCnt++] = nodeIdx;
	
	/* copy retire list */
	for (i = 0; i < RCnt; i++)
		TmpList[i] = RList[i];
	tmpcnt = RCnt;

	/* delete nodes from RList if they do not exist in any sharedIdx */
	RCnt = 0;
	for (i = 0; i < tmpcnt; i++) {
		for (k = 0; k < qd->maxUser; k++) {
			if ((qd->sharedIdxs)[k] == TmpList[i]) {
				RList[RCnt++] = TmpList[i];
				break;
			}
		}
		
		if (k == qd->maxUser) {
			/* delete */
			node_ptr = (void *)((unsigned long)(qd->slots) + TmpList[i] * (sizeof(lfmpscq_slot_head_t) + qd->nodeDataSize));
			node_ptr->next = UNDEFINED_NODE;
			node_ptr->used = 0; /* allow reuse of slot */
		}
	}
}


int lfmpscq_dequeue(lfmpscq_d_t *qd, void *data)
{
	int h_idx, h_next;
	lfmpscq_slot_head_t *h_ptr, *next_ptr;
	
	/* get current head */
	h_idx = qd->q->head;
	h_ptr = (void *)((unsigned long)(qd->slots) + h_idx * (sizeof(lfmpscq_slot_head_t) + qd->nodeDataSize));
	h_next = h_ptr->next;
	
	/* check if the selected the dummy node (queue is empty) */
	if (h_next == UNDEFINED_NODE)
		return -1;

	/* read data from next */
	next_ptr = (lfmpscq_slot_head_t *)((unsigned long)(qd->slots) + h_next * (sizeof(lfmpscq_slot_head_t) + qd->nodeDataSize));
	hwfunctions_memcpy(data, (void *)((unsigned long)next_ptr + sizeof(lfmpscq_slot_head_t)), qd->nodeDataSize);

	/* set new head */
	qd->q->head = h_next;

	/* add node to retire list and try to delete retired nodes */
	RetireNode(qd, h_idx);

	return 0;
}


int lfmpscq_release(lfmpscq_d_t *qd)
{
	if (shmman_release_shmseg(qd->objID) == -1) {
                perror("shmman_release_shmseg() failed\n");
                return -1;
        }

	return 0;
}

