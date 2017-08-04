#include "limits.h"
#include "LineParser.h"
#include "JobControl.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <pwd.h>
#define LINE_MAX_SIZE  2048
char *strsignal(int sig);


typedef struct envLink{
    char * name;
    char * value;
    struct envLink *nextNode;
} envLink;

char * findVal_(envLink * myLink,char * name);

void sig_handler(int signo){
    fprintf(stdout,"\n '%s' signal was ignored\n",(char *)strsignal(signo));
}

 char *_strClone_(char *source){
    char* clone = (char*)malloc(strlen(source) + 1);
    strcpy(clone, source);
    return clone;
}

int subArgs(envLink * myLink,cmdLine * parsedLine){
    char * tempArg = NULL;
    int i=0;
    for(i=0;i<parsedLine->argCount;i++){
        tempArg = parsedLine->arguments[i];
        if(strncmp(tempArg,"$",1) == 0){
            if(findVal_(myLink,tempArg+1) != 0){
                replaceCmdArg(parsedLine,i,findVal_(myLink,tempArg+1));
            }else{
                printf("varible not found\n"); 
                return 0;
            }
        }
    }                                             
    return 1;
}

char * findVal_(envLink * myLink,char * name){
    envLink *temp = myLink;
    while(temp->nextNode != NULL){
    if(strcmp(temp->name,name) == 0){
        return temp->value;
    }
    temp = temp->nextNode;
    }
    if(temp->name != NULL && strcmp(temp->name,name) == 0){
        return temp->value;
    }else{
        return NULL;
    }
}

void envSet(char * name, char * value,envLink *myLink){
    value[strlen(value)-1] = '\0';
    envLink *temp = myLink;
    while(temp->nextNode != NULL){
        if(strcmp(temp->name,name) == 0){
            temp->value = _strClone_(value);
            return;
        }
        temp = temp->nextNode;
    }
    if(temp->name != NULL && strcmp(temp->name,name) == 0){
    temp->value = _strClone_(value);
    }else{
        if(temp->name != NULL){
        envLink * tempEnvLink = (envLink *)malloc(sizeof(envLink));
        temp->nextNode = tempEnvLink;
        temp = temp->nextNode;
        }
        temp->name = _strClone_(name);
        temp->value = _strClone_(value);
        temp->nextNode = NULL;
    }



}

void envPrint(envLink *myLink){
    envLink *temp = myLink;
    while(temp->nextNode != NULL){
        printf("%s=%s\n",temp->name,temp->value);
        temp = temp->nextNode;
    }
    if(temp->name != NULL)
    printf("%s=%s\n",temp->name,temp->value);
}

void envDelete(envLink *myLink,char * name){
    if(myLink->name != NULL){
    envLink *curr = myLink;
    envLink *prev = myLink;
    while(curr->nextNode != NULL){
        if(strcmp(curr->name,name) == 0){
            prev->nextNode = curr->nextNode;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->nextNode;
    }
    if(strcmp(curr->name,name) == 0){
    prev->nextNode = curr->nextNode;
    free(curr);
    curr->name = NULL;
    }else{
        printf("var not found\n");
    }
}
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
    
    if(pCmdLine->next){
    int fd[2];
    pipe(fd);
    int pid1 = fork();
    switch(pid1)
        {
            case 0:
                close(1);
                dup(fd[1]);
                close(fd[1]);

                if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1){
                    perror("Error: ");
                    _exit(1);
                } 
        
                break;
            case -1:
                printf ("The child process has not created");
                _exit(1);
                
            default:
                waitpid(pid1,NULL,0);
                close(fd[1]);
                close(0);
                dup(fd[0]);
                close(fd[0]);
                if (execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments) == -1){
                perror("Error: ");
                _exit(1);
                }                                 
                close(fd[0]);
                break;
        }
        
    }else{
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1){
            perror("Error: ");
            _exit(1);
        }   
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
    envLink *myLink = (envLink *) malloc(sizeof(envLink));
    myLink->name = NULL;
    myLink->value = NULL;
    myLink->nextNode = NULL;

    struct termios attr_old;
    char cwd[PATH_MAX];
    char line_in[LINE_MAX_SIZE];
    int debug = 0;
    pid_t pid;
    int i;
    cmdLine * temp;
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
        char *tempName = line_in,*tempValue = line_in, *end = line_in;
        tcgetattr(STDIN_FILENO,&attr_old);
        (getcwd(cwd, sizeof(cwd)) != 0 ? printf("%s >", cwd) : 0);
        /* get new line from the user */
        memset(line_in,0,LINE_MAX_SIZE);
        fflush(stdin);
        fgets(line_in, LINE_MAX_SIZE, stdin);
        /*checks fg command*/
        if(strncmp("env",line_in,3)==0){
            envPrint(myLink);
        }else if(strncmp("delete",line_in,6)==0){
            strsep(&end, " ");
            tempName = end;
            tempName[strlen(tempName) -1] = '\0';
            envDelete(myLink,tempName);
        }else if(strncmp("set",line_in,3)==0){
        strsep(&end, " ");
        tempName = end;
        strsep(&end, " ");
        tempValue = end;

        envSet(tempName,tempValue,myLink);
        }else if(strncmp("fg",line_in,2)==0){
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
                struct cmdLine *parsedCmdLine = parseCmdLines(line_in);
                if(subArgs(myLink,parsedCmdLine) != 0){

            if (strncmp(parsedCmdLine->arguments[0], "cd", 2) == 0){
            if(strncmp(parsedCmdLine->arguments[1],"~",1) == 0){
                struct passwd *pw = getpwuid(getuid());
                const char *homedir = pw->pw_dir;
                replaceCmdArg(parsedCmdLine,1,homedir);
            }}
                pid = fork();
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
                        if (strncmp(parsedCmdLine->arguments[0], "cd", 2) == 0){
                            if(chdir(parsedCmdLine->arguments[1]) == -1)
                                perror("Cannot change directory: ");
                        }
                        else if(parsedCmdLine->blocking){
                            runJobInForeground(jobList,currentJob,0,&attr_old,getpgid(getpid()));}
                        else{
                            temp = parsedCmdLine;
                            while(temp->next)
                                temp = temp->next;
                            if(temp->blocking)
                                runJobInForeground(jobList,currentJob,0,&attr_old,getpgid(getpid()));
                            else
                                runJobInBackground(currentJob,0);
                        }                           
                        (debug ? fprintf(stderr, "child PID: %d\nexecuting command: %s\n\n", pid,parsedCmdLine->arguments[0]) : 0);
                        freeCmdLines(parsedCmdLine);
                        break;
                }
            }
        }
        }
    }
}

