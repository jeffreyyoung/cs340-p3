

#include <time.h>
#ifndef PQueue_h_
#define PQueue_h_

typedef struct
{
    int tid;
    int priority;
    int time_added;
} task;

typedef struct
{
    task queue[512]; //array of 64 tasks
    int size;
    int elements_added;
} PQueue;


PQueue* initQueue();
task enQ(PQueue* pq, int priority, int p);
task deQ(PQueue* pq, int priority);
task deQpop(PQueue* pq);
void printPQ(PQueue* pq);


#endif