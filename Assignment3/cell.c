#define SYS_WRITE 4
#define STDOUT 1
#define STDIN 0
#define STDERR 2
#define SYS_READ 3
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LESEEK 19
#define SYS_GETDENTS 141
int Cell(int x,int y);
extern char state[];
extern int WorldWidth;
extern int WorldLength;
extern int system_call(int op,int fd,char * ff,int size);

int Cell (int x,int y){
    int count =0;
    int leftX;
    int rightX;
    int topY;
    int bottomY;
    int isAlive = state[x+(y*WorldWidth)];
    
    int ans=0;
    if(x == 0){
      leftX = WorldWidth -1;
      rightX = 1;
    }else if(x == WorldWidth -1){
      leftX = WorldWidth -2;
      rightX = 0;
    }else{
      leftX = x - 1;
      rightX = x + 1;
    }
    if(y ==0){
      topY= WorldLength -1;
      bottomY=1;        
    }else if(y == WorldLength -1){
      topY=(WorldLength-2);
      bottomY=0;
    }else{
      topY= y+1;
      bottomY=y-1;
    }
    state[rightX+(y*WorldWidth)] > 0 ? count++ : 0; 
    state[(bottomY*WorldWidth)+x] > 0 ? count++ :0; 
    state[(topY*WorldWidth)+x] > 0 ? count++ :0; 
    state[leftX+(y*WorldWidth)] > 0 ? count++ :0; 
    state[rightX+(bottomY*WorldWidth)] > 0 ? count++ :0; 
    state[rightX+(topY*WorldWidth)] > 0 ? count++ :0; 
    state[leftX+(bottomY*WorldWidth)] > 0 ? count++ :0; 
    state[leftX+(topY*WorldWidth)] > 0 ? count++ :0; 
    if((isAlive>0 && (count==2 || count ==3)) || (isAlive==0 && count ==3))
      ans = 1;
    else
      ans = 0;

    return ans;
}


