#include "unistd.h"
#include "getopt.h"
#include "Vestimate.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define VCD_PATH_LENGTH 256

#include "paramb.h"

vluint64_t eval(vluint64_t main_time, Vestimate* verilator_top, VerilatedVcdC* tfp)
{
  verilator_top->clk = 0;
  verilator_top->eval();
  tfp->dump(main_time);

  verilator_top->clk = 1;
  verilator_top->eval();
  tfp->dump(main_time+50);

  return main_time + 100;
}

void Conv(int ci, int yi, int xi, unsigned char in[32+2][32+2][3],
          int fci, int fyi, int fxi, float f[32][3*3*3],
          float out[32][32][32])
{
  for(int c=0; c<fci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        out[y][x][c] = 0;
        for(int fc=0; fc<ci; fc++){
          for(int fy=0; fy<fyi; fy++){
            for(int fx=0; fx<fxi; fx++){
              out[y][x][c] += f[c][fy*fxi*ci+fx*ci+fc]*(in[fy+y][fx+x][fc]/255.0f);
            }
          }
        }    
      }
    }
  }
}
void Pool(int ci, int yi, int xi, float in[32][32][32],
          int pyi, int pxi, float out[16][16][32])
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
void Norm(int ci, int yi, int xi, float in[16][16][32],
          float mean[32], float var[32],
          float out[16][16][32])
{
  for(int c=0; c<ci; c++){
    for(int y=0; y<yi; y++){
      for(int x=0; x<xi; x++){
        out[y][x][c] = (in[y][x][c]-mean[c])/(sqrt(var[c]));
      }
    }
  }
}
void BinActivF(int ci, int yi, int xi,float in[16][16][32],
               int out[16][16][32])
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

int main(int argc, char **argv, char **env) {
  
  int w2b=0*32;
  int w3b=9*32;
  int w4b=1*32;
  int m2b=530*32;
  int m3b=531*32;
  int m4b=532*32;

  FILE *fp;
  if((fp = fopen("cifar10-test", "rb")) == NULL ) {
    fprintf(stderr,"There is no cifar10-test\n");
    exit(EXIT_FAILURE);
  }
  unsigned char label[1000];
  unsigned char pict[1000][32+2][32+2][3];

  float conv1out[32][32][32];
  float pool1out[16][16][32];
  float norm1out[16][16][32];
  int activ1out[16][16][32];

  uint activ1bin[16][16];

  int activ2out[8][8][32];
  uint activ2bin[8][8];

  int i, nloop;
  char vcdfile[VCD_PATH_LENGTH];

  strncpy(vcdfile,"tmp.vcd",VCD_PATH_LENGTH);
  srand((unsigned)time(NULL));
  i=0;

  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);
  VerilatedVcdC* tfp = new VerilatedVcdC;
  Vestimate* verilator_top = new Vestimate;
  verilator_top->trace(tfp, 99); // requires explicit max levels param
  tfp->open(vcdfile);
  vluint64_t main_time = 0;
  //  while (!Verilated::gotFinish()) {
  nloop = 1;
  for (int i=0;i<nloop;i++){
    // �摜����
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
    Norm(32,16,16,pool1out,mean1,var1,norm1out);
    BinActivF(32,16,16,norm1out,activ1out);
    for(int y=0; y<16; y++){
      for(int x=0; x<16; x++){
        activ1bin[y][x]=0;
        for(int c=0; c<32; c=c+1){
          if(activ1out[y][x][c]==-1){
            activ1bin[y][x]|=1<<c;
          }
        }
      }
    }

    for(int y=0; y<8; y=y+1){//layer2
      for(int x=0; x<8; x=x+1){
        for(int c=0; c<32; c=c+1){

          verilator_top->com=0;//ini
          main_time = eval(main_time, verilator_top, tfp);

          for(int p=0; p<4; p++){
            int py=p/2+y*2;
            int px=p%2+x*2;
            for(int fy=0; fy<3; fy=fy+1){
              for(int fx=0; fx<3; fx=fx+1){
                verilator_top->com=1;//acc
                verilator_top->addr=w2b*32+c*9+fy*3+fx;
                verilator_top->x=px+fx;//temp
                verilator_top->y=py+fy;//temp
                if(((py+fy)==0)|((py+fy)==17)|((px+fx)==0)|((px+fx)==17)){
                  verilator_top->data=0;
                }else{
                  verilator_top->data=activ1bin[py+fy-1][px+fx-1];
                }

                main_time = eval(main_time, verilator_top, tfp);
              }
            }

            verilator_top->com=2;//pool
            main_time = eval(main_time, verilator_top, tfp);
          }//pool

          verilator_top->com=3;//norm
          verilator_top->addr=m2b+c;
          main_time = eval(main_time, verilator_top, tfp);

          verilator_top->com=4;//activ
          main_time = eval(main_time, verilator_top, tfp);
          activ2out[y][x][c]=verilator_top->activ;
        }
      }
    }//layer2
    for(int y=0; y<8; y++){
      for(int x=0; x<8; x++){
        activ2bin[y][x]=0;
        for(int c=0; c<32; c=c+1){
          activ2bin[y][x]|=activ2out[y][x][c]<<c;
        }
      }
    }

  }
  delete verilator_top;
  tfp->close();

  for(int y=0; y<8; y++){
    for(int x=0; x<8; x++){
      printf("%08x, ",activ2bin[y][x]);
    }
    printf("\n");
  }
  
  exit(0);
}
