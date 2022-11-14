// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "memorymanager.h"
#include "processtable.h"
#include "openfile.h"
#include "synch.h"
#include "addrspace.h"
#include "synchconsole.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

#define BUFF_MAX 100

extern Lock* execLock;
extern processtable *processTable;
extern memorymanager *memoryManager;
extern Lock* exitLock;
extern synchconsole *synchConsole;

int pagefault=0;

int myExec(char* fName){

	OpenFile* executable = fileSystem->Open(fName);
	if(executable == NULL){
		printf("Unable to open file %s\n", fName);
			return -1;
	}

	AddrSpace* spc = new AddrSpace(executable);
	Thread* th = new Thread("test");
	printf(" creating thread for myExec \n");
	th->process_id=processTable->Alloc((void*)th);
	ASSERT(th->process_id!=-1);
	th->space=spc;

	//delete executable;

    printf("Created thread's process_id= %d\n", th->process_id );
	return th->process_id;
}

void running(void* spaceFrom){
	printf("===== running from frok function =======\n");
	AddrSpace* space=(AddrSpace*) spaceFrom;
	space->InitRegisters();
	space->RestoreState();
	machine->Run();
	//ASSERT(FALSE);
}

void incrementPC(){
	printf("===increment PC value ======\n");
	int pc = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, pc);
	pc = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, pc);
	pc+=4;
	machine->WriteRegister(NextPCReg, pc);
}

void myExit(int args){
	printf("== myExit starting =====\n");
	exitLock->Acquire();

	printf("Exit code= %d\n", args);
	
    int pageTblSize = machine->pageTableSize;
	for(int i=0;i<pageTblSize;i++){
		if(machine->pageTable[i].valid) {
			memoryManager->FreePage(machine->pageTable[i].physicalPage);
		}

	}
	printf("== After Freeing Pages ===\n");

	if(processTable->GetRunningPro() != 0){
		processTable->Remove(currentThread->process_id);

		printf("== After removing content from PageTable ====\n");
	}

	exitLock->Release();
          
	if(processTable->GetRunningPro() == 0){
		printf("===In Exit's Halt=== \n");
		//exitLock->Release();
		interrupt->Halt();
	}
	else{
		printf("==program exited for process_id= %d \n",currentThread->process_id);
		currentThread->Finish();
	}
	//incrementPC();
}


int Read(int buffer, int size, OpenFileId id){
   int bytes=synchConsole->Read(buffer,size,id);
   return bytes;
}

void myRead(int buffer,int size){
	printf("Reading from buffer  of ---> %d  size= %d\n",buffer,size);

	int bytesRead=Read(buffer,size,0);

    printf("Read feinsh from buffer, size = %d\n",bytesRead );
    machine->WriteRegister(2,bytesRead);
    incrementPC();
}

void Write(int buffer,int size, OpenFileId id){
	synchConsole->Write(buffer,size,id);
}

void myWrite(int buffer,int size){
   printf("Writing from buffer of ---> %d  size= %d\n",buffer,size);

   Write(buffer,size,0);

   printf(" After writing from myWrite\n");
   machine->WriteRegister(2,1);
   incrementPC();

}

void myPageManager(int vpn)
{
	int physicalPageNo;
	if(memoryManager->GetClearPages()>0)
	{
		physicalPageNo=memoryManager->AllocPage(currentThread->process_id,&(machine->pageTable[vpn]));
	}
	else{
		printf("AllocByFore is called \n");

		physicalPageNo=memoryManager->AllocByForce();
		int processNo = memoryManager->processMap[physicalPageNo];
		int virtNo = memoryManager->entry[physicalPageNo]->virtualPage;
		Thread* process = (Thread*)processTable->GetObject(processNo);
		//TranslationEntry* table = process->space->getPageTable();
				
		if( memoryManager->entry[physicalPageNo]->swapPage == -1 || memoryManager->entry[physicalPageNo]->dirty ){

			process->space->saveIntoSwapSpace(virtNo, processNo);

		}
		memoryManager->entry[physicalPageNo]->valid = false;
		memoryManager->entry[physicalPageNo]->physicalPage = -1;

		memoryManager->processMap[physicalPageNo]=process->process_id;
		memoryManager->entry[physicalPageNo]=&(machine->pageTable[vpn]);

		}
	
	printf("loading code segment.. data segment..\n");		
			
	currentThread->space->loadIntoFreePage(vpn,physicalPageNo);
}


void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    int arg1 = machine->ReadRegister(4);
    int arg2 = machine->ReadRegister(5);
    int arg3 = machine->ReadRegister(6);
    int arg4 = machine->ReadRegister(7);
    
    printf("__syscall = %d______\n",which);
    printf("___type = %d___\n",type);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    }

    else if((which == SyscallException) && (type == SC_Exec)){
    	execLock->Acquire();
    	  int path=arg1;
    	  char buffer[BUFF_MAX];
    	  int i=0;
    	  int var;

    	machine->ReadMem(path,1,&var);
		
		while((*(char*)&var) != '\0'){
			buffer[i] = (char) var;
			printf("\n----- %c --------\n",buffer[i]);
			i++;
			path++;
			machine->ReadMem(path, 1, &var);
				
		}
		buffer[i]='\0';

		int retval=myExec(buffer);
		printf(" === retval from myExec = %d \n", retval);

        execLock->Release();

		if(retval!=-1){
		 
		 Thread* t=(Thread*)processTable->GetObject(retval);
		 printf("===Getting Thread from pTable ====\n");
		 t->Fork(running,(void*)t->space);
		}
        machine->WriteRegister(2, retval);
        incrementPC();

    }
    else if((which == SyscallException) && (type == SC_Exit)){
    	printf("in Exit call start\n");
         myExit(arg1);
    }

    else if((which == SyscallException) && (type == SC_Read)){
    	printf("Starting SC_Read call \n");

    	myRead(arg1,arg2);

    }
    else if((which == SyscallException) && (type == SC_Write)){
    	printf("Starting SC_Write call \n");
    	myWrite(arg1,arg2);

    }
    else if(which == PageFaultException){
    	printf("There is a PageFaultException\n");
    	stats->numPageFaults++;
    	int virtualAddrs=machine->ReadRegister(39);
    	printf("virtualAddress== %d\n",virtualAddrs);
    	int vpn=virtualAddrs/PageSize;
    	printf("vpn==%d \n",vpn);
    	myPageManager(vpn);
    	//ASSERT(false);
    }
    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(false);
    }
}
