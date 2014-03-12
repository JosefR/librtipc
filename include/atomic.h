/** 
 * @file atomic.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * atomic operations
 */

#ifndef _ATOMIC_H_
#define _ATOMIC_H_


/** 
 * atomic compare and swap (int8)
 */
inline int atomic_cas8(volatile signed char *ptr, signed char oldval, signed char newval);


/** 
 * atomic compare and swap (uint8)
 */
inline int atomic_cas8u(volatile unsigned char *ptr, unsigned char oldval, unsigned char newval);


/** 
 * atomic compare and swap (int16)
 */
inline int atomic_cas16(volatile short *ptr, short oldval, short newval);


/** 
 * atomic compare and swap (uint16)
 */
inline int atomic_cas16u(volatile unsigned short *ptr, unsigned short oldval, unsigned short newval);


/** 
 * atomic compare and swap (int32)
 */
inline int atomic_cas32(volatile int *ptr, int oldval, int newval);


/** 
 * atomic compare and swap (uint32)
 */
inline int atomic_cas32u(volatile unsigned int *ptr, unsigned int oldval, unsigned int newval);


/** 
 * atomic compare and swap (int64)
 */
inline int atomic_cas64(volatile long long *ptr, long long oldval, long long newval);


/** 
 * atomic compare and swap (uint64)
 */
inline int atomic_cas64u(volatile unsigned long long *ptr, unsigned long long oldval, unsigned long long newval);


/**
 * atomic fetch and add (int8)
 */
inline signed char atomic_faa8(volatile signed char *ptr, signed char val);


/**
 * atomic fetch and add (uint8)
 */
inline unsigned char atomic_faa8u(volatile unsigned char *ptr, unsigned char val);


/**
 * atomic fetch and add (int16)
 */
inline short int atomic_faa16(volatile short *ptr, short);


/**
 * atomic fetch and add (uint16)
 */
inline unsigned short atomic_faa16u(volatile unsigned short *ptr, unsigned short val);


/**
 * atomic fetch and add (int32)
 */
inline int atomic_faa32(volatile int *ptr, int val);


/**
 * atomic fetch and add (uint32)
 */
inline unsigned int atomic_faa32u(volatile unsigned int *ptr, unsigned int val);


/**
 * atomic fetch and add (int64)
 */
inline long long atomic_faa64(volatile long long *ptr, long long val);


/**
 * atomic fetch and add (uint64)
 */
inline unsigned long long atomic_faa64u(volatile unsigned long long *ptr, unsigned long long val);

#endif /* _ATOMIC_H_ */
