module estimate
  (
   input wire         clk,
   input wire [2:0]   com,
   input wire [15:0]  addr,
   input wire [63:0]  data,
   output wire [63:0] activ
   );
   
   reg [1023:0]       param;
         
   ram0 ram0 (clk, addr, param);

   reg [2:0]          com_1;
   reg [63:0]         data_1;
// input stage
   always_ff @(posedge clk)begin
      com_1[2:0] <= com;
      data_1[63:0] <= data;
   end

   genvar               g;
   generate begin
      for(g=0;g<32;g=g+1) begin : estimate_block
         estimate_core core
            (.clk(clk), .com_1(com_1[2:0]), .data_1(data_1[63:0]),
             .param(param[32*(31-g)+31:32*(31-g)+0]),
             .activ(activ[2*g+1:2*g])
             );
      end : estimate_block
   end
   endgenerate

endmodule

module estimate_core
  (
   input wire        clk,
   input wire [2:0]  com_1,
   input wire [63:0] data_1,
   input wire [31:0] param,
   output reg [1:0]  activ
   );

   integer           i;

   reg signed [15:0] acc;
   reg signed [15:0] pool;

   reg [2:0]         com_2;
   reg [63:0]        data_2;
// 1st stage
   reg [63:0]        lparam;

   always_comb @(param)begin
      for(i=0; i<32; i=i+1)begin
         lparam[2*i+1] = param[i];
         lparam[2*i]   = param[i];
      end
   end

   always_ff @(posedge clk)begin
      com_2[2:0] <= com_1;
      case(com_1)
        3'd0 : begin //ini
           data_2 <= data_1;
        end
        3'd1 : begin //acc
           data_2 <= ~(data_1^lparam);
        end
        3'd2 : begin //pool
           data_2 <= data_1;
        end
        3'd3 : begin //norm
           data_2 <= param;
        end
      endcase
   end
// 2nd stage
   reg [15:0] sum;
   always_comb @(data_2)begin
      sum = 0;
      for(i=0; i<32;i=i+1)begin
         sum = sum + {1'b0,data_2[2*i+1],data_2[2*i],1'b0};
      end
   end
   always_ff @(posedge clk)begin
      case(com_2)
        3'd0 : begin //ini
           acc[15:0] <= data_2[15:0];
           pool[15:0] <= 16'h8000;
        end
        3'd1 : begin //acc
           acc <= acc + sum;
        end
        3'd2 : begin //pool
           if($signed(acc)-$signed(pool)>=0)begin
              pool[15:0] <= acc[15:0];
           end
           acc[15:0] <= data_2[15:0];
        end
        3'd3 : begin //norm
           pool[15:0] <= {pool[15:0],5'h00} - data_2[15:0];
        end
        3'd4 : begin //activ
           activ[1] <= pool[15];
           if(~pool[15])begin
              if(($signed(pool)-$signed(data_2[31:16]))<0)begin
                 activ[0] <= 1'b1;
              end else begin
                 activ[0] <= 1'b0;
              end
           end else begin
              if(($signed(pool)+$signed(data_2[31:16]))<0)begin
                 activ[0] <= 1'b1;
              end else begin
                 activ[0] <= 1'b0;
              end
           end
        end
      endcase
   end
endmodule

module ram0 (clk, addr, dout);
   input clk;
   input [15:0] addr;
   output [1023:0] dout;
`include "param0"
   reg [1024:0]    dout;

   always @(posedge clk)
     begin
        dout <= mem[addr];
     end
endmodule
