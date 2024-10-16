#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>

typedef enum { exiting , resting , entering ,empty} status;

typedef struct{
	int boatname;
	char parkingtype[5];
	char boattype[5];
	time_t arrival;
	//int costperhalfhour;
	status stat;
	time_t departure;
}parking;

typedef struct{
	int name,parkperiod,mantime;
	char type[5],postype[5];
}vstats;

typedef struct{
	int mantime,name;
	char parktype[5];
	status stat;
	time_t departure;
}traffic;

typedef struct{
	int Scapacity;
	int Mcapacity;
	int Lcapacity;
	vstats waiting;
	traffic move;
	float Scost;
	float Mcost;
	float Lcost;
	sem_t sp;
	sem_t semv;
	sem_t semove;
	sem_t semseat;
	char outfile[10];
	int Sboats;
	int Mboats;
	int Lboats;
	int Swait;
	int Mwait;
	int Lwait;
	int Sboatcost;
	int Mboatcost;
	int Lboatcost;
	int myportend;
	int portmasterend;
	parking* seats;
}port;


