/**********************
*
* Progam Name: MP1. Membership Protocol
* 
* Current file: params.c
* About this file: Setting Parameters.
* 
***********************/

#include "params.h"

short PORTNUM = 8001;

extern int globaltime;
/* 
Set all the parameters.
*/
void setparams(char *config_file){
	FILE *fp = fopen(config_file,"r");

	fscanf(fp,"MAX_NNB: %d", &MAX_NNB);
	fscanf(fp,"\nSINGLE_FAILURE: %d", &SINGLE_FAILURE);
	fscanf(fp,"\nDROP_MSG: %d", &DROP_MSG);
	fscanf(fp,"\nMSG_DROP_PROB: %lf", &MSG_DROP_PROB);

	printf("%d %d %d %lf\n", MAX_NNB, SINGLE_FAILURE, DROP_MSG, MSG_DROP_PROB);

	EN_GPSZ = MAX_NNB;
	STEP_RATE=.25;
	MAX_MSG_SIZE = 4000;
	globaltime = 0;
	dropmsg = 0;

	fclose(fp);
	return;
}

/* 
Return time since start of program, in time units. 
For a 'real' implementation, this return time would be the UTC time.
*/
int getcurrtime(){
    
    return globaltime;
}

