// os345tasks.c - OS create/kill task	08/08/2013
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the BYU CS345 projects.      **
// ** It comes "as is" and "unwarranted."  As such, when you use part   **
// ** or all of the code, it becomes "yours" and you are responsible to **
// ** understand any algorithm or method presented.  Likewise, any      **
// ** errors or problems become your responsibility to fix.             **
// **                                                                   **
// ** NOTES:                                                            **
// ** -Comments beginning with "// ??" may require some implementation. **
// ** -Tab stops are set at every 3 spaces.                             **
// ** -The function API's in "OS345.h" should not be altered.           **
// **                                                                   **
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// ***********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include "os345lc3.h"

#include "os345.h"
#include "os345signals.h"
//#include "os345config.h"
#include "PQueue.h"


extern TCB tcb[];							// task control block
extern PQueue* rq;
extern int curTask;							// current task #

extern int superMode;						// system mode
extern Semaphore* semaphoreList;			// linked list of active semaphores
extern Semaphore* taskSems[MAX_TASKS];		// task semaphore


// **********************************************************************
// **********************************************************************
// create task
int createTask(char* name,						// task name
					int (*task)(int, char**),	// task address
					int priority,				// task priority
					int argc,					// task argument count
					char* argv[])				// task argument pointers
{
	int tid;

	// find an open tcb entry slot
	for (tid = 0; tid < MAX_TASKS; tid++)
	{
		if (tcb[tid].name == 0)
		{
			char buf[8];

			// create task semaphore
			if (taskSems[tid]) deleteSemaphore(&taskSems[tid]);
			sprintf(buf, "task%d", tid);
			taskSems[tid] = createSemaphore(buf, 0, 0);
			taskSems[tid]->taskNum = tid;	// assign to shell




			// copy task name
			tcb[tid].name = (char*)malloc(strlen(name)+1);
			strcpy(tcb[tid].name, name);

			// set task address and other parameters
			tcb[tid].task = task;			// task address
			tcb[tid].state = S_NEW;			// NEW task state
			tcb[tid].priority = priority;	// task priority
			tcb[tid].parent = curTask;		// parent
			tcb[tid].argc = argc;			// argument count
            tcb[tid].time = 1000;
			// ?? malloc new argv parameters
			//malloc new argv variables
			//initialization of variables
			tcb[tid].argv = malloc(sizeof(char*) * MAX_ARGS);
			int i;

			for(i = 0; i < MAX_ARGS; i++){
				tcb[tid].argv[i] = 0;
			}
			tcb[tid].argv[0] = strdup(argv[0]);


			//allocate space for strings and copy them over to new array--so they won't be destroyed
			for(i = 1; i < argc; i++){
				if(argv[i] != 0){
					tcb[tid].argv[i] = strdup(argv[i]);
				}
			}

			//tcb[tid].argv = myArgv;			// argument pointers

			tcb[tid].event = 0;				// suspend semaphore
			tcb[tid].RPT = LC3_RPT + ((tid) ? ((tid - 1)<<6) : 0);					// root page table (project 5)
			tcb[tid].cdir = CDIR;			// inherit parent cDir (project 6)

			// define task signals
			createTaskSigHandlers(tid);

			// Each task must have its own stack and stack pointer.
			tcb[tid].stack = malloc(STACK_SIZE * sizeof(int));

			// ?? may require inserting task into "ready" queue
			enQ(rq, tid, tcb[tid].priority);

			if (tid) swapTask();				// do context switch (if not cli)
			return tid;							// return tcb index (curTask)
		}
	}
	// tcb full!
	return -1;
} // end createTask



// **********************************************************************
// **********************************************************************
// kill task
//
//	taskId == -1 => kill all non-shell tasks
//
static void exitTask(int taskId);
int killTask(int taskId)
{
	if (taskId != 0)			// don't terminate shell
	{
		if (taskId < 0)			// kill all tasks
		{
			int tid;
			for (tid = 1; tid < MAX_TASKS; tid++)
			{
				if (tcb[tid].name) exitTask(tid);
			}
		}
		else
		{
			// terminate individual task
			if (!tcb[taskId].name) return 1;
			exitTask(taskId);	// kill individual task
		}
	}
	if (!superMode) SWAP;
	return 0;
} // end killTask

static void exitTask(int taskId)
{
	assert("exitTaskError" && tcb[taskId].name);

	// 1. find task in system queue
	// 2. if blocked, unblock (handle semaphore)
	// 3. set state to exit

	// ?? add code here

	tcb[taskId].state = S_EXIT;			// EXIT task state
	return;
} // end exitTask



// **********************************************************************
// system kill task
//
int sysKillTask(int taskId)
{
	Semaphore* sem = semaphoreList;
	Semaphore** semLink = &semaphoreList;

	// assert that you are not pulling the rug out from under yourself!
	assert("sysKillTask Error" && tcb[taskId].name && superMode);
	printf("\nKill Task %s", tcb[taskId].name);

	// signal task terminated
	semSignal(taskSems[taskId]);

	// look for any semaphores created by this task
	while(sem = *semLink)
	{
		if(sem->taskNum == taskId)
		{
			// semaphore found, delete from list, release memory
			deleteSemaphore(semLink);
		}
		else
		{
			// move to next semaphore
			semLink = (Semaphore**)&sem->semLink;
		}
	}

	//delete argv params from the task
	//clean up the trash
	int i;
	for(i = 0; i < tcb[taskId].argc; i++){
		if(tcb[taskId].argv[i] != 0){
			free(tcb[taskId].argv[i]);
		}
	}

	// ?? delete task from system queues
	deQ(rq, taskId);
	sem = semaphoreList;
	semLink = &semaphoreList;
	while (sem)
	{
		deQ(sem->q, taskId);
		semLink = (Semaphore**)&sem->semLink;
		sem = (Semaphore*)sem->semLink;
	}
	free(tcb[taskId].name);			// release tcb slot
	tcb[taskId].name = 0;
	return 0;
} // end sysKillTask
