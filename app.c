/**********************
*
* Progam Name: MP1. Membership Protocol
*
* Current file: app.c
* About this file: App (Application) Layer. Main simulator loop ( main() ).
* 
***********************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "app.h"
#include "MPtemplate.h"
#include <stdlib.h>

/*
 *
 * Main function.
 *
 */
int main(int argc, char *argv[]){

 int i,removed;
 srand (time(NULL));
 
 if (argc < 1)
 {
	printf("Config File Required\n");
	return 1;
 }

 setparams(argv[1]);
 srand(time(NULL));

 MPinit=ENinit,MPp2psend=ENp2psend,MPrecv=ENrecv,MPcleanup=ENcleanup;
 group = malloc(MAX_NNB*sizeof(member)); 
 

 /* 
 Create an array of processes, 
 Call nodestart() on each of them, 
 Periodically call nodeloop() on each of them (once per time unit)
 Finally call finishup_thisnode() on each one of them
 */

 for(i=0;i<=EN_GPSZ-1;i++)
	 group[i].inited=0;

 
 for(globaltime=0; globaltime<500; ++globaltime) {
	 
	 /* call recvloop for all nodes currently in the system */
	 for(i=0;i<=EN_GPSZ-1;i++)
		 if(getcurrtime()>(int)(STEP_RATE*i) && group[i].bfailed==0)
			 recvloop(&group[i]);

	for(i=EN_GPSZ-1;i>=0;i--) {
		 if(getcurrtime() == (int)(STEP_RATE*i)) {
			 /* introduce the ith node into the system at time STEPRATE*i */
			 nodestart(&group[i], JOINADDR, PORTNUM); /* last two params not used here */
		 	printf("%d-th introduced node is assigned with the address: ", i);
		 }
		 else if(getcurrtime()>(int)(STEP_RATE*i) && group[i].bfailed==0) {
			nodeloop(&group[i]);
			 
#ifdef DEBUGLOG
			 if(i==0&&globaltime%500==0)
				 LOG(&group[0].addr, "@@time=%d", getcurrtime());
#endif
		 }
	 }

	/* fail half the members at time t=400 */
	if(DROP_MSG && getcurrtime()==50)
		dropmsg = 1;

	if(SINGLE_FAILURE && getcurrtime()==100)
	{
		removed = rand() % EN_GPSZ;

#ifdef DEBUGLOG
		LOG(&group[removed].addr, "Node failed at time=%d", getcurrtime());
#endif
		group[removed].bfailed=1;
	}
	else if(getcurrtime() == 100)
	{
		removed = rand() % EN_GPSZ/2;
		for (i = removed; i < removed + EN_GPSZ/2; i++)
		{
#ifdef DEBUGLOG
			LOG(&group[i].addr, "Node failed at time=%d", getcurrtime());
#endif
			group[i].bfailed = 1;
		}
	}

	if(DROP_MSG && getcurrtime()==300)
		dropmsg=0;

 }

 MPcleanup();
 
 for(i=0;i<=EN_GPSZ-1;i++)
	 finishup_thisnode(&group[i]);

 return 0;
}
