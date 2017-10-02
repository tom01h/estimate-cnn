#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "params.h"

void BinAffine(int xi,int ci, int in[xi], int f[ci][xi], float out[ci][1][1])
{
  for(int c=0; c<ci; c++){
    out[c][0][0] = 0;
    for(int x=0; x<xi; x++){
      out[c][0][0] += f[c][x]*in[x];
    }
  }
}

void Affine(int xi,int ci, int in[xi], float f[ci][xi], float out[ci])
{
  for(int c=0; c<ci; c++){
    out[c] = 0;
    for(int x=0; x<xi; x++){
      out[c] += f[c][x]*in[x];
    }
  }
}

void BinConv(int ci, int yi, int xi, int in[ci][yi+2][xi+2],
             int fci, int fyi, int fxi, int f[fci][ci*fyi*fxi],
             int out[fci][yi][xi])
{
  for(int c=0; c<fci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        out[c][y][x] = 0;
        for(int fc=0; fc<ci; fc++){
          for(int fy=0; fy<fyi; fy++){
            for(int fx=0; fx<fxi; fx++){
              out[c][y][x] += f[c][fc*fyi*fxi+fy*fxi+fx]*in[fc][fy+y][fx+x];
            }
          }
        }    
      }
    }
  }
}

void Conv(int ci, int yi, int xi, unsigned char in[ci][yi+2][xi+2],
          int fci, int fyi, int fxi, float f[fci][ci*fyi*fxi],
          float out[fci][yi][xi])
{
  for(int c=0; c<fci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        out[c][y][x] = 0;
        for(int fc=0; fc<ci; fc++){
          for(int fy=0; fy<fyi; fy++){
            for(int fx=0; fx<fxi; fx++){
              out[c][y][x] += f[c][fc*fyi*fxi+fy*fxi+fx]*(in[fc][fy+y][fx+x]/255.0f);
            }
          }
        }    
      }
    }
  }
}

void BinPool(int ci, int yi, int xi, int in[ci][yi][xi],
             int pyi, int pxi, float out[ci][yi/pyi][xi/pxi])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi/pyi; y++){
      for(int x=0; x<xi/pxi; x++){
        out[c][y][x] = in[c][y*pyi][x*pxi];
        for(int py=0; py<pyi; py++){
          for(int px=0; px<pxi; px++){
            if((float)in[c][y*pyi+py][x*pxi+px]>out[c][y][x]){
              out[c][y][x] = (float)in[c][y*pyi+py][x*pxi+px];
            }
          }
        }    
      }
    }
  }
}

void Pool(int ci, int yi, int xi, float in[ci][yi][xi],
          int pyi, int pxi, float out[ci][yi/pyi][xi/pxi])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi/pyi; y++){
      for(int x=0; x<xi/pxi; x++){
        out[c][y][x] = in[c][y*pyi][x*pxi];
        for(int py=0; py<pyi; py++){
          for(int px=0; px<pxi; px++){
            if(in[c][y*pyi+py][x*pxi+px]>out[c][y][x]){
              out[c][y][x] = in[c][y*pyi+py][x*pxi+px];
            }
          }
        }    
      }
    }
  }
}

void Norm(int ci, int yi, int xi, float in[ci][yi][xi],
          float mean[ci], float var[ci],
          float out[ci][yi][xi])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        out[c][y][x] = (in[c][y][x]-mean[c])/(sqrt(var[c]));
      }
    }
  }
}

void TriActiv(int ci, int yi, int xi, float in[ci][yi][xi],
              int out[ci][yi][xi])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        if(in[c][y][x]>=0.5){
          out[c][y][x] = 1;
        } else if (in[c][y][x]>=-0.5){
          out[c][y][x] = 0;
        } else {
          out[c][y][x] = -1;
        }
      }
    }
  }
}

void BinActiv(int ci, int yi, int xi, float in[ci][yi][xi],
              int out[ci][yi][xi])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        if(in[c][y][x]>=0){
          out[c][y][x] = 1;
        } else {
          out[c][y][x] = -1;
        }
      }
    }
  }
}

int main(int argc,char *argv[])
{
  FILE *fp;
  if((fp = fopen("cifar10-test", "rb")) == NULL ) {
    fprintf(stderr,"There is no cifar10-test\n");
    exit(EXIT_FAILURE);
  }
  unsigned char label[1000];
  unsigned char pict[1000][3][32+2][32+2];


  int pass = 0;
  for (int i=0;i<100;i++){
    label[i] = fgetc(fp);
    for(int c=0; c<3; c++){
      for(int y=0; y<32+2; y++){
        for(int x=0; x<32+2; x++){
          if((x==0)|(x==33)|(y==0)|(y==33)){
            pict[i][c][y][x] = 0;//padding=1
          }else{
            pict[i][c][y][x] = fgetc(fp);
          }
        }
      }
    }    

    float conv1out[32][32][32];
    Conv(3,32,32,pict[i],32,3,3,W1,conv1out);
    float pool1out[32][16][16];
    Pool(32,32,32,conv1out,2,2,pool1out);
    float norm1out[32][16][16];
    Norm(32,16,16,pool1out,mean1,var1,norm1out);
    int activ1out[32][16][16];
    BinActiv(32,16,16,norm1out,activ1out);
    int layer2in[32][18][18];
    for(int c=0; c<32; c++){
      for(int y=0; y<18; y++){
        for(int x=0; x<18; x++){
          if((x==0)|(x==17)|(y==0)|(y==17)){
            layer2in[c][y][x] = 0;//padding=1
          }else{
            layer2in[c][y][x] = activ1out[c][y-1][x-1];
          }
        }
      }
    }
    int conv2out[32][16][16];
    BinConv(32,16,16,layer2in,32,3,3,W2,conv2out);
    float pool2out[32][8][8];
    BinPool(32,16,16,conv2out,2,2,pool2out);
    float norm2out[32][8][8];
    Norm(32,8,8,pool2out,mean2,var2,norm2out);
    int activ2out[32][8][8];
    BinActiv(32,8,8,norm2out,activ2out);
    int layer3in[32][10][10];
    for(int c=0; c<32; c++){
      for(int y=0; y<10; y++){
        for(int x=0; x<10; x++){
          if((x==0)|(x==9)|(y==0)|(y==9)){
            layer3in[c][y][x] = 0;//padding=1
          }else{
            layer3in[c][y][x] = activ2out[c][y-1][x-1];
          }
        }
      }
    }
    int conv3out[64][8][8];
    BinConv(32,8,8,layer3in,64,3,3,W3,conv3out);
    float pool3out[64][4][4];
    BinPool(64,8,8,conv3out,2,2,pool3out);
    float norm3out[64][4][4];
    Norm(64,4,4,pool3out,mean3,var3,norm3out);
    int activ3out[64][4][4];
    BinActiv(64,4,4,norm3out,activ3out);
    int layer4in[64*4*4];
    for(int c=0; c<64; c++){
      for(int y=0; y<4; y++){
        for(int x=0; x<4; x++){
          layer4in[c*4*4+y*4+x] = activ3out[c][y][x];
        }
      }
    }
    float affine4out[512][1][1];
    BinAffine(1024,512,layer4in,W4,affine4out);
    float norm4out[512][1][1];
    Norm(512,1,1,affine4out,mean4,var4,norm4out);
    int activ4out[512][1][1];
    BinActiv(512,1,1,norm4out,activ4out);

    float affine5out[10];
    Affine(512,10,activ4out[0][0],W5,affine5out);

    int result = 0;
    float max = affine5out[0];
    for(int x=0; x<10; x++){
      if(affine5out[x]>max){
        max = affine5out[x];
        result = x;
      }
    }
    if(result==label[i]){
      pass++;
    }else{
      printf ("No. %02d / Right : %02d != Result : %02d\n",i,label[i],result);
    }
  }
  printf ("== Pass Count : %04d ==\n",pass);
}
