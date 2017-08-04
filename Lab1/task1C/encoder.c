#include <stdio.h>
#include <string.h>
int main(int argc, char **argv){
    int encrypt=0;
    int c=0;
    int i;
    FILE * input = stdin;
        for(i=1;i<argc;i++){
            if(strcmp(argv[i],"-i")==0){
                input=fopen(argv[++i],"r");
                if(input==NULL){
                  fprintf(stderr, "%s", "File cannot be opend!\n");
                  return 1;
                }
            }else {
                encrypt=argv[i][1]-48;
                
                if(argv[i][0]==45)
                    encrypt*=-1;
            }
        }
        while((c=fgetc(input))!=EOF){
                if(c>=65 && c<=90){
                    c+=32;
                    c+=encrypt;
                    if(c>122)
                        c-=26;
                    if(c<97)
                        c+=26;
                }else if(c>=97 && c<=122){
                    c-=32;
                    c+=encrypt;
                    if(c>90)
                        c-=26;
                    if(c<65)
                        c+=26;
                }
           
                fputc(c,stdout);
                    
        }
    return 0;
}