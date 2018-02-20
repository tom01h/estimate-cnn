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
  //tfp->dump(main_time);

  verilator_top->clk = 1;
  verilator_top->eval();
  //tfp->dump(main_time+50);

  return main_time + 100;
}

void Affine(int xi,int ci, uint in[16], int f[10][512], int out[10])
{
  for(int c=0; c<ci; c++){
    out[c] = 0;
    for(int x=0; x<xi; x++){
      if(in[x/32]&(1<<(x%32))){//-1
        out[c] -= f[c][x];
      }else{
        out[c] += f[c][x];
      }
    }
  }
}

int main(int argc, char **argv, char **env) {
  
  int w1b=0;
  int w2b=9;
  int w3b=18;
  int w4b=36;
  int m1b=548;
  int m2b=549;
  int m3b=550;
  int m4b=552;

  FILE *fp;
  if((fp = fopen("cifar10-test", "rb")) == NULL ) {
    fprintf(stderr,"There is no cifar10-test\n");
    exit(EXIT_FAILURE);
  }
  unsigned char label;
  unsigned char pict[32+2][32+2][3];

  uint activ1bin[18][18];

  uint activ2bin[8][8];

  uint activ3bin[4][4][2];

  uint activ4bin[16];

  int affine5out[10];

  int pass = 0;

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
  nloop = 1000;
  for (int i=0;i<nloop;i++){
    // load image
    label = fgetc(fp);
    for(int c=0; c<3; c++){
      for(int y=0; y<32+2; y++){
        for(int x=0; x<32+2; x++){
          if((x==0)|(x==33)|(y==0)|(y==33)){
            pict[y][x][c] = 0;//padding=1
          }else{
            pict[y][x][c] = fgetc(fp);
          }
        }
      }
    }    
    // 1st layer
    //Conv(3,32,32,pict[i],32/32,3,3,W1,mean1,activ1bin);
    for(int y=0; y<32; y+=2){
      for(int x=0; x<32; x+=2){
        verilator_top->com=0;//ini
        verilator_top->data=0;
        main_time = eval(main_time, verilator_top, tfp);

        for(int yy=0; yy<2; yy++){
          for(int xx=0; xx<2; xx++){

            for(int fy=0; fy<3; fy++){
              for(int fx=0; fx<3; fx++){
                int datai;
                verilator_top->com=5;//acc8
                verilator_top->addr=w1b+fy*3+fx;
                datai  = (((pict[fy+(y+yy)][fx+(x+xx)][0]*2-255)/2)&0x0ff)<<24;
                datai |= (((pict[fy+(y+yy)][fx+(x+xx)][1]*2-255)/2)&0x0ff)<<16;
                datai |= (((pict[fy+(y+yy)][fx+(x+xx)][2]*2-255)/2)&0x0ff)<<8;
                verilator_top->data=datai;

                main_time = eval(main_time, verilator_top, tfp);

                //acc += f[c*32+cc][fy*fxi*ci+fx*ci+fc]*(in[fy+(y+yy)][fx+(x+xx)][fc]*2-255)/4;
              }
            }
            verilator_top->com=2;//pool
            verilator_top->data=0;
            main_time = eval(main_time, verilator_top, tfp);
          }
        }
        verilator_top->com=6;//norm8
        verilator_top->addr=m1b;
        main_time = eval(main_time, verilator_top, tfp);

        verilator_top->com=4;//activ
        main_time = eval(main_time, verilator_top, tfp);
        main_time = eval(main_time, verilator_top, tfp);
        main_time = eval(main_time, verilator_top, tfp);
        activ1bin[y/2+1][x/2+1]=verilator_top->activ;
      }
    }

    // 2nd layer
    for(int y=0; y<8; y=y+1){
      for(int x=0; x<8; x=x+1){
        for(int c=0; c<32/32; c=c+1){

          verilator_top->com=0;//ini
          verilator_top->data=-288;
          main_time = eval(main_time, verilator_top, tfp);

          for(int p=0; p<4; p++){
            int py=p/2+y*2;
            int px=p%2+x*2;
            for(int fy=0; fy<3; fy=fy+1){
              for(int fx=0; fx<3; fx=fx+1){
                verilator_top->com=1;//acc
                verilator_top->addr=w2b+c*9+fy*3+fx;
                verilator_top->data=activ1bin[py+fy][px+fx];

                main_time = eval(main_time, verilator_top, tfp);
              }
            }

            verilator_top->com=2;//pool
            verilator_top->data=-288;
            main_time = eval(main_time, verilator_top, tfp);
          }//pool

          verilator_top->com=3;//norm
          verilator_top->addr=m2b+c;
          main_time = eval(main_time, verilator_top, tfp);

          verilator_top->com=4;//activ
          main_time = eval(main_time, verilator_top, tfp);
          main_time = eval(main_time, verilator_top, tfp);
          main_time = eval(main_time, verilator_top, tfp);
          activ2bin[y][x]=verilator_top->activ;
        }
      }
    }// 2nd layer
    // 3rd layer
    for(int y=0; y<4; y=y+1){
      for(int x=0; x<4; x=x+1){
        for(int c=0; c<64/32; c=c+1){

          verilator_top->com=0;//ini
          verilator_top->data=-288;
          main_time = eval(main_time, verilator_top, tfp);

          for(int p=0; p<4; p++){
            int py=p/2+y*2;
            int px=p%2+x*2;
            for(int fy=0; fy<3; fy=fy+1){
              for(int fx=0; fx<3; fx=fx+1){
                verilator_top->com=1;//acc
                verilator_top->addr=w3b+c*9+fy*3+fx;
                if(((py+fy)==0)|((py+fy)==9)|((px+fx)==0)|((px+fx)==9)){
                  verilator_top->data=0;
                }else{
                  verilator_top->data=activ2bin[py+fy-1][px+fx-1];
                }

                main_time = eval(main_time, verilator_top, tfp);
              }
            }

            verilator_top->com=2;//pool
            verilator_top->data=-288;
            main_time = eval(main_time, verilator_top, tfp);
          }//pool

          verilator_top->com=3;//norm
          verilator_top->addr=m3b+c;
          main_time = eval(main_time, verilator_top, tfp);

          verilator_top->com=4;//activ
          main_time = eval(main_time, verilator_top, tfp);
          main_time = eval(main_time, verilator_top, tfp);
          main_time = eval(main_time, verilator_top, tfp);
          activ3bin[y][x][c]=verilator_top->activ;
        }
      }
    }// 3rd layer
    // 4th layer
    for(int c=0; c<512/32; c=c+1){
      verilator_top->com=0;//ini
      verilator_top->data=-1024;
      main_time = eval(main_time, verilator_top, tfp);
      for(int y=0; y<4; y=y+1){
        for(int x=0; x<4; x=x+1){
          for(int i=0; i<2; i=i+1){
            verilator_top->com=1;//acc
            verilator_top->addr=w4b+c*4*4*2+y*4*2+x*2+i;
            verilator_top->data=activ3bin[y][x][i];
            main_time = eval(main_time, verilator_top, tfp);
          }
        }
      }
      verilator_top->com=2;//pool
      verilator_top->data=-1024;
      main_time = eval(main_time, verilator_top, tfp);
      verilator_top->com=3;//norm
      verilator_top->addr=m4b+c;
      main_time = eval(main_time, verilator_top, tfp);

      verilator_top->com=4;//activ
      main_time = eval(main_time, verilator_top, tfp);
      main_time = eval(main_time, verilator_top, tfp);
      main_time = eval(main_time, verilator_top, tfp);
      activ4bin[c]=verilator_top->activ;
    }// 4th layer

    Affine(512,10,activ4bin,W5,affine5out);

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
  delete verilator_top;
  tfp->close();

  printf ("== Pass Count : %04d ==\n",pass);

  exit(0);
}
