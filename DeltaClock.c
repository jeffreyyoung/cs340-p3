//
//  DeltaClock.c
//  
//
//  Created by Jeffrey Young on 10/14/15.
//
//

#include "DeltaClock.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "os345.h"


DeltaClock* initDeltaClock()
{
    DeltaClock* dc = (DeltaClock*)malloc(sizeof(DeltaClock));
    dc->head = NULL;
    dc->size = 0;
    return dc;
}

int insertDeltaClock(int ticks, Semaphore* event)
{
    semWait(deltaClockMutex);	//wait for clock to be signaled
//    
//    if (ticks <= 0)
//    {
//        semSignal(event);
//        semSignal(deltaClockMutex);
//        return 1;
//    }
    
    DeltaClock* dc = deltaQueue;
    DeltaClockNode* newNode = (DeltaClockNode*)malloc(sizeof(DeltaClockNode));		//malloc each node
    newNode->event = event;
    newNode->next = NULL;
    
    if(dc->head == NULL)
    {
        newNode->ticks = ticks;
        dc->head = newNode;
    }
    else if (ticks < dc->head->ticks)
    {
        newNode->ticks = ticks;
        dc->head->ticks -= ticks;		//subtract the difference for the new head node
        newNode->next = dc->head;
        dc->head = newNode;				//adjust the list and shuffle it down
    }
    else
    {
        int deltaTicks = ticks - dc->head->ticks;
        DeltaClockNode* previous = dc->head;
        DeltaClockNode* current = dc->head->next;
        
        while (1)
        {
            if (current == NULL)
            {
                previous->next = newNode;
                newNode->ticks = deltaTicks;
                break;
            }
            
            deltaTicks = deltaTicks - current->ticks;
            
            if(deltaTicks < 0) //iterate untill we jump past the node we want
            {
                newNode->ticks = deltaTicks + current->ticks;
                current->ticks = -deltaTicks;
                previous->next = newNode;
                newNode->next = current;
                break;
            }
            
            previous = previous->next;
            current = current->next;
        }
    }
    
    dc->size++;
    semSignal(deltaClockMutex);
    return 1;
}

int tickDeltaClock()
{
    DeltaClock* dc = deltaQueue;
    if (dc->head)
    {
        dc->head->ticks--;
        while(dc->head != NULL && dc->head->ticks <= 0 )
        {
            dc->size--;
            semSignal(dc->head->event);
            DeltaClockNode* toDestroy = dc->head;
            dc->head = dc->head->next;
            free(toDestroy);
        }
    }
    return 1;
}

int deleteDeltaClock()
{
    DeltaClock* dc = deltaQueue;
    if(dc == NULL)
    {
        return 1;
    }
    
    DeltaClockNode* delnode = dc->head;
    DeltaClockNode* temp;
    
    while(delnode != NULL)
    {
        temp = delnode;				//set the marker to delete
        delnode = delnode->next;	//advance the current marker
        free(temp);					//delete previous
    }
    
    free(dc);						//free deltaclock pointer
    return 1;
}

void printDeltaClock(DeltaClock* dc)
{
    DeltaClockNode* cur = dc->head;
    printf("\nDelta Clock size: %d\n", dc->size);
    printf("\n-----------------------------------------------------");
    int i = 0;
    while (cur != NULL)
    {
        printf("\nposition: %d, ticks: %d, semaphore: %s", i, cur->ticks, cur->event->name);
        i++;
        cur = cur->next;
    }
    
    printf("\n-----------------------------------------------------\n");
}