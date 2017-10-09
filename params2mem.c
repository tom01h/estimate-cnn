#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "params.h"
int main(int argc,char *argv[])
{
  FILE *fp;
  fp = fopen("param0", "w");
  fprintf(fp,"reg [32*32-1:0] mem [0:558-1] = '{\n");
  //printf("reg [31:0] W2 [0:31] [0:8] = \n");
  for(int j=0;j<9;j++){
    fprintf(fp,"    1024'h");
    for(int i=0;i<32;i++){
      unsigned int out=0;
      for(int k=0;k<32;k++){
        int y = j/3;
        int x = j%3;
        int c = k;
        if(W2[i][c*9+y*3+x]==-1) out |= (1<<k);
      }
      if(i==31){fprintf(fp,"%08x, // W2\n",out);}
      else     {fprintf(fp,"%08x_",out);}
    }
  }
  //printf("reg [31:0] W3 [0:63] [0:8] = \n");
  for(int ii=0;ii<64/32;ii++){
    for(int j=0;j<9;j++){
      fprintf(fp,"    1024'h");
      for(int i=0;i<32;i++){
        unsigned int out=0;
        for(int k=0;k<32;k++){
          int y = j/3;
          int x = j%3;
          int c = k;
          if(W3[ii*32+i][c*9+y*3+x]==-1) out |= (1<<k);
        }
        if(i==31){fprintf(fp,"%08x, // W3\n",out);}
        else     {fprintf(fp,"%08x_",out);}
      }
    }
  }
  //printf("reg [31:0] W4 [0:511] [0:31] = \n");
  for(int ii=0;ii<512/32;ii++){
    for(int jj=0;jj<16*64/32;jj++){
      fprintf(fp,"    1024'h");
      for(int i=0;i<32;i++){
        unsigned int out=0;
        for(int j=0;j<32;j++){
          int y = ((jj*32+j)/256);
          int x = ((jj*32+j)/64)%4;
          int c =  (jj*32+j)    %64;
          if(W4[ii*32+i][c*16+y*4+x]==-1) out |= (1<<(c%32));
        }
        if(i==31){fprintf(fp,"%08x, // W4\n",out);}
        else     {fprintf(fp,"%08x_",out);}
      }
    }
  }
  //printf("reg [15:0] mean2 [0:31] = \n");
  fprintf(fp,"    1024'h");
  for(int i=0;i<32;i++){
    if(i==31){fprintf(fp,"0000%04x, // mean2\n",(unsigned short)(mean2[i]*64));}
    else     {fprintf(fp,"0000%04x_",(unsigned short)(mean2[i]*64));}
  }
  //  printf("reg [15:0] mean3 [0:63] = \n");
  for(int i=0;i<64;i++){
    if((i%32)==31){
      fprintf(fp,"0000%04x, // mean3\n",(unsigned short)(mean3[i]*64));
    }else if((i%32)==0){
      fprintf(fp,"    1024'h0000%04x_",(unsigned short)(mean3[i]*64));
    }else{
      fprintf(fp,"0000%04x_",(unsigned short)(mean3[i]*64));
    }
  }
  //printf("reg [15:0] mean4 [0:511] = \n");
  for(int i=0;i<512;i++){
    if(i==511){
      fprintf(fp,"0000%04x  // mean4\n",(unsigned short)(mean4[i]*64));
    }else if((i%32)==31){
      fprintf(fp,"0000%04x, // mean4\n",(unsigned short)(mean4[i]*64));
    }else if((i%32)==0){
      fprintf(fp,"    1024'h0000%04x_",(unsigned short)(mean4[i]*64));
    }else{
      fprintf(fp,"0000%04x_",(unsigned short)(mean4[i]*64));
    }
  }
  fprintf(fp,"};\n");

}
