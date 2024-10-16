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

int main(int argc,char* argv[]){
	char *input;
	int id,idS,i,retval,err;
	port *myport;
	vstats mystats;
	void* shmem;
	time_t departure;
	if(!argc){
		perror("vessel command line");
	} 
	for(i=1;i<argc;i=i+2){
		if(strcmp(argv[i],"-t")==0){
			strcpy(mystats.type,argv[i+1]);
		}	
		else if(strcmp(argv[i],"-u")==0){
			strcpy(mystats.postype,argv[i+1]);
		}
		else if(strcmp(argv[i],"-p")==0){
			mystats.parkperiod=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-m")==0){
			mystats.mantime=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-s")==0){
			id=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-n")==0){
			mystats.name=atoi(argv[i+1]);
		}
	}
	printf("vessel created %d type of %s postype %s parkperiod %d mantime %d\n",mystats.name,mystats.type,mystats.postype,mystats.parkperiod,mystats.mantime);
	shmem = (void *) shmat ( id , NULL , 0) ;                        //shared mem
	if ( myport == (port*)(-1)) perror ("vessel Attachment") ;
	myport=(port*)shmem;
	sem_wait(&myport->semv);                                           //wait in line to enter port
	sem_wait(&myport->sp); 
		myport->waiting.name=mystats.name;
		myport->waiting.mantime=mystats.mantime;                               //write type and name of boat
		myport->waiting.parkperiod=mystats.parkperiod;
		sprintf(myport->waiting.type, "%s" , mystats.type);
		sprintf(myport->waiting.postype, "%s" , mystats.postype);
	sem_post(&myport->sp); 
	sem_wait(&myport->semseat);                                                //wait to find seat
	sem_wait(&myport->semove);                                                //wait for any traffic
	sem_wait(&myport->sp); 
		myport->move.name=mystats.name;
		myport->move.mantime=mystats.mantime;
		myport->move.stat=entering;
		myport->move.departure=time(NULL);
		strcpy(myport->move.parktype,mystats.type);
	sem_post(&myport->sp); 
	sleep(mystats.mantime);
	sleep(mystats.parkperiod); 						 //resting 
	departure=time(NULL);                                                   
	sem_wait(&myport->semove);                                                     //wait to leave
	sem_wait(&myport->sp); 
		myport->move.name=mystats.name;
		myport->move.mantime=mystats.mantime;
		myport->move.stat=exiting;
		myport->move.departure=departure;
		strcpy(myport->move.parktype,mystats.type);
	sem_post(&myport->sp); 
	sleep(mystats.mantime);
	err = shmdt((void*)myport);
	if ( err == -1 ) perror ( " portmaster Removal myport" ) ;
}
