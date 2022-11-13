#include "copyright.h"
#include "memorymanager.h"

memorymanager::memorymanager(int numPhysPages){
	bmap= new BitMap(numPhysPages);
	block= new Lock("bitmapblock");
}

memorymanager::~memorymanager()
{
  delete bmap;
  delete block;
}

int memorymanager::AllocPage(){
	int free_page;
	block->Acquire();
	free_page=bmap->Find();
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