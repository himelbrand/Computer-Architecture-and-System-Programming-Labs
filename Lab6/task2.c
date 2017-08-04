#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "linux/limits.h"
int waitpid(int pid,int * p,int o);
int main (int argc,char *argv[]){
    
    
    int fd[2];
    pipe(fd);
    int pid1 = fork();
    int pid2;
    char *args[3];
    char *args1[4];
    switch(pid1)
        {
            case 0:
                close(1);
                dup(fd[1]);
                close(fd[1]);
                args[0] = "ls";
                args[1] = "-l";
                args[2] = NULL;
                execvp("ls",args);
                break;
            case -1:
                printf ("The child process has not created");
                _exit(1);
                
            default:
                
                    waitpid(pid1,NULL,0);
                    close(fd[1]);
                    pid2 = fork();
                        switch(pid2)
                        {
                            case 0:
                                close(0);
                                dup(fd[0]);
                                close(fd[0]);
                                args1[0] = "tail";
                                args1[1] = "-n";
                                args1[2] = "2";
                                args1[3] = NULL;
                                execvp(args1[0],args1);
                                break;
                                
                            case -1:
                                printf ("The child process has not created");
                                _exit(1);
                                
                            default:

                                break;
                        }
                        close(fd[0]);
                        
                        waitpid(pid2,NULL,0);
                        
                break;
        }
        return 0;


}

