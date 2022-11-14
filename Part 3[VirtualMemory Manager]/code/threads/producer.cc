#include "copyright.h"
#include "producer.h"

producer::producer(int demand)
{
	this->demand=demand;
}

producer::~producer()
{


}






void
producer::_Produce()
{
	
	while(true)
	{
		pro_con_lock->Acquire();
		if(size == MAXSIZE)
		{
			pro_condition->Wait(pro_con_lock);
		}
		else
		{
		int i= curr_product;
		list_of_production->Append(i);
		size=size+1;
		curr_product=curr_product+1;
		printf("producer id %d : produced %d : BufferIndex  %d\n",demand,i,size);
        int k=0;
		while(k<500000){
			k++;
		}
		
		
		if(size == 1 ) con_condition->Broadcast(pro_con_lock);
		
        int l=0;
		while(l<500000){
			l++;
		}
	}

		pro_con_lock->Release();
				
		currentThread->Yield();



	}


}