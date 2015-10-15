// os345mmu.c - LC-3 Memory Management Unit
// **************************************************************************
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
#include <assert.h>
#include "os345.h"
#include "os345lc3.h"

// ***********************************************************************
// mmu variables

// LC-3 memory
unsigned short int memory[LC3_MAX_MEMORY];

extern int curTask;
extern TCB tcb[];

// statistics
int memAccess;						// memory accesses
int memHits;						// memory hits
int memPageFaults;					// memory faults
int nextPage;						// swap page size
int pageReads;						// page reads
int pageWrites;						// page writes

int RPTClock;						// clock for RPT

int getFrame(int);
int getAvailableFrame(void);


int getFrame(int notme)
{
	int UPTClock;						// clock for UPT
	int frame;
	frame = getAvailableFrame();
	if (frame >=0) return frame;

	if(RPTClock == 0){
		RPTClock = LC3_RPT;
	}


	int RPTBegin = RPTClock;
	int first = 1;
	int rptCount = 0;

	//iterate the first time through the tables
	while(first || RPTClock != RPTBegin){
		rptCount++;
		first = 0;
		//Check and make sure the notme condition holds
		if(DEFINED(memory[RPTClock])){
			UPTClock = (FRAME(memory[RPTClock])<<6);
			int UPTBegin = UPTClock;
			int uFirst = 1;

			int hadDefined = 0;
			int uptcount = 0;
			while(uFirst || UPTClock != UPTBegin){
				uptcount++;

				uFirst = 0;
				if(DEFINED(memory[UPTClock])){
					hadDefined = 1;
				}
				if(notme != UPTClock && DEFINED(memory[UPTClock])){
					//swap out if not referenced
					if(!REFERENCED(memory[UPTClock])){
						//if it's dirty, write it to memory
						int swapPage = 0;
						if (DIRTY(memory[UPTClock])){
							if(PAGED(memory[UPTClock + 1])) swapPage = accessPage(SWAPPAGE(memory[UPTClock + 1]), FRAME(memory[UPTClock]), PAGE_OLD_WRITE);
							else swapPage = accessPage(0, FRAME(memory[UPTClock]), PAGE_NEW_WRITE);
							memory[UPTClock] = CLEAR_DIRTY(memory[UPTClock]);
							memory[UPTClock+1] = SET_PAGED(swapPage);
							pageWrites++;
						}

						//no longer in main memory--it's swapped!
						memory[UPTClock] = CLEAR_DEFINED(memory[UPTClock]);
						//increment the clock
						if(RPTClock + 2 < LC3_RPT_END)
									RPTClock += 2;
								else
									RPTClock = LC3_RPT;
						return FRAME(memory[UPTClock]);
					}
					memory[UPTClock] = CLEAR_REF(memory[UPTClock]);
				}

				//sanity check
				assert(UPTClock >= 0x3000);
				if(UPTClock + 2 < UPTBegin + LC3_FRAME_SIZE)
					UPTClock += 2;
				else
					UPTClock = UPTBegin;
			}

			if(!REFERENCED(memory[RPTClock]) && !hadDefined && notme != RPTClock){
				int swapPage;
				if(PAGED(memory[RPTClock + 1])) swapPage = accessPage(SWAPPAGE(memory[RPTClock + 1]), FRAME(memory[RPTClock]), PAGE_OLD_WRITE);
				else swapPage = accessPage(0, FRAME(memory[RPTClock]), PAGE_NEW_WRITE);

				memory[RPTClock] = CLEAR_DIRTY(memory[RPTClock]);
				memory[RPTClock+1] = SET_PAGED(swapPage);
				pageWrites++;

				memory[RPTClock] = CLEAR_DEFINED(memory[RPTClock]);

				int rE1 = memory[RPTClock];

				if(RPTClock + 2 < LC3_RPT_END)
							RPTClock += 2;
						else
							RPTClock = LC3_RPT;
				return FRAME(rE1);
			}

			//clear the reference bit
			memory[RPTClock] = CLEAR_REF(memory[RPTClock]);
		}

		if(RPTClock + 2 < LC3_RPT_END){
			RPTClock += 2;
		}
		else{
			RPTClock = LC3_RPT;
		}
	}

	RPTBegin = RPTClock;
	first = 1;

	while(first || RPTClock != RPTBegin){
		rptCount++;
		first = 0;
		//Check and make sure the notme condition holds
		if(DEFINED(memory[RPTClock])){
			UPTClock = (FRAME(memory[RPTClock])<<6);
			int UPTBegin = UPTClock;
			int uFirst = 1;

			int hadDefined = 0;
			int uptcount = 0;
			while(uFirst || UPTClock != UPTBegin){
				uptcount++;

				uFirst = 0;
				if(DEFINED(memory[UPTClock])){
					hadDefined = 1;
				}
				if(notme != UPTClock && DEFINED(memory[UPTClock])){
					//swap out if not referenced
					if(!REFERENCED(memory[UPTClock])){
						//if it's dirty, write it to memory
						int swapPage = 0;
						if (DIRTY(memory[UPTClock])){
							if(PAGED(memory[UPTClock + 1])) swapPage = accessPage(SWAPPAGE(memory[UPTClock + 1]), FRAME(memory[UPTClock]), PAGE_OLD_WRITE);
							else swapPage = accessPage(0, FRAME(memory[UPTClock]), PAGE_NEW_WRITE);
							memory[UPTClock] = CLEAR_DIRTY(memory[UPTClock]);
							memory[UPTClock+1] = SET_PAGED(swapPage);
							pageWrites++;
						}

						//no longer in main memory--it's swapped!
						memory[UPTClock] = CLEAR_DEFINED(memory[UPTClock]);
						//increment the clock
						if(RPTClock + 2 < LC3_RPT_END)
									RPTClock += 2;
								else
									RPTClock = LC3_RPT;
						return FRAME(memory[UPTClock]);
					}
					memory[UPTClock] = CLEAR_REF(memory[UPTClock]);
				}

				//sanity check
				assert(UPTClock >= 0x3000);
				if(UPTClock + 2 < UPTBegin + LC3_FRAME_SIZE)
					UPTClock += 2;
				else
					UPTClock = UPTBegin;
			}

			if(!REFERENCED(memory[RPTClock]) && !hadDefined && notme != RPTClock){
				int swapPage;
				if(PAGED(memory[RPTClock + 1])) swapPage = accessPage(SWAPPAGE(memory[RPTClock + 1]), FRAME(memory[RPTClock]), PAGE_OLD_WRITE);
				else swapPage = accessPage(0, FRAME(memory[RPTClock]), PAGE_NEW_WRITE);

				memory[RPTClock] = CLEAR_DIRTY(memory[RPTClock]);
				memory[RPTClock+1] = SET_PAGED(swapPage);
				pageWrites++;

				memory[RPTClock] = CLEAR_DEFINED(memory[RPTClock]);

				int rE1 = memory[RPTClock];

				if(RPTClock + 2 < LC3_RPT_END)
							RPTClock += 2;
						else
							RPTClock = LC3_RPT;
				return FRAME(rE1);
			}

			//clear the reference bit
			memory[RPTClock] = CLEAR_REF(memory[RPTClock]);
		}

		if(RPTClock + 2 < LC3_RPT_END){
			RPTClock += 2;
		}
		else{
			RPTClock = LC3_RPT;
		}
	}




	//we should never ever be here
	assert(0);



	return -1;
}
// **************************************************************************
// **************************************************************************
// LC3 Memory Management Unit
// Virtual Memory Process
// **************************************************************************
//           ___________________________________Frame defined
//          / __________________________________Dirty frame
//         / / _________________________________Referenced frame
//        / / / ________________________________Pinned in memory
//       / / / /     ___________________________
//      / / / /     /                 __________frame # (0-1023) (2^10)
//     / / / /     /                 / _________page defined
//    / / / /     /                 / /       __page # (0-4096) (2^12)
//   / / / /     /                 / /       /
//  / / / /     / 	             / /       /
// F D R P - - f f|f f f f f f f f|S - - - p p p p|p p p p p p p p

#define MMU_ENABLE	1

unsigned short int *getMemAdr(int va, int rwFlg)
{
	unsigned short int pa;
	int rpta, rpte1, rpte2;
	int upta, upte1, upte2;
	int rptFrame, uptFrame;


	rpta = tcb[curTask].RPT + RPTI(va);
	rpte1 = memory[rpta];
	rpte2 = memory[rpta+1];

	// turn off virtual addressing for system RAM
	if (va < 0x3000) return &memory[va];


#if MMU_ENABLE
	if (DEFINED(rpte1))
	{
		memHits++;
	}
	else
	{
		// fault
		memPageFaults++;
		rptFrame = getFrame(-1);
		assert(rptFrame >= 192);
		rpte1 = SET_DEFINED(rptFrame);
		if (PAGED(rpte2))
		{
			accessPage(SWAPPAGE(rpte2), rptFrame, PAGE_READ);
			pageReads++;
		}
		else
		{
			rpte1 = SET_DIRTY(rpte1); rpte2 = 0;
			memset(&memory[(rptFrame<<6)], 0, 128);
		}
	}


	memory[rpta] = rpte1 = SET_REF(rpte1);
	memory[rpta+1] = rpte2;

	upta = (FRAME(rpte1)<<6) + UPTI(va);
	upte1 = memory[upta];
	upte2 = memory[upta+1];

	if (DEFINED(upte1))
	{
		memHits++;
	}
	else
	{
		// fault
		memPageFaults++;
		uptFrame = getFrame(FRAME(memory[rpta]));
		assert(uptFrame >= 192);
		upte1 = SET_DEFINED(uptFrame);
		if (PAGED(upte2))
		{
			accessPage(SWAPPAGE(upte2), uptFrame, PAGE_READ);
			pageReads++;
		}
		else
		{
			upte1 = SET_DIRTY(upte1); upte2 = 0;
		}
	}

	if(rwFlg != 0){
		upte1 = SET_DIRTY(upte1);
	}

	memory[upta] = SET_REF(upte1);
	memory[upta+1] = upte2;

	memAccess = memHits + memPageFaults;
	return &memory[(FRAME(upte1)<<6) + FRAMEOFFSET(va)];
#else
	pa = va;
#endif
	return &memory[pa];
} // end getMemAdr


// **************************************************************************
// **************************************************************************
// set frames available from sf to ef
//    flg = 0 -> clear all others
//        = 1 -> just add bits
//
void setFrameTableBits(int flg, int sf, int ef)
{	int i, data;
	int adr = LC3_FBT-1;             // index to frame bit table
	int fmask = 0x0001;              // bit mask

	// 1024 frames in LC-3 memory
	for (i=0; i<LC3_FRAMES; i++)
	{	if (fmask & 0x0001)
		{  fmask = 0x8000;
			adr++;
			data = (flg)?MEMWORD(adr):0;
		}
		else fmask = fmask >> 1;
		// allocate frame if in range
		if ( (i >= sf) && (i < ef)) data = data | fmask;
		MEMWORD(adr) = data;
	}
	return;
} // end setFrameTableBits


// **************************************************************************
// get frame from frame bit table (else return -1)
int getAvailableFrame()
{
	int i, data;
	int adr = LC3_FBT - 1;				// index to frame bit table
	int fmask = 0x0001;					// bit mask

	for (i=0; i<LC3_FRAMES; i++)		// look thru all frames
	{	if (fmask & 0x0001)
		{  fmask = 0x8000;				// move to next work
			adr++;
			data = MEMWORD(adr);
		}
		else fmask = fmask >> 1;		// next frame
		// deallocate frame and return frame #
		if (data & fmask)
		{  MEMWORD(adr) = data & ~fmask;
			return i;
		}
	}
	return -1;
} // end getAvailableFrame



// **************************************************************************
// read/write to swap space
int accessPage(int pnum, int frame, int rwnFlg)
{
   static unsigned short int swapMemory[LC3_MAX_SWAP_MEMORY];

   if ((nextPage >= LC3_MAX_PAGE) || (pnum >= LC3_MAX_PAGE))
   {
      printf("\nVirtual Memory Space Exceeded!  (%d)", LC3_MAX_PAGE);
      exit(-4);
   }
   switch(rwnFlg)
   {
      case PAGE_INIT:                    		// init paging
         nextPage = 0;
         return 0;

      case PAGE_GET_ADR:                    	// return page address
         return (int)(&swapMemory[pnum<<6]);

      case PAGE_NEW_WRITE:                   // new write (Drops thru to write old)
         pnum = nextPage++;

      case PAGE_OLD_WRITE:                   // write
         //printf("\n    (%d) Write frame %d (memory[%04x]) to page %d", p.PID, frame, frame<<6, pnum);
         memcpy(&swapMemory[pnum<<6], &memory[frame<<6], 1<<7);
         pageWrites++;
         return pnum;

      case PAGE_READ:                    // read
         //printf("\n    (%d) Read page %d into frame %d (memory[%04x])", p.PID, pnum, frame, frame<<6);
      	memcpy(&memory[frame<<6], &swapMemory[pnum<<6], 1<<7);
         pageReads++;
         return pnum;

      case PAGE_FREE:                   // free page
         break;
   }
   return pnum;
} // end accessPage
