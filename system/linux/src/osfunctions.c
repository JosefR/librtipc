/** 
 * @file osfunctions.c
 * @author Josef Raschen <josef@raschen.org>
 */


#define _GNU_SOURCE
#include <sched.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include "osfunctions.h"


int osfunctions_movetocpu(int cpu)
{
	cpu_set_t mask;

	/* set cpu mask */
        CPU_ZERO(&mask);
        CPU_SET(cpu, &mask);

	/* set the cpu affinity for calling process */
        return sched_setaffinity(0, sizeof(cpu_set_t), &mask);
}


int osfunctions_setMaxPriority()
{
	struct sched_param param;

        sched_getparam(getpid(), &param);

	param.sched_priority = sched_get_priority_max(SCHED_FIFO);

        return (sched_setscheduler(getpid(), SCHED_FIFO, &param) != -1);
}


void osfunctions_mLockAll()
{
	mlockall(MCL_CURRENT | MCL_FUTURE);
}

