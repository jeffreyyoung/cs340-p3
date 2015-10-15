#include "PQueue.h"


PQueue* initQueue(PQueue* pq)
{
    pq = (PQueue*)malloc(sizeof(PQueue));	//Changed after causing immense pain in lab 5
    pq->size = 0;
    pq->elements_added = 0;
    return pq;
}


void sortQ(PQueue* pq)
{
    int i;
    for(i = 0; i < pq->size; i++)
    {
        int j;
        for (j = i + 1; j < pq->size; j++)
        {
            if (pq->queue[i].priority > pq->queue[j].priority)
            {
                task a = pq->queue[i];
                pq->queue[i] = pq->queue[j];
                pq->queue[j] = a;
            }
            else if (pq->queue[i].priority == pq->queue[j].priority
                     && pq->queue[i].time_added < pq->queue[j].time_added)
            {
                task a = pq->queue[i];
                pq->queue[i] = pq->queue[j];
                pq->queue[j] = a;
            }
        }
    }
}

task enQ(PQueue* pq, int t_id, int prior)
{
    pq->elements_added++;
    task to_add;
    to_add.tid = t_id;
    to_add.priority = prior;
    to_add.time_added = pq->elements_added;
    int i;
    for(i = 0; i < pq->size; i++)
    {
        if (pq->queue[i].tid == t_id)
        {
            return to_add;
        }
    }
    
    pq->queue[pq->size] = to_add;
    pq->size++;
    sortQ(pq);
    return to_add;
}


task deQ(PQueue* pq, int taskID)
{
    if (taskID == -1)
    {
        return deQpop(pq);
    }
    int i;
    int found = 0;
    for(i = 0; i < pq->size; i++)
    {
        if (pq->queue[i].tid == taskID)
        {
            found = 1;
            break;
        }
    }
    task to_return;
    if (found && pq->size != 0)
    {
        to_return.tid = taskID;
        to_return.priority = pq->queue[i].tid;
        
        pq->queue[i].priority = 99999999;
        sortQ(pq);
        pq->size--;
        return to_return;
    }
    else
    {
        to_return.tid = -1;
        to_return.priority = -99;
        return to_return;
    }
}

task deQpop(PQueue* pq)
{
    task to_return;
    if(pq->size == 0)
    {
        to_return.tid = -1;
        to_return.priority = -1;
    }
    else
    {
        to_return = pq->queue[pq->size - 1];
        pq->size--;
    }
    
    return to_return;
}


void printPQ(PQueue* pq)
{
    int i = 0;
    for(i; i < pq->size; i++)
    {
        task t = pq->queue[i];
        printf("\nPQ[%d] : tid: %d    priority: %d ", i,t.tid, t.priority);
    }
}