//Memory Management requires when there is multiple processes running in Nachos.
//So every process gets its required spaces from this class[actually from a Bitmap object(global)]

#ifndef USER_PROGRAM_SYNCHCONSOLE_H
#define USER_PROGRAM_SYNCHCONSOLE_H

#include "copyright.h"
#include "syscall.h"
#include "console.h"
#include "synch.h"

class synchconsole
{
public:
	synchconsole();
	~synchconsole();

    int Read(int buffer,int size, OpenFileId id);
    void Write(int buffer,int size, OpenFileId id);


    Console* console;
    Lock* consoleLock;
	
};




#endif