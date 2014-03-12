#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>

#include <rtipc.h>

/* architecture specific */
#include <arch.h>
#ifdef ARCH_X86
#include "rdtsc.h"
#endif 
#ifdef ARCH_ARM
#include "perfcounters.h"
#endif

#define SHMID_BARRIER 0
#define SHMID_SHMSEG 1 

#define REPEAT 10
#define NUM_ITERATION 10000100
#define HISTOGRAM_STEP 100
#define HISTOGRAM_SIZE 10


int main()
{
	int i;
	long long cnt, d;
	unsigned long long t1, t2, dt, t_min, t_max, t_sum;
	int histogram[HISTOGRAM_SIZE];

#ifdef ARCH_X86
	iopl(3);
#endif


	osfunctions_mLockAll();

	if (osfunctions_movetocpu(0) == -1)
		printf("cannot move to cpu %d\n", 0);

	if (osfunctions_setMaxPriority() == -1)
		printf("cannot set priority\n");

	for (i = 0; i < HISTOGRAM_SIZE; i++) 
		histogram[i] = 0;

	t_max = 0;
	t_min = 0xffffffffffffffff;
	t_sum = 0;

#ifdef ARCH_ARM
	apm_init();
	apm_resetCCNTR();
	apm_startCCNTR();
	
#endif
	

#ifdef ARCH_X86
	hwfunctions_cli();
#endif

	d = 0;
	for (i = 0; i < REPEAT; i++) {
		cnt = 0;
		while(cnt < NUM_ITERATION) {
#ifdef ARCH_X86
			t2 = read_tsc();
#endif
#ifdef ARCH_ARM
			t2 = apm_fetchCCNTR();
#endif
			hwfunctions_nop();
			hwfunctions_nop();
			hwfunctions_nop();	
			if (cnt >= 100) {
				d++;
				dt = t2 - t1;
				if(dt > t_max)
					t_max = dt;
				if(dt < t_min)
					t_min = dt;
				t_sum += dt;
				histogram[(((dt / HISTOGRAM_STEP) < HISTOGRAM_SIZE) && (dt > 0)) ? (dt / HISTOGRAM_STEP) : (HISTOGRAM_SIZE - 1)]++;
			}
			t1 = t2;
			cnt++;
		}
#ifdef ARCH_ARM
	apm_stopCCNTR();
#endif
	}
#ifdef ARCH_ARM
	apm_stopCCNTR();
	if (apm_fetchCCNTRovfl() == 1) {
		printf("cycle counter overflow\n");
		apm_resetCCNTRovfl();
	}
#endif
		
	printf("min: %llu, max: %llu, avrg: %llu, num interation: %lld\n", t_min, t_max, t_sum / d, d);
	printf("\n histogram \n range (cycles) \t\t abs. frequency\n");
	for (i = 0; i < HISTOGRAM_SIZE - 1; i++)
		printf("%d \t <= # < \t %d :\t %d\n", i * HISTOGRAM_STEP, (i + 1) * HISTOGRAM_STEP, histogram[i]);
	printf("%d \t <= # \t\t :\t %d\n", (HISTOGRAM_SIZE - 1) * HISTOGRAM_STEP, histogram[HISTOGRAM_SIZE - 1]);
		

#ifdef ARCH_X86
	hwfunctions_sti();
#endif
		

	return 0;
}
