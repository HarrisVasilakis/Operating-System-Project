#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include "myport.h"

int main(int argc , char* argv[]){                   ///portmaster
	void* shmem;
	port *myport;
	int id=0,idS=0,idsem,err,retval,i=0,count,countstart,progend=0;
	char str[10];
	time_t start,end;
	FILE *fp,*fp2;
	float cost;
	if(argc!=5){
		//perror("argc");
	}
	for(i=1;i<argc;i=i+2){
		if(strcmp(argv[i],"-c")==0){
			fp = fopen(argv[i+1],"r");
			if(fp==NULL){
				perror("open txt");
			}
		}
		else if(strcmp(argv[i],"-s")==0){
			id=atoi(argv[++i]);
			idS=atoi(argv[i+1]);
		}
	}
	shmem = (void *) shmat ( id , NULL , 0) ;                             //shared memory
	if ( myport == (port*)(-1)) perror ("portmaster Attachment") ;
	myport=(port*)shmem;
	i=0; //!!!!!!!!!!!!!!!!! 
	while (fgets(str, 10, fp) != NULL){
		i++;                                                        //initializing cost
		if(i==7){
			myport->Scost=atof(str);
		}
		else if(i==8){
			myport->Mcost=atof(str);
		}
		else if(i==9){
			myport->Lcost=atof(str);
			break;
		}
	}  
 
	fclose(fp);   
	fp2=fopen(myport->outfile,"w");                                    //open outputfile          
	myport->seats = (parking*)(((port*)myport)+sizeof(myport));
	while(myport->myportend || progend){   //end if myport has ended and port is empty
		sem_wait(&myport->sp);
		if(myport->waiting.name==0){                     //if portmaster is free to find a seat for the next boat
			myport->waiting.name=-1;
			sem_post(&myport->semv);		//sem post
											//portmaster has already checked if is free to find a seat for the next boat
		}
		sem_post(&myport->sp);
		sem_wait(&myport->sp);
		if(myport->move.name==0){                       //same if boat can move inside port
			myport->move.name=-1;
			sem_post(&myport->semove);
		}	
		sem_post(&myport->sp);
		sem_wait(&myport->sp);
		if(myport->waiting.name!=0 && myport->waiting.name!=-1){              //find seat for boat waiting.name
			if(strcmp(myport->waiting.type,"S")==0){                     //where to search for seat
				countstart=0;
				count=myport->Scapacity;
			}
			else if(strcmp(myport->waiting.type,"M")==0){
				countstart=myport->Scapacity;
				count=myport->Scapacity+myport->Mcapacity;
			}
			else{
				countstart=myport->Scapacity+myport->Mcapacity;
				count=myport->Scapacity+myport->Mcapacity+myport->Lcapacity;
			}
			if(strcmp(myport->waiting.postype,"M")==0){
				count=myport->Scapacity+myport->Mcapacity;
			}
			else if(strcmp(myport->waiting.postype,"L")==0){
				count=myport->Scapacity+myport->Mcapacity+myport->Lcapacity;
			}
			for(i=countstart;i<count;i++){
				if(myport->seats[i].stat==empty){                            //found empty seat
					myport->seats[i].boatname=myport->waiting.name;
					strcpy(myport->seats[i].boattype,myport->waiting.type);
					myport->seats[i].arrival=0;
					myport->seats[i].stat=entering;
					myport->seats[i].departure=0;
					myport->waiting.name=0;
					sem_post(&myport->semseat);
					break;
				}
			}
		}
		sem_post(&myport->sp);
		sem_wait(&myport->sp);
		if(myport->move.name!=0 && myport->move.name!=-1){
			if((int)myport->move.departure+myport->move.mantime<(int)time(NULL)){   //if boat has finished moving inside port
				if(strcmp(myport->move.parktype,"S")==0){
					countstart=0;
				}
				else if(strcmp(myport->move.parktype,"M")==0){
					countstart=myport->Scapacity;
				}
				else{
					countstart=myport->Scapacity+myport->Mcapacity;
				}
				for(i=countstart;i<myport->Scapacity+myport->Mcapacity+myport->Lcapacity;i++){
					if(myport->seats[i].boatname==myport->move.name){  //find boat that left or entered port
						if(myport->move.stat==entering){
							myport->seats[i].arrival=myport->move.departure+myport->move.mantime;
							myport->seats[i].stat=resting;
							fprintf(fp2,"boat %d entered port at %d time at a seat of type %s.The boat is %s type and it is currently resting.\n",myport->seats[i].boatname,(int)myport->seats[i].arrival,myport->seats[i].parkingtype,myport->seats[i].boattype);
						}
						else if(myport->move.stat==exiting){
							myport->seats[i].departure=myport->move.departure+myport->move.mantime;
							myport->seats[i].stat=empty;
							fprintf(fp2,"boat %d left port at %d time from a seat of type %s.The boat is %s type and it has left\n",myport->seats[i].boatname,(int)myport->seats[i].departure,myport->seats[i].parkingtype,myport->seats[i].boattype);
							if(strcmp(myport->seats[i].parkingtype,"S")==0){
								cost=myport->Scost;
							}
							else if(strcmp(myport->seats[i].parkingtype,"M")==0){
								cost=myport->Mcost;
							}
							else if(strcmp(myport->seats[i].parkingtype,"L")==0){
								cost=myport->Lcost;
							}
							fprintf(fp2,"cost of boat %d of %s type was %.2f euros \n",myport->seats[i].boatname,myport->seats[i].boattype,cost*(myport->move.departure-myport->seats[i].arrival));
							if(strcmp(myport->seats[i].boattype,"S")==0){
								myport->Sboats++;
								myport->Swait=myport->Swait+(myport->seats[i].departure-myport->seats[i].arrival);
								myport->Sboatcost=myport->Sboatcost+cost*(myport->seats[i].departure-myport->seats[i].arrival);
							}
							else if(strcmp(myport->seats[i].boattype,"M")==0){
								myport->Mboats++;
								myport->Mwait=myport->Mwait+(myport->seats[i].departure-myport->seats[i].arrival);
								myport->Mboatcost=myport->Mboatcost+cost*(myport->seats[i].departure-myport->seats[i].arrival);
							}
							else if(strcmp(myport->seats[i].boattype,"L")==0){
								myport->Lboats++;
								myport->Lwait=myport->Lwait+(myport->seats[i].departure-myport->seats[i].arrival);
								myport->Lboatcost=myport->Lboatcost+cost*(myport->seats[i].departure-myport->seats[i].arrival);
							}
							myport->seats[i].boatname=0;
						}
						myport->move.name=0;
						break;
					}
				}
			}
		}
		sem_post(&myport->sp);
		for(i=0;i<myport->Scapacity+myport->Mcapacity+myport->Lcapacity;i++){  //check if there are any boats in port
			if(myport->seats[i].boatname!=0){
				progend=1;
				break;
			}
			if(i==myport->Scapacity+myport->Mcapacity+myport->Lcapacity-1){
				progend=0;
			}
		}
	}
	fclose(fp2);
	myport->portmasterend=0;
	err = shmdt((void*)myport);
	if ( err == -1 ) perror ( " portmaster Removal myport" ) ;
	printf("exiting portmaster\n");
}
