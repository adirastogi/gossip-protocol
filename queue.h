/**********************
*
* Progam Name: MP1. Membership Protocol.
* 
* Current file: queue.h
* About this file: Header file.
* 
***********************/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "stdincludes.h"

#define MAXQUEUESIZE 1000

enum queuestatus{QSUCCESS, QFULL, QEMPTY};
/* QEMPTY not used */

typedef struct q_elt{
	void *elt;
	int size;
} q_elt;

typedef struct queue{

	q_elt q[MAXQUEUESIZE];
	int qleft, qright;		/* all elements are in qleft...qright-1 modulo MAXQUEUESIZE */

} queue;

int initqueue(queue *q);
int enqueue(queue *q, void *elt, int size);
void *dequeue(queue *q, int *sz);

#endif	/* _QUEUE_H_ */
