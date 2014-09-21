/**********************
*
* Progam Name: MP1. Membership Protocol
* 
* Current file: emulnet.c
* About this file: EmulNet Layer.
* 
***********************/

#include "emulnet.h"

#define MAX_NODES 1000
#define MAX_TIME 3600

static int sent_msgs[MAX_NODES + 1][MAX_TIME];
static int recv_msgs[MAX_NODES + 1][MAX_TIME];

/* 
Initialize the EmulNet. Called by each member when it starts up. 
Returns NULL if and only if initialization is incomplete.
*/
void *ENinit(struct address *myaddr, short port, char *joinaddr){

	static int eninited=0;
   int i, j;

	/* 639 == magic number */
	if(eninited != 639){ /* Initialize EmulNet. */
		eninited=639;

		emulnet.nextid=1;
		emulnet.currbuffsize=0;

		for (i = 0; i < MAX_NODES; i++) {
			for (j = 0; j < MAX_TIME; j++) {
				sent_msgs[i][j] = 0;
				recv_msgs[i][j] = 0;
			}
      }
	}
	
	/* Initialize data structures for this member. */
	*(int *)(myaddr)=emulnet.nextid++;
    *(short *)(&myaddr->addr[4])=0;


	return myaddr;
}

/* 
Called at peer myaddr to send a message (data,size) to peer toaddr. Send is reliable. 
*/
int ENp2psend(struct address *myaddr, struct address *toaddr, char *data, int size){
	en_msg *em;
	static char buff[2048];

	int sendmsg = rand() % 100;

	if(emulnet.currbuffsize>=ENBUFFSIZE || size+ (int) sizeof(en_msg)>=MAX_MSG_SIZE || (dropmsg && sendmsg < (int) (MSG_DROP_PROB * 100)))
			return 0;

	em=(en_msg *)malloc(sizeof(en_msg) + size);
	em->size=size;
	memcpy(&em->from, myaddr, sizeof(address));
	memcpy(&em->to, toaddr, sizeof(address));

	memcpy(em+1, data, size);

	emulnet.buff[emulnet.currbuffsize++]=em;

	int src = *(int *)(myaddr->addr);
	int time = getcurrtime();

	assert(src <= MAX_NODES);
	assert(time < MAX_TIME);

	sent_msgs[src][time]++;

#ifdef DEBUGLOG
	sprintf(buff, "Sending 4+%d B msg type %d to %d.%d.%d.%d:%d ", size-4, *(int *)data, toaddr->addr[0], toaddr->addr[1], toaddr->addr[2], toaddr->addr[3], *(short *)&toaddr->addr[4]);
#endif

	return size;
}

/* 
Called at peer myaddr to receive all messages in EmulNet destined for the peer. 
For each message, function *enq is called. 
*/
int ENrecv(struct address *myaddr, int (* enq)(void *, char *, int), struct timeval *t, int times, void *env){
	/* 'times' always assumed to be 1 */
	int i;
	char *tmp;
	int sz;
	en_msg *emsg;

	for(i=emulnet.currbuffsize-1; i>=0;i--){
		emsg = emulnet.buff[i];

		if(memcmp(&emsg->to, myaddr, sizeof(address)) == 0){
			sz=emsg->size;
			tmp=(char*) malloc(sz * sizeof(char));
			memcpy(tmp, (char *)(emsg+1), sz);

			emulnet.buff[i]=emulnet.buff[emulnet.currbuffsize-1];
			emulnet.currbuffsize--;
			
			(*enq)(env, tmp, sz);

			free(emsg);

			int dst = *(int *)(myaddr->addr);
			int time = getcurrtime();

			assert(dst <= MAX_NODES);
			assert(time < MAX_TIME);

			recv_msgs[dst][time]++;
		}
	}

	return 0;
}

/* 
Cleanup the EmulNet. Called exactly once at the end of the program. 
*/
int ENcleanup(){
	emulnet.nextid=0;
	int i, j;
	int sent_total, recv_total;

	FILE* file = fopen("msgcount.log", "w+");

	while(emulnet.currbuffsize>0) 
		free(emulnet.buff[--emulnet.currbuffsize]);

	for (i = 1; i <= EN_GPSZ; i++) {

		fprintf(file, "node %3d ", i);
		sent_total = 0;
		recv_total = 0;

		for (j = 0; j < getcurrtime(); j++) {

			sent_total += sent_msgs[i][j];
			recv_total += recv_msgs[i][j];
         if (i != 67) {
				fprintf(file, " (%4d, %4d)", sent_msgs[i][j], recv_msgs[i][j]);
				if (j % 10 == 9) { fprintf(file, "\n         "); }
			} else {
				fprintf(file, "special %4d %4d %4d\n", j, sent_msgs[i][j], recv_msgs[i][j]);
			}
		}
		fprintf(file, "\n");
		fprintf(file, "node %3d sent_total %6u  recv_total %6u\n\n", i, sent_total, recv_total);
	}

	fclose(file);
	return 0;
}
