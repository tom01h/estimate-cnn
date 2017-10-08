#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "params.h"
int main(int argc,char *argv[])
{
  FILE *fp[32];
  fp[0] = fopen("param0", "w");
  fprintf(fp[0],"reg [31:0] mem [0:558*32-1] = '{\n");
  //printf("reg [31:0] W2 [0:31] [0:8] = \n");
  for(int i=0;i<32;i++){
    for(int j=0;j<9;j++){
      unsigned int out=0;
      for(int k=0;k<32;k++){
        int y = j/3;
        int x = j%3;
        int c = k;
        if(W2[i][c*9+y*3+x]==-1) out |= (1<<k);
      }
      fprintf(fp[0],"    32'h%08x, // W2\n",out);
    }
  }
  //printf("reg [31:0] W3 [0:63] [0:8] = \n");
  for(int i=0;i<64;i++){
    for(int j=0;j<9;j++){
      unsigned int out=0;
      for(int k=0;k<32;k++){
        int y = j/3;
        int x = j%3;
        int c = k;
        if(W3[i][c*9+y*3+x]==-1) out |= (1<<k);
      }
      fprintf(fp[0],"    32'h%08x, // W3\n",out);
    }
  }
  //printf("reg [31:0] W4 [0:511] [0:31] = \n");
  for(int i=0;i<512;i++){
    unsigned int out=0;
    for(int j=0;j<16*64;j++){
      int y = (j/256);
      int x = (j/64)%4;
      int c =  j    %64;
      if(W4[i][c*16+y*4+x]==-1) out |= (1<<(c%32));
      if((c%32)==31){
        fprintf(fp[0],"    32'h%08x, // W4\n",out);
        out=0;
      }
    }        
  }
  //printf("reg [15:0] mean2 [0:31] = \n");
  for(int i=0;i<32;i++){
    fprintf(fp[0],"    32'h0000%04x, // mean2\n",(unsigned short)(mean2[i]*64));
  }
  //  printf("reg [15:0] mean3 [0:63] = \n");
  for(int i=0;i<64;i++){
    fprintf(fp[0],"    32'h0000%04x, // mean3\n",(unsigned short)(mean3[i]*64));
  }
  //printf("reg [15:0] mean4 [0:511] = \n");
  for(int i=0;i<512;i++){
    if(i==511){
      fprintf(fp[0],"    32'h0000%04x  // mean4\n",(unsigned short)(mean4[i]*64));
    }else{
      fprintf(fp[0],"    32'h0000%04x, // mean4\n",(unsigned short)(mean4[i]*64));
    }
  }
  fprintf(fp[0],"};\n");

}
