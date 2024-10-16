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


int main(int argc, char *argv[]){
	void* shmem;
	port *myport;
	FILE *fp;
	char *filename,inputa[40],inputb[40],inputc[40],inputd[40];
	char str[5];
	int i=0,id,idS,err,*mem,child,retval,count=0,ran,n=0, k,Scap,Mcap,Lcap;
	vstats r;
	time_t start;
	pid_t childpid,vesselpid;
	sem_t *sp;

	srand(time(NULL)); // randomize seed

	if(argc!=3){
		perror("command line");
	}
	else{
		if(strcmp(argv[1],"-l")==0){
			filename=argv[2];
		}	
		else{
			perror("-l");
		}
	}
	fp = fopen(filename,"r");            //open configfile.txt
	if(fp==NULL){
		perror("open txt");
	}
	while (fgets(str, 5, fp) != NULL){                              //reading configfile.txt
		i++;
        	if(i==4){                                                   ///initializing struct
			Scap=atoi(str);
		}
		else if(i==5){
			Mcap=atoi(str);
		}
		else if(i==6){
			Lcap=atoi(str);
			break;
		}
	}
	id=shmget(IPC_PRIVATE,sizeof(port)+sizeof(parking)*(Scap+Mcap+Lcap),IPC_CREAT | 0666);            ////////////////shmget
	if ( id == -1 ) perror ( " Creating " ) ;
	shmem = (void *) shmat ( id , NULL , 0) ;
	if ( myport == (port*)(-1)) perror ( " Attachment " ) ;
	myport=(port*)shmem;
	retval = sem_init(&myport->sp ,1 ,1) ;                         //initializing semaphores
	if(retval!=0){
		perror("Couldn’tinitialize."); exit(3);
	}
	retval = sem_init(&myport->semv ,1 ,0) ;
	if(retval!=0){
		perror("Couldn’tinitialize semv."); exit(3);
	}
	retval = sem_init(&myport->semove ,1 ,0) ;
	if(retval!=0){
		perror("Couldn’tinitialize semove."); exit(3);
	}
	retval = sem_init(&myport->semseat ,1 ,0) ;
	if(retval!=0){
		perror("Couldn’tinitialize semseat."); exit(3);
	}
	myport->Scapacity=Scap;
	myport->Mcapacity=Mcap;
	myport->Lcapacity=Lcap;
	strcpy(myport->outfile,"outputfile.txt");
	myport->waiting.name=0;
	myport->move.name=0;
	myport->seats = (parking*)(((port*)myport)+sizeof(myport));
	fclose(fp);
	for(i=0;i<myport->Scapacity+myport->Mcapacity+myport->Lcapacity;i++){
		myport->seats[i].stat=empty;                                          ///initializing struct seats
		myport->seats[i].boatname=0;
		if(i<myport->Scapacity){
			strcpy(myport->seats[i].parkingtype,"S");
		}
		else if(i<myport->Scapacity+myport->Mcapacity){
			strcpy(myport->seats[i].parkingtype,"M");
		}
		else{
			strcpy(myport->seats[i].parkingtype,"L");
		}	
		
	}
	myport->waiting.name=0;
	myport->move.name=0;
	myport->Sboats=0;
	myport->Mboats=0;
	myport->Lboats=0;
	myport->Swait=0;
	myport->Mwait=0;
	myport->Lwait=0;
	myport->Sboatcost=0;
	myport->Mboatcost=0;
	myport->Lboatcost=0;
	myport->myportend=1;
	myport->portmasterend=1;
	childpid=fork();
	if(childpid==0){
		sprintf(inputa , "%d", id);
		sprintf(inputb , "2");
		sprintf(inputc , "5");   
		sprintf(inputd , "%d", idS);                           
		if(execl("monitor","monitor","-d",inputb,"-t",inputc,"-s",inputa,inputd,NULL)<0){            //////////////////execl monitor
			perror("exec monitor");
		}
	}
	childpid=fork();
	if(childpid==0){
		sprintf(inputa , "%d", id);
		sprintf(inputb , "%d", idS);                                
		if(execl("portmaster","portmaster","-c",argv[2],"-s",inputa,inputb,NULL)<0){            //////////////////execl portmaster
			perror("exec portmaster");
		}
	}
	else{
		sleep(3);
		start=time(NULL);
		while(start+30>time(NULL)){                          //work for 30 seconds
			k = rand()%100000000; //[0,99999999]
			//printf("---Enter 1---\n");
			//scanf("%d", &k);                           // creating vessels with random numbers
			//k=time(NULL)%10;
			//sleep(1);
			if(k == 546555){
				n++;
				vesselpid=fork();
				if(vesselpid==0){
					ran=rand()%10;
					if(ran<4){
						strcpy(r.type,"S");
					}
					else if(ran<7){
						strcpy(r.type,"M");
					}
					else{
						strcpy(r.type,"L");
					}
					ran=rand()%10;
					if(ran<4){
						strcpy(r.postype,r.type);
					}
					else if(ran<8){
						if(strcmp(r.type,"S")==0){
							strcpy(r.postype,"M");
						}
						else{
							strcpy(r.postype,"L");
						}
					}
					else{
						strcpy(r.postype,"L");
					}
					ran=rand()%20+1;
					r.parkperiod=ran;
					if(strcmp(r.type,"S")==0){
						count=2;
					}
					else if(strcmp(r.type,"M")==0){
						count=4;
					}
					else{
						count=8;
					}
					ran=rand()%count+1;
					r.mantime=ran;
					r.name=n;
					sprintf(inputa, "%d", r.name);
					sprintf(inputb, "%d", r.parkperiod);
					sprintf(inputc, "%d", r.mantime);
					sprintf(inputd, "%d", id);
					execl("vessel","vessel","-n",inputa,"-t",r.type,"-u",r.postype,"-p",inputb,"-m",inputc,"-s",inputd,NULL);
				}
			}
		}
	}
			myport->myportend=0;
			do{
				n--;
				childpid=wait(&child);
			}while(n>-1);
	err = shmdt((void*)myport);
	if ( err == -1 ) perror ( " Removal myport" ) ;
	err = shmctl ( id , IPC_RMID , 0) ;
	if ( err == -1 ) perror ( " Removal id" ) ;
	printf("exiting myport\n");
} 
