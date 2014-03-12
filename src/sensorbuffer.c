/**
 * @file sensorbuffer.c
 * @author Josef Raschen <josef@raschen.org>
 *
 */

#include "sensorbuffer.h"
#include "shmman.h"
#include "hwfunctions.h"
#include "atomic.h"

#include <stdio.h>
#include <string.h>


int sensorbuffer_get(sensorbuffer_d_t *bfd, int objID, int bufferDataSize, int numBuffer) 
{
	int i;
	void *ptr;
	void *b;

	bfd->objID = objID;
	bfd->bufferDataSize = bufferDataSize;
	bfd->numBuffer = numBuffer;

        /* shared memory segment for the buffer */
        if (shmman_get_shmseg(bfd->objID, sizeof(sensorbuffer_head_t) + bfd->numBuffer * (sizeof(sensorbuffer_bufferhead_t) + bfd->bufferDataSize), &ptr) == -1) {
                perror("sensorbuffer: shmman_get_shmseg() failed\n");
                return -1;
        }

	bfd->mem = (sensorbuffer_head_t *)ptr;
	bfd->buffer = (void *)((unsigned long)ptr + sizeof(sensorbuffer_head_t));

	/* get lock */
	while (!atomic_cas32(&(bfd->mem->lock), 0, 1)) {
		hwfunctions_nop();
	}

	if (bfd->mem->init == 0) {
		bfd->mem->last_written = -1;	

		b = bfd->buffer;
		for (i = 0; i < bfd->numBuffer; i++) {
			b = (void *)((unsigned long)b + sizeof(sensorbuffer_bufferhead_t) + bfd->bufferDataSize);
			((sensorbuffer_bufferhead_t *)b)->r = 0;
			((sensorbuffer_bufferhead_t *)b)->v = 0;
		}
		bfd->mem->init = 1;	
	}

	/* release lock */
	bfd->mem->lock = 0;	

        return 0;
}


int sensorbuffer_update(sensorbuffer_d_t *bfd, void *data)
{
	int current;
	sensorbuffer_bufferhead_t *b;

	/* index of buffer to write to */
	current = (bfd->mem->last_written + 1) % bfd->numBuffer;
	
	/* pointer to buffer */
	b = (sensorbuffer_bufferhead_t *)((unsigned long)bfd->buffer + current * (sizeof(sensorbuffer_bufferhead_t) + bfd->bufferDataSize));

	/* reset reader flag and valid flag */
	b->v = 0;
	b->r = 0;

	/* write to buffer */
	hwfunctions_memcpy((void *)((unsigned long)b + sizeof(sensorbuffer_bufferhead_t)), data, bfd->bufferDataSize);
	
	/* set valid flag */
	b->v = 1;

	/* indicate last written buffer */
	bfd->mem->last_written = current;

	return 0;	
}


int sensorbuffer_read(sensorbuffer_d_t *bfd, void *data)
{
	int current;
	sensorbuffer_bufferhead_t *b;

	/* spin until reading was successful */
	while (1) {
		/* get the buffer to read from */
		current = bfd->mem->last_written;

		if (current == -1)
			return -1;

		/* pointer to buffer */
		b = (sensorbuffer_bufferhead_t *)((unsigned long)bfd->buffer + current * (sizeof(sensorbuffer_bufferhead_t) + bfd->bufferDataSize));

		/* set reader flag */	
		b->r = 1;

		/* check valid flag */
		if (b->v != 1)
			continue;

		/* copy data */
		hwfunctions_memcpy(data, (void *)((unsigned long)b + sizeof(sensorbuffer_bufferhead_t)), bfd->bufferDataSize);

		/* check reader flag */
		if (b->r != 1)
			continue;

		break;
	}
	
	return 0;
}


int sensorbuffer_release(sensorbuffer_d_t *bfd)
{
	/* deattach shared memory */
        if (shmman_release_shmseg(bfd->objID) == -1) {
                perror("shmman_release_shmseg failed\n");
                return -1;
        }

        return 0;
}

