#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char censor(char c);
char encrypt(char c); 
char decrypt(char c); 
char xprt(char c); 
char cprt(char c); 
char my_get(char c); 
char quit(char c);
void printMenu();
typedef struct fun_desc {
  char *name;
  char (*fun)(char);
}fun_desc;

fun_desc fun_array[] = {
    {"Censor",censor},
    {"Encrypt",encrypt},
    {"Decrypt",decrypt},
    {"Print hex",xprt},
    {"Print string",cprt},
    {"Get string",my_get},
    {"Quit",quit},
    {NULL,NULL}
};
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  int i;
  for(i=0;i<array_length;++i)
      mapped_array[i]=f(*(array+i));
  return mapped_array;
}
 
int main(int argc, char **argv){
    char carray[5];
    carray[0]='\0';
    int c=0;
    int* pc = &c;
    char *a=NULL;
    int i;
    while(1){
        printMenu();
        scanf("%d",pc);
        if(c>6 || c<0){
            printf("Not within bounds\n");
            return 0;
        }else{
            printf("Within bounds\n");
            a=map(carray,5,fun_array[c].fun);
            for(i=0;i<5;i++)
                carray[i]=a[i];
            printf("Done.\n");
            c=0;
        }
        
    }
    free(a);
    return 0;
}
void printMenu(){
    int i=0;
    printf("Please choose a function:\n");
    while(fun_array[i].name!=NULL){
     printf("%d) %s\n",i,fun_array[i].name);
     ++i;
    }
    printf("Option: ");
}
char censor(char c) {
  if(c == '!')
    return '.';
  else
    return c;
}
char encrypt(char c){
    if(c>0x20 && c<0x70)
        return c+3;
    else
        return c;
}
char decrypt(char c){
 if(c>0x20 && c<0x70)
        return c-3;
    else
        return c;
    
}
char xprt(char c){
    printf("%x\n",c);
    return c;
    
}

char cprt(char c){
    if(c>0x20 && c<0x70)
        printf("%c\n",c);
    else
        printf(".\n");
    return c;
    
}
 
char my_get(char c){
    c = fgetc(stdin);
    return c;
}
char quit(char c){
    exit(c);
}