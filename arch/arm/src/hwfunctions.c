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
	asm volatile("cpsid if");
}


inline void hwfunctions_sti()
{
	asm volatile("cpsie if");
}


inline void hwfunctions_membarrier()
{
	asm volatile("dmb" ::: "memory");
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
