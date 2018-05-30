#define SIZEOFBUFFER 40

typedef struct Record{
	char Name[SIZEOFBUFFER];
	short ElectionCenter;
	char valid;
}Record;

typedef struct Candidate{
	char Name[SIZEOFBUFFER];
	long Votes;
	struct Candidate * Next;
}Candidate;

typedef struct CandidateList{
	long InvalidVotes;
	
	Candidate * FirstCandidate;
}CandidateList;

typedef struct ElectionCenter{
	short ID;
	long Votes;
	struct ElectionCenter * Next;
}ElectionCenter;

typedef struct ElectionCenterList{
	ElectionCenter * FirstCenter;
}ElectionCenterList;

typedef struct ExecutionTimes{
	int ProcessID;
	double RealTime;
	double CPUTime;
	struct ExecutionTimes* Next;
}ExecutionTimes;

typedef struct Times{
	ExecutionTimes * FirstProcess; 
}Times;

extern volatile sig_atomic_t signalsreceived;

void signalhandler(int);
void MakeCandidateList(CandidateList*);
void MakeElectionCenterList(ElectionCenterList *);
void MakeTimes(Times *);
void InsertNewCandidate(char *,long,CandidateList *);
void InsertNewCenter(short,long,ElectionCenterList *);
void UpdateCandidateList(CandidateList *,Record *);
void UpdateElectionCenterList(ElectionCenterList *,Record *);
void MergeResults(int,CandidateList *,ElectionCenterList *);
void GetTimes(int,Times *);
void SendTimes(int,ExecutionTimes *);
void ForwardResults(int,CandidateList *,ElectionCenterList *);
int PrintCandidateList(CandidateList *,FILE *);
void PrintElectionCenterList(ElectionCenterList *,FILE*,int,float);
CandidateList * ClassifyCandidateList(CandidateList *);
ElectionCenterList * ClassifyElectionCenterList(ElectionCenterList *);
void DeleteCandidateList(CandidateList *);
void DeleteElectionCenterList(ElectionCenterList *);
void DeleteTimesList(Times *);
