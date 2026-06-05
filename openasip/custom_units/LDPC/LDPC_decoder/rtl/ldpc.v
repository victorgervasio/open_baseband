`include "cn_core.v"

module ldpc
#(
    parameter WIDTH = 8
)
(
    input clk,
    input rst_n,
    input signed [WIDTH-1:0] in_llr_group_0,
    input signed [WIDTH-1:0] in_llr_group_1,
    input signed [WIDTH-1:0] in_llr_group_2,
    output signed [WIDTH-1:0] out_llr
);
    
    wire [18*WIDTH-1:0] in_llr_group_0_w;
    wire [18*WIDTH-1:0] in_llr_group_1_w;
    wire [18*WIDTH-1:0] in_llr_group_2_w;
    wire [WIDTH-1:0] out_llr_w;

    reg [18*WIDTH-1:0] in_llr_group_0_r;
    reg [18*WIDTH-1:0] in_llr_group_1_r;
    reg [18*WIDTH-1:0] in_llr_group_2_r;
   
    assign in_llr_group_0_w = in_llr_group_0_r; 
    assign in_llr_group_1_w = in_llr_group_1_r; 
    assign in_llr_group_2_w = in_llr_group_2_r; 
    assign out_llr = out_llr_w;
    
    integer i;
    always @(posedge clk or negedge rst_n) begin : in_llr_buffer
        if (!rst_n) begin
            i <= 0;
            in_llr_group_0_r = {(18*WIDTH){1'bx}};
            in_llr_group_1_r = {(18*WIDTH){1'bx}};
            in_llr_group_2_r = {(18*WIDTH){1'bx}};
        end else begin
            i <= i + 1;
            in_llr_group_0_r[(i+1)*WIDTH-1 -: WIDTH] = in_llr_group_0;
            in_llr_group_1_r[(i+1)*WIDTH-1 -: WIDTH] = in_llr_group_1;
            in_llr_group_2_r[(i+1)*WIDTH-1 -: WIDTH] = in_llr_group_2;
        end
    end

    cn_core 
    #(
        .DEGREE(19),
        .WIDTH(WIDTH),

        .sel_mux_cn_proc_3(2'b11),
        .sel_mux_cn_proc_4(2'b11),
        .sel_mux_cn_proc_5(2'b11),
        .sel_mux_cn_proc_6(2'b11),
        .sel_mux_cn_proc_7(2'b11),
        .sel_mux_cn_proc_8(2'b11),
        .sel_mux_cn_proc_9(2'b11),
        .sel_mux_cn_proc_10(2'b11),
        .sel_mux_cn_proc_19(2'b00),
    
        .sel_demux_min_0(4'b1000),
        .sel_demux_min_1(4'b1111),
        .sel_demux_min_2(4'b1111),
    
        .sel_demux_mul_0(4'b1000),
        .sel_demux_mul_1(4'b1111),
        .sel_demux_mul_2(4'b1111),
    
        .sel_demux_min_30(1'b0),
        .sel_demux_min_31(1'b0),
    
        .sel_demux_mul_30(1'b0),
        .sel_demux_mul_31(1'b0)
    ) cn_core_19 (
        .in_llr_group_0(in_llr_group_0_w),
        .in_llr_group_1(in_llr_group_1_w),
        .in_llr_group_2(in_llr_group_2_w),
        .out_llr(out_llr_w)
    );

endmodule
