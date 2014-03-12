/** 
 * @file shmman.c
 * @author Josef Raschen <josef@raschen.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shmman.h"
#include "atomic.h"
#include "hwfunctions.h"


#define MAX_SVSHMSEG 25 /* maximum number of System V shared memory segments objects */
#define MAX_SHMSEG 1000 /* the maximum number of shared memory segments */
#define MIN_SVSHMSEGSIZE 4 /* the minimum number of pages used for System V shared memory segments */


/**
 *  entry of System V shared memory segment table 
 */
typedef struct shared_sysVshmseg_table_entry {
	int sysVkey;
	unsigned int size;
	unsigned int offset; /* beginning of the unsused memory of the segment */
} shared_sysVshmseg_table_entry_t;
	

/** 
 * entry of the shared memory segment table 
 */
typedef struct shared_shmseg_table_entry {
	int init; /* indicates if the segment exists */
	int sysVtable_idx; /* index of the table entry of  the usef System V shared memory segment */
	int offset; /* the beginning of the memory area in the System v shared memory segment */ 
	int size; /* the size of the shared memory segment */
	int cnt; /* the current number of users of the segment */
} shared_shmseg_table_entry_t;

int linux_pagesize; /* size of a single memory page size */

int shmman_sysVkey = 0; /* the System V key of the shared data of shmared memory manager */
int shmman_sysVid = 0; /* System V shared memory ID; local for each user */
void *shmman_shmaddr; /* pointer to the shared meory segment of shmman */

/* pointers to shared data */
int *shared_lock; /* mutex for exclusive writing access to shared data */
int *shared_initflag; /* flag to signal that the library is initialized */
int *shared_sysVseg_cnt; /* pointer to shared counter of used SystemV shared memory segments */
int *shared_shmseg_cnt; /* pointer to shared memory segment counter */
int *shared_key_cnt; /* pointer to shared counter for creating System V keys */
shared_sysVshmseg_table_entry_t *shared_sysVshmseg_table;
shared_shmseg_table_entry_t *shared_shmseg_table;

/**
 * entry of the table of attached System V shared memory segments */
typedef struct local_sysVshmseg_table_entry {
	int sysVid;
	void *ptr;
	int cnt;
} local_sysVshmseg_table_entry_t;

/* table of System V shmsegs the user is connected to (local)*/
local_sysVshmseg_table_entry_t *local_sysVshmseg_table = NULL;

/* entry of local shmseg table */
typedef struct local_shmseg_table_entry {
	void *ptr;
} local_shmseg_table_entry_t;

/* local shared memory segment table */
local_shmseg_table_entry_t *local_shmseg_table = NULL;


key_t createSysVKey()
{
	int cnt = atomic_faa32(shared_key_cnt, 1);
	return ftok("/tmp", cnt);
}


int shmman_create_handle(int handle)
{
	return ftok("/tmp", 0);
}


void shmman_printShmsegTable()
{
	int i;
	shared_sysVshmseg_table_entry_t *t1;
	shared_shmseg_table_entry_t *t2;
	

	printf("\nidx \t| SysVkey \t| size \t| offset\n");
	t1 = shared_sysVshmseg_table;
	for (i = 0; i < *shared_sysVseg_cnt; i++) {
		printf("%d \t| 0x%x \t| %d \t| %d\n", i, t1->sysVkey, t1->size, t1->offset);
		t1++;
	}

	printf("\ninit \t| sysVtable_idx \t| offset \t| size \t| cnt\n");
	t2 = shared_shmseg_table;
	for (i = 0; i < *shared_shmseg_cnt; i++)
	{
		printf("%d \t| %d \t| %d \t| %d \t| %d\n", t2->init, t2->sysVtable_idx, t2->offset, t2->size, t2->cnt);
		t2++;
	}
	
}


int shmman_initialize(int handle)
{
	int i, sysVid, sysVkey;
	void *mem;
	local_sysVshmseg_table_entry_t *lSVtbl_entry;
	local_shmseg_table_entry_t *lshmtbl_entry;
	shared_shmseg_table_entry_t *shshmtbl_entry;

	linux_pagesize = getpagesize();

	sysVkey = handle;
	
	/* set up shared memory for shmman */

	/* create SystemV shared memory segment for the shared memory manager */
	if ((sysVid = shmget(sysVkey, 5 * sizeof(int) + MAX_SVSHMSEG * sizeof(shared_sysVshmseg_table_entry_t) + MAX_SHMSEG * sizeof(shared_shmseg_table_entry_t), IPC_CREAT|0600)) == -1) {
		perror("shmman: shmget() failed\n");
		return -1;
	}

        if ((unsigned long)(mem = shmat(sysVid, NULL, 0)) == -1) {
                perror("shmman: shmget() failed\n");
                return -1;
        }	

	/* pointers to shared data */
	shared_lock = (int *)mem;
	shared_initflag = (int *)((unsigned long)mem + sizeof(int));
	shared_sysVseg_cnt = ((int *)((unsigned long)mem + 2 *sizeof(int)));
	shared_shmseg_cnt = ((int *)((unsigned long)mem + 3 * sizeof(int)));
	shared_key_cnt = ((int *)((unsigned long)mem + 4 * sizeof(int)));
	shared_sysVshmseg_table = (shared_sysVshmseg_table_entry_t *)((unsigned long)mem + 5 * sizeof(int));
	shared_shmseg_table = (shared_shmseg_table_entry_t *)((unsigned long)mem + 5 * sizeof(int) + MAX_SVSHMSEG * sizeof(shared_sysVshmseg_table_entry_t));
	
	/* get lock */
	while(!atomic_cas32(shared_lock, 0, 1))
		hwfunctions_nop();

	/* if shmman is not initialized: do so */
	if (*shared_initflag == 0) {
		*shared_sysVseg_cnt = 0;
		*shared_shmseg_cnt = 0;
		*shared_key_cnt = 1;

		/* init shared shmseg table */
		shshmtbl_entry = shared_shmseg_table;
		for (i = 0; i < MAX_SHMSEG; i++) {
			shshmtbl_entry->init = 0;
			shshmtbl_entry++;
		}
	
		/* librtipc is initialized now */
		*shared_initflag = 1;
	}

	/* release lock */
	*shared_lock = 0;

	
	/* set up local data structures */		

	shmman_shmaddr = mem;
	shmman_sysVkey = sysVkey;
	shmman_sysVid = sysVid; 
	
	/* set up local table for list of attached System V shmsegments */
	local_sysVshmseg_table = (local_sysVshmseg_table_entry_t *)malloc(MAX_SVSHMSEG * sizeof(local_sysVshmseg_table_entry_t));

	lSVtbl_entry = local_sysVshmseg_table;
	for (i = 0; i < MAX_SVSHMSEG; i++) {
		lSVtbl_entry->sysVid = -1;
		lSVtbl_entry->cnt = 0;
		lSVtbl_entry++;
	}

	/* set up local shmseg table */
	local_shmseg_table = (local_shmseg_table_entry_t *)malloc(MAX_SHMSEG * sizeof(local_shmseg_table_entry_t));
	
	lshmtbl_entry = local_shmseg_table;
	for (i = 0; i < MAX_SHMSEG; i++) {
		lshmtbl_entry->ptr = NULL;
		lshmtbl_entry++;
	}

	return 0;
}


int shmman_create_shmseg(int shmid, int size) 
{
	int i, shm_size, sysVkey;
	int sysVshmseg_table_idx, shmseg_table_idx;
	shared_sysVshmseg_table_entry_t *it;
	void *ptr;

	/* to achieve 8 byte alignment of shared memory segments */
	shm_size = size + (8 - (size % 8));

	/* index in shmseg table is defined by shmid from shmseg_d_t */
	shmseg_table_idx = shmid;

	/* get lock */
	while (!atomic_cas32(shared_lock, 0, 1)) {
		hwfunctions_nop();
	}

	/* if shared memory segment exists: nothing to do */
	if (shared_shmseg_table[shmseg_table_idx].init == 1) {
		*shared_lock = 0;
		return 0;
	}		

	/* check if another entry in shmseg table is possible */
	if (*shared_shmseg_cnt >= MAX_SHMSEG) {
		*shared_lock = 0; /* release lock */
		perror("shmman: create shmseg not possible, maximum number of shared memory segments reached\n");
		return -1;
	}

	/* check for free space in existing System V shared memory segments */
	it = shared_sysVshmseg_table;
	for (i = 0; i < MAX_SVSHMSEG; i++) {
		if (it->sysVkey != 0 && (it->size - it->offset) >= shm_size) 
			break;
		it++;
	}

	if (i != MAX_SVSHMSEG) {
		/* requested memory fits into existing Sys V shared memory segment */
		sysVshmseg_table_idx = i;
	} else {
		int numpages, sysVid, sysVshmseg_size;
		void *mem;

		/* check if another entry in System V shmseg table is possible */
		if ((sysVshmseg_table_idx = *shared_sysVseg_cnt) >= MAX_SVSHMSEG) {
			*shared_lock = 0; /* release lock */
			perror("shmman: create shmseg not possible, maximum number of System V shared memory segments reached\n");
			return -1;
		}


		/* create new SystemV shared memory segment */

		/* determine number of pages used for the segment */
		numpages = shm_size / linux_pagesize + 1;
		if (numpages < MIN_SVSHMSEGSIZE)
			numpages = MIN_SVSHMSEGSIZE;

		sysVshmseg_size = numpages * linux_pagesize;
		
		sysVkey = createSysVKey();
		
		if ((sysVid = shmget(sysVkey, sysVshmseg_size, IPC_CREAT|0600)) == -1) {
			perror("shmman: allocation failed, shmget() failed\n");
			return -1;
		}

	        if ((unsigned long)(mem = shmat(sysVid, NULL, 0)) == -1) {
        	        perror("shmman: allocation failed, shmat() failed\n");
                	return -1;
	        }

		/* set System V shared memory table entry */
		shared_sysVshmseg_table[sysVshmseg_table_idx].sysVkey = sysVkey;
		shared_sysVshmseg_table[sysVshmseg_table_idx].size = sysVshmseg_size;
		shared_sysVshmseg_table[sysVshmseg_table_idx].offset = 0;

		/* set up local table entry */
		local_sysVshmseg_table[sysVshmseg_table_idx].sysVid =  sysVid;
		local_sysVshmseg_table[sysVshmseg_table_idx].ptr =  mem;
		local_sysVshmseg_table[sysVshmseg_table_idx].cnt =  0;
		
 
		(*shared_sysVseg_cnt)++;	
	}

	/* add new shmseg table entry */
	shared_shmseg_table[shmseg_table_idx].sysVtable_idx = sysVshmseg_table_idx;
	shared_shmseg_table[shmseg_table_idx].offset = 	shared_sysVshmseg_table[sysVshmseg_table_idx].offset;
	shared_shmseg_table[shmseg_table_idx].size = shm_size;
	shared_shmseg_table[shmseg_table_idx].cnt = 0;
	shared_shmseg_table[shmseg_table_idx].init = 1;

	/* set lock and init flag at the beginning of shared memory segment*/
	ptr = (void *)((unsigned long)(local_sysVshmseg_table[shared_shmseg_table[shmseg_table_idx].sysVtable_idx].ptr) + shared_shmseg_table[shmseg_table_idx].offset);
	*((int *)ptr) = 0; /* lock */
	*((int *)((unsigned long)ptr + sizeof(int))) = 0; /* init */

	/* update Sys V shmseg table entry */
	shared_sysVshmseg_table[sysVshmseg_table_idx].offset += shm_size;
	
	(*shared_shmseg_cnt)++;

	/* release lock */
	*shared_lock = 0;

	return 0;	
}


int shmman_get_shmseg(int shmid, int size, void **ptr) 
{
	int sysVshmseg_table_idx, offset, sysVkey, sysVid, shmseg_table_idx;
	void *SVshm_ptr;

	/* create the shmseg, if necessary */

	if (shmman_create_shmseg(shmid, size) == -1) {
		perror("shmman: shman_create_shmseg failed\n");
		return -1;
	}

	shmseg_table_idx = shmid;

	/* if shmseg is not connected to calling process: do so */
	if (local_shmseg_table[shmseg_table_idx].ptr == NULL) {
		sysVshmseg_table_idx = shared_shmseg_table[shmseg_table_idx].sysVtable_idx;
		offset = shared_shmseg_table[shmseg_table_idx].offset;
		sysVkey = shared_sysVshmseg_table[sysVshmseg_table_idx].sysVkey;
	
		/* check if the shmseg is attached and attach, if not */
		if (local_sysVshmseg_table[sysVshmseg_table_idx].sysVid == -1) {
			/* get shmid */
			if ((sysVid = shmget(sysVkey, 0, 0)) == -1) {
				perror("shmman: shmget() failed\n");
				return -1;
			}
	
			/* get pointer to shared memory segment  */
	        	if ((unsigned long)(SVshm_ptr = shmat(sysVid, NULL, 0)) == -1) {
        	        	perror("shmman: shmat() failed\n");
	                	return -1;
		        }
	
			/* set local Sys V shmseg table entry */	
			local_sysVshmseg_table[sysVshmseg_table_idx].sysVid = sysVid;
			local_sysVshmseg_table[sysVshmseg_table_idx].ptr = SVshm_ptr;
			local_sysVshmseg_table[sysVshmseg_table_idx].cnt = 0;
		} else {
			SVshm_ptr = local_sysVshmseg_table[sysVshmseg_table_idx].ptr;
		}
		
		/* an additional segment is used */
		local_sysVshmseg_table[sysVshmseg_table_idx].cnt++;

		/* new user of shared memory segment */
		shared_shmseg_table[shmseg_table_idx].cnt++;

		/* set up local shmseg_table */
		local_shmseg_table[shmseg_table_idx].ptr = (void *)((unsigned long)SVshm_ptr + offset);
	}

	/* set pointer to shared memory */
	*ptr = local_shmseg_table[shmseg_table_idx].ptr;

	return 0;		
}


int shmman_release_shmseg(int shmid)
{
	/* remove from local shared table */
	local_shmseg_table[shmid].ptr = NULL;

	/* decrement number of users in shared table */
	shared_shmseg_table[shmid].cnt--;

	return 0;
}


int shmman_disconnect()
{
	int i;
	shared_sysVshmseg_table_entry_t *seg;

	/* disconnect all attached SysV shared memory segments  */
	seg = shared_sysVshmseg_table;
	for (i = 0; i < *shared_sysVseg_cnt; i++) {
		if (local_sysVshmseg_table[i].sysVid != -1) {
	        	
			if (shmdt(local_sysVshmseg_table[i].ptr) == -1) {
        	        	perror("shmman: shmdt() failed\n");
	        	        return -1;
	        	}
			
			local_sysVshmseg_table[i].sysVid = -1;
			local_sysVshmseg_table[i].ptr = NULL;
			local_sysVshmseg_table[i].cnt = 0;
		}

	}

	/* clean up local data structures */

	if (local_shmseg_table != NULL) {
		free(local_shmseg_table);
	}

	if (local_sysVshmseg_table != NULL) {
		free(local_sysVshmseg_table);
	}
	
	return 0;
}


int shmman_finalize() 
{
	int i, sysVid;
	void *ptr;
	shared_sysVshmseg_table_entry_t *e;

	shared_lock = 0;
	shared_initflag = 0;
	
	/* delete Sytem V shared memory segments from table */
	e = shared_sysVshmseg_table;
	for (i = 0; i < *shared_sysVseg_cnt; i++) {
		/* get id */
		if((sysVid = shmget(e->sysVkey, 0, 0)) == -1) {
			perror("shmman: shmget() failed\n");
			return -1;
		}

		/* get pointer */
		if ((unsigned long)(ptr = shmat(sysVid, NULL, 0)) == -1) {
                	perror("shmman: shmat() failed\n");
	                return -1;
        	}	

		/* deattach */
        	if (shmdt(ptr) == -1) {
                	perror("shmman: shmdt() failed\n");
	                return -1;
        	}

		/* delete */
	        if (shmctl(sysVid, IPC_RMID, NULL) == -1) {
        	        perror("shmman: shmctl() failed\n");
                	return -1;
        	}

		e++;
	}

	/* delete Sys V shmseg of shmman */

        if (shmdt(shmman_shmaddr) == -1) {
                perror("shmman: shmdt() failed\n");
                return -1;
        }

        if (shmctl(shmman_sysVid, IPC_RMID, NULL) == -1) {
                perror("shmman: shmctl() failed\n");
                return -1;
        }

	return 0;
}


