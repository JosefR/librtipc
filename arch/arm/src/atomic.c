/** 
 * @file atomic.c
 * @author Josef Raschen <josef@raschen.org>
 *
 * atomic operations for ARMv7 achitecture
 */

#include <atomic.h>


inline int atomic_cas8(volatile signed char *ptr, signed char oldval, signed char newval)
{
	int tmp, tmpret, ret;

	asm volatile (
		"	mov %[tmpret], #0			\n" /* temporary return value in tmpret */ 
		"1: 	ldrexb %[tmp], [%[ptr]]           	\n" /* load old value and set exclusive flag */
		"	teq %[oldval], %[tmp]			\n" /* compare the old value and the one given by oldval */
		"	bne 2f					\n" /* if not equal: CAS fails and returns 0 */
		"	strexb %[tmp], %[newval], [%[ptr]] 	\n" /* try to update the value */
		"	teq %[tmp], #1				\n"
		"	beq 1b					\n" /* store failed for some reason: retry */
		"	mov %[tmpret], #1			\n" /* return 1 because CAS was successful */
		"2:	mov %[ret], %[tmpret]			\n" /* return value in tmpret */
		: [ret]"=&r" (ret), [ptr]"+r" (ptr), [tmp]"=&r" (tmp), [tmpret]"=&r" (tmpret)
		: [oldval]"r" (oldval), [newval]"r" (newval)
		: "cc", "memory"
	);

	return ret;
}


inline int atomic_cas8u(volatile unsigned char *ptr, unsigned char oldval, unsigned char newval)
{
	int tmp, tmpret, ret;

	asm volatile (
		"	mov %[tmpret], #0			\n" /* temporary return value in tmpret */
		"1: 	ldrexb %[tmp], [%[ptr]]           	\n" /* load old value and set exclusive flag */
		"	teq %[oldval], %[tmp]			\n" /* compare the old value and the one given by oldval */
		"	bne 2f					\n" /* if not equal: CAS fails and returns 0 */
		"	strexb %[tmp], %[newval], [%[ptr]] 	\n" /* try to update the value */
		"	teq %[tmp], #1				\n" 
		"	beq 1b					\n" /* store failed for some reason: retry */
		"	mov %[tmpret], #1			\n" /* return 1 because CAS was successful */
		"2:	mov %[ret], %[tmpret]			\n" /* return value in tmpret */
		: [ret]"=&r" (ret), [ptr]"+r" (ptr), [tmp]"=&r" (tmp), [tmpret]"=&r" (tmpret)
		: [oldval]"r" (oldval), [newval]"r" (newval)
		: "cc", "memory"
	);

	return ret;
}


inline int atomic_cas16(volatile short *ptr, short oldval, short newval)
{
	int tmp, tmpret, ret;

	asm volatile (
		"	mov %[tmpret], #0			\n"
		"1: 	ldrexh %[tmp], [%[ptr]]           	\n"
		"	teq %[oldval], %[tmp]			\n" 
		"	bne 2f					\n" 
		"	strexh %[tmp], %[newval], [%[ptr]] 	\n" 
		"	teq %[tmp], #1				\n"
		"	beq 1b					\n" 
		"	mov %[tmpret], #1			\n" 
		"2:	mov %[ret], %[tmpret]			\n" 
		: [ret]"=&r" (ret), [ptr]"+r" (ptr), [tmp]"=&r" (tmp), [tmpret]"=&r" (tmpret)
		: [oldval]"r" (oldval), [newval]"r" (newval)
		: "cc", "memory"
	);

	return ret;
}


inline int atomic_cas16u(volatile unsigned short *ptr, unsigned short oldval, unsigned short newval)
{
	int tmp, tmpret, ret;

	asm volatile (
		"	mov %[tmpret], #0			\n" /* temporary return value in tmpret */ 
		"1: 	ldrexh %[tmp], [%[ptr]]           	\n" /* load old value and set exclusive flag */
		"	teq %[oldval], %[tmp]			\n" /* compare the old value and the one given by oldval */
		"	bne 2f					\n" /* if not equal: CAS fails and returns 0 */
		"	strexh %[tmp], %[newval], [%[ptr]] 	\n" /* try to update the value */
		"	teq %[tmp], #1				\n"
		"	beq 1b					\n" /* store failed for some reason: retry */
		"	mov %[tmpret], #1			\n" /* return 1 because CAS was successful */
		"2:	mov %[ret], %[tmpret]			\n" /* return value in tmpret */
		: [ret]"=&r" (ret), [ptr]"+r" (ptr), [tmp]"=&r" (tmp), [tmpret]"=&r" (tmpret)
		: [oldval]"r" (oldval), [newval]"r" (newval)
		: "cc", "memory"
	);

	return ret;
}


inline int atomic_cas32(volatile int *ptr, int oldval, int newval)
{
	int tmp, tmpret, ret;

	asm volatile (
		"	mov %[tmpret], #0			\n" /* temporary return value in tmpret */ 
		"1: 	ldrex %[tmp], [%[ptr]]           	\n" /* load old value and set exclusive flag */
		"	teq %[oldval], %[tmp]			\n" /* compare the old value and the one given by oldval */
		"	bne 2f					\n" /* if not equal: CAS fails and returns 0 */
		"	strex %[tmp], %[newval], [%[ptr]] 	\n" /* try to update the value */
		"	teq %[tmp], #1				\n"
		"	beq 1b					\n" /* store failed for some reason: retry */
		"	mov %[tmpret], #1			\n" /* return 1 because CAS was successful */
		"2:	mov %[ret], %[tmpret]			\n" /* return value in tmpret */
		: [ret]"=&r" (ret), [ptr]"+r" (ptr), [tmp]"=&r" (tmp), [tmpret]"=&r" (tmpret)
		: [oldval]"r" (oldval), [newval]"r" (newval)
		: "cc", "memory"
	);

	return ret;
}


inline int atomic_cas32u(volatile unsigned int *ptr, unsigned int oldval, unsigned int newval)
{
	int tmp, tmpret, ret;

	asm volatile (
		"	mov %[tmpret], #0			\n" /* temporary return value in tmpret */ 
		"1: 	ldrex %[tmp], [%[ptr]]           	\n" /* load old value and set exclusive flag */
		"	teq %[oldval], %[tmp]			\n" /* compare the old value and the one given by oldval */
		"	bne 2f					\n" /* if not equal: CAS fails and returns 0 */
		"	strex %[tmp], %[newval], [%[ptr]] 	\n" /* try to update the value */
		"	teq %[tmp], #1				\n"
		"	beq 1b					\n" /* store failed for some reason: retry */
		"	mov %[tmpret], #1			\n" /* return 1 because CAS was successful */
		"2:	mov %[ret], %[tmpret]			\n" /* return value in tmpret */
		: [ret]"=&r" (ret), [ptr]"+r" (ptr), [tmp]"=&r" (tmp), [tmpret]"=&r" (tmpret)
		: [oldval]"r" (oldval), [newval]"r" (newval)
		: "cc", "memory"
	);

	return ret;
}


inline int atomic_cas64(volatile long long *ptr, long long oldval, long long newval)
{
	int tmp0, tmp1, tmpret, ret;

	asm volatile (
		"	mov %[tmpret], #0				\n" /* temporary return value in tmpret */ 
		"1: 	ldrexd %[tmp0], %[tmp1], [%[ptr]]        	\n" /* load old value and set exclusive flag */
		"	teq %[oldval0], %[tmp0]				\n" /* tmp[0..32] == oldval[0..32] ? */
		"	bne 2f						\n" /* if not equal: CAS fails and returns 0 */
		"	teq %[oldval1], %[tmp1]				\n" /* tmp[0..32] == oldval[0..32] ? */
		"	bne 2f						\n" /* if not equal: CAS fails and returns 0 */
		"	strexd %[tmp0], %[newval0], %[newval1], [%[ptr]]\n" /* try to update the value */
		"	teq %[tmp0], #1					\n" /* test if store failed */
		"	beq 1b						\n" /* store failed for some reason: retry */
		"	mov %[tmpret], #1				\n" /* return 1 because CAS was successful */
		"2:	mov %[ret], %[tmpret]				\n" /* return value in tmpret */
		: [ret]"=&r" (ret), [ptr]"+r" (ptr), [tmp0]"=&r" (tmp0), [tmp1]"=&r" (tmp1), [tmpret]"=&r" (tmpret)
		: [oldval0]"r" ((unsigned int)(oldval & 0x00000000ffffffff)), [oldval1]"r" ((unsigned int)(oldval >> 32)), [newval0]"r" ((unsigned int)(oldval & 0x00000000ffffffff)), [newval1]"r" ((unsigned int)(newval >> 32))
		: "cc", "memory"
	);

	return ret;
}


inline int atomic_cas64u(volatile unsigned long long *ptr, unsigned long long oldval, unsigned long long newval)
{
	int tmp0, tmp1, tmpret, ret;

	asm volatile (
		"	mov %[tmpret], #0				\n" /* temporary return value in tmpret */ 
		"1: 	ldrexd %[tmp0], %[tmp1], [%[ptr]]        	\n" /* load old value and set exclusive flag */
		"	teq %[oldval0], %[tmp0]				\n" /* tmp[0..32] == oldval[0..32] ? */
		"	bne 2f						\n" /* if not equal: CAS fails and returns 0 */
		"	teq %[oldval1], %[tmp1]				\n" /* tmp[0..32] == oldval[0..32] ? */
		"	bne 2f						\n" /* if not equal: CAS fails and returns 0 */
		"	strexd %[tmp0], %[newval0], %[newval1], [%[ptr]]\n" /* try to update the value */
		"	teq %[tmp0], #1					\n" /* test if store failed */
		"	beq 1b						\n" /* store failed for some reason: retry */
		"	mov %[tmpret], #1				\n" /* return 1 because CAS was successful */
		"2:	mov %[ret], %[tmpret]				\n" /* return value in tmpret */
		: [ret]"=&r" (ret), [ptr]"+r" (ptr), [tmp0]"=&r" (tmp0), [tmp1]"=&r" (tmp1), [tmpret]"=&r" (tmpret)
		: [oldval0]"r" ((unsigned int)(oldval & 0x00000000ffffffff)), [oldval1]"r" ((unsigned int)(oldval >> 32)), [newval0]"r" ((unsigned int)(oldval & 0x00000000ffffffff)), [newval1]"r" ((unsigned int)(newval >> 32))
		: "cc", "memory"
	);

	return ret;
}


inline signed char atomic_faa8(volatile signed char *ptr, signed char val)
{
	signed char ret, tmp1, tmp2;

	asm volatile (
		"1:	ldrexb %[ret], [%[ptr]]			\n" /* read old value and set exclusive flag */
		"	add %[tmp1], %[ret], %[val] 		\n" /* tmp = oldval + val */
		"	strexb %[tmp2], %[tmp1], [%[ptr]]	\n" /* try to update the value in memory */
		"	teq %[tmp2], #1				\n"
		"	beq 1b					\n" /* if tmp2 == 1 branch to 1 */
		: [ret]"=&r" (ret), [tmp1]"=&r" (tmp1), [tmp2]"=&r" (tmp2)
		: [ptr]"r" (ptr), [val]"r" (val)
		: "cc", "memory"
	);

        return ret;
}


inline unsigned char atomic_faa8u(volatile unsigned char *ptr, unsigned char val)
{
	unsigned char ret, tmp1, tmp2;

	asm volatile (
		"1:	ldrexb %[ret], [%[ptr]]			\n" /* read old value and set exclusive flag */
		"	add %[tmp1], %[ret], %[val] 		\n" /* tmp = oldval + val */
		"	strexb %[tmp2], %[tmp1], [%[ptr]]	\n" /* try to update the value in memory */
		"	teq %[tmp2], #1				\n"
		"	beq 1b					\n" /* if tmp2 == 1 branch to 1 */
		: [ret]"=&r" (ret), [tmp1]"=&r" (tmp1), [tmp2]"=&r" (tmp2)
		: [ptr]"r" (ptr), [val]"r" (val)
		: "cc", "memory"
	);

	return ret;
}


inline short atomic_faa16(volatile short *ptr, short val)
{
	short ret, tmp1, tmp2;

	asm volatile (
		"1:	ldrexh %[ret], [%[ptr]]			\n" /* read old value and set exclusive flag */
		"	add %[tmp1], %[ret], %[val] 		\n" /* tmp = oldval + val */
		"	strexh %[tmp2], %[tmp1], [%[ptr]]	\n" /* try to update the value in memory */
		"	teq %[tmp2], #1				\n"
		"	beq 1b					\n" /* if tmp2 == 1 branch to 1 */
		: [ret]"=&r" (ret), [tmp1]"=&r" (tmp1), [tmp2]"=&r" (tmp2)
		: [ptr]"r" (ptr), [val]"r" (val)
		: "cc", "memory"
	);

        return ret;
}


inline unsigned short atomic_faa16u(volatile unsigned short *ptr, unsigned short val)
{
	unsigned short ret, tmp1, tmp2;

	asm volatile (
		"1:	ldrexh %[ret], [%[ptr]]			\n" /* read old value and set exclusive flag */
		"	add %[tmp1], %[ret], %[val] 		\n" /* tmp = oldval + val */
		"	strexh %[tmp2], %[tmp1], [%[ptr]]	\n" /* try to update the value in memory */
		"	teq %[tmp2], #1				\n"
		"	beq 1b					\n" /* if tmp2 == 1 branch to 1 */
		: [ret]"=&r" (ret), [tmp1]"=&r" (tmp1), [tmp2]"=&r" (tmp2)
		: [ptr]"r" (ptr), [val]"r" (val)
		: "cc", "memory"
	);

        return ret;
}


inline int atomic_faa32(volatile int *ptr, int val)
{
	int ret, tmp1, tmp2;

	asm volatile (
		"1:	ldrex %[ret], [%[ptr]]			\n" /* read old value and set exclusive flag */
		"	add %[tmp1], %[ret], %[val] 		\n" /* tmp = oldval + val */
		"	strex %[tmp2], %[tmp1], [%[ptr]]	\n" /* try to update the value in memory */
		"	teq %[tmp2], #1				\n"
		"	beq 1b					\n" /* if tmp2 == 1 branch to 1 */
		: [ret]"=&r" (ret), [tmp1]"=&r" (tmp1), [tmp2]"=&r" (tmp2)
		: [ptr]"r" (ptr), [val]"r" (val)
		: "cc", "memory"
	);

        return ret;
}


inline unsigned int atomic_faa32u(volatile unsigned int *ptr, unsigned int val)
{
	unsigned int ret, tmp1, tmp2;

	asm volatile (
		"1:	ldrex %[ret], [%[ptr]]			\n" /* read old value and set exclusive flag */
		"	add %[tmp1], %[ret], %[val] 		\n" /* tmp = oldval + val */
		"	strex %[tmp2], %[tmp1], [%[ptr]]	\n" /* try to update the value in memory */
		"	teq %[tmp2], #1				\n"
		"	beq 1b					\n" /* if tmp2 == 1 branch to 1 */
		: [ret]"=&r" (ret), [tmp1]"=&r" (tmp1), [tmp2]"=&r" (tmp2)
		: [ptr]"r" (ptr), [val]"r" (val)
		: "cc", "memory"
	);

        return ret;
}


inline long long atomic_faa64(volatile long long *ptr, long long val)
{
	unsigned int ret0, ret1, tmp0, tmp1, tmp2;
	long long ret;

	asm volatile (
		"1:	ldrexd %[ret0], %[ret1], [%[ptr]]		\n" /* read old value and set exclusive flag */
		"	add %[tmp0], %[ret0], %[val0] 			\n" /* tmp0 = oldval[0..31] + val[0..31] */
		"	adc %[tmp1], %[ret1], %[val1] 			\n" /* tmp1 = oldval[32..63] + val[32..63] + CARRY */
		"	strexd %[tmp2], %[tmp0], %[tmp1], [%[ptr]]	\n" /* try to update value in memory */
		"	teq %[tmp2], #1					\n"
		"	beq 1b						\n" /* if tmp2 == 1 branch to 1 */
		: [ret0]"=&r" (ret0), [ret1]"=&r" (ret1), [tmp0]"=&r" (tmp0), [tmp1]"=&r" (tmp1), [tmp2]"=&r" (tmp2)
		: [ptr]"r" (ptr), [val0]"r" ((unsigned int)(val & 0x00000000ffffffff)), [val1]"r" ((unsigned int)(val >> 32))
		: "cc", "memory"
	);

	ret = (long long)(((long long)ret1 << 32) | (unsigned long long)ret0);

        return ret;
}


inline unsigned long long atomic_faa64u(volatile unsigned long long *ptr, unsigned long long val)
{
	unsigned int ret0, ret1, tmp0, tmp1, tmp2;
	unsigned long long ret;

	asm volatile (
		"1:	ldrexd %[ret0], %[ret1], [%[ptr]]		\n" /* read old value and set exclusive flag */
		"	add %[tmp0], %[ret0], %[val0] 			\n" /* tmp0 = oldval[0..31] + val[0..31] */
		"	adc %[tmp1], %[ret1], %[val1] 			\n" /* tmp1 = oldval[32..63] + val[32..63] + CARRY */
		"	strexd %[tmp2], %[tmp0], %[tmp1], [%[ptr]]	\n" /* try to update the value in memory */
		"	teq %[tmp2], #1					\n"
		"	beq 1b						\n" /* if tmp2 == 1 branch to 1 */
		: [ret0]"=&r" (ret0), [ret1]"=&r" (ret1), [tmp0]"=&r" (tmp0), [tmp1]"=&r" (tmp1), [tmp2]"=&r" (tmp2)
		: [ptr]"r" (ptr), [val0]"r" ((unsigned int)(val & 0x00000000ffffffff)), [val1]"r" ((unsigned int)(val >> 32))
		: "cc", "memory"
	);

	ret = (unsigned long long)(((unsigned long long)ret1 << 32) | (unsigned long long)ret0);
        
	return ret;
}

