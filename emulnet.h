/**********************
*
* Progam Name: MP1. Membership Protocol.
* 
* Current file: emulnet.h
* About this file: Header file.
* 
***********************/

#ifndef _EMULNET_H_
#define _EMULNET_H_

#include "nodeaddr.h"
#include "log.h"
#include "params.h"

#define ENBUFFSIZE 30000

typedef struct en_msg{
	int size;			/* num.	of bytes after the struct */
	
	address from;		/* src node */
	address to;			/* dest node */

} en_msg;

struct EM{
	int nextid;			/* max id assigned so far */

	en_msg *buff[ENBUFFSIZE];

	int currbuffsize;
	int firsteltindex;
};

struct EM emulnet;

void *ENinit(struct address *myaddr, short port, char *joinaddr);
int ENp2psend(struct address *myaddr, struct address *addr, char *data, int size);
int ENrecv(struct address *myaddr, int (* enq)(void *, char *, int), struct timeval *t, int times, void *env);
int ENcleanup();

#endif /* _EMULNET_H_ */
