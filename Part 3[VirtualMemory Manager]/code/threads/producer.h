#ifndef THREADS_PRODUCER_H_
#define THREADS_PRODUCER_H_
#include "copyright.h"
#include "synch.h"
#include "thread.h"
#include "system.h"

#define MAXSIZE 10

extern int size;
extern Lock* pro_con_lock;
extern Condition* pro_condition;
extern Condition* con_condition;
extern List<int> *list_of_production;
extern int curr_product;


class producer
{
public:
	producer(int demand);
	~producer();
	void _Produce();
	int demand;
	
};

#endif /* THREADS_PRODUCER_H_ */
