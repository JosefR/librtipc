/** 
 * @file hwfunctions.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * some functions with hardware specific implementations
 */

#ifndef _HWFUNCTIONS_H_
#define _HWFUNCTIONS_H_


/** 
 * no operation
 */
inline void hwfunctions_nop();


/** 
 * clear interrupt flag
 */
inline void hwfunctions_cli();


/** 
 * set interrupt flag
 */
inline void hwfunctions_sti();


/**
 * hardware memory barrier
 */ 
inline void hwfunctions_membarrier();


/**
 * simple mempcpy
 */
inline void hwfunctions_memcpy(void *dst, void *src, int size);


/**
 * invalidate a cache region
 *
 * @param ptr pointer to memory region which we want to remove from cache
 * @param size the size (in byte) of the memory region we want to remove from cache
 */
inline void hwfunctions_invalidateCacheRegion(void *ptr, int size);


#endif /* _HWFUNCTIONS_H_ */
