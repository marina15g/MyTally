#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/times.h>
#include "MyDataStructs.h"
volatile sig_atomic_t signalsreceived;
//Programm must be called the following way:
//	/.sorter <data file name> <starting point> <ending point> <number of SMs> <Depth> <fifo name> <root process id>
//arguments must be in that specific order	
int main(int argc,char ** argv){
	double t1,t2,cpu_time;
	struct tms tb1,tb2;
	double ticspersec;
	ticspersec=(double)sysconf(_SC_CLK_TCK);
	t1=(double)times(&tb1);
	if(argc<8){
		printf("Missing Arguments!\n");
		exit(-1);
	}
	FILE * Input;
	Input=fopen(argv[1],"r");	/*Attempting to opnen input file for reading*/
	if(Input==NULL){
		printf("Invalid data file name!: %s\n",argv[1]);
		exit(-1);
	}
	long Start=atoi(argv[2]);	/*Argument #2 indicates the starting point of the file for this splitter/merger*/
	long End=atoi(argv[3]);		/*Argument #3 indicates the ending point of the file for this splitter/merger*/
	if(End<=Start){
		printf("Invalid range!\n");
		exit(-1);
	}
	int fifodesc=open(argv[4],O_WRONLY|O_NONBLOCK);	/*Argument #4 indicates the fifo name to the parent process*/
	if(fifodesc<0){
		perror(argv[4]);
		exit(-1);
	}
	int root=atoi(argv[5]);	/*Argument #5 indicates the process id of the root process*/
	int numOfSMs=atoi(argv[6]);
	int depth=atoi(argv[7]);
	if(numOfSMs<=0 || depth<1){	/*Checking for invalid arguments*/
		printf("Invalid node data!\n");
		exit(-1);
	}
	int i,numOfArguments,childpid[numOfSMs],pipedesc[numOfSMs];
	char pipe[SIZEOFBUFFER],buff[SIZEOFBUFFER],**arg,pipename[numOfSMs][SIZEOFBUFFER];
	if(depth==1){	/*If the children processes are to be Sorters, fewer arguments are required */
		numOfArguments=6;
	}else{
		numOfArguments=8;
	}
	arg=malloc(numOfArguments*sizeof(char*));
	for(i=0;i<=numOfArguments;i++){
		arg[i]=malloc(SIZEOFBUFFER*sizeof(char));
	}	/*Creating arguments' vector for child-processes */
	strcpy(arg[1],argv[1]);
	strcpy(arg[5],argv[5]);
	strcpy(pipe,"pipe");
	sprintf(buff,"%d",getpid());
	strcat(pipe,buff);
	strcat(pipe,"ch");
	for(i=0;i<numOfSMs;i++){
		sprintf(buff,"%d",i);
		strcpy(arg[4],pipe);
		strcat(arg[4],buff);
		if(mkfifo(arg[4],0666)==-1){	/*Creating one fifo for every child*/
			if(errno!=EEXIST){
				printf("Error!\n");
				exit(-1);
			}
		}
		strcpy(pipename[i],arg[4]);
		pipedesc[i]=open(arg[4],O_RDONLY|O_NONBLOCK);
		if(pipedesc[i]==-1){
			perror(arg[4]);
		}
		childpid[i]=fork();	/*Creating child process*/
		if(childpid[i]==0){
			sprintf(arg[2],"%ld",Start+i*((End-Start)/numOfSMs));	/*Setting starting point*/
			if(i==numOfSMs-1){		/*Setting Ending point*/
				strcpy(arg[3],argv[3]);
			}else{
				sprintf(arg[3],"%ld",Start+(1+i)*((End-Start)/numOfSMs)-1);
			}
			if(depth==1){
				arg[6]=NULL;
				execvp("./Sorter",arg);
				printf("failure!\n");
			}else{
				strcpy(arg[6],argv[6]);	/*If child process is to be a Splitter/Merger more arguments are required*/
				sprintf(arg[7],"%d",depth-1);
				arg[8]=NULL;
				execvp("./SplitterMerger",arg);
				printf("failure\n");
			}	
		}
	}
	CandidateList * cd;
	ElectionCenterList * el;
	cd=malloc(sizeof(CandidateList));
	el=malloc(sizeof(ElectionCenterList));
	int returningprocess,status,j;	
	for(i=0;i<numOfSMs;i++){	/*Wait for child processes to finish their work*/
		returningprocess=waitpid(-1,&status,0);
		if(WEXITSTATUS(status)==-1){
			printf("An error occured in process %d\n",returningprocess);	
		}else{
			for(j=0;j<numOfSMs;j++){
				if(childpid[j]==returningprocess){	/*Find out which was the returning process*/
					break;
				}
			}
			MergeResults(pipedesc[j],cd,el);	/*Updating Lists including new data*/
			
		}
		
	}
	ForwardResults(fifodesc,cd,el);	/*Forwarding data to parent through parent fifo*/
	ExecutionTimes * T,*r;
	Times * TL;
	TL=malloc(sizeof(Times));
	for(i=0;i<numOfSMs;i++){
		GetTimes(pipedesc[i],TL);	/*Updating Times List*/
	}
	r=malloc(sizeof(ExecutionTimes));
	r->ProcessID=getpid();
	t2=(double)times(&tb2);
	cpu_time=(double)((tb2.tms_utime+tb2.tms_stime)-(tb1.tms_utime+tb1.tms_stime));
	int rd;
	r->RealTime=(t2-t1)/ticspersec;
	r->CPUTime=cpu_time/ticspersec;
	r->Next=TL->FirstProcess;
	TL->FirstProcess=r;	/*Inserting current process' own time*/
	SendTimes(fifodesc,TL->FirstProcess);	/*Forwarding Times through parent fifo*/
	for(i=0;i<numOfSMs;i++){
		unlink(pipename[i]);	/*Deleting children's fifos that are no longer in use*/
	}
	for(i=0;i<=numOfArguments;i++){
		free(arg[i]);
	}
	free(arg);
	DeleteCandidateList(cd);
	DeleteElectionCenterList(el);
}











