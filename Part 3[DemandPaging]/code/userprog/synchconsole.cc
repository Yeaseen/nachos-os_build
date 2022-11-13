#include "synchconsole.h"
#include "system.h"


static Semaphore *syncReadAvail;
static Semaphore *syncWriteDone;

static void isReadAvail(void* d) {
    syncReadAvail->V();
}

static void isWriteDone(void* f) {
    syncWriteDone->V();
}


synchconsole::synchconsole()
{
	int i=0;
	console = new Console(NULL, NULL, isReadAvail,isWriteDone, (void*)i);
    consoleLock = new Lock("Sync Console Lock");
    syncReadAvail = new Semaphore("read avail", 0);
    syncWriteDone = new Semaphore("write done", 0);
}


int synchconsole::Read(int buffer,int size, OpenFileId id){

    consoleLock->Acquire();
    printf("Start Reading from synchconsole \n");

    char data;
    int destination;
    int bytesRead=0;
    for (int i = 0; i < size; i++)
    {
    	syncReadAvail->P();
    	data=console->GetChar();
    	destination=buffer+i;

    	machine->WriteMem(destination,1,data);

    	bytesRead++;

    	if(data=='\n') break;
    	/* code */
    }

    consoleLock->Release();

    printf("Ending Read from synchconsole with returning \n");

    return bytesRead;
}


void synchconsole::Write(int buffer,int size, OpenFileId id){

    consoleLock->Acquire();

    printf("start writing from sychconsole\n");

    for (int i = 0; i < size; ++i)
    {
    	int source=buffer+i;
    	int dataInteger;
    	machine->ReadMem(source,1,&dataInteger);
    	char data=(char)dataInteger;
    	console->PutChar(data);

    	if(data=='\0' || data == '\n') break;
    	syncWriteDone->P();

    	/* code */
    }
    consoleLock->Release();

}