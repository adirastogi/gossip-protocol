/**********************
*
* Progam Name: MP1. Membership Protocol.
* 
* Current file: log.c
* About this file: Debug logging.
* 
***********************/

#include "log.h"
#define MAXWRITES 1			/* num. of writes after which to flush file. */

extern long getcurrtime();

/*
Print out to file dbg.log, along with address of node.
*/
void LOG(address *addr, char * str, ...){

	static FILE *fp;
	static FILE *fp2;
	va_list vararglist;
	static char buffer[30000];
	static int numwrites;
	static char stdstring[30];
	static char stdstring2[40];
	static char stdstring3[40]; 
	static int dbg_opened=0;

	if(dbg_opened != 639){
		numwrites=0;

		stdstring2[0]=0;

		strcpy(stdstring3, stdstring2);

		strcat(stdstring2, "dbg.log");
		strcat(stdstring3, "stats.log");

		fp = fopen(stdstring2, "a+");
		fp2 = fopen(stdstring3, "a+");

		dbg_opened=639;
	}
	else 

	sprintf(stdstring, "%d.%d.%d.%d:%d ", addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], *(short *)&addr->addr[4]);

	va_start(vararglist, str);
	vsprintf(buffer, str, vararglist);
	va_end(vararglist);

	if(memcmp(buffer, "#STATSLOG#", 10)==0){
		fprintf(fp2, "\n %s", stdstring);
		fprintf(fp2, "[%ld] ", getcurrtime());

		fprintf(fp2, buffer);
	}
	else{
		fprintf(fp, "\n %s", stdstring);
		fprintf(fp, "[%ld] ", getcurrtime());
		fprintf(fp, buffer);

	}

	if(++numwrites >= MAXWRITES){
		fflush(fp);
		fflush(fp2);
		numwrites=0;
	}

}

void logNodeAdd(address *thisNode, address *addedAddr)
{
	static char stdstring[30];
	sprintf(stdstring, "Node %d.%d.%d.%d:%d joined", addedAddr->addr[0], addedAddr->addr[1], addedAddr->addr[2], addedAddr->addr[3], *(short *)&addedAddr->addr[4]);
    LOG(thisNode, stdstring);
}

void logNodeRemove(address *thisNode, address *removedAddr)
{
	static char stdstring[30];
	sprintf(stdstring, "Node %d.%d.%d.%d:%d removed", removedAddr->addr[0], removedAddr->addr[1], removedAddr->addr[2], removedAddr->addr[3], *(short *)&removedAddr->addr[4]);
    LOG(thisNode, stdstring);
}
