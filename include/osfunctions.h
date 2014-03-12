/** 
 * @file osfunctions.h
 * @author Josef Raschen <josef@raschen.org>
 *
 * some functions using the operating system's api
 */

#ifndef _OSFUNCTIONS_H_
#define _OSFUNCTIONS_H_


/** 
 * moves the calling process to the named cpu
 *
 * @param cpu the id if the cpu
 * @return 0 on success, -1 on failure
 */
int osfunctions_movetocpu(int cpu);


/**
 * set the calling process to maximum priority
 * @return -1 oin fail, 0 on success
 */
int osfunctions_setMaxPriority();


/**
 * prevent swapping
 */
void osfunctions_mLockAll();


#endif /* _OSFUNCTIONS_H_ */
