module estimate
  (
   input wire        clk,
   input wire [2:0]  com,
   input wire [15:0] addr,
   input wire [31:0] data,
   output wire       activ
   );
   
   reg [31:0]         param;
   reg signed [15:0]  acc;
   reg signed [15:0]  pool;
         
   integer            i;

   ram0 ram0 (clk, addr, param);

   reg [2:0]          com_1;
   reg [31:0]         data_1;
// input stage
   always_ff @(posedge clk)begin
      com_1[2:0] <= com;
      data_1[31:0] <= data;
   end

   reg [2:0]          com_2;
   reg [31:0]         data_2;
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
           if(acc>pool)begin
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
   output [31:0] dout;
`include "param0"
   reg [31:0]    dout;

   always @(posedge clk)
     begin
        dout <= mem[addr];
     end
endmodule
