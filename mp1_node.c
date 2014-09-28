/**********************
*
* Progam Name: MP1. Membership Protocol
* 
* Code authors: Aditya Rastogi ( arastog2 ) 
*
* Current file: mp1_node.c
* About this file: Member Node Implementation
* 
***********************/

#include "mp1_node.h"
#include "emulnet.h"
#include "MPtemplate.h"
#include "log.h"


/*
 *
 * Routines for introducer and current time.
 *
 */

char NULLADDR[] = {0,0,0,0,0,0};
int isnulladdr( address *addr){
    return (memcmp(addr, NULLADDR, 6)==0?1:0);
}

/* 
Return the address of the introducer member. 
*/
address getjoinaddr(void){

    address joinaddr;

    memset(&joinaddr, 0, sizeof(address));
    *(int *)(&joinaddr.addr)=1;
    *(short *)(&joinaddr.addr[4])=0;

    return joinaddr;
}

/*
 *
 * Message Processing routines.
 *
 */


/* returns string representation of an address */
static void print_address(char* address_buf,address* addrs){
    char* addr = (char*)addrs;
	sprintf(address_buf, "%d.%d.%d.%d:%d ", addr[0], addr[1], addr[2], addr[3], *(short *)&addr[4]);
}


/* This function whenever called , sends a heartbeat out to one of 
the proccesses inside the process's membership list */
void sendHeartbeat(member* self){


}


/* Adds the node with address req_add to the local table */
static void addNodeToTable(member* self, address* req_add){

}

/*Takes in a serialized repn of the member list coming from node n and 
 parses it to update your own table, also update the entry of the resp_addr */
static void updateNodeTable(member* self, address* resp_addr,char* memberList){



} 

/* This function sends data contained in the memebrship table
to node with address toNode */
static void sendMemberTable(member* self, address* toNode){

    //right now a dummy implementation of JOINREP, just send your address
    char * joinResp = "Hi There I am sending you a JOIONREP!";
    int msgsize = sizeof(messagehdr)+sizeof(address);
    messagehdr * msg = malloc(msgsize);
    msg->msgtype = JOINREP;
    memcpy(msg+1,&self->addr,sizeof(address));
    MPp2psend(&self->addr,toNode,(char*)msg,msgsize);
    free(msg);

}

/* 
Received a JOINREQ (joinrequest) message. This is called when
a JOINREQ message type is received by the process.
here env is pointer to the node on which joinreq is recvd 
*/
void Process_joinreq(void *env, char *data, int size)
{
	/* <your code goes in here> */
    /* 
    Do the following thigs in here 
    1. The JOINREQ is received by the introducer
    2. It responds back with a current copy of the membership table.
    3. Adds the node's entry to its table
    */

    if(size<sizeof(address) ){
        printf("Bad Packet");
        return;
    }

    char addr_str[20];
    member *self = (member*) env; //own address
    address* req_addr = (address*)data; //extract address of the requesting node
    data = (char*)(req_addr+1);
    size -= sizeof(address);
    print_address(addr_str,req_addr);

    LOG(&self->addr,"Recived a JOINREQ from %s",addr_str);

    /* data now corresponds to the actual content of the message */

    //respond with your own copy of the table by sending a buffer of the size of the member lists
    sendMemberTable(self,req_addr);


    //add the node to the local table.
    addNodeToTable(self,req_addr);

    
    return;
}


/* 
Received a JOINREP (joinreply) message. 
*/
void Process_joinrep(void *env, char *data, int size)
{
	/* <your code goes in here> */
    /* over here you need to decode the response (it will be in the 
    form of a character buffer that represents a list of nodes and add the entries to your own list */

    if(size<sizeof(address) ){
        printf("Bad Packet");
        return;
    }

    char addr_str[20];
    member *self = (member*) env;
    address* resp_addr = (address*)data;
    data = (char*)(resp_addr+1); 
    size -= sizeof(address);
    print_address(addr_str,resp_addr);

    LOG(&self->addr,"Received a JOINREP from %s",addr_str);

    /* data now points to whatever data that a node sent as we have extracted the address out */
    
    //update your local copy of the node table using the data
    if(size>0) updateNodeTable(self,resp_addr,data);
    else{
        printf("Join response is empty!");
    }

    return;
}


/* 
Array of Message handlers. 
*/
void ( ( * MsgHandler [20] ) STDCLLBKARGS )={
/* Message processing operations at the P2P layer. */
    Process_joinreq, 
    Process_joinrep
};

/* 
Called from nodeloop() on each received packet dequeue()-ed from node->inmsgq. 
Parse the packet, extract information and process. 
env is member *node, data is 'messagehdr'. 
*/
int recv_callback(void *env, char *data, int size){

    member *node = (member *) env;
    messagehdr *msghdr = (messagehdr *)data;
    char *pktdata = (char *)(msghdr+1);

    if(size < sizeof(messagehdr)){
#ifdef DEBUGLOG
        LOG(&((member *)env)->addr, "Faulty packet received - ignoring");
#endif
        return -1;
    }

#ifdef DEBUGLOG
    LOG(&((member *)env)->addr, "Received msg type %d with %d B payload", msghdr->msgtype, size - sizeof(messagehdr));
#endif

    if((node->ingroup && msghdr->msgtype >= 0 && msghdr->msgtype <= DUMMYLASTMSGTYPE)
        || (!node->ingroup && msghdr->msgtype==JOINREP))            
            /* if not yet in group, accept only JOINREPs */
        MsgHandler[msghdr->msgtype](env, pktdata, size-sizeof(messagehdr));
    /* else ignore (garbled message) */
    free(data);

    return 0;

}

/*
 *
 * Initialization and cleanup routines.
 *
 */

/* 
Find out who I am, and start up. 
*/
int init_thisnode(member *thisnode, address *joinaddr){
    
    if(MPinit(&thisnode->addr, PORTNUM, (char *)joinaddr)== NULL){ /* Calls ENInit */
#ifdef DEBUGLOG
        LOG(&thisnode->addr, "MPInit failed");
#endif
        exit(1);
    }
#ifdef DEBUGLOG
    else LOG(&thisnode->addr, "MPInit succeeded. Hello.");
#endif

    thisnode->bfailed=0;
    thisnode->inited=1;
    thisnode->ingroup=0;
    /* node is up! */

    return 0;
}


/* 
Clean up this node. 
*/
int finishup_thisnode(member *node){

	/* <your code goes in here> */
    return 0;
}


/* 
 *
 * Main code for a node 
 *
 */

/* 
Introduce self to group. 
*/
int introduceselftogroup(member *node, address *joinaddr){
    
    messagehdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if(memcmp(&node->addr, joinaddr, 4*sizeof(char)) == 0){
        /* I am the group booter (first process to join the group). Boot up the group. */
#ifdef DEBUGLOG
        LOG(&node->addr, "Starting up group...");
#endif

        node->ingroup = 1;
    }
    else{
        size_t msgsize = sizeof(messagehdr) + sizeof(address);
        msg=malloc(msgsize);

    /* create JOINREQ message: format of data is {struct address myaddr} */
        msg->msgtype=JOINREQ;
        memcpy((char *)(msg+1), &node->addr, sizeof(address));

#ifdef DEBUGLOG
        sprintf(s, "Trying to join...");
        LOG(&node->addr, s);
#endif

    /* send JOINREQ message to introducer member. */
        MPp2psend(&node->addr, joinaddr, (char *)msg, msgsize);
        
        free(msg);
    }

    return 1;

}

/* 
Called from nodeloop(). 
*/
void checkmsgs(member *node){
    void *data;
    int size;

    /* Dequeue waiting messages from node->inmsgq and process them. */
	
    while((data = dequeue(&node->inmsgq, &size)) != NULL) {
        recv_callback((void *)node, data, size); 
    }
    return;
}


/* 
Executed periodically for each member. 
Performs necessary periodic operations. 
Called by nodeloop(). 
*/
void nodeloopops(member *node){

	/* <your code goes in here> */

    return;
}

/* 
Executed periodically at each member. Called from app.c.
*/
void nodeloop(member *node){
    if (node->bfailed) return;

    checkmsgs(node);

    /* Wait until you're in the group... */
    if(!node->ingroup) return ;

    /* ...then jump in and share your responsibilites! */
    nodeloopops(node);
    
    return;
}

/* 
All initialization routines for a member. Called by app.c. 
*/
void nodestart(member *node, char *servaddrstr, short servport){

    address joinaddr=getjoinaddr();

    /* Self booting routines */
    if(init_thisnode(node, &joinaddr) == -1){

#ifdef DEBUGLOG
        LOG(&node->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if(!introduceselftogroup(node, &joinaddr)){
        finishup_thisnode(node);
#ifdef DEBUGLOG
        LOG(&node->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/* 
Enqueue a message (buff) onto the queue env. 
*/
int enqueue_wrppr(void *env, char *buff, int size){    return enqueue((queue *)env, buff, size);}

/* 
Called by a member to receive messages currently waiting for it. 
*/
int recvloop(member *node){
    if (node->bfailed) return -1;
    else return MPrecv(&(node->addr), enqueue_wrppr, NULL, 1, &node->inmsgq); 
    /* Fourth parameter specifies number of times to 'loop'. */
}

