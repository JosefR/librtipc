/**
 * @file sensorbuffer.h
 * @author Josef Raschen <josef@raschen.org>
 *
 */

#ifndef _SENSORBUFFER_H_
#define _SENSORBUFFER_H_

#include <stdint.h>


/**
 * shared variables of the buffer
 */
typedef struct sensorbuffer_head {
	volatile int lock;
	volatile int init;
	volatile int last_written; /* idx of the last written buffer */
} sensorbuffer_head_t;


/**
 * head of each buffer
 */
typedef struct sensorbuffer_bufferhead {
	volatile int r; /* 1: reader is reading or has read from buffer and writer did not modify it */
	volatile int v; /* 1: data is valid; 0: data is invalid */
} sensorbuffer_bufferhead_t;


/**
 * descriptor of the sensorbuffer object
 */
typedef struct sensorbuffer_d {
	int objID; 
	sensorbuffer_head_t *mem; /* pointer to the shared memory */
	void *buffer; /* pointer to the buffers */
	int bufferDataSize; /* size of a buffer in byte (without sensorbuffer_bufferhead) */
	int numBuffer; /* the number of buffers which are available */
} sensorbuffer_d_t;


/**
 * get/create sensorbuffer object
 *
 * @param bfd descriptor of the sensorbuffer
 * @return 0 on success, -1 on fail
 */
int sensorbuffer_get(sensorbuffer_d_t *bfd, int objID, int bufferDataSize, int numBuffer);


/**
 * update the content of the buffer with a new data
 *
 * @param bfd descriptor of the buffer
 * @param data pointer to the data to be copied to the buffer
 * @return 
 */
int sensorbuffer_update(sensorbuffer_d_t *bfd, void *data);


/**
 * read data from the buffer
 *
 * @param bfd descriptor of the buffer
 * @param data pointer to the target in which the data read from the buffer should be copied to
 * @return 
 */
int sensorbuffer_read(sensorbuffer_d_t *bfd, void *data);


/**
 * deattach sensorbuffer
 *
 * @param bfd descriptor of the buffer
 * @return 0 on success, -1 on fail
 */
int sensorbuffer_release(sensorbuffer_d_t *bfd);


#endif
