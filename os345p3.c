// os345p3.c - Jurassic Park
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the CS345 projects.          **
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
#include "os345.h"
#include "os345park.h"
#include "DeltaClock.h"
// ***********************************************************************
// project 3 variables

// Jurassic Park
extern JPARK myPark;
extern Semaphore* parkMutex;						// protect park access
extern Semaphore* fillSeat[NUM_CARS];			// (signal) seat ready to fill
extern Semaphore* seatFilled[NUM_CARS];		// (wait) passenger seated
extern Semaphore* rideOver[NUM_CARS];			// (signal) ride over
extern DeltaClock* deltaQueue;
Semaphore* tickets;
Semaphore* spotsInGiftShop;
Semaphore* spotsInPark;
Semaphore* spotsInMuseum;
Semaphore* carSpotAvailable;
Semaphore* seatTaken;
Semaphore* rideDoneHolderMutex;
Semaphore* driverDoneHolderMutex;
Semaphore* passengerSeated;
Semaphore* wakeupDriver;
Semaphore* needDriver;
Semaphore* driverReady;
Semaphore* driverTaken;
Semaphore* needTicket;
Semaphore* ticketReady;

Semaphore* carMutex;
Semaphore* carAvailable;
Semaphore* carTaken;

Semaphore* interactingWithDriverMutex;

Semaphore* ticketTakerMutex;




Semaphore* rideDoneSemaphoreHolder;
Semaphore* driverDoneSemaphoreHolder;
int carNum;


// ***********************************************************************
// project 3 functions and tasks
void CL3_project3(int, char**);
void CL3_dc(int, char**);


// ***********************************************************************
// ***********************************************************************
// project3 command
int P3_project3(int argc, char* argv[])
{
	char buf[32];
	char* newArgv[2];

	// start park
	sprintf(buf, "jurassicPark");
	newArgv[0] = buf;
	createTask( buf,				// task name
		jurassicTask,				// task
		MED_PRIORITY,				// task priority
		1,								// task count
		newArgv);					// task argument

	// wait for park to get initialized...
	while (!parkMutex) SWAP;
	printf("\nStart Jurassic Park...");

	//?? create car, driver, and visitor tasks here

	spotsInGiftShop = createSemaphore("spotsInGiftShop", COUNTING, MAX_IN_GIFTSHOP);	SWAP;
	spotsInMuseum = createSemaphore("spotsInMuseum", COUNTING, MAX_IN_MUSEUM);			SWAP;
	spotsInPark = createSemaphore("spotsInPark", COUNTING, MAX_IN_PARK);				SWAP;
	tickets = createSemaphore("tickets", COUNTING, MAX_TICKETS);						SWAP;

	rideDoneHolderMutex = createSemaphore("rideDoneHolderMutex", BINARY, 1); SWAP;
	passengerSeated = createSemaphore("passengerSeated", BINARY, 0); SWAP;

	carSpotAvailable = createSemaphore("carSpotAvailable", BINARY, 0); SWAP;
	seatTaken = createSemaphore("seatTaken", BINARY, 0);SWAP;

	needTicket = createSemaphore("needTicket", BINARY, 0); SWAP;

	wakeupDriver = createSemaphore("wakeupDriver", BINARY, 0); SWAP;
	needDriver = createSemaphore("needDriver", BINARY, 0); SWAP;



	ticketReady = createSemaphore("ticketReady", BINARY, 0); SWAP;

	driverTaken = createSemaphore("driverTaken", BINARY, 0); SWAP;
	driverReady = createSemaphore("driverReady", BINARY, 0); SWAP;

	driverDoneHolderMutex = createSemaphore("driverDoneHolderMutex", BINARY, 1);	SWAP;

	carMutex = createSemaphore("carMutex", BINARY, 1);	SWAP;
	carAvailable = createSemaphore("carAvailable", BINARY, 0);	SWAP;
	carTaken = createSemaphore("carTaken", BINARY, 0);	SWAP;

	ticketTakerMutex = createSemaphore("ticketTakerMutex", BINARY, 1); SWAP;
	interactingWithDriverMutex = createSemaphore("interactingWithDriverMutex", BINARY, 1); SWAP;

	int numVisitors = NUM_VISITORS; SWAP;
	if(argv[1] != 0){ SWAP;
		numVisitors = atoi(argv[1]); SWAP;
	}
	int i; SWAP;
    char id[15]; SWAP;
    
	//create visitor tasks
	for(i = 0; i < numVisitors; i++){ SWAP;
		sprintf(buf, "visitor%d", i); SWAP;
        sprintf(id, "%d", i); SWAP;
        newArgv[0] = buf; SWAP;
		newArgv[1] = id; SWAP;
		createTask(buf, P3_visitor, MED_PRIORITY, 2, newArgv); SWAP;
	}

	//create driver tasks
	for(i = 0; i < NUM_DRIVERS; i++){ SWAP;
		sprintf(buf, "driver%d", i); SWAP;
        sprintf(id, "%d", i); SWAP;
		newArgv[0] = buf; SWAP;
		newArgv[1] = id; SWAP;
		createTask(buf, P3_driver, MED_PRIORITY, 2, newArgv);	SWAP;
	}

	//create car tasks
	for(i = 0; i < NUM_CARS; i++){ SWAP;
        sprintf(buf, "car%d", i); SWAP;
        sprintf(id, "%d", i); SWAP;
		newArgv[0] = buf; SWAP;
		newArgv[1] = id; SWAP;
		createTask(buf, P3_car, MED_PRIORITY, 2, newArgv);	SWAP;
	}


	return 0;
} // end project3

int P3_car(int argc, char* argv[]){ SWAP;
	Semaphore* carRideDone[3]; SWAP;
	Semaphore* driverDone; SWAP;
	int carID= atoi(argv[1]); SWAP;
	int i;
	while(1){
        //clear the semaphores...
		for(i = 0; i < NUM_SEATS; i++){
			carRideDone[i] = 0;	SWAP;
		}
		driverDone = 0;	SWAP;


		for(i = 0; i < NUM_SEATS; i++){ SWAP;
			semWait(fillSeat[carID]); SWAP;
			semSignal(carSpotAvailable);	SWAP;
			semWait(seatTaken);	SWAP;

			//take visitor's semaphore from the holder
			carRideDone[i] = rideDoneSemaphoreHolder;	SWAP;
			semSignal(passengerSeated);	SWAP;
            
            //WE GOT OUR PASSENGERS, NOW LETS GET OUR DRIVER
			if(i == NUM_SEATS-1){ SWAP;
                //SIGNAL THAT DRIVER IS NEEDED
				semSignal(needDriver);	SWAP;
				semSignal(wakeupDriver);	SWAP;
				
                //WAIT FOR AVAILABLE DRIVER
				semWait(driverReady);	SWAP;
				driverDone = driverDoneSemaphoreHolder;	SWAP;
				semSignal(driverTaken);	SWAP;
                
                //GET CAR ID
				semWait(carMutex);	SWAP;
				carNum = carID;	SWAP;
				semSignal(carAvailable);	SWAP;
				semWait(carTaken);	SWAP;
				semSignal(carMutex);	SWAP;
			}
			semSignal(seatFilled[carID]); SWAP;
		}
		semWait(rideOver[carID]);	SWAP;

		//release the visitors and driver
		for(i = 0; i < NUM_SEATS; i++){ SWAP;
			semSignal(carRideDone[i]);	SWAP;
		}
		semSignal(driverDone);	SWAP;
	}

	return 0;
}

int P3_driver(int argc, char* argv[]){	SWAP;
	char buf[32];	SWAP;
	Semaphore* driverDone;	SWAP;
	int driverID = atoi(argv[1]);	SWAP;
	printf(buf, "Starting driverTask %d", driverID);	SWAP;
	sprintf(buf, "driverFinished %d", driverID);	SWAP;
    //signal driverDone semaphore when ride finished
	driverDone = createSemaphore(buf, BINARY, 0);	SWAP;

	while(1){
        //wait for the driver to wake up
		semWait(wakeupDriver);	SWAP;
        //when the driver awakes, use the semTryLock to determine if a driver or a ticket seller is needed
		if(semTryLock(needDriver)){ SWAP;
			//driver needed
			semWait(driverDoneHolderMutex);	SWAP;
            //put driverDone semaphore in the mail box to be picked up by car task
			driverDoneSemaphoreHolder = driverDone;	SWAP;
			semSignal(driverReady);	SWAP;
			semWait(driverTaken); SWAP;
			driverDoneSemaphoreHolder = 0;	SWAP;
			
            //wait for a car to be available
			semWait(carAvailable); SWAP;
			semWait(parkMutex);	SWAP;
			myPark.drivers[driverID] = carNum + 1;	SWAP;
			semSignal(parkMutex);	SWAP;
			semSignal(carTaken);	SWAP;
			semSignal(driverDoneHolderMutex); SWAP;

			semWait(driverDone); SWAP;
		}
		else if(semTryLock(needTicket)){	SWAP;
            
			semWait(ticketTakerMutex);	SWAP;
            //DRIVER IS AT TICKET KIOSK
			myPark.drivers[driverID] = -1;	SWAP;
			semWait(parkMutex);	SWAP;
            //READY TO GIVE TICKET
			semSignal(parkMutex);	SWAP;
			semWait(tickets);	SWAP;
			semSignal(ticketReady);	SWAP;
			semSignal(ticketTakerMutex);	SWAP;

		}

		semWait(parkMutex); SWAP;
		myPark.drivers[driverID] = 0; SWAP;
		semSignal(parkMutex); SWAP;

	}

	return 0;

}



// ***********************************************************************
// ***********************************************************************
// delta clock command
int P3_dc(int argc, char* argv[])
{
	printf("\nDelta Clock"); SWAP;
	// ?? Implement a routine to display the current delta clock contents
    printDeltaClock(deltaQueue);
    return 0;
} // end CL3_dc

int P3_visitor(int argc, char* argv[]){
	char buf[32];	SWAP;
	sprintf(buf, "visitor ride complete%d", atoi(argv[1]));	SWAP;
	Semaphore* visitorSem = createSemaphore(buf, BINARY, 0);	SWAP;

	//MEANDER AROUND BEFORE ARRIVING AT PARK
    int ticsBeforeArrivingAtPark = rand() % 100;	SWAP;
	insertDeltaClock(ticsBeforeArrivingAtPark, visitorSem);	SWAP;
	semWait(visitorSem);	SWAP;
	semWait(parkMutex);	SWAP;
	myPark.numOutsidePark++; SWAP;
	semSignal(parkMutex);		SWAP;

    //WAIT FOR THERE TO BE AN AVAILABLE SPOT IN THE PARK
	semWait(spotsInPark);
	semWait(parkMutex); SWAP;
	myPark.numOutsidePark--;	SWAP;
	myPark.numInPark++; SWAP;
	myPark.numInTicketLine++;	SWAP;
	semSignal(parkMutex);	SWAP;

	//WAIT TO GET A TICKET
    int ticsB4EntranceRqTicket = rand() % 30; SWAP;
	insertDeltaClock(ticsB4EntranceRqTicket, visitorSem);	SWAP;
	semWait(visitorSem);	SWAP;
    
    //WAKE UP A DRIVER AND GET A TICKET
	semWait(interactingWithDriverMutex); SWAP;
	semSignal(needTicket);	SWAP;
	semSignal(wakeupDriver); SWAP;
	semWait(ticketReady);	SWAP;
	semWait(parkMutex); SWAP;
	myPark.numTicketsAvailable--; SWAP;
	semSignal(parkMutex); SWAP;
	semSignal(interactingWithDriverMutex); SWAP;

    //GOT A TICKET, NOW GO TO THE MUSEUM LINE
	semWait(parkMutex);	SWAP;
	myPark.numInTicketLine--;	SWAP;
	myPark.numInMuseumLine++;	SWAP;
	semSignal(parkMutex); SWAP;
    int ticsBeforeMuseumQueue = rand() % 30;	SWAP;
	insertDeltaClock(ticsBeforeMuseumQueue, visitorSem);	SWAP;
	semWait(visitorSem); SWAP;

	//WAIT FOR A SPOT IN THE MUSEUM
	semWait(spotsInMuseum); SWAP;
	semWait(parkMutex);	SWAP;
	myPark.numInMuseumLine--;	SWAP;
	myPark.numInMuseum++;	SWAP;
	semSignal(parkMutex); SWAP;

	//MEANDOR AROUND IN THE MUSEUM FOR A LITTLE WHILE
    int ticsInMuseum = rand() % 30;	SWAP;
	insertDeltaClock(ticsInMuseum, visitorSem); SWAP;
	semWait(visitorSem); SWAP;

    //LEAVE MUSEUM AND GET IN THE CAR LINE
	semWait(parkMutex); SWAP;
	myPark.numInMuseum--;	SWAP;
	myPark.numInCarLine++;	SWAP;
	semSignal(parkMutex); SWAP;
	semSignal(spotsInMuseum); SWAP;
    int ticsInCarQueue = rand() % 30;	SWAP;
	insertDeltaClock(ticsInCarQueue, visitorSem); SWAP;
	semWait(visitorSem);	SWAP;


	//WAIT FOR A SPOT IN A CAR
	semWait(carSpotAvailable); SWAP;

	//SET THE RIDEDONESEMAPHORE HOLDER
	semWait(rideDoneHolderMutex);	SWAP;
	sprintf(buf, "rideDoneVisitor%d", atoi(argv[1]));	SWAP;
	Semaphore* rideDone = createSemaphore(buf, BINARY, 0);	SWAP;
	rideDoneSemaphoreHolder = rideDone;	SWAP;
	semSignal(seatTaken); SWAP;
	semWait(passengerSeated); SWAP;
	rideDoneSemaphoreHolder = 0; SWAP;
	semSignal(rideDoneHolderMutex); SWAP;

    //GIVE BACK THE TICKET
	semSignal(tickets); SWAP;
	semWait(parkMutex); SWAP;
	myPark.numTicketsAvailable++; SWAP;
	semSignal(parkMutex); SWAP;
    //GET IN THE CAR
	semWait(parkMutex); SWAP;
	myPark.numInCarLine--; SWAP;
	myPark.numInCars++; SWAP;
	semSignal(parkMutex); SWAP;

    //RIDE AROUND IN THE CAR
	semWait(rideDone); SWAP;
    
    //GET OUT OF CAR AND GET INTO THE GIFT LINE
	semWait(parkMutex); SWAP;
	myPark.numInCars--; SWAP;
	myPark.numInGiftLine++; SWAP;
	semSignal(parkMutex); SWAP;

	//wait in line at gift shop
    int ticsInGiftShopLine = rand() % 30;
	insertDeltaClock(ticsInGiftShopLine, visitorSem); SWAP;
	semWait(visitorSem);	SWAP;
    
    //MAKE SURE THERES A SPOT IN THE GIFT SHOP
	semWait(spotsInGiftShop); SWAP;
	semWait(parkMutex); SWAP;
	myPark.numInGiftLine--; SWAP;
	myPark.numInGiftShop++; SWAP;
	semSignal(parkMutex); SWAP;

    //MEANDOR AROUND IN THE GIFT SHOP
    int ticsInGiftShop = rand() % 30; SWAP;
	insertDeltaClock(ticsInGiftShop, visitorSem); SWAP;
	semWait(visitorSem); SWAP;
	semSignal(spotsInGiftShop); SWAP;
	semWait(parkMutex); SWAP;
	myPark.numInGiftShop--;	SWAP;
	myPark.numInPark--; SWAP;
	myPark.numExitedPark++;
	semSignal(parkMutex); SWAP;
	semSignal(spotsInPark); SWAP;
	semSignal(tickets); SWAP;
    
    //HALLELUJUAH
    
	return 0;

}
