/** 
 * @file atomic.c
 * @author Josef Raschen <josef@raschen.org>
 *
 */

#include <atomic.h>


inline int atomic_cas8(volatile signed char *ptr, char signed oldval, signed char newval)
{
	char ret;

	asm volatile (
		"   lock                 \n"
		"   cmpxchgb %4, %0      \n"
		"   setz %1              \n"
		: "=m" (*ptr), "=a" (ret)
		: "m" (*ptr), "a" (oldval), "r" (newval)
		: "memory"
	);

	return (int)ret;
}


inline int atomic_cas8u(volatile unsigned char *ptr, unsigned char oldval, unsigned char newval)
{
	char ret;

	asm volatile (
		"   lock                 \n"
		"   cmpxchgb %4, %0      \n"
		"   setz %1              \n"
		: "=m" (*ptr), "=a" (ret)
		: "m" (*ptr), "a" (oldval), "r" (newval)
		: "memory"
	);

	return (int)ret;
}


inline int atomic_cas16(volatile short *ptr, short oldval, short newval)
{
	char ret;

	asm volatile (
		"   lock                 \n"
		"   cmpxchgw %4, %0      \n"
		"   setz %1              \n"
		: "=m" (*ptr), "=a" (ret)
		: "m" (*ptr), "a" (oldval), "r" (newval)
		: "memory"
	);

	return (int)ret;
}


inline int atomic_cas16u(volatile unsigned short *ptr, unsigned short oldval, unsigned short newval)
{
	char ret;

	asm volatile (
		"   lock                 \n"
		"   cmpxchgw %4, %0      \n"
		"   setz %1              \n"
		: "=m" (*ptr), "=a" (ret)
		: "m" (*ptr), "a" (oldval), "r" (newval)
		: "memory"
	);

	return (int)ret;
}


inline int atomic_cas32(volatile int *ptr, int oldval, int newval)
{
	char ret;

	asm volatile (
		"   lock                 \n"
		"   cmpxchgl %4, %0      \n"
		"   setz %1              \n"
		: "=m" (*ptr), "=a" (ret)
		: "m" (*ptr), "a" (oldval), "r" (newval)
		: "memory"
	);

	return (int)ret;
}


inline int atomic_cas32u(volatile unsigned int *ptr, unsigned int oldval, unsigned int newval)
{
	char ret;

	asm volatile (
		"   lock                 \n"
		"   cmpxchgl %4, %0      \n"
		"   setz %1              \n"
		: "=m" (*ptr), "=a" (ret)
		: "m" (*ptr), "a" (oldval), "r" (newval)
		: "memory"
	);

	return (int)ret;
}


inline int atomic_cas64(volatile long long *ptr, long long oldval, long long newval)
{
	char ret;
	unsigned int oldval0, oldval1, newval0, newval1;
	oldval0 = oldval & 0x00000000ffffffff;
	oldval1 = oldval >> 32;
	newval0 = newval & 0x00000000ffffffff;
	newval1 = newval >> 32;

	asm volatile (
		"   lock		\n"
		"   cmpxchg8b %0	\n"
		"   setz %1		\n"
		: "=m" (*ptr), "=a" (ret) 
		: "m" (*ptr), "a" (oldval0), "d" (oldval1), "b" (newval0), "c" (newval1)
		: "memory"
	);

	return (int)ret;
}


inline int atomic_cas64u(volatile unsigned long long *ptr, unsigned long long oldval, unsigned long long newval)
{
	char ret;
	unsigned int oldval0, oldval1, newval0, newval1;
	oldval0 = oldval & 0x00000000ffffffff;
	oldval1 = oldval >> 32;
	newval0 = newval & 0x00000000ffffffff;
	newval1 = newval >> 32;

	asm volatile (
		"   lock		\n"
		"   cmpxchg8b %0	\n"
		"   setz %1		\n"
		: "=m" (*ptr), "=a" (ret) 
		: "m" (*ptr), "a" (oldval0), "d" (oldval1), "b" (newval0), "c" (newval1)
		: "memory"
	);

	return (int)ret;
}


inline signed char atomic_faa8(volatile signed char *ptr, signed char val)
{
	signed char ret;

	asm volatile (
		"   lock                  \n"
		"   xaddb %%al, %2       \n"
		: "=a" (ret)
		: "a" (val), "m" (*ptr)
		: "memory" 
	);

        return ret;
}


inline unsigned char atomic_faa8u(volatile unsigned char *ptr, unsigned char val)
{
	unsigned char ret;

	asm volatile (
		"   lock                  \n"
		"   xaddb %%al, %2       \n"
		: "=a" (ret)
		: "a" (val), "m" (*ptr)
		: "memory" 
	);

	return ret;
}


inline short atomic_faa16(volatile short *ptr, short val)
{
	short ret;

	asm volatile (
		"   lock                  \n"
		"   xaddw %%ax, %2       \n"
		: "=a" (ret)
		: "a" (val), "m" (*ptr)
		: "memory" 
	);

        return ret;
}


inline unsigned short atomic_faa16u(volatile unsigned short *ptr, unsigned short val)
{
	unsigned short ret;

	asm volatile (
		"   lock                  \n"
		"   xaddw %%ax, %2       \n"
		: "=a" (ret)
		: "a" (val), "m" (*ptr)
		: "memory" 
	);

        return ret;
}


inline int atomic_faa32(volatile int *ptr, int val)
{
	int ret;

	asm volatile (
		"   lock                  \n"
		"   xaddl %%eax, %2       \n"
		: "=a" (ret)
		: "a" (val), "m" (*ptr)
		: "memory" 
	);

        return ret;
}


inline unsigned int atomic_faa32u(volatile unsigned int *ptr, unsigned int val)
{
	unsigned int ret;

	asm volatile (
		"   lock                  \n"
		"   xaddl %%eax, %2       \n"
		: "=a" (ret)
		: "a" (val), "m" (*ptr)
		: "memory" 
	);

        return ret;
}


inline long long atomic_faa64(volatile long long *ptr, long long val)
{
#ifdef __AMD64__
	long long ret;

	asm volatile (
		"   lock                  \n"
		"   xaddq %%rax, %2       \n"
		: "=a" (ret)
		: "a" (val), "m" (*ptr)
		: "memory" 
	);

        return ret;
#else
	return 0;
#endif
}


inline unsigned long long atomic_faa64u(volatile unsigned long long *ptr, unsigned long long val)
{
#ifdef __AMD64__
	unsigned long long ret;

	asm volatile (
		"   lock                  \n"
		"   xaddq %%rax, %2       \n"
		: "=a" (ret)
		: "a" (val), "m" (*ptr)
		: "memory" 
	);

        return ret;
#else
	return 0;
#endif
}


