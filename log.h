/**********************
*
* Progam Name: MP1. Membership Protocol.
* 
* Current file: debug.h
* About this file:  Header file.
* 
***********************/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "stdincludes.h"
#include "nodeaddr.h"

void LOG(address *, char * str, ...);
void logNodeAdd(address *, address *);
void logNodeRemove(address *, address *);

#endif /* _DEBUG_H_ */
