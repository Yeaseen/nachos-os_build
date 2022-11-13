#include "processtable.h"


processtable::processtable(int tsize){
	runningprocess=0;
	tablesize=tsize;
	tablelock= new Lock("tablelock");
	ProcTable =new void* [tablesize];

	for (int i = 0; i < tablesize; ++i)
	{
		ProcTable[i]='\0';
		/* code */
	}
}

processtable::~processtable(){
	//delete [] ProcTable;
	delete tablelock;

}

int processtable::Alloc(void* object){
	int res=-1;
	tablelock->Acquire();
	for (int i = 0; i < tablesize; ++i)
	{
		if(ProcTable[i]=='\0')
		{
			ProcTable[i]=object;
			runningprocess++;
			res=i;
			break;
		}
		/* code */
	}
	tablelock->Release();
	return res;
}


void* processtable::GetObject(int idx){
	void* content;
	tablelock->Acquire();
	if(idx >=0 && idx<tablesize)
	{
		content=ProcTable[idx];
	}
	else content='\0';
	tablelock->Release();
	return content;
}


void processtable::Remove(int idx){
	tablelock->Acquire();
	ProcTable[idx]='\0';
	runningprocess--;
	tablelock->Release();
}

int processtable::Find(void* object){
	int res=-1;
	tablelock->Acquire();
	for (int i = 0; i < tablesize; ++i)
	{
		if(ProcTable[i]==object)
		{
			res=i;
			break;
		}
		/* code */
	}
	tablelock->Release();
	return res;

}

int processtable::GetRunningPro()
{
	int res;
	tablelock->Acquire();
	res=runningprocess;
	tablelock->Release();
	return res;
}



