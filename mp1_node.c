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
Packet format

    | messgetyepe | sender's address | 64bit list size | memebership list |

for JOINREQ data is 64bits of membership list size + membership list
for JOINREP data is 64bits of membership list size +membership list 
for GOSSIP data is 64bits of membership list size + membership list

*/

/*
 *
 * Routines for introducer and current time.
 *
 */

char * bufferTest[1000];

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

/* This function copies a serialized repn of the memberlist into the 
   buffer pointed to by buffer*/
static void serializeMemberTable(member* self, char* buffer){

    memcpy(buffer,&self->numMemberEntries,sizeof(int)); 
    memcpy(buffer+sizeof(int),self->memberList,sizeof(MemberEntry)*self->numMemberEntries); 
    
}

/* This function picks a random node and sends it a gossip */
static void sendGossip(member* self){
    
    if(self->numMemberEntries==1) return; /*without gossiping */
    
    int maxtries = 3;
    int randnode;
    while(maxtries--){
        randnode = 1+rand()%(self->numMemberEntries-1);
        if(self->memberList[randnode].mark_fail==1) continue;
        else break;
    }
    if(self->memberList[randnode].mark_fail==1) return /*without gossiping*/;
    else{
        char debug_buffer[50];
        print_address(debug_buffer,&self->memberList[randnode].addr);
        LOG(&self->addr,"Sending a GOSSIP message to %s",debug_buffer);
        address* send_addr = &self->memberList[randnode].addr;
        size_t messagesize = sizeof(messagehdr) + sizeof(address) + sizeof(int) + sizeof(MemberEntry)*self->numMemberEntries;
        char* msg = malloc(messagesize);
        ((messagehdr*)msg)->msgtype = GOSSIP;
        memcpy(msg+sizeof(messagehdr),&self->addr,sizeof(address));
        serializeMemberTable(self,msg+sizeof(messagehdr)+sizeof(address));
        MPp2psend(&self->addr,send_addr, (char *)msg, messagesize);
        free(msg);
    }

}

/* This function updates your own hearbeat counter */
void keepSelfAlive(member* self){

    self->memberList[0].last_hb++;
    self->memberList[0].last_local_timestamp = getcurrtime();
    self->memberList[0].mark_fail = 0;
    self->memberList[0].mark_del = 0;
}


/* This function checks the node table at a particular node and 
   marks up any deleted or failed entries */
static void checkNodeTable(member* self){
    int i;
    char debug_buffer[50];
    for(i=1;i<self->numMemberEntries;++i){

        if( !self->memberList[i].mark_fail ){ 
            if( (getcurrtime()-self->memberList[i].last_local_timestamp ) > self->tfail){
            /* tfail timer has expired , mark node as failed */
                int64_t oldts = self->memberList[i].last_local_timestamp;
                self->memberList[i].mark_fail=1;
                self->memberList[i].last_local_timestamp = getcurrtime();
                print_address(debug_buffer,&self->memberList[i].addr);
                LOG(&self->addr,"\t\tMarking node %s as Failed on %d , entry last updated at %d",debug_buffer,getcurrtime(),oldts);
            }
        }else{
            /* tdelete timer has expired, mark node for deletion */
            if( (getcurrtime()-self->memberList[i].last_local_timestamp ) > self->tdelete){
                //swap it with the last member in the list to delete it.
                int64_t oldts = self->memberList[i].last_local_timestamp;
                logNodeRemove(&self->addr,&self->memberList[i].addr); 
                print_address(debug_buffer,&self->memberList[i].addr);
                LOG(&self->addr,"\t\tMarking node %s as Deleted on %d , entry last updated at %d",debug_buffer,getcurrtime(),oldts);
                self->memberList[i] = self->memberList[self->numMemberEntries-1];
                self->numMemberEntries--;
            }

        }
    }

}


/*Takes in a serialized repn of the member list coming from node n and 
 parses it to update your own table, also update the entry of the resp_addr */
static void updateNodeTable(member* self, address* other_addr,char* data,int datasize){
    
    char debug_buffer[100];
    int i,j;
    int* otherListSize = (int*)data;
    if((*otherListSize)*sizeof(MemberEntry) < (datasize-sizeof(int))){
        LOG(&self->addr,"Bad Packet");
        return ;
    }
    /*iterate over their list */
    MemberEntry* otherList = (MemberEntry*)(otherListSize+1);
    for(j=0;j<*otherListSize;++j){
        int updateMade = 0;
        
        /* ignore this entry if this entry is not reliable */
        if(otherList[j].mark_fail) continue;

        /* iterate over my list */
        for(i=1;i<self->numMemberEntries;++i){
            if( memcmp(&otherList[j].addr,&self->memberList[i].addr,sizeof(address))==0) {
                updateMade = 1;
                /* DOUBT: if the process has been marked as failed locally and we still have a good entry from that process,
                   then can i mark this process as alive ? For now , I am doing so */   
  
                if(self->memberList[i].last_hb>=otherList[j].last_hb) break; //no need to update
                else{
                    if(!self->memberList[i].mark_fail){
                        //update the heartbeat of the process and add a local timestamp
                        int64_t oldhb=self->memberList[i].last_hb;
                        self->memberList[i].last_hb = otherList[j].last_hb;
                        self->memberList[i].last_local_timestamp = getcurrtime();   
                        print_address(debug_buffer,&self->memberList[i].addr);
                        LOG(&self->addr,"\t\tUpdated the entry for %s with hb_new %d vs %d hb_old",debug_buffer,self->memberList[i].last_hb,oldhb);        
                    }else{
                        int64_t oldhb=self->memberList[i].last_hb;
                        self->memberList[i].last_hb = otherList[j].last_hb;
                        self->memberList[i].last_local_timestamp = getcurrtime();   
                        self->memberList[i].mark_fail=0; //reverse your decision as you got a greater hb
                        print_address(debug_buffer,&self->memberList[i].addr);
                        LOG(&self->addr,"\t\tReviving the node at %s with hb_new %d vs  %d hb_old",debug_buffer,self->memberList[i].last_hb,oldhb);
                    }
                }
            }
        }

        if(!updateMade && memcmp(&otherList[j].addr,&self->memberList[0].addr,sizeof(address))!=0){
            //this is a new node. append it at the end of the list
            if(self->numMemberEntries<MAX_NNB){ 
                self->memberList[self->numMemberEntries] = otherList[j];
                self->memberList[self->numMemberEntries].last_local_timestamp = getcurrtime(); //stamp it with a local timestamp
                print_address(debug_buffer,&self->memberList[self->numMemberEntries].addr);
                LOG(&self->addr,"\t\tAdded the entry for %s with hb %d",debug_buffer,self->memberList[self->numMemberEntries].last_hb);        
                self->numMemberEntries++;
#ifdef DEBUGLOG
                logNodeAdd(&self->addr,&otherList[j].addr); 
#endif
            }
            else        
                LOG(&self->addr,"Membership list overflow!");
        }
    }    
} 


/* Initialize the membership list */
static void initMemberList(member* self){
    /* Initialize to a default value of 10 at first */
    self->memberList = (struct MemberEntry*)malloc(MAX_NNB*sizeof(MemberEntry));
    memset(self->memberList,0,sizeof(MemberEntry)*MAX_NNB);

    //add your own entry to the beg of list so it stays there
    self->memberList[0].addr = self->addr;
    self->memberList[0].last_hb = 0; 
    self->memberList[0].last_local_timestamp = getcurrtime();
    self->memberList[0].mark_fail = 0;
    self->memberList[0].mark_del = 0;
    self->numMemberEntries = 1;
    logNodeAdd(&self->addr,&self->addr);
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
    2. Update its own table
    3. It responds back with a current copy of the membership table.
    */

    if(size<sizeof(address) ){
        LOG(&(((member*)(env))->addr),"Bad Packet");
        return;
    }

    char addr_str[20];
    member *self = (member*) env; //own address
    address* req_addr = (address*)data; //extract address of the requesting node
    data = (char*)(req_addr+1);
    size -= sizeof(address);
    print_address(addr_str,req_addr);

    LOG(&self->addr,"Recieved a JOINREQ from %s",addr_str);


    /* data now corresponds to the actual content of the message */
    /*add the node to the local table.*/
    updateNodeTable(self,req_addr,data,size);

    /* build your response ( your copy of the membership table */
    size_t msgsize = sizeof(messagehdr)+sizeof(address)+sizeof(int)+sizeof(MemberEntry)*(self->numMemberEntries);
    char * msg = malloc(msgsize);
    ((messagehdr*) msg)->msgtype = JOINREP;
    memcpy(msg+sizeof(messagehdr),&self->addr,sizeof(address));
    serializeMemberTable(self,msg+sizeof(messagehdr)+sizeof(address));

    /* send your respose */
    MPp2psend(&self->addr,req_addr,(char*)msg,msgsize);
    free(msg);
    
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
        LOG(&(((member*)(env))->addr),"Bad Packet");
        return;
    }

    char addr_str[20];
    member *self = (member*) env;
    address* resp_addr = (address*)data;
    data = (char*)(resp_addr+1); 
    size -= sizeof(address);
    print_address(addr_str,resp_addr);

    LOG(&self->addr,"Received a JOINREP from %s",addr_str);

    /* I can join the group now */
    self->ingroup = 1;

    /* data now points to whatever data that a node sent as we have extracted the address out */
    
    //update your local copy of the node table using the data
    if(size>0) updateNodeTable(self,resp_addr,data,size);
    else{
        LOG(&self->addr,"Join response is empty!");
    }

    return;
}

/* Received a GOSSIP message */
void Process_gossip(void* env,char* data,int size){

    /*Over here i take the table received in a gossip message and merge it with 
     my own table */    

    if(size<sizeof(address) ){
        LOG(&(((member*)(env))->addr),"Bad Packet");
        return;
    }

    char addr_str[20];
    member *self = (member*) env;
    address* resp_addr = (address*)data;
    data = (char*)(resp_addr+1); 
    size -= sizeof(address);
    print_address(addr_str,resp_addr);
    
    LOG(&self->addr,"Received a GOSSIP message from %s",addr_str);

    /* data now points to the actual message contents */
    if(size>0) updateNodeTable(self,resp_addr,data,size);
    else{
        LOG(&self->addr,"Join response is empty!");
    }
    return;

}


/* 
Array of Message handlers. 
*/
void ( ( * MsgHandler [20] ) STDCLLBKARGS )={
/* Message processing operations at the P2P layer. */
    Process_joinreq, 
    Process_joinrep,
    Process_gossip
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

    /*initialize the membership list to a default size of 10*/
    initMemberList(thisnode);

    /* Init the timers */   
    /*values that work = tafail=5 , tdelete=5*/
    thisnode->tfail =10;
    thisnode->tdelete = 20;
    //thisnode->tgossip = 5;


    /* node is up! */

    return 0;
}


/* 
Clean up this node. 
*/
int finishup_thisnode(member *node){

	/* <your code goes in here> */
    //free the member list over here
    free(node->memberList);
    return 0;
}


/* 
 *
 * Main code for a node 
 *
 */


/* create JOINREQ message: format of data is msghdr|myaddr|listlen|list */
static void createJoinReq(member* self,char*buffer){

    ((messagehdr*)buffer)->msgtype=JOINREQ;
    memcpy(buffer+sizeof(messagehdr), &self->addr, sizeof(address));
    serializeMemberTable(self,buffer+sizeof(address)+sizeof(messagehdr));
}


/* 
Introduce self to group. 
*/
int introduceselftogroup(member *node, address *joinaddr){
    
    char *msg;
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

        size_t msgsize = sizeof(messagehdr) + sizeof(address) + sizeof(int)+ sizeof(MemberEntry)*(node->numMemberEntries);
        char * msg=malloc(msgsize);
        createJoinReq(node,msg);
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
    // Over here, update the heartbeat counter and keep yourself alive;
    keepSelfAlive(node);

    /*gossip your table to a random member in your list */
    sendGossip(node);

    /*check for expired entries in your table */
    checkNodeTable(node);

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

