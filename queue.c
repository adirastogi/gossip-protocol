/**********************
*
* Progam Name: MP1. Membership Protocol.
* 
* Current file: queue.c
* About this file: Queue implementation.
* 
***********************/

#include "queue.h"

int initqueue(queue *q){

	return QSUCCESS;
}

int enqueue(queue *q, void *elt, int size){
	if(q->qleft%MAXQUEUESIZE == (q->qright+1)%MAXQUEUESIZE)
		return QFULL;

		q->q[q->qright].elt = elt;
		q->q[q->qright].size=size;
		
		q->qright = (q->qright+1)%MAXQUEUESIZE;
	
	return QSUCCESS;	
}

void *dequeue(queue *q, int *sz){
	void *retptr;
	if(q->qleft == q->qright)
		return NULL;

		*sz=q->q[q->qleft].size;
		retptr = q->q[q->qleft].elt;

		q->qleft = (q->qleft+1)%MAXQUEUESIZE;
	
	return retptr;
}

int destroyqueue(queue *q){

	return QSUCCESS;
}

