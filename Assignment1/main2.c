#include <stdio.h>
#define	BUFFER_SIZE	(128)

extern int calc_div(int x,int k);
int check(int x,int k);

int main(int argc, char** argv){
  int x,k;
  scanf("%d",&x);
  scanf("%d",&k);
  calc_div(x,k);

  return 0;
}

int check(int x,int k){
	return !(x < 0 || k <= 0 || k >31);
}


