#ifndef USER_PROGRAM_PROCESSTABLE_H
#define USER_PROGRAM_PROCESSTABLE_H

#include "copyright.h"
#include "thread.h"
#include "synch.h"
class processtable
{
public:
	processtable(int tsize);
	~processtable();
	int Alloc(void* object);
	void* GetObject(int idx);
	void Remove(int idx);
	int Find(void* object);
	int GetRunningPro();

private:
	void** ProcTable;
	int tablesize;
	int runningprocess;
	Lock* tablelock;
	
};
#endif