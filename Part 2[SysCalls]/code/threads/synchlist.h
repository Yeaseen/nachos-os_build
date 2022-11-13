// synchlist.h 
//	Data structures for synchronized access to a list.
//
//	Implemented by surrounding the List abstraction
//	with synchronization routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYNCHLIST_H
#define SYNCHLIST_H

#include "copyright.h"
#include "list.h"
#include "synch.h"

// The following class defines a "synchronized list" -- a list for which:
// these constraints hold:
//	1. Threads trying to remove an item from a list will
//	wait until the list has an element on it.
//	2. One thread at a time can access list data structures

template <class Item>
class SynchList {
  public:
    SynchList();		// initialize a synchronized list
    ~SynchList();		// de-allocate a synchronized list

    void Append(Item item);	// append item to the end of the list,
				// and wake up any thread waiting in remove
    Item Remove();		// remove the first item from the front of
				// the list, waiting if the list is empty
				
    void Apply(void (*func)(Item));
				// apply function to every item in the list

  private:
    List<Item> *list;		// the unsynchronized list
    Lock *lock;			// enforce mutual exclusive access to the list
    Condition *listEmpty;	// wait in Remove if the list is empty
};

//----------------------------------------------------------------------
// SynchList::SynchList
//	Allocate and initialize the data structures needed for a 
//	synchronized list, empty to start with.
//	Elements can now be added to the list.
//----------------------------------------------------------------------

template <class Item>
SynchList<Item>::SynchList()
{
    list = new List<Item>;
    lock = new Lock("list lock"); 
    listEmpty = new Condition("list empty cond");
    }

//----------------------------------------------------------------------
// SynchList::~SynchList
//	De-allocate the data structures created for synchronizing a list. 
//----------------------------------------------------------------------

template <class Item>
SynchList<Item>::~SynchList()
{ 
    delete list; 
    delete lock;
    delete listEmpty;
}

//----------------------------------------------------------------------
// SynchList::Append
//      Append an "item" to the end of the list.  Wake up anyone
//	waiting for an element to be appended.
//
//	"item" is the thing to put on the list
//----------------------------------------------------------------------

template <class Item>
void
SynchList<Item>::Append(Item item)
{
    lock->Acquire();		// enforce mutual exclusive access to the list 
    list->Append(item);  	// wake up a waiter, if any
    listEmpty->Signal(lock);	// wake up a waiter, if any
    lock->Release();
}

//----------------------------------------------------------------------
// SynchList::Remove
//      Remove an "item" from the beginning of the list.  Wait if
//	the list is empty.
// Returns:
//	The removed item. 
//----------------------------------------------------------------------

template <class Item>
Item 
SynchList<Item>::Remove()
{
    Item item;

    lock->Acquire();			// enforce mutual exclusion
    while (list->IsEmpty())
	listEmpty->Wait(lock);  	// wait until list isn't empty
    item = list->Remove();
    //ASSERT(item != NULL);
    lock->Release();
    return item;
}

//----------------------------------------------------------------------
// SynchList::Apply
//      Apply function to every item on the list.  Obey mutual exclusion
//	constraints.
//
//	"func" is the procedure to be applied.
//----------------------------------------------------------------------

template <class Item>
void
SynchList<Item>::Apply(void (*func)(Item))
{ 
    lock->Acquire(); 
    list->Apply(func);
    lock->Release(); 
}

#endif // SYNCHLIST_H
