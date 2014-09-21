/**********************
*
* Progam Name: MP1. Membership Protocol
* 
* Current file: requests.h
* About this file: Header file.
* 
***********************/

#ifndef _REQUESTS_H_
#define _REQUESTS_H_


enum Lkupstatus { RFULL,RQUEUED,RFAIL };
	/*	RFULL : outstanding request bufer is full, 
		RQUEUED : request has been queued.
		others imply success or failure of lookup in the DHT */

#endif /* _REQUESTS_H_ */
