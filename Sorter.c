#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/times.h>
#include "MyDataStructs.h"
volatile sig_atomic_t signalsreceived;
//Programm must be called the following way:
//	/.sorter <data file name> <starting point> <ending point> <fifo name> <root process id>
//arguments must be in that specific order	
int main(int argc,char** argv){
	double t1,t2,cpu_time;
	struct tms tb1,tb2;
	double ticspersec;
	ticspersec=(double)sysconf(_SC_CLK_TCK);
	t1=(double)times(&tb1);
	signal(SIGUSR1,signalhandler);
	if(argc<6){
		printf("Missing arguments!\n");
		exit(-1);
	}
	FILE * Input;
	Input=fopen(argv[1],"r");	/*Attempting to open Input file*/
	if(Input==NULL){
		printf("Invalid data file name!\n");
		exit(-1);
	}
	long Start=atoi(argv[2]);
	long End=atoi(argv[3]);
	int root=atoi(argv[5]);
	if(Start<0 || Start>End){	/*Checking for starting and ending point positions*/
		printf("Invalid range!\n");
		exit(-1);
	}
	if(fseek(Input,Start,SEEK_SET)<0){
		perror("fseek");
		exit(-1);
	}
	if(Start!=0){	/*checking if the starting point is at the beggining of a line or not*/
		char Invalidline[SIZEOFBUFFER],c[SIZEOFBUFFER];
		fread(c,sizeof(char),1,Input);
		fseek(Input,Start-1,SEEK_SET);
		fread(c,sizeof(char),1,Input);
		if(c[0]!='\n'){	/*if start pointer is in the middle of a record, this record is being ignored*/
			fscanf(Input,"%[^\n]s",Invalidline);
		}	
		if(ftell(Input)>=End){
			exit(0);
		}
	}	/*Allocating memory for the Lists needed*/
	CandidateList * CL;
	CL=malloc(sizeof(CandidateList));
	MakeCandidateList(CL);
	ElectionCenterList * EL;
	EL=malloc(sizeof(ElectionCenterList));
	MakeElectionCenterList(EL);
	ExecutionTimes * T;
	T=malloc(sizeof(ExecutionTimes));
	T->ProcessID=getpid();
	T->Next=NULL;
	Record rec;	
	do{	/*reading the input file line-by-line*/
		fscanf(Input,"%s %hd %c",rec.Name,&(rec.ElectionCenter),&(rec.valid));
		if(feof(Input)){
			break;
		}
		UpdateCandidateList(CL,&rec);	/*Updating Data Lists*/
		UpdateElectionCenterList(EL,&rec);
	}while(!feof(Input)&&ftell(Input)<End);
	int fifodesc;	/*Attempting to open parent fifo for wrinting only*/
	fifodesc=open(argv[4],O_WRONLY);
	if(fifodesc<0){
		perror("Opening fifo");
		exit(-1);
	}
	ForwardResults(fifodesc,CL,EL);	/*forwarding results through parent fifo*/
	DeleteCandidateList(CL);
	DeleteElectionCenterList(EL);
	t2=(double)times(&tb2);
	cpu_time=(double)((tb2.tms_utime+tb2.tms_stime)-(tb1.tms_utime+tb1.tms_stime));
	T->ProcessID=getpid();	/*Sending process's run times*/
	T->CPUTime=cpu_time/ticspersec;
	T->RealTime=(t2-t1)/ticspersec;
	T->Next=NULL;
	SendTimes(fifodesc,T);
	kill(root,SIGUSR1);	/*After having completed its job, sorter sends a USR1 signal to the root process*/
	exit(0);
}
