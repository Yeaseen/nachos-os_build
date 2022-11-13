// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "memorymanager.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

extern memorymanager *memoryManager;
extern memorymanager* swapMemoryManager;
extern SwapSpace* swapSpace;

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------



AddrSpace::AddrSpace(OpenFile *executable)
{
    unsigned int i, j, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    this->executable=executable;
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
    
    //ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    printf("Creating new PageTable ====\n");
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = -1;
	//pageTable[i].physicalPage=memoryManager->AllocPage();
	//ASSERT(pageTable[i].physicalPage != -1);
    pageTable[i].timer = 0;
    pageTable[i].swapPage = -1;
	pageTable[i].valid = false;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
 //   bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
 //   if (noffH.code.size > 0) {
     //   DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
//			noffH.code.virtualAddr, noffH.code.size);
  //      executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
//			noffH.code.size, noffH.code.inFileAddr);
  //  }
  //  if (noffH.initData.size > 0) {
  //      DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
	//		noffH.initData.virtualAddr, noffH.initData.size);
  //      executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
//			noffH.initData.size, noffH.initData.inFileAddr);
  //  }

/*
    IntStatus iStat = interrupt->SetLevel(IntOff);
    for(unsigned int x=0; x < numPages; x++){
        	bzero(&machine -> mainMemory[(pageTable[x].physicalPage) * PageSize], PageSize);
        }

        

        if(noffH.code.size>0)
        {
            printf("code size== %d \n",noffH.code.size);
            unsigned int codeNumPages;
            codeNumPages=divRoundUp(noffH.code.size,PageSize);
            printf("Code page nUm= %d \n", codeNumPages);
            
            unsigned int initPageNum=noffH.code.virtualAddr/PageSize;

            printf("initPageNum== %d \n",initPageNum);
            unsigned int initOffset=PageSize - (noffH.code.virtualAddr % PageSize);


            printf("initOffset== %d \n",initOffset);

            printf("initial fileOff %d == \n",noffH.code.inFileAddr);
            unsigned int sizeRemain;

            if(noffH.code.size>initOffset){
            executable->ReadAt(&(machine->mainMemory[(pageTable[initPageNum].physicalPage*PageSize)+
            (noffH.code.virtualAddr % PageSize)]),
            initOffset,noffH.code.inFileAddr);
            sizeRemain =noffH.code.size - initOffset;

            }
            else{
             executable->ReadAt(&(machine->mainMemory[(pageTable[initPageNum].physicalPage*PageSize)+
            (noffH.code.virtualAddr % PageSize)]),
            noffH.code.size,noffH.code.inFileAddr);
            sizeRemain =noffH.code.size - noffH.code.size;   
            }
            
            //unsigned int sizeRemain =noffH.code.size - initOffset;
            printf("sizeRemain[after 1st read] == %d \n",sizeRemain);
            if(sizeRemain>0){
            for ( j = 1; j < codeNumPages; j++)
            {
            
            int fileOff = noffH.code.inFileAddr + initOffset  + (j-1) * PageSize;
    
            printf("fileOff %d == \n",fileOff);
            if(sizeRemain > PageSize){
                executable->ReadAt(&(machine->mainMemory[pageTable[initPageNum+j].physicalPage*PageSize]),
            PageSize, fileOff);
               sizeRemain-=PageSize;
               printf(" sizeRemain from if case = %d \n",sizeRemain);    
            }
            else {

                 executable->ReadAt(&(machine->mainMemory[pageTable[initPageNum+j].physicalPage*PageSize]),
            sizeRemain, fileOff);
            printf(" sizeRemain from if case = %d \n",sizeRemain);   

            }
            
            
            }
        }
        
        }

        if(noffH.initData.size>0)
        {
            printf("Data Size== %d \n",noffH.initData.size);

        	unsigned int dataNumPages;
        	dataNumPages=divRoundUp(noffH.initData.size,PageSize);
            
            printf("Code page nUm= %d \n", dataNumPages);
        	
            unsigned int initDataPageNum=noffH.initData.virtualAddr/PageSize;
            printf("initPageNum== %d \n",initDataPageNum);


            printf("Data virtual % PageSize = %d \n",(noffH.initData.virtualAddr % PageSize) );

            unsigned int initDataOffset=PageSize-(noffH.initData.virtualAddr % PageSize);
            printf("initDataOffset== %d \n",initDataOffset);

            printf("initial Data fileOff %d == \n",noffH.initData.inFileAddr);

            unsigned int sizeRemain2 ;
            if(noffH.initData.size > initDataOffset){            
            executable->ReadAt(&(machine->mainMemory[(pageTable[initDataPageNum].physicalPage*PageSize)+
            (noffH.initData.virtualAddr % PageSize)]),
            initDataOffset,noffH.initData.inFileAddr);
            sizeRemain2 =noffH.initData.size - initDataOffset;
        }
            else{
                executable->ReadAt(&(machine->mainMemory[(pageTable[initDataPageNum].physicalPage*PageSize)+
            (noffH.initData.virtualAddr % PageSize)]),
            noffH.initData.size,noffH.initData.inFileAddr);
                sizeRemain2 =noffH.initData.size - noffH.initData.size;

            } 
            printf("sizeRemain2[after 1st read of Data] == %d \n",sizeRemain2);

            if(sizeRemain2>0){
            for (j=1; j < dataNumPages; j++)
        	{
        		int fileOff = noffH.initData.inFileAddr + initDataOffset + (j-1) * PageSize;
            
            if(sizeRemain2 > PageSize){
                executable->ReadAt(&(machine->mainMemory[pageTable[initDataPageNum+j].physicalPage*PageSize]),
            PageSize, fileOff);
               sizeRemain2-=PageSize;    
            }
            else {
                 executable->ReadAt(&(machine->mainMemory[pageTable[initDataPageNum+j].physicalPage*PageSize]),
            sizeRemain2, fileOff);   

            }
        		
        	}
        }

        }
    interrupt->SetLevel(iStat);
*/



}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
   delete executable;
}

void AddrSpace::ReleaseMemory(){
    for (int i = 0; i < numPages; i++)
    {
        memoryManager->FreePage(pageTable[i].physicalPage);
        /* code */
    }
}

void AddrSpace::loadIntoFreePage(int vpn,int phyPageNo){
    int physicalPageNo=phyPageNo;
    pageTable[vpn].physicalPage = physicalPageNo;
    pageTable[vpn].valid = true;

    if(isSwapPageExists(vpn)){
        printf("swap page no = %d\n", pageTable[vpn].swapPage);
        loadFromSwapSpace(vpn);
        printf("\n\n______________Loading from swap memory_________\n\n");
        return;
    }
    
    bzero(&machine -> mainMemory[(pageTable[vpn].physicalPage) * PageSize], PageSize);

                 
     if(noffH.code.size>0)
        {
            printf("code size== %d \n",noffH.code.size);
            unsigned int codeNumPages;
            codeNumPages=divRoundUp(noffH.code.size,PageSize);
            //printf("Code page nUm= %d \n", codeNumPages);
            
            unsigned int initPageNum=noffH.code.virtualAddr/PageSize;

            //printf("initPageNum== %d \n",initPageNum);

            unsigned int lastPageNum=(noffH.code.virtualAddr+noffH.code.size)/ PageSize;

            //printf("lastPageNum== %d\n",lastPageNum);
            if( (vpn >= initPageNum) && (vpn <= lastPageNum) ){

                //printf("noffH.code.virtualAddr== %d\n", noffH.code.virtualAddr);
                //printf("vpn*PageSize==%d \n",vpn*PageSize);
                if(noffH.code.virtualAddr>= vpn*PageSize){
                    unsigned int initOffset=PageSize - (noffH.code.virtualAddr % PageSize);
                    printf("starting from vpn in Code= %d\n",(pageTable[vpn].physicalPage*PageSize)+
            (noffH.code.virtualAddr % PageSize));
                    printf("initOffset== %d \n",initOffset);
                    printf("code file init==%d \n",noffH.code.inFileAddr);
                    executable->ReadAt(&(machine->mainMemory[(pageTable[vpn].physicalPage*PageSize)+
            (noffH.code.virtualAddr % PageSize)]),
            initOffset,noffH.code.inFileAddr);
                }
                else if((noffH.code.virtualAddr+noffH.code.size) < (vpn+1)*PageSize){
                    //printf("else if\n");
                    unsigned int extra=(noffH.code.virtualAddr+noffH.code.size)-vpn*PageSize;
                    executable->ReadAt(&(machine->mainMemory[(pageTable[vpn].physicalPage*PageSize)]),
            extra,noffH.code.inFileAddr+((vpn*PageSize)-noffH.code.virtualAddr));
                }
                else
                {
                    //printf("else \n");
                    executable->ReadAt(&(machine->mainMemory[(pageTable[vpn].physicalPage*PageSize)]),
            PageSize,noffH.code.inFileAddr+((vpn*PageSize)-noffH.code.virtualAddr));
                }

            }
        
        }

        if(noffH.initData.size>0){

            printf("Data Size== %d \n",noffH.initData.size);
            unsigned int dataNumPages;
            dataNumPages=divRoundUp(noffH.initData.size,PageSize);
            printf("Code page nUm= %d \n", dataNumPages);
            unsigned int initDataPageNum=noffH.initData.virtualAddr/PageSize;
            printf("initDataPageNum== %d \n",initDataPageNum);

            unsigned int lastDataPageNum=(noffH.initData.virtualAddr+noffH.initData.size)/ PageSize;

            if(vpn>=initDataPageNum && vpn <= lastDataPageNum){

                  if(noffH.initData.virtualAddr>= vpn*PageSize){
                    unsigned int initDataOffset=PageSize - (noffH.initData.virtualAddr % PageSize);
                    printf("starting from vpn for Data= %d\n",(pageTable[vpn].physicalPage*PageSize)+
            (noffH.initData.virtualAddr % PageSize));
                    printf("initDataOffset== %d \n",initDataOffset);
                    printf("Data file init==%d \n",noffH.initData.inFileAddr);
                    executable->ReadAt(&(machine->mainMemory[(pageTable[vpn].physicalPage*PageSize)+
            (noffH.initData.virtualAddr % PageSize)]),
            initDataOffset,noffH.initData.inFileAddr);
                }
                else if((noffH.initData.virtualAddr+noffH.initData.size) < (vpn+1)*PageSize){
                    //printf("else if\n");
                    unsigned int extra=(noffH.initData.virtualAddr+noffH.initData.size)-vpn*PageSize;
                    executable->ReadAt(&(machine->mainMemory[(pageTable[vpn].physicalPage*PageSize)]),
            extra,noffH.initData.inFileAddr+((vpn*PageSize)-noffH.initData.virtualAddr));
                }
                else
                {
                    //printf("else \n");
                    executable->ReadAt(&(machine->mainMemory[(pageTable[vpn].physicalPage*PageSize)]),
            PageSize,noffH.initData.inFileAddr+((vpn*PageSize)-noffH.initData.virtualAddr));
                }


            }

        }
}


TranslationEntry*
AddrSpace::getPageTable(){
    return pageTable;
}
int
AddrSpace::getNumPages(){
    return numPages;
}


void
AddrSpace::loadFromSwapSpace(int vpn){
    printf("load from swap space\n");
    int physAddr = pageTable[vpn].physicalPage*PageSize;
    int swapPageNo = pageTable[vpn].swapPage;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    bcopy(swapSpace[swapPageNo].swapPageData, &machine->mainMemory[physAddr], PageSize);
    stats->pageIns++;
    interrupt->SetLevel(oldLevel);
}

void
AddrSpace::saveIntoSwapSpace(int vpn, int processId){
    printf("save into swap space\n");
    int freeSwapPage;
    if(isSwapPageExists(vpn)){
        freeSwapPage = pageTable[vpn].swapPage;
    }
    else {
        freeSwapPage = swapMemoryManager->AllocPage(processId,&(machine->pageTable[vpn]));
        pageTable[vpn].swapPage = freeSwapPage;
    }

    swapSpace[freeSwapPage].virtualPageNo = vpn;
    swapSpace[freeSwapPage].processId = processId;
    int physAddr = pageTable[vpn].physicalPage*PageSize;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    bcopy(&machine->mainMemory[physAddr], swapSpace[freeSwapPage].swapPageData, PageSize);
    stats->pageOuts++;
    interrupt->SetLevel(oldLevel);
    printf("\n____saving into swap space____\n");


}

bool
AddrSpace::isSwapPageExists(int vpn){
    return pageTable[vpn].swapPage != -1;
}




//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
