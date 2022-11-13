#ifndef THREADS_CONSUMER_H_
#define THREADS_CONSUMER_H_
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


class consumer
{
public:
	consumer(int demand);
	~consumer();

	void _Consume();
	int demand;
};

#endif /* THREADS_PRODUCER_H_ */
