#include <sys/types.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DONE -1
#define RUNNING 1
#define SUSPENDED 0
#define CONTINUE 1
#define TRUE 1
#define FALSE 0

typedef struct job
{
    char *cmd;
    int idx;				/* index of current job (1 for the first) */
	pid_t pgid; 		/* save group id of the job*/
	int status; /* 1 running 0 suspendid -1 Done  TODO: see if there's a need for it print done jobs before a command is performed*/
	struct termios *tmodes; /* saved terminal modes */
    struct job *next;	/* next job in chain */
} job;

/* implemented functions */
job* addJob(job** job_list, char* cmd);
void removeJob(job** job_list, job* tmp);
void printJobs(job** job_list);
void freeJobList(job** job_list);
char* statusToStr(int status);


/* to be implemented during the lab */
job* initializeJob(char* cmd);
void freeJob(job* job_to_remove);
job* findJobByIndex(job * job_list, int idx);
void updateJobList(job **job_list, int remove_done_jobs);
void runJobInForeground (job** job_list, job *j, int cont, struct termios* shell_tmodes, pid_t shell_pgid);
void runJobInBackground (job *j, int cont);

char *strClone_(char *source);

pid_t getpgid(pid_t pid);
int kill(pid_t pid, int sig);