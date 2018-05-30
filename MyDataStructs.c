#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "MyDataStructs.h"

/*Signal handler function increases signal counter global volatile variable*/
void signalhandler(int signum){
	signal(SIGUSR1,signalhandler);
	signalsreceived++;
}

/*Initializing CandidateList*/
void MakeCandidateList(CandidateList* CL){
	if(CL==NULL){
		printf("An Error occured while creating Candidate List\n");
		return;
	}
	CL->InvalidVotes=0;
	CL->FirstCandidate=NULL;
}

/*Initializing ElectionCenterList*/
void MakeElectionCenterList(ElectionCenterList * EL){
	if(EL==NULL){
		printf("An Error occured while creating Election Center List\n");
		return;
	}
	EL->FirstCenter=NULL;
}

/*Initializing Times List*/
void MakeTimes(Times * T){
	if(T==NULL){
		printf("An Error occured while creating Processes Times List\n");
		return;
	}
	T->FirstProcess=NULL;
}

/*Adding a new candidate with the information given in the CandidateList according to the name 
If candidate already exists, it merges the new with the existing one*/
void InsertNewCandidate(char * name,long votes,CandidateList * CL){
	if(CL->FirstCandidate==NULL || strcmp(CL->FirstCandidate->Name,name)>0){
		Candidate * C;
		C=malloc(sizeof(Candidate));
		strcpy(C->Name,name);
		C->Votes=votes;
		C->Next=CL->FirstCandidate;
		CL->FirstCandidate=C;
		return;
	}
	Candidate * current;	/*Using "current" variable to access the existing candidates in the list*/
	current=CL->FirstCandidate;
	do{
		if(strcmp(current->Name,name)==0){	/*If candidate already exists, their votes are being merged*/
			current->Votes=current->Votes+votes;
			return;
		}
		if(current->Next==NULL || strcmp(current->Next->Name,name)>0){
			Candidate * C;
			C=malloc(sizeof(Candidate));
			strcpy(C->Name,name);
			C->Votes=votes;
			C->Next=current->Next;
			current->Next=C;
			return;
		}
		
		current=current->Next;
	}while(current!=NULL);
}

/*Adding a new election center with the information given in the ElectionCenterList according to the center ID 
If election center already exists, it merges the new with the existing one*/
void InsertNewCenter(short id,long votes,ElectionCenterList * EL){
	if(EL->FirstCenter==NULL || EL->FirstCenter->ID<id){
		ElectionCenter * C;
		C=malloc(sizeof(ElectionCenter));
		C->ID=id;
		C->Votes=votes;
		C->Next=EL->FirstCenter;
		EL->FirstCenter=C;
		return;
	}
	ElectionCenter * current;
	current=EL->FirstCenter;
	do{
		if(current->ID==id){	/*If election center with ID "id" already exists, its votes are being merged*/
			current->Votes=current->Votes+votes;
			return;
		}
		if(current->Next==NULL || current->Next->ID<id){
			ElectionCenter * C;
			C=malloc(sizeof(ElectionCenter));
			C->ID=id;
			C->Votes=votes;
			C->Next=current->Next;
			current->Next=C;
			return;
		}
		current=current->Next;	/*Using current pointer to access existing election centers in the list*/
	}while(current!=NULL);
}


/*Updating Candidate List by adding one vote.Candidate list is being sorted according to the candidate name*/
void UpdateCandidateList(CandidateList * CL,Record * Rec){
	if(Rec==NULL){
		printf("NULL\n");
		return;
	}
	if(Rec->valid=='0'){
		CL->InvalidVotes++;
		return;
	}
	
	if(CL->FirstCandidate==NULL || strcmp(CL->FirstCandidate->Name,Rec->Name)>0){
		Candidate * C;
		C=malloc(sizeof(Candidate));
		strcpy(C->Name,Rec->Name);
		C->Votes=1;
		C->Next=CL->FirstCandidate;
		CL->FirstCandidate=C;
		return;
	}
	Candidate * current;
	current=CL->FirstCandidate;
	do{
		if(strcmp(current->Name,Rec->Name)==0){
			current->Votes++;	/*If Candidate Already exists in list, one more vote is being added*/
			return;
		}
		if(current->Next==NULL || strcmp(current->Next->Name,Rec->Name)>0){
			Candidate * C;
			C=malloc(sizeof(Candidate));
			strcpy(C->Name,Rec->Name);
			C->Votes=1;
			C->Next=current->Next;
			current->Next=C;
			return;
		}
		
		current=current->Next;
	}while(current!=NULL);
}

/*Updating Election Center List by adding one vote. ElectionCenter list is being sorted according to the center ID*/
void UpdateElectionCenterList(ElectionCenterList * EL,Record * Rec){
	if(EL->FirstCenter==NULL || (EL->FirstCenter->ID > Rec->ElectionCenter)){
		ElectionCenter * EC;
		EC=malloc(sizeof(ElectionCenter));
		EC->ID=Rec->ElectionCenter;
		EC->Votes=1;
		EC->Next=EL->FirstCenter;
		EL->FirstCenter=EC;
		return;
	}
	ElectionCenter * current;
	current=EL->FirstCenter;
	do{
		if(current->ID==Rec->ElectionCenter){
			current->Votes++;
			return;
		}
		if(current->Next==NULL || (current->Next->ID > Rec->ElectionCenter)){
			ElectionCenter * EC;
			EC=malloc(sizeof(ElectionCenter));
			EC->ID=Rec->ElectionCenter;
			EC->Votes=1;
			EC->Next=current->Next;
			current->Next=EC;
			return;
		}
		current=current->Next;
	}while(current!=NULL);

}

/*Reading results from fifo file and updating Data lists*/
void MergeResults(int filedesc,CandidateList *CL,ElectionCenterList * EL){
	char buff[SIZEOFBUFFER],name[SIZEOFBUFFER];
	long votes;
	int rd=read(filedesc,buff,SIZEOFBUFFER);
	if(strcmp(buff,"CandidateList")==0){	/*"CandidateList" string indicates that we are about to update Candidate List*/
		rd=read(filedesc,buff,SIZEOFBUFFER*sizeof(char));
		while(rd>0 && strcmp(buff,"EndOfCandidateList")!=0){	
			if(strcmp(buff,"InvalidVotes")==0){	/*"InvalidVotes" string indicates that the next data in fifo*/
				rd=read(filedesc,buff,SIZEOFBUFFER*sizeof(char));	/*...is the amount of invalid votes*/
				CL->InvalidVotes=CL->InvalidVotes+atoi(buff);
				rd=read(filedesc,buff,SIZEOFBUFFER*sizeof(char));
			}	/*"EndOfCandidateList" string indicates that we are done with Candidate list*/
			if(strcmp(buff,"InvalidVotes") && strcmp(buff,"EndOfCandidateList")){
				rd=read(filedesc,name,SIZEOFBUFFER*sizeof(char));
				votes=atoi(name);
				InsertNewCandidate(buff,votes,CL);
				rd=read(filedesc,buff,SIZEOFBUFFER*sizeof(char));
			}
		}
	}
	rd=read(filedesc,buff,SIZEOFBUFFER*sizeof(char));
	short id;	/*"ElectionCenterList" string indicates that we are about to update Election Center List*/
	if(strcmp(buff,"ElectionCenterList")==0){
		rd=read(filedesc,buff,SIZEOFBUFFER*sizeof(char));
		while(rd>0 && strcmp(buff,"EndOfCenterList")!=0){/*"EndOfCenterList" string indicates that...*/	
			id=atoi(buff);				/*...we are done with the list*/
			rd=read(filedesc,buff,SIZEOFBUFFER*sizeof(char));
			votes=atoi(buff);
			InsertNewCenter(id,votes,EL);
			rd=read(filedesc,buff,SIZEOFBUFFER*sizeof(char));
		}
	}
}

/*Reading resuls from fifo file concerning execution times and updating Time list*/
void GetTimes(int pipedesc,Times *TL){
	ExecutionTimes * new;
	char buff[SIZEOFBUFFER];
	int rd=read(pipedesc,buff,SIZEOFBUFFER*sizeof(char));
	if(strcmp(buff,"Times")!=0){	/*"Times" string indicates that we are about to update Times list*/
		return;
	}
	rd=read(pipedesc,buff,SIZEOFBUFFER*sizeof(char));
	while(rd>0){
		if(strcmp(buff,"End")==0){	/*"End" string designates the end of data concerning times */
			break;
		}
		new=malloc(sizeof(ExecutionTimes));
		new->ProcessID=atoi(buff);
		rd=read(pipedesc,buff,SIZEOFBUFFER*sizeof(char));
		new->RealTime=atof(buff);
		rd=read(pipedesc,buff,SIZEOFBUFFER*sizeof(char));
		new->CPUTime=atof(buff);
		new->Next=TL->FirstProcess;
		TL->FirstProcess=new;
		rd=read(pipedesc,buff,SIZEOFBUFFER*sizeof(char));		
	}
}


/*Writing data from the lists given in the parent fifo with the filedesc given*/
void ForwardResults(int filedesc,CandidateList * CL,ElectionCenterList * EL){
	int wr;
	char str[SIZEOFBUFFER];	/*Writing CandidateList data in proper mode*/
	wr=write(filedesc,"CandidateList",SIZEOFBUFFER*sizeof(char));
	wr=write(filedesc,"InvalidVotes",SIZEOFBUFFER*sizeof(char));
	sprintf(str,"%ld",CL->InvalidVotes);
	wr=write(filedesc,str,SIZEOFBUFFER*sizeof(char));
	Candidate * currentCandidate;
	currentCandidate=CL->FirstCandidate;
	while(currentCandidate!=NULL){
		wr=write(filedesc,currentCandidate->Name,SIZEOFBUFFER*sizeof(char));
		sprintf(str,"%ld",currentCandidate->Votes);
		wr=write(filedesc,str,SIZEOFBUFFER*sizeof(char));
		currentCandidate=currentCandidate->Next;		
	}
	wr=write(filedesc,"EndOfCandidateList",SIZEOFBUFFER*sizeof(char));
	/*Writing ElectionCenterList data in proper mode*/
	wr=write(filedesc,"ElectionCenterList",SIZEOFBUFFER*sizeof(char));
	ElectionCenter * currentCenter;
	currentCenter=EL->FirstCenter;
	while(currentCenter!=NULL){
		sprintf(str,"%hd",currentCenter->ID);
		wr=write(filedesc,str,SIZEOFBUFFER*sizeof(char));
		sprintf(str,"%ld",currentCenter->Votes);
		wr=write(filedesc,str,SIZEOFBUFFER*sizeof(char));
		currentCenter=currentCenter->Next;		
	}
	wr=write(filedesc,"EndOfCenterList",SIZEOFBUFFER*sizeof(char));
}

/*Writing time data of process in the parent fifo file with file descriptor fifodesc*/
void SendTimes(int fifodesc,ExecutionTimes * T){
	int wr;
	char buff[SIZEOFBUFFER];
	wr=write(fifodesc,"Times",SIZEOFBUFFER*sizeof(char));
	while(T!=NULL){
		sprintf(buff,"%d",T->ProcessID);
		wr=write(fifodesc,buff,SIZEOFBUFFER*sizeof(char));
		sprintf(buff,"%lf",T->RealTime);
		wr=write(fifodesc,buff,SIZEOFBUFFER*sizeof(char));
		sprintf(buff,"%lf",T->CPUTime);
		wr=write(fifodesc,buff,SIZEOFBUFFER*sizeof(char));
		T=T->Next;
	}
	wr=write(fifodesc,"End",SIZEOFBUFFER*sizeof(char));
}

/*Printing all candidates and thei votes in descending order in stdout or output file*/
int PrintCandidateList(CandidateList * CL,FILE * Out){
	if(CL==NULL){
		return;
	}
	if(Out==NULL){
		printf("\nVotes per Candidate:\nVotes\tCandidate Name\n");
	}else{	/*If there is an output file given, the list is being printed there*/
		fprintf(Out,"\nVotes per Candidate:\nVotes\tCandidate Name\n");	
	}
	long total=0;
	Candidate * current=CL->FirstCandidate;
	while(current!=NULL){
		if(Out==NULL){
			printf("%ld\t%s\n",current->Votes,current->Name);
		}else{
			fprintf(Out,"%ld\t%s\n",current->Votes,current->Name);	
		}
		total=total+current->Votes;
		current=current->Next;
	}
	total=total+CL->InvalidVotes;
	if(Out==NULL){
		printf("Total:%ld\n",total);
		printf("Invalid Votes:%ld\n",CL->InvalidVotes);
	}else{
		fprintf(Out,"Total:%ld\n",total);
		fprintf(Out,"Invalid Votes:%ld\n",CL->InvalidVotes);
	}
	return total;	/*Returning the total number of votes (valid and invalid)*/
}

/*Printing top election centers, according to percentile given
Default Percentile is 100% */
void PrintElectionCenterList(ElectionCenterList * EL,FILE * Out,int total,float Percentile){
	ElectionCenter * current=EL->FirstCenter;
	if(Out==NULL){
		printf("\nTop Election Centers:\nVotes\tElection Center ID\n");	
	}else{
		fprintf(Out,"\nTop Election Centers:\nVotes\tElection Center ID\n");
	}
	float p=0,min=0,max=0;	
	max=max+((((double)current->Votes)/((double)total))*100);
	if(Out==NULL){
		printf("%ld\t%hd\n",current->Votes,current->ID);	
	}else{
		fprintf(Out,"%ld\t%hd\n",current->Votes,current->ID);
	}
	if(max>=Percentile){
		if(Out==NULL){
			printf("Exact percentile:%.2f%%\n\n",max);
		}else{
			fprintf(Out,"Exact percentile:%.2f%%\n\n",max);
		}
		return;
	}	
	min=min+(((current->Votes)/(total))*100);
	current=current->Next;
	while(current!=NULL){
		max=max+((((double)current->Votes)/((double)total))*100);
		if(max<=Percentile){
			if(Out==NULL){
				printf("%ld\t%hd\n",current->Votes,current->ID);	
			}else{
				fprintf(Out,"%ld\t%hd\n",current->Votes,current->ID);
			}	
			min=min+(((current->Votes)/(total))*100);
			current=current->Next;
		}else{
			if((Percentile-min)>(max-Percentile)){
				if(Out==NULL){
					printf("%ld\t%hd\n",current->Votes,current->ID);	
				}else{
					fprintf(Out,"%ld\t%hd\n",current->Votes,current->ID);
				}
			}
			break;			
		}
	}
	if(Out==NULL){
		printf("Exact percentile:%.2f%%\n\n",max);
	}else{
		fprintf(Out,"Exact percentile:%.2f%%\n\n",max);
	}
}

/*Classifying CandidateList according to the votes that each candidate has taken*/
CandidateList * ClassifyCandidateList(CandidateList * CL){
	Candidate * max,*current,*last;
	last=NULL;
	CandidateList * NL;	/*Creating a new list with the classified data*/
	int empty=1,total;
	NL=malloc(sizeof(CandidateList));
	MakeCandidateList(NL);
	max=CL->FirstCandidate;
	while(CL->FirstCandidate!=NULL){
		current=CL->FirstCandidate;
		while(current!=NULL){
			if(current->Votes!=-1){
				empty=0;
			}
			if(current->Votes>max->Votes){	/*geting candidate with the maximum votes existing in the list*/
				max=current;
			}
			current=current->Next;
		}
		if(empty==1){
			break;
		}
		current=malloc(sizeof(Candidate));	/*allocating new candidate memory*/
		strcpy(current->Name,max->Name);
		current->Votes=max->Votes;	/*Copying data from max candidate*/
		total=total+current->Votes;
		current->Next=NULL;	
		if(last==NULL){	/*Inserting new candidate on the top of the new list*/
			NL->FirstCandidate=current;	
			last=current;
		}else{
			last->Next=current;
			last=current;
		}
		max->Votes=-1;	/*Marking max candidate's votes*/
		empty=1;
	}
	NL->InvalidVotes=CL->InvalidVotes;	/*Getting invalid votes*/
	DeleteCandidateList(CL);	/*Deleting old list*/
	return NL;	/*Returning new list*/
}

/*Classifying ElectionCenterList according to the votes of each center*/
ElectionCenterList * ClassifyElectionCenterList(ElectionCenterList * CL){
	ElectionCenter * max,*current,*last;
	last=NULL;
	ElectionCenterList * NL;
	int empty=1,total;
	NL=malloc(sizeof(ElectionCenterList));	/*Creating a new list for the classified data*/
	MakeElectionCenterList(NL);
	max=CL->FirstCenter;
	while(CL->FirstCenter!=NULL){
		current=CL->FirstCenter;
		while(current!=NULL){
			if(current->Votes!=-1){
				empty=0;
			}
			if(current->Votes>max->Votes){
				max=current;	/*Getting election center with the maximum votes existing*/
			}
			current=current->Next;
		}
		if(empty==1){
			break;
		}
		current=malloc(sizeof(ElectionCenter));
		current->ID=max->ID;
		current->Votes=max->Votes;
		total=total+current->Votes;
		current->Next=NULL;	
		if(last==NULL){	/*New center is being placed in the top of the new classified list*/
			NL->FirstCenter=current;
			last=current;
		}else{
			last->Next=current;
			last=current;
		}
		max->Votes=-1;	/*Marking max center's votes*/
		empty=1;
	}
	return NL;
}

void DeleteCandidateList(CandidateList * CL){
	Candidate * C,* n;
	if(CL->FirstCandidate==NULL){
		free(CL);
		return;
	}
	C=CL->FirstCandidate;
	n=C->Next;
	while(n!=NULL){
		free(C);
		C=n;
		n=n->Next;
	}
	free(C);
	free(CL);
	return;
}
void DeleteElectionCenterList(ElectionCenterList * EL){
	ElectionCenter * C,* n;
	if(EL->FirstCenter==NULL){
		free(EL);
		return;
	}
	C=EL->FirstCenter;
	n=C->Next;
	while(n!=NULL){
		free(C);
		C=n;
		n=n->Next;
	}
	free(C);
	free(EL);
	return;
}
void DeleteTimesList(Times *TL){
	ExecutionTimes * C,* n;
	if(TL->FirstProcess==NULL){
		free(TL);
		return;
	}
	C=TL->FirstProcess;
	n=C->Next;
	while(n!=NULL){
		free(C);
		C=n;
		n=n->Next;
	}
	free(C);
	free(TL);
	return;
}


