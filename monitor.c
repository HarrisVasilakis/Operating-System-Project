#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <semaphore.h>
#include "myport.h"

int main(int argc, char* argv[]){
	int id,i,idS,err;
	void* shmem;
	time_t stime,stattime,start , end;
	port* myport;
	char *pos;
	parking *temp;
	if(argc!=8){
		//exit(1);	
	}
	for(i=1;i<argc;i=i+2){
		if(strcmp(argv[i],"-d")==0){
			stime=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-t")==0){
			stattime=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-s")==0){
			id=atoi(argv[++i]);
			idS=atoi(argv[i+1]);
		}
 	}
	shmem = (void *) shmat ( id , NULL , 0) ;
	if ( myport == (port*)(-1)) perror ("monitor Attachment") ;
	myport=(port*)shmem;
	sleep(5);
	temp=(parking*)(((port*)myport)+sizeof(myport));
	while(myport->portmasterend){
		if(time(NULL)%stime==1){
			sem_wait(&myport->sp);
			printf("Portseats Start Here\n");
			for(i=0;i<myport->Scapacity+myport->Mcapacity+myport->Lcapacity;i++){
				if(i<myport->Scapacity){
					printf("S[%d] \n",temp[i].boatname);
				}
				else if(i<myport->Scapacity+myport->Mcapacity){
					printf("M[%d] \n",temp[i].boatname);
				}
				else if(i<myport->Scapacity+myport->Mcapacity+myport->Lcapacity){
					printf("L[%d] \n",temp[i].boatname);
				}
			}
			sem_post(&myport->sp);
			sleep(1);
		}
		if(time(NULL)%stattime==1){                  //every stattime seconds
			sem_wait(&myport->sp);
			printf("income in total=%d\n",myport->Sboatcost+myport->Mboatcost+myport->Lboatcost);
			if(myport->Sboats!=0){
				printf("average cost for Small boats is=%d\n",myport->Sboatcost/myport->Sboats);
				printf("average wait time for Small boats is=%d\n",myport->Swait/myport->Sboats);
			}
			if(myport->Mboats!=0){
				printf("average cost for Medium boats is=%d\n",myport->Mboatcost/myport->Mboats);
				printf("average wait time for Medium boats is=%d\n",myport->Mwait/myport->Mboats);
			}
			if(myport->Lboats!=0){
				printf("average cost for Large boats is=%d\n",myport->Lboatcost/myport->Lboats);
				printf("average wait time for Large boats is=%d\n",myport->Lwait/myport->Lboats);
			}
			if(myport->Sboats!=0 || myport->Mboats!=0 || myport->Lboats){
				printf("cost for all boats is=%d\n",(myport->Sboatcost+myport->Mboatcost+myport->Lboatcost)/(myport->Sboats+myport->Mboats+myport->Lboats));
				printf("wait time for all boats is=%d\n",(myport->Swait+myport->Mwait+myport->Lwait)/(myport->Sboats+myport->Mboats+myport->Lboats));
			}
			sem_post(&myport->sp);
			sleep(1);
		}
	}
	err = shmdt((void*)myport);
	if ( err == -1 ) perror ( " portmaster Removal myport" ) ;
	printf("exiting monitor\n");
}
