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
    semWait(deltaClockMutex);	SWAP;//wait for clock to be signaled
//
//    if (ticks <= 0)
//    {
//        semSignal(event);
//        semSignal(deltaClockMutex);
//        return 1;
//    }

    DeltaClock* dc = deltaQueue;SWAP;
    DeltaClockNode* newNode = (DeltaClockNode*)malloc(sizeof(DeltaClockNode));SWAP;		//malloc each node
    newNode->event = event;SWAP;
    newNode->next = NULL;SWAP;

    if(dc->head == NULL)
    { SWAP;
        newNode->ticks = ticks;SWAP;
        dc->head = newNode; SWAP;
    }
    else if (ticks < dc->head->ticks)
    { SWAP;
        newNode->ticks = ticks; SWAP;
        dc->head->ticks -= ticks;	SWAP;	//subtract the difference for the new head node
        newNode->next = dc->head; SWAP;
        dc->head = newNode;	SWAP;			//adjust the list and shuffle it down
    }
    else
    { SWAP;
        int deltaTicks = ticks - dc->head->ticks; SWAP;
        DeltaClockNode* previous = dc->head; SWAP;
        DeltaClockNode* current = dc->head->next; SWAP;

        while (1)
        { SWAP;
            if (current == NULL)
            { SWAP;
                previous->next = newNode; SWAP;
                newNode->ticks = deltaTicks; SWAP;
                break;
            }

            deltaTicks = deltaTicks - current->ticks; SWAP;

            if(deltaTicks < 0) //iterate untill we jump past the node we want
            { SWAP;
                newNode->ticks = deltaTicks + current->ticks; SWAP;
                current->ticks = -deltaTicks; SWAP;
                previous->next = newNode; SWAP;
                newNode->next = current; SWAP;
                break;
            }

            previous = previous->next; SWAP;
            current = current->next; SWAP;
        }
    }

    dc->size++; SWAP;
    semSignal(deltaClockMutex); SWAP;
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
