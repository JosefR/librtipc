/** 
 * @file hwfunctions.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include "hwfunctions.h"

/*read from 'cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size'*/
#define CACHE_LINE_SIZE 64

inline void hwfunctions_nop()
{
	asm volatile("nop");
}


inline void hwfunctions_cli()
{
	asm volatile("cli");
}


inline void hwfunctions_sti()
{
	asm volatile("sti");
}

	
inline void hwfunctions_membarrier()
{
	asm volatile("mfence" ::: "memory");
}


/* a really bad performing memcpy */
inline void hwfunctions_memcpy(void *dst, void *src, int size)
{
	char *srcptr, *dstptr;
	int i;

	srcptr = (char *)src;
	dstptr = (char *)dst;
	
	for(i = 0; i < size; i++)
		dstptr[i] = srcptr[i];
}


inline void hwfunctions_invalidateCacheRegion(void *ptr, int size)
{
	int i, cachelines;
	volatile char *cptr;

	cptr = (volatile char *)ptr;

	cachelines = (size / CACHE_LINE_SIZE) + 1; 

	for (i = 0; i < cachelines ; i++) {
		cptr += CACHE_LINE_SIZE;
		asm volatile ( "clflush %0" : "+m" (*cptr));
	}

}

