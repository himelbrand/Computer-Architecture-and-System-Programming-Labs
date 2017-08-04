#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define STDERR 2
#define STDIN 0
#define SYS_READ 3
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LESEEK 19
#define SYS_GETDENTS 141
#define O_RDONLY  0
#define O_WRONLY  1
#define O_RDWR    2
#define O_CREAT   64
#define O_TRUNC   512
#define O_APPEND 1024

extern int system_call();  
void debugger(int code,int ret);
int main (int argc , char* argv[], char* envp[])
{
  int debug_mode=0;
  int encrypt=0;
  int i;
  int input = STDIN;
  int output = STDOUT;
  int ret_val=0;
  for(i=1;i<argc;i++){
      if(strcmp(argv[i],"-i")==0){
          input=system_call(SYS_OPEN, argv[++i], O_RDONLY);
          if(input < 0){
            system_call(SYS_WRITE,STDERR,"Input file cannot be opend!\n", 29);
            return 1;
          }else if(debug_mode)
              debugger(SYS_OPEN,input);
          
      }else if(strcmp(argv[i],"-o")==0){
                output=system_call(SYS_OPEN, argv[++i], O_CREAT|O_WRONLY|O_TRUNC,0644);
                if(output<0){
                  system_call(SYS_WRITE,STDERR,"Output File could not be created!\n", 35);
                  return 1;
                }else if(debug_mode)
                  debugger(SYS_OPEN,output);
      }else if(strcmp(argv[i],"-d")==0) {
          debug_mode=1;
      }else{
          encrypt=argv[i][1]-48;
          if(argv[i][0]==45)
              encrypt*=-1;
      }
  }
  char c;
  char buff[1];  
  while((ret_val=system_call(SYS_READ, input, &c, 1))>0 && c!=-1){
    if(debug_mode)
      debugger(SYS_READ,ret_val);
      if(c==65 && c<=90){
        c+=32;
        c+=encrypt;
        if (c>122)
          c-=26;
        if (c<97)
          c+=26;
      }else if (c>=97 && c<=122){
        c-=32;
        c+=encrypt;
        if (c>90)
          c-=26;
        if (c<65)
          c+=26;
      }
      buff[0]=c;
      ret_val=system_call(SYS_WRITE,output,buff,1);
      if(debug_mode)
        debugger(SYS_WRITE,ret_val);
      
  }
  if(output!=STDOUT){
    ret_val=system_call(SYS_CLOSE,output);
    if(ret_val<0)
        system_call(SYS_WRITE,STDERR,"Error while closing output file\n", 33);
    else if(debug_mode)
        debugger(SYS_CLOSE,ret_val);
  }
  if(input!=STDIN){
    ret_val=system_call(SYS_CLOSE,input);
    if(ret_val<0)
        system_call(SYS_WRITE,STDERR,"Error while closing input file\n", 32);
    else if(debug_mode)
        debugger(SYS_CLOSE,ret_val);
  }
  return 0;
}
void debugger(int code,int ret){
  system_call(SYS_WRITE,STDERR,"dbg: system_call - ID: ",23);
  system_call(SYS_WRITE,STDERR,itoa(code),strlen(itoa(code)));
  system_call(SYS_WRITE,STDERR," return value: ",16);
  system_call(SYS_WRITE,STDERR,itoa(ret),strlen(itoa(ret))); 
  system_call(SYS_WRITE,STDERR,"\n",2); 
}
