//Memory Management requires when there is multiple processes running in Nachos.
//So every process gets its required spaces from this class[actually from a Bitmap object(global)]

#ifndef USER_PROGRAM_MEMORYMANAGER_H
#define USER_PROGRAM_MEMORYMANAGER_H

#include "copyright.h"
#include "bitmap.h"
#include "synch.h"


class SwapSpace{
public:
	int virtualPageNo;
	int processId;
	char* swapPageData;
	SwapSpace(){
		processId = -1;
		virtualPageNo = -1;
		swapPageData = new char[PageSize];
	}
};

class memorymanager
{
public:
	memorymanager(int numPhysPages); 
	~memorymanager();
	int AllocPage(int process_id, TranslationEntry * entry);
	void FreePage(int physPageNum);
	bool PageIsAllocated(int physPageNum);
	int GetClearPages();

	int AllocByForce();
	int *processMap;
	TranslationEntry **entry;

private:
	BitMap *bmap;
	Lock *block;
	
	int numPages;
};

#endif