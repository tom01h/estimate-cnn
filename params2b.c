#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "params.h"
int main(int argc,char *argv[])
{
  printf("float W1[32][27]={\n");
  for(int i=0;i<32;i++){
    for(int j=0;j<27;j++){
      int y = (j/9);
      int x = (j/3)%3;
      int c =  j   %3;
      int last = (i==31)&(j==26);
      if(last){
        printf("%e",W1[i][c*9+y*3+x]);
      }else{
        printf("%e,",W1[i][c*9+y*3+x]);
      }
    }
    printf("\n");
  }
  printf("};\n");

  printf("float mean1[32]={\n");
  for(int i=0;i<32;i++){
    int last = (i==31);
    if(last){
      printf("%e\n",mean1[i]);
    }else{
      printf("%e,\n",mean1[i]);
    }
  }
  printf("};\n");

  printf("float var1[32]={\n");
  for(int i=0;i<32;i++){
    int last = (i==31);
    if(last){
      printf("%e\n",var1[i]);
    }else{
      printf("%e,\n",var1[i]);
    }
  }
  printf("};\n");

  printf("int W2[32][9*32]={\n");
  for(int i=0;i<32;i++){
    for(int j=0;j<9*32;j++){
      int y = (j/96);
      int x = (j/32)%3;
      int c =  j    %32;
      int last = (i==31)&(j==9*32-1);
      if(last){
        printf("%d",W2[i][c*9+y*3+x]);
      }else{
        printf("%d,",W2[i][c*9+y*3+x]);
      }
    }
    printf("\n");
  }
  printf("};\n");

  printf("int mean2[32]={\n");
  for(int i=0;i<32;i++){
    int last = (i==31);
    if(last){
      printf("%d\n",(int)(mean2[i]*64));
    }else{
      printf("%d,\n",(int)(mean2[i]*64));
    }
  }
  printf("};\n");

  printf("int W3[64][9*32]={\n");
  for(int i=0;i<64;i++){
    for(int j=0;j<9*32;j++){
      int y = (j/96);
      int x = (j/32)%3;
      int c =  j    %32;
      int last = (i==63)&(j==9*32-1);
      if(last){
        printf("%d",W3[i][c*9+y*3+x]);
      }else{
        printf("%d,",W3[i][c*9+y*3+x]);
      }
    }
    printf("\n");
  }
  printf("};\n");

  printf("int mean3[64]={\n");
  for(int i=0;i<64;i++){
    int last = (i==63);
    if(last){
      printf("%d\n",(int)(mean3[i]*64));
    }else{
      printf("%d,\n",(int)(mean3[i]*64));
    }
  }
  printf("};\n");

  printf("int W4[512][1024]={\n");
  for(int i=0;i<512;i++){
    for(int j=0;j<16*64;j++){
      int y = (j/256);
      int x = (j/64)%4;
      int c =  j    %64;
      int last = (i==511)&(j==1023);
      if(last){
        printf("%d",W4[i][c*16+y*4+x]);
      }else{
        printf("%d,",W4[i][c*16+y*4+x]);
      }
    }
    printf("\n");
  }
  printf("};\n");

  printf("int mean4[512]={\n");
  for(int i=0;i<512;i++){
    int last = (i==511);
    if(last){
      printf("%d\n",(int)(mean4[i]*64));
    }else{
      printf("%d,\n",(int)(mean4[i]*64));
    }
  }
  printf("};\n");

  printf("float W5[10][512]={\n");
  for(int i=0;i<10;i++){
    for(int j=0;j<512;j++){
      int last = (i==9)&(j==511);
      if(last){
        printf("%e",W5[i][j]);
      }else{
        printf("%e,",W5[i][j]);
      }
    }
    printf("\n");
  }
  printf("};\n");

}
