/**
 * @file rtipc.h
 * @author Josef Raschen <josef@raschen.org>
 *
 */

#ifndef _RTIPC_H_
#define _RTIPC_H_

#include "system.h"
#include "arch.h"
#include "shmman.h"
#include "barrier.h"
#include "flag.h"
#include "mutex.h"
#include "spscq.h"
#include "wfmpscq.h"
#include "lfmpscq.h"
#include "mpmcq.h"
#include "sensorbuffer.h"


/**
 * create a handle for librtipc
 *
 * @return handle
 */
int rtipc_create_handle();


/**
 * initialize librtipc
 *
 * Should only be called once to set up librtipc. 
 *
 * @return handle on success, -1 on failure
 */
int rtipc_initialize(int handle);


/**
 * clean up
 * 
 * @return 0 success, -1 on failure
 */
int rtipc_finalize();


#endif
