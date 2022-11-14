#include "copyright.h"
#include "memorymanager.h"

memorymanager::memorymanager(int numPhysPages){
	bmap= new BitMap(numPhysPages);
	block= new Lock("bitmapblock");
	this->numPages=numPhysPages;
	processMap= new int[numPhysPages];
	entry=new TranslationEntry*[numPhysPages];

}

memorymanager::~memorymanager()
{
  delete bmap;
  delete block;
}

int memorymanager::AllocPage(int process_id, TranslationEntry * ent){
	int free_page;
	block->Acquire();
	free_page=bmap->Find();
	processMap[free_page] = process_id;
	entry[free_page] = ent;
	block->Release();
	return free_page;
}

int memorymanager::AllocByForce(){
	block->Acquire();
	int free_page=random()%numPages;
	unsigned long int time=entry[free_page]->timer;

	for (int i = 0; i < numPages; i++)
	{
		if(entry[i]->timer < time && !(entry[i]->dirty)){
			free_page = i;
			time=entry[i]->timer;
		}
	}
	block->Release();
	return free_page;
}


void memorymanager::FreePage(int physPageNum){
	block->Acquire();
	bmap->Clear(physPageNum);
	block->Release();
}

bool memorymanager::PageIsAllocated(int physPageNum){
	bool isFree;
	block->Acquire();
	isFree=bmap->Test(physPageNum);
	block->Release();
	return isFree;
}

int memorymanager::GetClearPages(){
	int ans;
	block->Acquire();
	ans=bmap->NumClear();
	block->Release();
	return ans;
}