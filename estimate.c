#include <stdio.h>
#include <stdlib.h>

#include "paramb.h"

void BinAffine(int xi,int ci, int in[xi], int f[ci][xi], int out[1][1][ci])
{
  for(int c=0; c<ci; c++){
    out[0][0][c] = 0;
    for(int x=0; x<xi; x++){
      out[0][0][c] += f[c][x]*in[x];
    }
  }
}

void Affine(int xi,int ci, int in[1][1][xi], int f[ci][xi], int out[ci])
{
  for(int c=0; c<ci; c++){
    out[c] = 0;
    for(int x=0; x<xi; x++){
      out[c] += f[c][x]*in[0][0][x];
    }
  }
}

void BinConv(int ci, int yi, int xi, int in[yi+2][xi+2][ci],
             int fci, int fyi, int fxi, int f[fci][ci*fyi*fxi],
             int out[yi][xi][fci])
{
  for(int c=0; c<fci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        out[y][x][c] = 0;
        for(int fc=0; fc<ci; fc++){
          for(int fy=0; fy<fyi; fy++){
            for(int fx=0; fx<fxi; fx++){
              out[y][x][c] += f[c][fy*fxi*ci+fx*ci+fc]*in[fy+y][fx+x][fc];
            }
          }
        }    
      }
    }
  }
}

void Conv(int ci, int yi, int xi, unsigned char in[yi+2][xi+2][ci],
          int fci, int fyi, int fxi, int f[fci][ci*fyi*fxi],
          int out[yi][xi][fci])
{
  for(int c=0; c<fci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        out[y][x][c] = 0;
        for(int fc=0; fc<ci; fc++){
          for(int fy=0; fy<fyi; fy++){
            for(int fx=0; fx<fxi; fx++){
              out[y][x][c] += f[c][fy*fxi*ci+fx*ci+fc]*(in[fy+y][fx+x][fc]*2-255);
            }
          }
        }    
      }
    }
  }
}

void BinPool(int ci, int yi, int xi, int in[yi][xi][ci],
             int pyi, int pxi, int out[yi/pyi][xi/pxi][ci])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi/pyi; y++){
      for(int x=0; x<xi/pxi; x++){
        out[y][x][c] = in[y*pyi][x*pxi][c];
        for(int py=0; py<pyi; py++){
          for(int px=0; px<pxi; px++){
            if(in[y*pyi+py][x*pxi+px][c]>out[y][x][c]){
              out[y][x][c] = in[y*pyi+py][x*pxi+px][c];
            }
          }
        }    
      }
    }
  }
}

void Pool(int ci, int yi, int xi, int in[yi][xi][ci],
          int pyi, int pxi, int out[yi/pyi][xi/pxi][ci])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi/pyi; y++){
      for(int x=0; x<xi/pxi; x++){
        out[y][x][c] = in[y*pyi][x*pxi][c];
        for(int py=0; py<pyi; py++){
          for(int px=0; px<pxi; px++){
            if(in[y*pyi+py][x*pxi+px][c]>out[y][x][c]){
              out[y][x][c] = in[y*pyi+py][x*pxi+px][c];
            }
          }
        }    
      }
    }
  }
}

void BinNorm(int ci, int yi, int xi, int in[yi][xi][ci],
             int mean[ci],
             int out[yi][xi][ci])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        //        out[c][y][x] = (in[c][y][x]-mean[c])/(sqrt(var[c]));
        if((in[y][x][c]>512)||(in[y][x][c]<-512)){printf("Overflow\n");}
        out[y][x][c] = (in[y][x][c]*64-mean[c]);
      }
    }
  }
}

void Norm(int ci, int yi, int xi, int in[yi][xi][ci],
          int mean[ci],
          int out[yi][xi][ci])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        //        out[y][x][c] = (in[y][x][c]-mean[c])/(sqrt(var[c]));
        out[y][x][c] = (in[y][x][c]-mean[c]);
      }
    }
  }
}

void BinActivF(int ci, int yi, int xi, int in[yi][xi][ci],
               int out[yi][xi][ci])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        if(in[y][x][c]>=0){
          out[y][x][c] = 1;
        } else {
          out[y][x][c] = -1;
        }
      }
    }
  }
}

void BinActiv(int ci, int yi, int xi,int in[yi][xi][ci],
              int out[yi][xi][ci])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        if(in[y][x][c]>=0){
          out[y][x][c] = 1;
        } else {
          out[y][x][c] = -1;
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
  unsigned char pict[1000][32+2][32+2][3];

  int conv1out[32][32][32];
  int pool1out[16][16][32];
  int norm1out[16][16][32];
  int activ1out[16][16][32];
  int layer2in[18][18][32];

  int conv2out[16][16][32];
  int pool2out[8][8][32];
  int norm2out[8][8][32];
  int activ2out[8][8][32];
  int layer3in[10][10][32];

  int conv3out[8][8][64];
  int pool3out[4][4][64];
  int norm3out[4][4][64];
  int activ3out[4][4][64];
  int layer4in[64*4*4];

  int affine4out[1][1][512];
  int norm4out[1][1][512];
  int activ4out[1][1][512];

  int affine5out[10];

  int pass = 0;
  for (int i=0;i<1000;i++){
    label[i] = fgetc(fp);
    for(int c=0; c<3; c++){
      for(int y=0; y<32+2; y++){
        for(int x=0; x<32+2; x++){
          if((x==0)|(x==33)|(y==0)|(y==33)){
            pict[i][y][x][c] = 0;//padding=1
          }else{
            pict[i][y][x][c] = fgetc(fp);
          }
        }
      }
    }    

    Conv(3,32,32,pict[i],32,3,3,W1,conv1out);
    Pool(32,32,32,conv1out,2,2,pool1out);
    Norm(32,16,16,pool1out,mean1,norm1out);
    BinActivF(32,16,16,norm1out,activ1out);
    for(int c=0; c<32; c++){
      for(int y=0; y<18; y++){
        for(int x=0; x<18; x++){
          if((x==0)|(x==17)|(y==0)|(y==17)){
            layer2in[y][x][c] = 1;//padding=1 //BNN
            //layer2in[y][x][c] = 0;//padding=1 //other
          }else{
            layer2in[y][x][c] = activ1out[y-1][x-1][c];
          }
        }
      }
    }
    BinConv(32,16,16,layer2in,32,3,3,W2,conv2out);
    BinPool(32,16,16,conv2out,2,2,pool2out);
    BinNorm(32,8,8,pool2out,mean2,norm2out);
    BinActiv(32,8,8,norm2out,activ2out);
    for(int c=0; c<32; c++){
      for(int y=0; y<10; y++){
        for(int x=0; x<10; x++){
          if((x==0)|(x==9)|(y==0)|(y==9)){
            layer3in[y][x][c] = 1;//padding=1 //BNN
            //layer3in[y][x][c] = 0;//padding=1 //other
          }else{
            layer3in[y][x][c] = activ2out[y-1][x-1][c];
          }
        }
      }
    }
    BinConv(32,8,8,layer3in,64,3,3,W3,conv3out);
    BinPool(64,8,8,conv3out,2,2,pool3out);
    BinNorm(64,4,4,pool3out,mean3,norm3out);
    BinActiv(64,4,4,norm3out,activ3out);
    for(int c=0; c<64; c++){
      for(int y=0; y<4; y++){
        for(int x=0; x<4; x++){
          layer4in[y*64*4+x*64+c] = activ3out[y][x][c];
        }
      }
    }
    BinAffine(1024,512,layer4in,W4,affine4out);
    BinNorm(512,1,1,affine4out,mean4,norm4out);
    BinActiv(512,1,1,norm4out,activ4out);

    Affine(512,10,activ4out,W5,affine5out);

    int result = 0;
    int max = affine5out[0];
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

  //int conv2out[16][16][32];
  //int pool2out[8][8][32];
  //int norm2out[8][8][32];
  //int activ2out[8][8][32];
  //int activ3out[4][4][64];
  //int activ4out[1][1][512];

  uint out;

  for(int y=0; y<8; y++){
    for(int x=0; x<8; x++){
      out = 0;
      for(int c=0; c<32; c++){
        if(activ2out[y][x][c]==-1){
          out |= 1<<c;
        }
      }
      printf("%08x, ",out);
    }
    printf("\n");
  }

  printf("\n");

  uint out0, out1;
  for(int y=0; y<4; y++){
    for(int x=0; x<4; x++){
      out0 = 0;
      out1 = 0;
      for(int c=0; c<32; c++){
        if(activ3out[y][x][c]==-1){
          out0 |= 1<<c;
        }
        if(activ3out[y][x][c+32]==-1){
          out1 |= 1<<c;
        }
      }
      printf("%08x, %08x, ",out0,out1);
    }
    printf("\n");
  }

  printf("\n");

  for(int i=0; i<16; i=i+1){
    out = 0;
    for(int c=0; c<32; c++){
      if(activ4out[0][0][i*32+c]==-1){
        out |= 1<<c;
      }
    }
    printf("%08x, ",out);
    if(i==7){printf("\n");}
  }
  printf("\n");
}
