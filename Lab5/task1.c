#include "linux/limits.h"
#include "LineParser.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define LINE_MAX_SIZE  2048
char *strsignal(int sig);

void sig_handler(int signo)
{
    printf("\n '%s' signal was ignored\n",(char *)strsignal(signo));
}

void execute(cmdLine *pCmdLine){
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1){
            perror("Error: ");
            _exit(1);
        }
}


int main (int argc,char *argv[]){


    char cwd[PATH_MAX];
    char line_in[LINE_MAX_SIZE];
    int debug = 0;
    pid_t pid;
    int i;
    int status;

    for(i=1; i<argc; i++){
        if(strcmp(argv[i],"-d")==0){
            debug = 1;
        }
    }
    signal(SIGQUIT, sig_handler);
    signal(SIGTSTP, sig_handler);
    signal(SIGCHLD, sig_handler);

    while(1){



        (getcwd(cwd, sizeof(cwd)) != 0 ? printf("%s > ", cwd) : 0);
        /* get new line from the user */
        fgets (line_in, LINE_MAX_SIZE, stdin);

        /* check for quit command */
        if(strcmp(line_in,"quit\n") == 0){
            exit(0);
        }
        /* fork cases */
        if(strcmp(line_in,"\n")) {
            pid = fork();
            struct cmdLine *parsedCmdLine = parseCmdLines(line_in);

                switch (pid) {
                    case 0:
                        execute(parsedCmdLine);
                        freeCmdLines(parsedCmdLine);
                        _exit(0);
                        break;

                    case -1:
                        freeCmdLines(parsedCmdLine);
                        printf("The child process has not created");
                        _exit(1);

                    default:
                        /* dealing with 'cd' command */
                        if (strncmp(parsedCmdLine->arguments[0], "cd", 2) == 0) {
                            if (chdir(parsedCmdLine->arguments[1]) == -1) {
                                perror("Cannot change directory: ");
                            }
                        }

                        (debug ? fprintf(stderr, "child PID: %d\nexecuting command: %s\n\n", pid, parsedCmdLine->arguments[0]) : 0);
                        freeCmdLines(parsedCmdLine);
                        if(parsedCmdLine->blocking == 1){
                            waitpid(pid, &status, 0);
                        }
                        break;
                }
            }
            }
        }






