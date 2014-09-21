/**********************
*
* Progam Name: MP1. Membership Protocol
* 
* Current file: params.h
* About this file: Header file.
* 
***********************/

#ifndef _PARAMS_H_
#define _PARAMS_H_

#include "stdincludes.h"

/*
System parameters
*/

int MAX_NNB;                // max number of neighbors
int SINGLE_FAILURE;			// single/multi failure
double MSG_DROP_PROB;		// message drop probability
double STEP_RATE;		/* dictates the rate of insertion. */
int EN_GPSZ;			/* actual number of peers. */
int MAX_MSG_SIZE;
int DROP_MSG;
int dropmsg;
int globaltime;

void setparams();
int getcurrtime();

#endif /* _PARAMS_H_ */
