

#include "copyright.h"
#include "consumer.h"


consumer::consumer(int demand)
{
	this->demand=demand;
}

consumer::~consumer()
{

}

void
consumer::_Consume()
{
	
	while(true)
	{
		pro_con_lock->Acquire();
		if(size == 0)
		{
			con_condition->Wait(pro_con_lock);
		}
		else
		{
		int i=list_of_production->Remove();
		size=size-1;
        printf("Consumer id %d :  got %d :  BufferIndex=  %d\n",demand,i,size );
		int k=0;
		while(k<500000){
			k++;
		}
		

		
		if(size == MAXSIZE-1)
		{
			pro_condition->Broadcast(pro_con_lock);
		}

		int l=0;
		while(l<500000){
			l++;
		}
	    }
		
		pro_con_lock->Release();

		currentThread->Yield();
	}
}