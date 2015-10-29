// os345mmu.c - LC-3 Memory Management Unit	03/12/2015
//
//		03/12/2015	added PAGE_GET_SIZE to accessPage()
//
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

// statistics
int memAccess;						// memory accesses
int memHits;						// memory hits
int memPageFaults;					// memory faults

int getFrame(int);
int getAvailableFrame(void);

int nextPage;
extern TCB tcb[];					// task control block
extern int curTask;					// current task #

int nextupt = 0;
int nextrpt = 0;
int pageReads = 0;
int pageWrites = 0;


int getFrame(int notme)
{
    int frame;
    frame = getAvailableFrame();
    if (frame >=0)
    {
        return frame;
    }
    // run clock
    else
    {
        frame = getClockFrame(notme);
    }
    return frame;
}

//marker for current rpt/upt
int getClockFrame(int marker)
{
    int rpta = LC3_RPT;
    
    //get to first / next entry in the root page table
    if(nextrpt)
    {
        rpta = nextrpt;
    }
    
    while(1)
    {
        //go through each item in the root page table
        rpta = rpta + 2;
        
        //if we're past the root page table end, start over
        //make things circulat
        if(rpta >= LC3_RPT_END)
        {
            rpta = LC3_RPT;
        }
        
        //if in memory
        if(MEMWORD(rpta))
        {
            //unlock frame
            memory[rpta] = CLEAR_PINNED(memory[rpta]);
            int rpte1 = MEMWORD(rpta);
            int rpte2 = MEMWORD(rpta + 1);
            
            //get the first level of UPT address
            int upt_bound = FRAME(rpte1) << 6;
            
            //move through the user page table
            int upta;
            for(upta = upt_bound; upta < upt_bound + 64; upta += 2)
            {
                if(memory[upta])
                {
                    memory[rpta] = SET_PINNED(memory[rpta]);
                    int upte1 = MEMWORD(upta);
                    int upte2 = MEMWORD(upta + 1);
                    
                    // Not Referenced - 1st Priority
                    // if Ref == 0 and we aren't on the same frame
                    if( !REFERENCED(upte1) && (FRAME(upte1) != marker) )
                    {
                        //swape the frame
                        nextrpt = rpta; //set markers for the clock to point to the next one
                        nextupt = upta;
                        //reallocate UPT
                        memory[upta] = 0;
                        
                        //if it's paged, and dirty, rewrite
                        if(PAGED(upte2))
                        {
                            if(DIRTY(upte1))
                            {
                                accessPage(SWAPPAGE(upte2), FRAME(upte1), PAGE_OLD_WRITE);
                            }
                        }
                        
                        //not found in swap space
                        else
                        {
                            memory[upta + 1] = SET_PAGED(nextPage);
                            accessPage(nextPage, FRAME(upte1), PAGE_NEW_WRITE);
                        }
                        
                        //free page and return it
                        accessPage(nextPage,FRAME(upte1), PAGE_FREE);
                        return FRAME(upte1);
                    }
                    
                    //Referenced - 2nd Priority
                    //if it's referenced clear the ref bit.
                    if(REFERENCED(memory[upta]))
                    {
                        memory[upta] = CLEAR_REF(memory[upta]);
                    }
                }
            }
            //end for loop
            
            //if not pinned and we aren't on the same frame
            if(!PINNED(memory[rpta]) && FRAME(rpte1) != marker)
            {
                
                nextrpt = rpta + 2; //nextrpte is equal to the address of the rpt + 2
                nextupt = 0;  //nextUPTE freed
                memory[rpta] = 0;  //currentRPT freed
                
                //if rpte2 is in swap space
                if(PAGED(rpte2))
                {
                    //if rpte1 is dirty and already in swap
                    if(DIRTY(rpte1))
                    {
                        accessPage(SWAPPAGE(rpte2), FRAME(rpte1), PAGE_OLD_WRITE);
                    }
                }
                
                //if not in swap space
                if(!PAGED(rpte2))
                {
                    memory[rpta + 1] = SET_PAGED(nextPage);		//set paged bit of next page
                    accessPage(nextPage, FRAME(rpte1), PAGE_NEW_WRITE);	//do a new write by swapping it
                }
                
                accessPage(nextPage, FRAME(rpte1), PAGE_FREE); //free rpt1
                return FRAME(rpte1); //return rpte1 for reallocation
            }
        }
        
    }
    //we should never be here
    assert(0);
    return -1;
}
// **************************************************************************
// **************************************************************************
// LC3 Memory Management Unit
// Virtual Memory Process
// **************************************************************************
//           ___________________________________Frame defined (1 if referenced is in main memory, 0 otherwise)
//          / __________________________________Dirty frame (1 if referenced frame has been altered, 0 otherwise)
//         / / _________________________________Referenced frame (1 if frame has been referenced, 0 otherwise)
//        / / / ________________________________Pinned in memory (1 if pinned in memory, 0 otherwise)
//       / / / /     ___________________________
//      / / / /     /                 __________frame # (0-1023) (2^10) (if referenced page is in memory, this value specifies whichFrmeItOccupies
//     / / / /     /                 / _________page defined (1 if page has been allocated in swap space)
//    / / / /     /                 / /       __page # (0-4096) (2^12) (specifies where referenced page is stored in swap space)
//   / / / /     /                 / /       /
//  / / / /     / 	             / /       /
// F D R P - - f f|f f f f f f f f|S - - - p p p p|p p p p p p p p

#define MMU_ENABLE	1

unsigned short int *getMemAdr(int va, int rwFlg)
{
    int rpta, rpte1, rpte2;
    int upta, upte1, upte2;
    int rptFrame, uptFrame;
    
    //set root page table address to the address of the current task's root page table
    rpta = TASK_RPT + RPTI(va);
    rpte1 = MEMWORD(rpta);
    rpte2 = MEMWORD(rpta + 1);
    
    // turn off virtual addressing for system RAM
    if (va < 0x3000)
    {
        return &memory[va];
    }
    
#if MMU_ENABLE

    memAccess++;
    //if rpte1 is defined... "hit!" - dr roper
    if (DEFINED(rpte1))
    {
        memHits++;
        rptFrame = FRAME(rpte1);//get frame address
    }
    //else "miss!"
    else
    {
        memPageFaults++;
        //if rpte is undefined:
        //1. get a UPT frame from memory (may have to free up frame)
        //2. if paged out(defined) load swapped page into UPT frame
        //else initialize UPT for all UPT's
        
        //get a frame that isnt me!
        rptFrame = getFrame(-1);
        rpte1 = SET_DEFINED(SET_DIRTY(rptFrame));
        
        //if it's in the swap space
        if(PAGED(rpte2))
        {
            accessPage(SWAPPAGE(rpte2), rptFrame, PAGE_READ); //access frame, read it into frame
        }
        //if not in swap space, define new upt frame and reference with rpt
        else
        {
            rpte1 = SET_DIRTY(rpte1);
            rpte2 = 0;
            
            int i;
            for( i = 0; i < 64; i++)
            {
                //initialize every every entry in root page table
                MEMWORD((rptFrame << 6) + i) = 0;
            }
        }
    }
    
    memory[rpta] = rpte1 =  SET_REF(SET_PINNED(rpte1)); // set rpt frame access bit
    memory[rpta + 1] = rpte2;
   
    //get user page table address
    upta = (FRAME(rpte1)<<6) + UPTI(va);
    upte1 = MEMWORD(upta);
    upte2 = MEMWORD(upta + 1);
    
    memAccess++;
    //if hit
    if (DEFINED(upte1))	//hit
    {
        memHits++;
        uptFrame = FRAME(upte1);
    }
    //else miss
    else
    {
        memPageFaults++;
        //1. get a physical frame (may have to free up frame) (x3000 - limit) (192-1023)
        //2. if paged out (DEFINED) load swapped page into physical frame)
        //else new frame
        
        uptFrame = getFrame(rptFrame);
        upte1 = SET_DEFINED(uptFrame);
        
        //if it's in swap space
        if(PAGED(upte2))
        {
            //read page into uptFrame
            accessPage(SWAPPAGE(upte2), uptFrame, PAGE_READ);
        }
        //else not in swap space
        else
        {
            upte1 = SET_DIRTY(upte1);
            upte2 = 0;
            
            int i;
            for(i = 0; i < 64; i++)
            {
                //trying to get stuff to work...
                MEMWORD((uptFrame << 6) + i) = 0xf025;
            }
        }
    }
    
    //if read write flag, set that dirty bit
    if(rwFlg)
    {
        upte1 = SET_DIRTY(upte1);
    }
    
    memory[upta] =  SET_REF(upte1);
    memory[upta + 1] = upte2;
    
    return &memory[(FRAME(upte1)<<6) + FRAMEOFFSET(va)];
#else
    return &memory[va];
#endif
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
    {
        if (fmask & 0x0001)
        {
            fmask = 0x8000;
            adr++;
            data = (flg)?MEMWORD(adr):0;
        }
        
        else
            fmask = fmask >> 1;
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
    {
        if (fmask & 0x0001)
        {
            fmask = 0x8000;				// move to next work
            adr++;
            data = MEMWORD(adr);
        }
        else fmask = fmask >> 1;		// next frame
        // deallocate frame and return frame #
        if (data & fmask)
        {
            MEMWORD(adr) = data & ~fmask;
            //int accessPage(int pnum, int frame, int rwnFlg)
            //just changing rwnFlag to PAGE_FREE will make it deallocate the page for you.
            accessPage(nextPage, FRAME(i), PAGE_FREE);
            return i;
        }
    }
    return -1;
} // end getAvailableFrame



// **************************************************************************
// read/write to swap space

int accessPage(int pnum, int frame, int rwnFlg)
{
    
    
    int q;
    static unsigned short int swapMemory[LC3_MAX_SWAP_MEMORY];
    
    if ((nextPage >= LC3_MAX_PAGE) || (pnum >= LC3_MAX_PAGE))
    {
        printf("\nVirtual Memory Space Exceeded!  (%d)", LC3_MAX_PAGE);
        exit(-4);
    }
    switch(rwnFlg)
    {
        case PAGE_INIT:                    		// init paging
            memAccess = 0;						// memory accesses
            memHits = 0;						// memory hits
            memPageFaults = 0;					// memory faults
            nextPage = 0;						// disk swap space size
            pageReads = 0;						// disk page reads
            pageWrites = 0;						// disk page writes
            return 0;
            
        case PAGE_GET_SIZE:                    	// return swap size
            return nextPage;
            
        case PAGE_GET_READS:                   	// return swap reads
            return pageReads;
            
        case PAGE_GET_WRITES:                    // return swap writes
            return pageWrites;
            
        case PAGE_GET_ADR:                    	// return page address
            return (int)(&swapMemory[pnum<<6]);
            
        case PAGE_NEW_WRITE:                   // new write (Drops thru to write old)
            pnum = nextPage++;
            
        case PAGE_OLD_WRITE:                   // write
            //printf("\n    (%d) Write frame %d (memory[%04x]) to page %d", p.PID, frame, frame<<6, pnum);
            memcpy(&swapMemory[pnum<<6], &memory[frame<<6], 1<<7);
            pageWrites++;
            return pnum;
            
        case PAGE_READ:                    	// read
            //printf("\n    (%d) Read page %d into frame %d (memory[%04x])", p.PID, pnum, frame, frame<<6);
            memcpy(&memory[frame<<6], &swapMemory[pnum<<6], 1<<7);
            pageReads++;
            return pnum;
            
        case PAGE_FREE:                   // free page
            //printf("\nPAGE_FREE not implemented");
            //LC3_FRAME_SIZE
            for(q = 0; q < 64; q++)
            {
                MEMWORD((frame << 6) + q) = 0;
            }
            break;
    }
    return pnum;
} // end accessPage
