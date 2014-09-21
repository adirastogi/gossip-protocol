/**********************
*
* Progam Name: MP1. Membership Protocol
* 
* Current file: MPtemplate.h
* About this file: Header file.
* 
***********************/

#ifndef _MPTEMPLATE_H_
#define _MPTEMPLATE_H_

void *(*MPinit)(struct address *, short, char *);
int (*MPp2psend)(struct address *, struct address *addr, char *data, int size);
int (*MPrecv)(struct address *myaddr, int (* enq)(void *, char *, int), struct timeval *t, int times, void *env);
int (*MPcleanup)();

#endif /* _MPTEMPLATE_H_ */
