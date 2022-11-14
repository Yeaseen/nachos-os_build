// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "producer.h"
#include "consumer.h"


int size;
Lock* pro_con_lock;
Condition* pro_condition;
Condition* con_condition;
List<int> *list_of_production;
int curr_product;




//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void SimpleThread(void* which){
  		int num;
    		for (num = 0; num < 5; num++) {
			printf("*** thread %d looped %d times\n", *((int*)which), num);
        		currentThread->Yield();
    		}
	}
//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------


void proThread(void* demand)
{
   int d=*((int*)demand);
   producer *produce = new producer(d);
   produce->_Produce();
}

void conThread(void* demand)
{
    int d=*((int*)demand);
    consumer *consume = new consumer(d);
    consume->_Consume();

}

void init()
{
    list_of_production = new List<int>;
    size=0;
    curr_product=1;
    pro_con_lock = new Lock("pro_con_lock");
    pro_condition= new Condition("pro_condition");
    con_condition= new Condition("con_condition");
    
}

void
ThreadTest()
{   
    DEBUG('t', "Entering SimpleTest");
    //printf("ami tomari romari \n");

    init();
    Thread* prThread[5];
    Thread* cnThread[5];
   
    for(int i=0;i<5;i++){
        prThread[i] = new Thread("prod");
        int* j=new int(i+1);
        prThread[i]->Fork(proThread, j);

    }

    for(int i=0;i<5;i++){
        cnThread[i] = new Thread("cons");
        int* j=new int(i+1);
        cnThread[i]->Fork(conThread, j);
    }

    //Thread *t = new Thread("forked thread");

      //  int* j=new int(1);
    	//t->Fork(SimpleThread, i);
    	//SimpleThread(i);

}

