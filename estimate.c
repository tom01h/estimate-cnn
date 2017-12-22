#include <stdio.h>

#include "paramb.h"

void BinAffine(int xi,int ci, signed char in[xi*32], int f[ci][xi], int mean[ci], signed char out[1][1][ci])
{
  int acc;
  for(int c=0; c<ci; c++){
    acc = 0;
    for(int x=0; x<xi; x++){
      for(int i=0; i<32; i++){
        if(f[c][x]&(1<<i)){
          acc -= in[x*32+i];
        }else{
          acc += in[x*32+i];
        }
      }
    }
    if((acc*64-mean[c])>=0){
      out[0][0][c] = 1;
    } else {
      out[0][0][c] = -1;
    }
  }
}

void Affine(int xi,int ci, signed char in[1][1][xi], int f[ci][xi], int out[ci])
{
  for(int c=0; c<ci; c++){
    out[c] = 0;
    for(int x=0; x<xi; x++){
      out[c] += f[c][x]*in[0][0][x];
    }
  }
}

void BinConv(int ci, int yi, int xi, signed char in[yi+2][xi+2][ci*32],
             int fci, int fyi, int fxi, int f[fci][ci*fyi*fxi], int mean[ci],
             int pad, signed char out[yi/2+pad][xi/2+pad][fci])
{
  int acc;
  int pool;
  for(int c=0; c<fci; c++){
    for(int y=0; y<yi; y+=2){
      for(int x=0; x<xi; x+=2){
        pool = 0x80000000;
        for(int yy=0; yy<2; yy++){
          for(int xx=0; xx<2; xx++){
            acc = 0;
            for(int fc=0; fc<ci; fc++){
              for(int fy=0; fy<fyi; fy++){
                for(int fx=0; fx<fxi; fx++){
                  for(int i=0; i<32; i++){
                    if(f[c][fy*fxi*ci+fx*ci+fc]&(1<<i)){
                      acc -= in[fy+(y+yy)][fx+(x+xx)][fc*32+i];
                    }else{
                      acc += in[fy+(y+yy)][fx+(x+xx)][fc*32+i];
                    }
                  }
                }
              }
            }
            if(acc>pool){
              pool=acc;
            }
          }
        }    
        if((pool*64-mean[c])>=0){
          out[(y+pad)/2][(x+pad)/2][c] = 1;
        } else {
          out[(y+pad)/2][(x+pad)/2][c] = -1;
        }
      }
    }
  }
}

void Conv(int ci, int yi, int xi, unsigned char in[yi+2][xi+2][ci],
          int fci, int fyi, int fxi, int f[fci][ci*fyi*fxi], int mean[ci],
          signed char out[yi/2+2][xi/2+2][fci])
{
  int acc;
  int pool;
  for(int c=0; c<fci; c++){
    for(int y=0; y<yi; y+=2){
      for(int x=0; x<xi; x+=2){
        pool = 0x80000000;
        for(int yy=0; yy<2; yy++){
          for(int xx=0; xx<2; xx++){
            acc = 0;
            for(int fc=0; fc<ci; fc++){
              for(int fy=0; fy<fyi; fy++){
                for(int fx=0; fx<fxi; fx++){
                  acc += f[c][fy*fxi*ci+fx*ci+fc]*(in[fy+(y+yy)][fx+(x+xx)][fc]*2-255);
                }
              }
            }
            if(acc>pool){
              pool=acc;
            }
          }
        }
        if((pool-mean[c])>=0){
          out[y/2+1][x/2+1][c] = 1;
        }else{
          out[y/2+1][x/2+1][c] = -1;
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
    //    exit(EXIT_FAILURE);
    return(1);
  }
  unsigned char label;
  unsigned char pict[32+2][32+2][3];

  signed char activ1out[16+2][16+2][32]; // +2=padding

  signed char activ2out[8+2][8+2][32]; // +2=padding

  signed char activ3out[4][4][64]; // +2=padding(DUMMY)
  signed char layer4in[64*4*4];

  signed char activ4out[1][1][512];

  int affine5out[10];

  // padding
  for(int c=0; c<3; c++){
    for(int y=0; y<32+2; y++){
      for(int x=0; x<32+2; x++){
        if((x==0)|(x==33)|(y==0)|(y==33)){
          pict[y][x][c] = 0;
        }
      }
    }
  }
  for(int c=0; c<32; c++){
    for(int y=17; y>=0; y--){
      for(int x=17; x>=0; x--){
        if((x==0)|(x==17)|(y==0)|(y==17)){
          activ1out[y][x][c] = 1;
        }
      }
    }
  }
  for(int c=0; c<32; c++){
    for(int y=9; y>=0; y--){
      for(int x=9; x>=0; x--){
        if((x==0)|(x==9)|(y==0)|(y==9)){
          activ2out[y][x][c] = 1;
        }
      }
    }
  }

  // main loop
  int pass = 0;
  for (int i=0;i<1000;i++){
    label = fgetc(fp);
    for(int c=0; c<3; c++){
      for(int y=1; y<32+1; y++){
        for(int x=1; x<32+1; x++){
          pict[y][x][c] = fgetc(fp);
        }
      }
    }

    Conv(3,32,32,pict,32,3,3,W1,mean1,activ1out);
    BinConv(32/32,16,16,activ1out,32,3,3,W2,mean2,2,activ2out);
    BinConv(32/32,8,8,activ2out,64,3,3,W3,mean3,0,activ3out);
    for(int c=0; c<64; c++){
      for(int y=0; y<4; y++){
        for(int x=0; x<4; x++){
          layer4in[y*64*4+x*64+c] = activ3out[y][x][c];
        }
      }
    }
    BinAffine(1024/32,512,layer4in,W4,mean4,activ4out);

    Affine(512,10,activ4out,W5,affine5out);

    int result = 0;
    int max = affine5out[0];
    for(int x=0; x<10; x++){
      if(affine5out[x]>max){
        max = affine5out[x];
        result = x;
      }
    }
    if(result==label){
      pass++;
    }else{
      printf ("No. %02d / Answer : %02d != Result : %02d\n",i,label,result);
    }
  }
  printf ("== Pass Count : %04d ==\n",pass);
}
