#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <sys/times.h>
#include "MyDataStructs.h"
#include <string.h>

volatile sig_atomic_t signalsreceived;	/*Global variable for counting received signals*/

/*MakeGnuplotDataFiles function creates the two data files that are being used to produce the plots required*/
void MakeGnuplotDataFiles(CandidateList * cl,ElectionCenterList * el,int total){
	FILE * candidates,*electioncenters;
	candidates=fopen("ResultsPerCandidate.txt","w");/*Opening file to write the data for the first plot*/
	if(candidates!=NULL){
		Candidate * cand=cl->FirstCandidate;	/*cand variable is being used to access all the candidates ...*/
		while(cand!=NULL){			/*... on the Candidate List*/
			fprintf(candidates,"%s %.2f\n",cand->Name,(((double)cand->Votes)/((double)total))*100);
			cand=cand->Next;
		}
		fclose(candidates);
	}
	electioncenters=fopen("ResultsPerElectionCenter.txt","w");/*Opening file to write the data for the second plot*/
	if(electioncenters!=NULL){
		ElectionCenter* elcent=el->FirstCenter;	/*el variable is being used to access all the election centers...*/
		while(elcent!=NULL){			/*... on the ElectionCenterList*/
			fprintf(electioncenters,"%hd %ld\n",elcent->ID,elcent->Votes);
			elcent=elcent->Next;
		}
		fclose(electioncenters);
	}
}

/*MakeGnuplotScript function creates the batch file that will be loaded when the gnuplot programm will be called*/
void MakeGnuplotScript(){
	FILE * plot1;
	plot1=fopen("plot1.p","w");
	if(plot1!=NULL){
		fprintf(plot1,"set boxwidth 0.5\nset style fill solid\nset xtics rotate by 315\nset xlabel \"Candidate Name\"\nset ylabel \"Percentage(%%)\"\nset xtics font \"Arial,8\"\nplot \"ResultsPerCandidate.txt\" using 2:xtic(1) with boxes\npause -1\nset terminal jpeg\nset output \"results.jpg\"\nreplot\n");
		fprintf(plot1,"set xlabel \"Election Center ID\"\nset ylabel \"Total Votes\"\nplot \"ResultsPerElectionCenter.txt\" using 2:xtic(1) with boxes\nset output \"resultspercenter.jpg\"\nplot \"ResultsPerElectionCenter.txt\" using 2:xtic(1) with boxes\nreplot\n");
	}
	fclose(plot1);
}


int main(int argc,char ** argv){
	double t1,t2,cpu_time;
	struct tms tb1,tb2;
	double ticspersec;
	ticspersec=(double)sysconf(_SC_CLK_TCK);
	t1=(double)times(&tb1);
	signalsreceived=0;
	signal(SIGUSR1,signalhandler);
	if(argc>1){
		int i,numOfSMs,Depth;
		numOfSMs=0;
		Depth=0;
		char TextFile[SIZEOFBUFFER],OutputFile[SIZEOFBUFFER];
		strcpy(TextFile,"");
		strcpy(OutputFile,"");
		FILE * Output;
		Output=NULL;
		float Percentile=100.0;
		for(i=0;i<argc;i++){
			if(strcmp("-i",argv[i])==0 && i<argc-1){
				strcpy(TextFile,argv[i+1]);
			}else if(strcmp("-l",argv[i])==0 && i<argc-1){
				numOfSMs=atoi(argv[i+1]);
			}else if(strcmp("-d",argv[i])==0 && i<argc-1){
				Depth=atoi(argv[i+1]);
			}else if(strcmp("-p",argv[i])==0 && i<argc-1){
				Percentile=atof(argv[i+1]);
			}else if(strcmp("-o",argv[i])==0 && i<argc-1){
				strcpy(OutputFile,argv[i+1]);
			}
		}
		if(numOfSMs==0){
			printf("Missing arguments!\n");
			exit(-1);
		}
		if(Depth<=1){
			printf("Invalid depth value!\n");
			exit(-1);
		}
		if(Percentile<0 || Percentile>100){
			printf("Invalid Percentile!\n");
			exit(-1);
		}
		FILE * Input;
		Input=fopen(TextFile,"r");
		if(Input==NULL){
			printf("Could not open TextFile %s!\n",TextFile);
			exit(-1);
		}
		if(fseek(Input,0,SEEK_END)<0){
			perror("fseek");
			exit(-1);
		}/*Getting the end of the file to find out the size of the file*/
		long End=ftell(Input);
		if(fseek(Input,0,SEEK_SET)<0){
			perror("fseek");
			exit(-1);
		}
		fclose(Input);
		if(strcmp(OutputFile,"")!=0){	/*If the user has given an output file, it's being opened*/
			Output=fopen(OutputFile,"w");
			if(Output==NULL){
				perror("Opening output file");
				exit(-1);
			}
		}	/*childpid array contains the process ids of all child-processes*/
		int childpid[numOfSMs],pipedesc[numOfSMs],k;/*pipedesc array contains the file descriptors of every child's fifo*/
		char **pipename,pipe[SIZEOFBUFFER],buff[SIZEOFBUFFER],**arg;
		arg=malloc(8*sizeof(char*));
		pipename=malloc(numOfSMs*sizeof(char*));
		for(i=0;i<8;i++){	/*allocating memory for the arguments' vector*/
			arg[i]=malloc(SIZEOFBUFFER*sizeof(char));
		}
		for(i=0;i<numOfSMs;i++){
			pipename[i]=malloc(SIZEOFBUFFER*sizeof(char));
		}
		strcpy(buff,"pipe");
		strcpy(arg[1],TextFile);	
		sprintf(arg[5],"%d",getpid());	/*Argument #5 contains the root process id*/
		sprintf(arg[6],"%d",numOfSMs);
		sprintf(arg[7],"%d",Depth-1);	/*Depth is being reduced by 1 for the the child processes*/
		for(i=0;i<numOfSMs;i++){
			sprintf(arg[2],"%ld",0+i*(End/numOfSMs));/*argument #2 indicates the starting point...*/
			if(i!=numOfSMs-1){			/*... in TextFile for the child process*/
				sprintf(arg[3],"%ld",0+(i+1)*(End/numOfSMs)-1);	/*Each child process has to work on the ...*/
			}else{							/*... 1/numOfSMs portion of the file*/
				sprintf(arg[3],"%ld",End);
			}
			strcpy(pipename[i],"pipe");	/*Each pipe is being named in the following way:*/
			sprintf(buff,"%d",getpid());	/* pipeXXXXchY where XXXX is the process id of the parent ... */
			strcat(pipename[i],buff);	/*... and Y is the number of the child (it's succession number)*/
			strcat(pipename[i],"ch");
			sprintf(buff,"%d",i);
			strcat(pipename[i],buff);
			strcpy(arg[4],pipename[i]);
			if(mkfifo(pipename[i],0666)==-1){	/*fifo is being created*/
				if(errno!=EEXIST){
					printf("Error!\n");
					exit(-1);
				}
			}
			pipedesc[i]=open(pipename[i],O_RDONLY|O_NONBLOCK);	/*fifo is being opened for reading only*/
			childpid[i]=fork();	/*A child process is being created*/
			if(childpid[i]==0){
				execvp("./SplitterMerger",arg);	/*splitter program is being called with the arguments above*/
				printf("Could not execute SplitterMerger program\n");
				exit(-1);
			}
		}
		CandidateList * cd, *n;
		ElectionCenterList * el;
		el=malloc(sizeof(ElectionCenterList));
		cd=malloc(sizeof(CandidateList));
		MakeCandidateList(cd);
		MakeElectionCenterList(el);
		int returningprocess,status,j;
		for(i=0;i<numOfSMs;i++){	/*Getting results from running child processes*/
			returningprocess=waitpid(-1,&status,0);
			if(WEXITSTATUS(status)==-1){
				printf("An error occured in process %d\n",returningprocess);	
			}else{
				for(j=0;j<numOfSMs;j++){
					if(childpid[j]==returningprocess){
						break;	/*find out which was the returning process*/
					}
				}
				MergeResults(pipedesc[j],cd,el);/*Updating lists including using new results*/
			}
		}
		printf("Root Process has received %d USR1 signals\n",signalsreceived);
		if(signalsreceived<pow(numOfSMs,Depth)){
			DeleteCandidateList(cd);
			DeleteElectionCenterList(el);
			for(i=0;i<numOfSMs;i++){	/*Deleting the fifos that the root has created*/
				unlink(pipename[i]);
			}		
			for(i=0;i<numOfSMs;i++){
				free(pipename[i]);
			}
			exit(0);	/*If the root receives less than l^d signals, it terminates */
		}	/*If all signals were received, the root process starts producing results*/
		cd=ClassifyCandidateList(cd);	/*Classifying lists according to the votes*/
		el=ClassifyElectionCenterList(el);
		int rd;
		ExecutionTimes * T,*r;
		Times * TL;
		TL=malloc(sizeof(Times));
		for(i=0;i<numOfSMs;i++){
			GetTimes(pipedesc[i],TL);	/*Updating Times List*/
		}
		int total=PrintCandidateList(cd,Output); /*Printing Results per candidate in descenting order*/
		PrintElectionCenterList(el,Output,total,Percentile);	 /*Printing top electioncenters*/
		t2=(double)times(&tb2);
		cpu_time=(double)((tb2.tms_utime+tb2.tms_stime)-(tb1.tms_utime+tb1.tms_stime));
		r=malloc(sizeof(ExecutionTimes));
		r->ProcessID=getpid();
		r->RealTime=(t2-t1)/ticspersec;
		r->CPUTime=cpu_time/ticspersec;
		r->Next=TL->FirstProcess;
		TL->FirstProcess=r;	/*Adding its own time data in the times list*/
		ExecutionTimes * p;
		p=r;	/*Printing all offspring processes time data*/
		printf("PROCESS ID\tREAL TIME\tCPU TIME\n");
		while(p!=NULL){
			printf("%d\t\t%lf\t%lf\n",p->ProcessID,p->RealTime,p->CPUTime);
			p=p->Next;
		}
		for(i=0;i<numOfSMs;i++){	/*Deleting the fifos that the root has created*/
			unlink(pipename[i]);
		}
		for(i=0;i<numOfSMs;i++){
			free(pipename[i]);
		}
		free(pipename);
		DeleteTimesList(TL);
		MakeGnuplotDataFiles(cd,el,total);
		MakeGnuplotScript();
		DeleteCandidateList(cd);
		DeleteElectionCenterList(el);
		int gnu=fork();	/*Creating new process to execute gnuplot*/
		if(gnu==0){
			char **args;
			args=malloc(sizeof(char *));
			args[0]=malloc(sizeof(char));
			strcpy(args[0],"script.p");	/*Adding as argument the name of the script to load*/
			args[1]=NULL;
			execlp("gnuplot"," ","plot1.p",NULL);
			printf("gnuplot failed\n");
		}
	}
	exit(-1);
}
