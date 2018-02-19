module estimate
  (
   input wire         clk,
   input wire [2:0]   com,
   input wire [15:0]  addr,
   input wire [31:0]  data,
   output wire [31:0] activ
   );
   
   reg [1023:0]       param;

   ram0 ram0 (clk, addr, param);

   reg [2:0]          com_1;
   reg [31:0]         data_1;
   // input stage
   always_ff @(posedge clk)begin
      com_1[2:0] <= com;
      data_1[31:0] <= data;
   end

   genvar               g;
   generate begin
      for(g=0;g<32;g=g+1) begin : estimate_block
         estimate_core core
            (.clk(clk), .com_1(com_1[2:0]), .data_1(data_1[31:0]),
             .param(param[32*(31-g)+31:32*(31-g)+0]),
             .activ(activ[g])
             );
      end : estimate_block
   end
   endgenerate

endmodule

module estimate_core
  (
   input wire        clk,
   input wire [2:0]  com_1,
   input wire [31:0] data_1,
   input wire [31:0] param,
   output reg        activ
   );

   integer           i;

   reg signed [15:0] acc;
   reg signed [15:0] pool;

   reg [2:0]         com_2;
   reg [31:0]        data_2;
// 1st stage

   always_ff @(posedge clk)begin
      com_2[2:0] <= com_1;
      case(com_1)
        3'd0 : begin //ini
           data_2 <= data_1;
        end
        3'd1 : begin //acc
           data_2 <= ~(data_1^param);
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
         sum = sum + {1'b0,data_2[i],1'b0};
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
           pool[15:0] <= {pool[15:0],6'h00} - data_2[15:0];
        end
        3'd4 : begin //activ
           activ <= pool[15];
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
