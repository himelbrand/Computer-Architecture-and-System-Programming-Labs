#include "linux/limits.h"
#include "LineParser.h"
#include "JobControl.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#define LINE_MAX_SIZE  2048
char *strsignal(int sig);

void sig_handler(int signo){
    fprintf(stdout,"\n '%s' signal was ignored\n",(char *)strsignal(signo));
}


void execute(cmdLine *pCmdLine, job ** jobList, job * j,struct termios * shell_tmode ){
    int save_in, save_out;



    if(pCmdLine->inputRedirect){
        save_in = dup(STDIN_FILENO);
        close(STDIN_FILENO);
        fopen(pCmdLine->inputRedirect,"r");
    }
    
    if(pCmdLine->outputRedirect){
        save_out = dup(STDOUT_FILENO);
        close(STDOUT_FILENO);
        fopen(pCmdLine->outputRedirect,"w+");
    } 
    
    if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1){
        perror("Error: ");
        _exit(1);
    }
    
    if(pCmdLine->inputRedirect){
        dup2(save_in, STDIN_FILENO);
        close(save_in);
    }
    
    if(pCmdLine->outputRedirect){
        dup2(save_out, STDOUT_FILENO);
        close(save_out);
    }
    
}

int main (int argc,char *argv[]){
    struct termios attr_old;
    char cwd[PATH_MAX];
    char line_in[LINE_MAX_SIZE];
    int debug = 0;
    pid_t pid;
    int i;
    job ** jobList = (job **)malloc(4);
    *jobList = NULL;
    for(i=1; i<argc; i++){
        if(strcmp(argv[i],"-d")==0)
            debug = 1;
    }
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    setpgid(getpid(),getpid());
    
    while(1){
        tcgetattr(STDIN_FILENO,&attr_old);
        (getcwd(cwd, sizeof(cwd)) != 0 ? printf("%s >", cwd) : 0);
        /* get new line from the user */
        memset(line_in,0,LINE_MAX_SIZE);
        fflush(stdin);
        fgets(line_in, LINE_MAX_SIZE, stdin);
        /*checks fg command*/
        if(strncmp("fg",line_in,2)==0){
        	int index = atoi(&line_in[2]);
        	job * fgJob = findJobByIndex(*jobList,index);
        	runJobInForeground(jobList,fgJob,1,&attr_old,getpgid(getpid()));
        }else if(strncmp("bg",line_in,2)==0){
        	int index = atoi(&line_in[2]);
        	job * bgJob = findJobByIndex(*jobList,index);
        	runJobInBackground(bgJob,1);
        }else if(strcmp(line_in,"quit\n") == 0){
            exit(0);
        }else if(strcmp(line_in,"\n") && line_in[0]!='\0') {
            if(strcmp(line_in,"jobs\n") == 0){
                printf("\n");
                printJobs(jobList);
            }else {
                job  * currentJob = addJob(jobList, line_in);
                tcgetattr(STDIN_FILENO,currentJob->tmodes); 
                pid = fork();
                struct cmdLine *parsedCmdLine = parseCmdLines(line_in);
            	switch (pid) {
                    case 0:
                        setpgid(getpid(),getpid());
  			currentJob->pgid = getpid();
                    	signal(SIGQUIT, SIG_DFL);
	    		signal(SIGTSTP, SIG_DFL);
	    		signal(SIGCHLD, SIG_DFL);
	    		signal(SIGTTIN, SIG_DFL);
	    		signal(SIGTTOU, SIG_DFL);
	    		tcgetattr(STDIN_FILENO,&attr_old);    
                        execute(parsedCmdLine,jobList,currentJob,&attr_old);
                        freeCmdLines(parsedCmdLine);
                        _exit(0);
                        break;
                    case -1:
                        freeCmdLines(parsedCmdLine);
                        printf("The child process has not created");
                        _exit(1);
                    default: 
                        currentJob->pgid = pid;
                        setpgid(pid,pid);
                       /* dealing with 'cd' command */
                        if (strncmp(parsedCmdLine->arguments[0], "cd", 2) == 0 && chdir(parsedCmdLine->arguments[1]) == -1) 
                            perror("Cannot change directory: ");
                        if(parsedCmdLine->blocking)
                            runJobInForeground(jobList,currentJob,0,&attr_old,getpgid(getpid()));
                        else
                            runJobInBackground(currentJob,0);
                        (debug ? fprintf(stderr, "child PID: %d\nexecuting command: %s\n\n", pid,parsedCmdLine->arguments[0]) : 0);
                        freeCmdLines(parsedCmdLine);
                        break;
                }
            }
        }
    }
}

