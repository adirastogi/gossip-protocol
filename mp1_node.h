/**********************
*
* Progam Name: MP1. Membership Protocol.
* 
* Code authors: <your name here>
*
* Current file: mp2_node.h
* About this file: Header file.
* 
***********************/

#ifndef _NODE_H_
#define _NODE_H_

#include "stdincludes.h"
#include "params.h"
#include "queue.h"
#include "requests.h"
#include "emulnet.h"

/* Configuration Parameters */
char JOINADDR[30];                    /* address for introduction into the group. */
extern char *DEF_SERVADDR;            /* server address. */
extern short PORTNUM;                /* standard portnum of server to contact. */



/* Definition of one entry of the membership table */
typedef struct MemberEntry{
    struct address addr;                // the address of the node
    int last_local_timestamp;           // the local timestamp for the last update
    int64_t last_hb;                        // the heartbeat on the last update
    int mark_fail;                      // whether the node has been marked for fail
    int mark_del;                       //whether the node has been marked for deletion
} MemberEntry;

/* Miscellaneous Parameters */
extern char *STDSTRING;

typedef struct member{            
        struct address addr;            // my address
        int inited;                     // boolean indicating if this member is up
        int ingroup;                    // boolean indiciating if this member is in the group

        queue inmsgq;                   // queue for incoming messages

        int bfailed;                    // boolean indicating if this member has failed
        
        /* Fields specific to the membership table that is maintatined at this node */
        int numMemberEntries;           // the number of member entries currently in the table

        struct MemberEntry* memberList; // the membership table at this node

        int tfail;                      // the time after which this node marks entries as failed
        int tdelete;                    // the time after which this node deletes a failed entry
        int tgossip;                    // the time after which this node gossips its membership table to a random node.

} member;

/* Message types */
/* Meaning of different message types
  JOINREQ - request to join the group
  JOINREP - replyto JOINREQ
*/
enum Msgtypes{
		JOINREQ,			
		JOINREP,
        GOSSIP,
		DUMMYLASTMSGTYPE
};

/* Generic message template. */
typedef struct messagehdr{ 	
	enum Msgtypes msgtype;
} messagehdr;


/* Functions in mp2_node.c */

/* Message processing routines. */
STDCLLBKRET Process_joinreq STDCLLBKARGS;
STDCLLBKRET Process_joinrep STDCLLBKARGS;

/*
int recv_callback(void *env, char *data, int size);
int init_thisnode(member *thisnode, address *joinaddr);
*/

/*
Other routines.
*/

void nodestart(member *node, char *servaddrstr, short servport);
void nodeloop(member *node);
int recvloop(member *node);
int finishup_thisnode(member *node);

/* Aditya's definitions */


#endif /* _NODE_H_ */

