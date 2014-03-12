/**
 * @file rdtsc.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * functions for using the rdtsc instruction
 */

#ifndef __RDTSC_H__
#define __RDTSC_H__

unsigned long long read_tsc() 
{
	unsigned int l, h;
	unsigned long long result; 

	asm volatile (
		"rdtsc  \n"
		: "=a" (l), "=d" (h)
	);

	result = (unsigned long long)l | ((unsigned long long)h << 32);

	return result;		
}

#endif
