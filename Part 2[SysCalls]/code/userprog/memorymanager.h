//Memory Management requires when there is multiple processes running in Nachos.
//So every process gets its required spaces from this class[actually from a Bitmap object(global)]

#ifndef USER_PROGRAM_MEMORYMANAGER_H
#define USER_PROGRAM_MEMORYMANAGER_H

#include "copyright.h"
#include "bitmap.h"
#include "synch.h"

class memorymanager
{
public:
	memorymanager(int numPhysPages); 
	~memorymanager();
	int AllocPage();
	void FreePage(int physPageNum);
	bool PageIsAllocated(int physPageNum);
	int GetClearPages();

private:
	BitMap *bmap;
	Lock *block;

};

#endif