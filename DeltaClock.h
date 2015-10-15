//
//  DeltaClock.h
//  
//
//  Created by Jeffrey Young on 10/14/15.
//
//

#ifndef ____DeltaClock__
#define ____DeltaClock__

#include <stdio.h>
#include "os345.h"

typedef struct DeltaClockNode
{
    int ticks;
    Semaphore* event;
    struct DeltaClockNode* next;
} DeltaClockNode;

typedef struct
{
    DeltaClockNode* head;
    int size;
} DeltaClock;

extern DeltaClock* deltaQueue;
extern Semaphore* deltaClockMutex;;

DeltaClock* initDeltaClock();
int deleteDeltaClock();
int insertDeltaClock(int tics, Semaphore* sem);
int tickDeltaClock();
void printDeltaClock(DeltaClock* dc);




#endif /* defined(____DeltaClock__) */
