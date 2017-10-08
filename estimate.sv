module estimate
  (
   input wire        clk,
   input wire [2:0]  com,
   input wire [15:0] addr,
   input wire [31:0] data,
   output wire       activ
   );
   
`include "param.v"

   parameter w2b=0*32;
   parameter w3b=9*32;
   parameter w4b=27*32;
   parameter m2b=539*32;
   parameter m3b=540*32;
   parameter m4b=542*32;

   reg [31:0]         param;
   reg signed [15:0]  acc;
   reg signed [15:0]  pool;
         
   integer            i;

   reg [15:0]         tmp;

// input stage
//   always_ff @ (posedge clk)begin
//TEMP//TEMP// <=
   always @ (posedge clk)begin
      case(com)
        3'd0 : begin //ini
           tmp[15:0] = pool;
           acc[15:0] = data[15:0];
//           acc[15:0] = -288;
           pool[15:0] = 16'h8000;
        end
        3'd1 : begin //acc
           if(addr<w3b)
             param[31:0] = W2[(addr-w2b)/9][(addr-w2b)%9];
           else if(addr<w4b)
             param[31:0] = W3[(addr-w3b)/9][(addr-w3b)%9];
           else
             param[31:0] = W4[(addr-w4b)/32][(addr-w4b)%32];
           for(i=0; i<32;i=i+1)begin
              acc = acc + {1'b0,~(data[i]^param[i]),1'b0};
           end
        end
        3'd2 : begin //pool
           if(acc>pool)begin
              pool[15:0] = acc[15:0];
           end
           acc[15:0] = data[15:0];
//           acc[15:0] = -288;
        end
        3'd3 : begin //norm
           if(addr<m3b)
             param[31:0] = mean2[addr-m2b];
           else if(addr<m4b)
             param[31:0] = mean3[addr-m3b];
           else
             param[31:0] = mean4[addr-m4b];
           pool[15:0] = {pool[15:0],6'h00} - param[15:0];
        end
        3'd4 : begin //activ
           activ = pool[15];
        end
      endcase
   end

// 1st stage
// 2nd stage
endmodule
