`include "defines.vh"
`include "cn_proc.v"

module min_2x1
#(
    parameter WIDTH = 8
)
(
    input signed [WIDTH-1:0] in0,
    input signed [WIDTH-1:0] in1,
    output signed [WIDTH-1:0] out
);
    reg [WIDTH-1:0] out_r;    
    always @(*) begin : calculate_min
        if (in0 < in1)
            out_r = in0;
        else
            out_r = in1;
    end

    assign out = out_r;
endmodule

module mul_2x1
#(
    parameter WIDTH = 8
)
(
    input signed [WIDTH-1:0] in0,
    input signed [WIDTH-1:0] in1,
    output signed [WIDTH-1:0] out
);
    reg [WIDTH-1:0] out_r;    
    always @(*) begin : calculate_mul
        out_r = in0 * in1;
    end

    assign out = out_r;
endmodule

module cn_core
#(
    parameter DEGREE = 19,
    parameter WIDTH = 8,

    parameter [1:0] sel_mux_cn_proc_3 = 2'b11,
    parameter [1:0] sel_mux_cn_proc_4 = 2'b11,
    parameter [1:0] sel_mux_cn_proc_5 = 2'b11,
    parameter [1:0] sel_mux_cn_proc_6 = 2'b11,
    parameter [1:0] sel_mux_cn_proc_7 = 2'b11,
    parameter [1:0] sel_mux_cn_proc_8 = 2'b11,
    parameter [1:0] sel_mux_cn_proc_9 = 2'b11,
    parameter [1:0] sel_mux_cn_proc_10 = 2'b11,
    parameter [1:0] sel_mux_cn_proc_19 = 2'b00,

    parameter [3:0] sel_demux_min_0 = 4'b1000,
    parameter [3:0] sel_demux_min_1 = 4'b1111,
    parameter [3:0] sel_demux_min_2 = 4'b1111,

    parameter [3:0] sel_demux_mul_0 = 4'b1000,
    parameter [3:0] sel_demux_mul_1 = 4'b1111,
    parameter [3:0] sel_demux_mul_2 = 4'b1111,

    parameter sel_demux_min_30 = 1'b0,
    parameter sel_demux_min_31 = 1'b0,

    parameter sel_demux_mul_30 = 1'b0,
    parameter sel_demux_mul_31 = 1'b0
)
(
    input signed [(DEGREE-1)*WIDTH-1:0] in_llr_group_0,
    input signed [(DEGREE-1)*WIDTH-1:0] in_llr_group_1,
    input signed [(DEGREE-1)*WIDTH-1:0] in_llr_group_2,
    output signed [WIDTH-1:0] out_llr

);
    
    wire [(DEGREE-1)*WIDTH-1:0] in_llr_group [0:2];
    assign in_llr_group[0] = in_llr_group_0;
    assign in_llr_group[1] = in_llr_group_1;
    assign in_llr_group[2] = in_llr_group_2;

    genvar i;
    generate
        case (DEGREE)
            4:
            begin : gen_bus_4
                for (i = 0; i < `MAX_LEVEL_CN_PROC_AGGREGATION; i = i + 1) begin : bus_gen_4
                    wire [2*WIDTH-1:0] in_cn_proc_3;
                    wire [3*WIDTH-1:0] in_cn_proc_4;
                    assign in_cn_proc_3 = in_llr_group[i][2*WIDTH-1:0];
                    assign in_cn_proc_4 = in_llr_group[i];
                end
            end
            5:
            begin : gen_bus_5
                for (i = 0; i < `MAX_LEVEL_CN_PROC_AGGREGATION; i = i + 1) begin : bus_gen_5
                    wire [2*WIDTH-1:0] in_cn_proc_3;
                    wire [3*WIDTH-1:0] in_cn_proc_4;
                    wire [4*WIDTH-1:0] in_cn_proc_5;
                    assign in_cn_proc_3 = in_llr_group[i][2*WIDTH-1:0];
                    assign in_cn_proc_4 = in_llr_group[i][3*WIDTH-1:0];
                    assign in_cn_proc_5 = in_llr_group[i];
                end
            end
            6:
            begin : gen_bus_6
                for (i = 0; i < `MAX_LEVEL_CN_PROC_AGGREGATION; i = i + 1) begin : bus_gen_6
                    wire [2*WIDTH-1:0] in_cn_proc_3;
                    wire [3*WIDTH-1:0] in_cn_proc_4;
                    wire [4*WIDTH-1:0] in_cn_proc_5;
                    wire [5*WIDTH-1:0] in_cn_proc_6;
                    assign in_cn_proc_3 = in_llr_group[i][2*WIDTH-1:0];
                    assign in_cn_proc_4 = in_llr_group[i][3*WIDTH-1:0];
                    assign in_cn_proc_5 = in_llr_group[i][4*WIDTH-1:0];
                    assign in_cn_proc_6 = in_llr_group[i];
                end
            end
            7:
            begin : gen_bus_7
                for (i = 0; i < `MAX_LEVEL_CN_PROC_AGGREGATION; i = i + 1) begin : bus_gen_7
                    wire [2*WIDTH-1:0] in_cn_proc_3;
                    wire [3*WIDTH-1:0] in_cn_proc_4;
                    wire [4*WIDTH-1:0] in_cn_proc_5;
                    wire [5*WIDTH-1:0] in_cn_proc_6;
                    wire [6*WIDTH-1:0] in_cn_proc_7;
                    assign in_cn_proc_3 = in_llr_group[i][2*WIDTH-1:0];
                    assign in_cn_proc_4 = in_llr_group[i][3*WIDTH-1:0];
                    assign in_cn_proc_5 = in_llr_group[i][4*WIDTH-1:0];
                    assign in_cn_proc_6 = in_llr_group[i][5*WIDTH-1:0];
                    assign in_cn_proc_7 = in_llr_group[i];
                end
            end
            19:
            begin : gen_bus_19
                for (i = 0; i < `MAX_LEVEL_CN_PROC_AGGREGATION; i = i + 1) begin : bus_gen_19
                    wire [2*WIDTH-1:0] in_cn_proc_3;
                    wire [3*WIDTH-1:0] in_cn_proc_4;
                    wire [4*WIDTH-1:0] in_cn_proc_5;
                    wire [5*WIDTH-1:0] in_cn_proc_6;
                    wire [6*WIDTH-1:0] in_cn_proc_7;
                    wire [7*WIDTH-1:0] in_cn_proc_8;
                    wire [8*WIDTH-1:0] in_cn_proc_9;
                    wire [9*WIDTH-1:0] in_cn_proc_10;
                    wire [18*WIDTH-1:0] in_cn_proc_19;
                    assign in_cn_proc_3 = in_llr_group[i][2*WIDTH-1:0];
                    assign in_cn_proc_4 = in_llr_group[i][3*WIDTH-1:0];
                    assign in_cn_proc_5 = in_llr_group[i][4*WIDTH-1:0];
                    assign in_cn_proc_6 = in_llr_group[i][5*WIDTH-1:0];
                    assign in_cn_proc_7 = in_llr_group[i][6*WIDTH-1:0];
                    assign in_cn_proc_8 = in_llr_group[i][7*WIDTH-1:0];
                    assign in_cn_proc_9 = in_llr_group[i][8*WIDTH-1:0];
                    assign in_cn_proc_10 = in_llr_group[i][9*WIDTH-1:0];
                    assign in_cn_proc_19 = in_llr_group[i];
                end
            end
        endcase
    endgenerate
    

    generate
        case (DEGREE)
            4:
            begin : gen_cn_buses_4
                wire [2*WIDTH-1:0] in_cn_proc_3;
                reg [2*WIDTH-1:0] in_cn_proc_3_r;
                wire [WIDTH-1:0] min_cn_proc_3;
                wire [WIDTH-1:0] sgn_mul_cn_proc_3;

                wire [3*WIDTH-1:0] in_cn_proc_4;
                reg [3*WIDTH-1:0] in_cn_proc_4_r;
                wire [WIDTH-1:0] min_cn_proc_4;
                wire [WIDTH-1:0] sgn_mul_cn_proc_4;
            end
            5:
            begin : gen_cn_buses_5
                wire [2*WIDTH-1:0] in_cn_proc_3;
                reg [2*WIDTH-1:0] in_cn_proc_3_r;
                wire [WIDTH-1:0] min_cn_proc_3;
                wire [WIDTH-1:0] sgn_mul_cn_proc_3;

                wire [3*WIDTH-1:0] in_cn_proc_4;
                reg [3*WIDTH-1:0] in_cn_proc_4_r;
                wire [WIDTH-1:0] min_cn_proc_4;
                wire [WIDTH-1:0] sgn_mul_cn_proc_4;

                wire [4*WIDTH-1:0] in_cn_proc_5;
                reg [4*WIDTH-1:0] in_cn_proc_5_r;
                wire [WIDTH-1:0] min_cn_proc_5;
                wire [WIDTH-1:0] sgn_mul_cn_proc_5;
            end
            6:
            begin : gen_cn_buses_6
                wire [2*WIDTH-1:0] in_cn_proc_3;
                reg [2*WIDTH-1:0] in_cn_proc_3_r;
                wire [WIDTH-1:0] min_cn_proc_3;
                wire [WIDTH-1:0] sgn_mul_cn_proc_3;

                wire [3*WIDTH-1:0] in_cn_proc_4;
                reg [3*WIDTH-1:0] in_cn_proc_4_r;
                wire [WIDTH-1:0] min_cn_proc_4;
                wire [WIDTH-1:0] sgn_mul_cn_proc_4;

                wire [4*WIDTH-1:0] in_cn_proc_5;
                reg [4*WIDTH-1:0] in_cn_proc_5_r;
                wire [WIDTH-1:0] min_cn_proc_5;
                wire [WIDTH-1:0] sgn_mul_cn_proc_5;

                wire [5*WIDTH-1:0] in_cn_proc_6;
                reg [5*WIDTH-1:0] in_cn_proc_6_r;
                wire [WIDTH-1:0] min_cn_proc_6;
                wire [WIDTH-1:0] sgn_mul_cn_proc_6;
            end
            7:
            begin : gen_cn_buses_7
                wire [2*WIDTH-1:0] in_cn_proc_3;
                reg [2*WIDTH-1:0] in_cn_proc_3_r;
                wire [WIDTH-1:0] min_cn_proc_3;
                wire [WIDTH-1:0] sgn_mul_cn_proc_3;

                wire [3*WIDTH-1:0] in_cn_proc_4;
                reg [3*WIDTH-1:0] in_cn_proc_4_r;
                wire [WIDTH-1:0] min_cn_proc_4;
                wire [WIDTH-1:0] sgn_mul_cn_proc_4;

                wire [4*WIDTH-1:0] in_cn_proc_5;
                reg [4*WIDTH-1:0] in_cn_proc_5_r;
                wire [WIDTH-1:0] min_cn_proc_5;
                wire [WIDTH-1:0] sgn_mul_cn_proc_5;

                wire [5*WIDTH-1:0] in_cn_proc_6;
                reg [5*WIDTH-1:0] in_cn_proc_6_r;
                wire [WIDTH-1:0] min_cn_proc_6;
                wire [WIDTH-1:0] sgn_mul_cn_proc_6;

                wire [6*WIDTH-1:0] in_cn_proc_7;
                reg [6*WIDTH-1:0] in_cn_proc_7_r;
                wire [WIDTH-1:0] min_cn_proc_7;
                wire [WIDTH-1:0] sgn_mul_cn_proc_7;
            end
            19:
            begin : gen_cn_buses_19
                wire [2*WIDTH-1:0] in_cn_proc_3;
                //reg [2*WIDTH-1:0] in_cn_proc_3_r;
                wire [WIDTH-1:0] min_cn_proc_3;
                wire [WIDTH-1:0] sgn_mul_cn_proc_3;

                wire [3*WIDTH-1:0] in_cn_proc_4;
                //reg [3*WIDTH-1:0] in_cn_proc_4_r;
                wire [WIDTH-1:0] min_cn_proc_4;
                wire [WIDTH-1:0] sgn_mul_cn_proc_4;

                wire [4*WIDTH-1:0] in_cn_proc_5;
                //reg [4*WIDTH-1:0] in_cn_proc_5_r;
                wire [WIDTH-1:0] min_cn_proc_5;
                wire [WIDTH-1:0] sgn_mul_cn_proc_5;

                wire [5*WIDTH-1:0] in_cn_proc_6;
                //reg [5*WIDTH-1:0] in_cn_proc_6_r;
                wire [WIDTH-1:0] min_cn_proc_6;
                wire [WIDTH-1:0] sgn_mul_cn_proc_6;

                wire [6*WIDTH-1:0] in_cn_proc_7;
                //reg [6*WIDTH-1:0] in_cn_proc_7_r;
                wire [WIDTH-1:0] min_cn_proc_7;
                wire [WIDTH-1:0] sgn_mul_cn_proc_7;

                wire [7*WIDTH-1:0] in_cn_proc_8;
                //reg [7*WIDTH-1:0] in_cn_proc_8_r;
                wire [WIDTH-1:0] min_cn_proc_8;
                wire [WIDTH-1:0] sgn_mul_cn_proc_8;

                wire [8*WIDTH-1:0] in_cn_proc_9;
                //reg [8*WIDTH-1:0] in_cn_proc_9_r;
                wire [WIDTH-1:0] min_cn_proc_9;
                wire [WIDTH-1:0] sgn_mul_cn_proc_9;

                wire [9*WIDTH-1:0] in_cn_proc_10;
                //reg [9*WIDTH-1:0] in_cn_proc_10_r;
                wire [WIDTH-1:0] min_cn_proc_10;
                wire [WIDTH-1:0] sgn_mul_cn_proc_10;

                wire [18*WIDTH-1:0] in_cn_proc_19;
                //reg [18*WIDTH-1:0] in_cn_proc_19_r;
                wire [WIDTH-1:0] min_cn_proc_19;
                wire [WIDTH-1:0] sgn_mul_cn_proc_19;
            end
        endcase
    endgenerate

    generate
        case (DEGREE)
            4:
            begin : gen_cn_buses_4_assignments_first
                case (sel_mux_cn_proc_3)
                    0:
                        assign gen_cn_buses_4.in_cn_proc_3_r = gen_bus_4.bus_gen_4[0].in_cn_proc_3;
                    1:
                        assign gen_cn_buses_4.in_cn_proc_3_r = gen_bus_4.bus_gen_4[1].in_cn_proc_3;
                    2:
                        assign gen_cn_buses_4.in_cn_proc_3_r = gen_bus_4.bus_gen_4[2].in_cn_proc_3;
                    default:
                        assign gen_cn_buses_4.in_cn_proc_3_r = {(2*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_4)
                    0:
                        assign gen_cn_buses_4.in_cn_proc_4_r = gen_bus_4.bus_gen_4[0].in_cn_proc_4;
                    1:
                        assign gen_cn_buses_4.in_cn_proc_4_r = gen_bus_4.bus_gen_4[1].in_cn_proc_4;
                    2:
                        assign gen_cn_buses_4.in_cn_proc_4_r = gen_bus_4.bus_gen_4[2].in_cn_proc_4;
                    default:
                        assign gen_cn_buses_4.in_cn_proc_4_r = {(3*WIDTH){1'bx}};
                endcase
            end
            5:
            begin : gen_cn_buses_5_assignments_first
                case (sel_mux_cn_proc_3)
                    0:
                        assign gen_cn_buses_5.in_cn_proc_3_r = gen_bus_5.bus_gen_5[0].in_cn_proc_3;
                    1:
                        assign gen_cn_buses_5.in_cn_proc_3_r = gen_bus_5.bus_gen_5[1].in_cn_proc_3;
                    2:
                        assign gen_cn_buses_5.in_cn_proc_3_r = gen_bus_5.bus_gen_5[2].in_cn_proc_3;
                    default:
                        assign gen_cn_buses_5.in_cn_proc_3_r = {(2*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_4)
                    0:
                        assign gen_cn_buses_5.in_cn_proc_4_r = gen_bus_5.bus_gen_5[0].in_cn_proc_4;
                    1:
                        assign gen_cn_buses_5.in_cn_proc_4_r = gen_bus_5.bus_gen_5[1].in_cn_proc_4;
                    2:
                        assign gen_cn_buses_5.in_cn_proc_4_r = gen_bus_5.bus_gen_5[2].in_cn_proc_4;
                    default:
                        assign gen_cn_buses_5.in_cn_proc_4_r = {(3*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_5)
                    0:
                        assign gen_cn_buses_5.in_cn_proc_5_r = gen_bus_5.bus_gen_5[0].in_cn_proc_5;
                    1:
                        assign gen_cn_buses_5.in_cn_proc_5_r = gen_bus_5.bus_gen_5[1].in_cn_proc_5;
                    2:
                        assign gen_cn_buses_5.in_cn_proc_5_r = gen_bus_5.bus_gen_5[2].in_cn_proc_5;
                    default:
                        assign gen_cn_buses_5.in_cn_proc_5_r = {(4*WIDTH){1'bx}};
                endcase
            end
            6:
            begin : gen_cn_buses_6_assignments_first
                case (sel_mux_cn_proc_3)
                    0:
                        assign gen_cn_buses_6.in_cn_proc_3_r = gen_bus_6.bus_gen_6[0].in_cn_proc_3;
                    1:
                        assign gen_cn_buses_6.in_cn_proc_3_r = gen_bus_6.bus_gen_6[1].in_cn_proc_3;
                    2:
                        assign gen_cn_buses_6.in_cn_proc_3_r = gen_bus_6.bus_gen_6[2].in_cn_proc_3;
                    default:
                        assign gen_cn_buses_6.in_cn_proc_3_r = {(2*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_4)
                    0:
                        assign gen_cn_buses_6.in_cn_proc_4_r = gen_bus_6.bus_gen_6[0].in_cn_proc_4;
                    1:
                        assign gen_cn_buses_6.in_cn_proc_4_r = gen_bus_6.bus_gen_6[1].in_cn_proc_4;
                    2:
                        assign gen_cn_buses_6.in_cn_proc_4_r = gen_bus_6.bus_gen_6[2].in_cn_proc_4;
                    default:
                        assign gen_cn_buses_6.in_cn_proc_4_r = {(3*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_5)
                    0:
                        assign gen_cn_buses_6.in_cn_proc_5_r = gen_bus_6.bus_gen_6[0].in_cn_proc_5;
                    1:
                        assign gen_cn_buses_6.in_cn_proc_5_r = gen_bus_6.bus_gen_6[1].in_cn_proc_5;
                    2:
                        assign gen_cn_buses_6.in_cn_proc_5_r = gen_bus_6.bus_gen_6[2].in_cn_proc_5;
                    default:
                        assign gen_cn_buses_6.in_cn_proc_5_r = {(4*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_6)
                    0:
                        assign gen_cn_buses_6.in_cn_proc_6_r = gen_bus_6.bus_gen_6[0].in_cn_proc_6;
                    1:
                        assign gen_cn_buses_6.in_cn_proc_6_r = gen_bus_6.bus_gen_6[1].in_cn_proc_6;
                    2:
                        assign gen_cn_buses_6.in_cn_proc_6_r = gen_bus_6.bus_gen_6[2].in_cn_proc_6;
                    default:
                        assign gen_cn_buses_6.in_cn_proc_6_r = {(5*WIDTH){1'bx}};
                endcase
            end
            7:
            begin : gen_cn_buses_7_assignments_first
                case (sel_mux_cn_proc_3)
                    0:
                        assign gen_cn_buses_7.in_cn_proc_3_r = gen_bus_7.bus_gen_7[0].in_cn_proc_3;
                    1:
                        assign gen_cn_buses_7.in_cn_proc_3_r = gen_bus_7.bus_gen_7[1].in_cn_proc_3;
                    2:
                        assign gen_cn_buses_7.in_cn_proc_3_r = gen_bus_7.bus_gen_7[2].in_cn_proc_3;
                    default:
                        assign gen_cn_buses_7.in_cn_proc_3_r = {(2*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_4)
                    0:
                        assign gen_cn_buses_7.in_cn_proc_4_r = gen_bus_7.bus_gen_7[0].in_cn_proc_4;
                    1:
                        assign gen_cn_buses_7.in_cn_proc_4_r = gen_bus_7.bus_gen_7[1].in_cn_proc_4;
                    2:
                        assign gen_cn_buses_7.in_cn_proc_4_r = gen_bus_7.bus_gen_7[2].in_cn_proc_4;
                    default:
                        assign gen_cn_buses_7.in_cn_proc_4_r = {(3*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_5)
                    0:
                        assign gen_cn_buses_7.in_cn_proc_5_r = gen_bus_7.bus_gen_7[0].in_cn_proc_5;
                    1:
                        assign gen_cn_buses_7.in_cn_proc_5_r = gen_bus_7.bus_gen_7[1].in_cn_proc_5;
                    2:
                        assign gen_cn_buses_7.in_cn_proc_5_r = gen_bus_7.bus_gen_7[2].in_cn_proc_5;
                    default:
                        assign gen_cn_buses_7.in_cn_proc_5_r = {(4*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_6)
                    0:
                        assign gen_cn_buses_7.in_cn_proc_6_r = gen_bus_7.bus_gen_7[0].in_cn_proc_6;
                    1:
                        assign gen_cn_buses_7.in_cn_proc_6_r = gen_bus_7.bus_gen_7[1].in_cn_proc_6;
                    2:
                        assign gen_cn_buses_7.in_cn_proc_6_r = gen_bus_7.bus_gen_7[2].in_cn_proc_6;
                    default:
                        assign gen_cn_buses_7.in_cn_proc_6_r = {(5*WIDTH){1'bx}};
                 endcase
                case (sel_mux_cn_proc_7)
                    0:
                        assign gen_cn_buses_7.in_cn_proc_7_r = gen_bus_7.bus_gen_7[0].in_cn_proc_7;
                    1:
                        assign gen_cn_buses_7.in_cn_proc_7_r = gen_bus_7.bus_gen_7[1].in_cn_proc_7;
                    2:
                        assign gen_cn_buses_7.in_cn_proc_7_r = gen_bus_7.bus_gen_7[2].in_cn_proc_7;
                    default:
                        assign gen_cn_buses_7.in_cn_proc_7_r = {(6*WIDTH){1'bx}};
                endcase
            end
            19:
            begin : gen_cn_buses_19_assignments_first
                case (sel_mux_cn_proc_3)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_3_r = gen_bus_19.bus_gen_19[0].in_cn_proc_3;
                        assign gen_cn_buses_19.in_cn_proc_3 = gen_bus_19.bus_gen_19[0].in_cn_proc_3;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_3_r = gen_bus_19.bus_gen_19[1].in_cn_proc_3;
                        assign gen_cn_buses_19.in_cn_proc_3 = gen_bus_19.bus_gen_19[1].in_cn_proc_3;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_3_r = gen_bus_19.bus_gen_19[2].in_cn_proc_3;
                        assign gen_cn_buses_19.in_cn_proc_3 = gen_bus_19.bus_gen_19[2].in_cn_proc_3;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_3_r = {(2*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_3 = {(2*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_4)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_4_r = gen_bus_19.bus_gen_19[0].in_cn_proc_4;
                        assign gen_cn_buses_19.in_cn_proc_4 = gen_bus_19.bus_gen_19[0].in_cn_proc_4;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_4_r = gen_bus_19.bus_gen_19[1].in_cn_proc_4;
                        assign gen_cn_buses_19.in_cn_proc_4 = gen_bus_19.bus_gen_19[1].in_cn_proc_4;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_4_r = gen_bus_19.bus_gen_19[2].in_cn_proc_4;
                        assign gen_cn_buses_19.in_cn_proc_4 = gen_bus_19.bus_gen_19[2].in_cn_proc_4;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_4_r = {(3*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_4 = {(3*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_5)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_5_r = gen_bus_19.bus_gen_19[0].in_cn_proc_5;
                        assign gen_cn_buses_19.in_cn_proc_5 = gen_bus_19.bus_gen_19[0].in_cn_proc_5;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_5_r = gen_bus_19.bus_gen_19[1].in_cn_proc_5;
                        assign gen_cn_buses_19.in_cn_proc_5 = gen_bus_19.bus_gen_19[1].in_cn_proc_5;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_5_r = gen_bus_19.bus_gen_19[2].in_cn_proc_5;
                        assign gen_cn_buses_19.in_cn_proc_5 = gen_bus_19.bus_gen_19[2].in_cn_proc_5;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_5_r = {(4*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_5 = {(4*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_6)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_6_r = gen_bus_19.bus_gen_19[0].in_cn_proc_6;
                        assign gen_cn_buses_19.in_cn_proc_6 = gen_bus_19.bus_gen_19[0].in_cn_proc_6;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_6_r = gen_bus_19.bus_gen_19[1].in_cn_proc_6;
                        assign gen_cn_buses_19.in_cn_proc_6 = gen_bus_19.bus_gen_19[1].in_cn_proc_6;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_6_r = gen_bus_19.bus_gen_19[2].in_cn_proc_6;
                        assign gen_cn_buses_19.in_cn_proc_6 = gen_bus_19.bus_gen_19[2].in_cn_proc_6;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_6_r = {(5*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_6 = {(5*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_7)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_7_r = gen_bus_19.bus_gen_19[0].in_cn_proc_7;
                        assign gen_cn_buses_19.in_cn_proc_7 = gen_bus_19.bus_gen_19[0].in_cn_proc_7;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_7_r = gen_bus_19.bus_gen_19[1].in_cn_proc_7;
                        assign gen_cn_buses_19.in_cn_proc_7 = gen_bus_19.bus_gen_19[1].in_cn_proc_7;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_7_r = gen_bus_19.bus_gen_19[2].in_cn_proc_7;
                        assign gen_cn_buses_19.in_cn_proc_7 = gen_bus_19.bus_gen_19[2].in_cn_proc_7;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_7_r = {(6*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_7 = {(6*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_8)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_8_r = gen_bus_19.bus_gen_19[0].in_cn_proc_8;
                        assign gen_cn_buses_19.in_cn_proc_8 = gen_bus_19.bus_gen_19[0].in_cn_proc_8;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_8_r = gen_bus_19.bus_gen_19[1].in_cn_proc_8;
                        assign gen_cn_buses_19.in_cn_proc_8 = gen_bus_19.bus_gen_19[1].in_cn_proc_8;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_8_r = gen_bus_19.bus_gen_19[2].in_cn_proc_8;
                        assign gen_cn_buses_19.in_cn_proc_8 = gen_bus_19.bus_gen_19[2].in_cn_proc_8;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_8_r = {(7*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_8 = {(7*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_9)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_9_r = gen_bus_19.bus_gen_19[0].in_cn_proc_9;
                        assign gen_cn_buses_19.in_cn_proc_9 = gen_bus_19.bus_gen_19[0].in_cn_proc_9;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_9_r = gen_bus_19.bus_gen_19[1].in_cn_proc_9;
                        assign gen_cn_buses_19.in_cn_proc_9 = gen_bus_19.bus_gen_19[1].in_cn_proc_9;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_9_r = gen_bus_19.bus_gen_19[2].in_cn_proc_9;
                        assign gen_cn_buses_19.in_cn_proc_9 = gen_bus_19.bus_gen_19[2].in_cn_proc_9;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_9_r = {(8*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_9 = {(8*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_10)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_10_r = gen_bus_19.bus_gen_19[0].in_cn_proc_10;
                        assign gen_cn_buses_19.in_cn_proc_10 = gen_bus_19.bus_gen_19[0].in_cn_proc_10;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_10_r = gen_bus_19.bus_gen_19[1].in_cn_proc_10;
                        assign gen_cn_buses_19.in_cn_proc_10 = gen_bus_19.bus_gen_19[1].in_cn_proc_10;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_10_r = gen_bus_19.bus_gen_19[2].in_cn_proc_10;
                        assign gen_cn_buses_19.in_cn_proc_10 = gen_bus_19.bus_gen_19[2].in_cn_proc_10;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_10_r = {(9*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_10 = {(9*WIDTH){1'bx}};
                endcase
                case (sel_mux_cn_proc_19)
                    0:
                        //assign gen_cn_buses_19.in_cn_proc_19_r = gen_bus_19.bus_gen_19[0].in_cn_proc_19;
                        assign gen_cn_buses_19.in_cn_proc_19 = gen_bus_19.bus_gen_19[0].in_cn_proc_19;
                    1:
                        //assign gen_cn_buses_19.in_cn_proc_19_r = gen_bus_19.bus_gen_19[1].in_cn_proc_19;
                        assign gen_cn_buses_19.in_cn_proc_19 = gen_bus_19.bus_gen_19[1].in_cn_proc_19;
                    2:
                        //assign gen_cn_buses_19.in_cn_proc_19_r = gen_bus_19.bus_gen_19[2].in_cn_proc_19;
                        assign gen_cn_buses_19.in_cn_proc_19 = gen_bus_19.bus_gen_19[2].in_cn_proc_19;
                    default:
                        //assign gen_cn_buses_19.in_cn_proc_19_r = {(18*WIDTH){1'bx}};
                        assign gen_cn_buses_19.in_cn_proc_19 = {(18*WIDTH){1'bx}};
                endcase
            end
        endcase
    endgenerate

    generate
        case (DEGREE)
            4:
            begin : gen_cn_buses_4_assignments
                assign gen_cn_buses_4.in_cn_proc_3 = gen_cn_buses_4.in_cn_proc_3_r;
                assign gen_cn_buses_4.in_cn_proc_4 = gen_cn_buses_4.in_cn_proc_4_r;
            end
            5:
            begin : gen_cn_buses_5_assignments
                assign gen_cn_buses_5.in_cn_proc_3 = gen_cn_buses_5.in_cn_proc_3_r;
                assign gen_cn_buses_5.in_cn_proc_4 = gen_cn_buses_5.in_cn_proc_4_r;
                assign gen_cn_buses_5.in_cn_proc_5 = gen_cn_buses_5.in_cn_proc_5_r;
            end
            6:
            begin : gen_cn_buses_6_assignments
                assign gen_cn_buses_6.in_cn_proc_3 = gen_cn_buses_6.in_cn_proc_3_r;
                assign gen_cn_buses_6.in_cn_proc_4 = gen_cn_buses_6.in_cn_proc_4_r;
                assign gen_cn_buses_6.in_cn_proc_5 = gen_cn_buses_6.in_cn_proc_5_r;
                assign gen_cn_buses_6.in_cn_proc_6 = gen_cn_buses_6.in_cn_proc_6_r;
            end
            7:
            begin : gen_cn_buses_7_assignments
                assign gen_cn_buses_7.in_cn_proc_3 = gen_cn_buses_7.in_cn_proc_3_r;
                assign gen_cn_buses_7.in_cn_proc_4 = gen_cn_buses_7.in_cn_proc_4_r;
                assign gen_cn_buses_7.in_cn_proc_5 = gen_cn_buses_7.in_cn_proc_5_r;
                assign gen_cn_buses_7.in_cn_proc_6 = gen_cn_buses_7.in_cn_proc_6_r;
                assign gen_cn_buses_7.in_cn_proc_7 = gen_cn_buses_7.in_cn_proc_7_r;
            end
            //19:
            //begin : gen_cn_buses_19_assignments
            //    assign gen_cn_buses_19.in_cn_proc_3 = gen_cn_buses_19.in_cn_proc_3_r;
            //    assign gen_cn_buses_19.in_cn_proc_4 = gen_cn_buses_19.in_cn_proc_4_r;
            //    assign gen_cn_buses_19.in_cn_proc_5 = gen_cn_buses_19.in_cn_proc_5_r;
            //    assign gen_cn_buses_19.in_cn_proc_6 = gen_cn_buses_19.in_cn_proc_6_r;
            //    assign gen_cn_buses_19.in_cn_proc_7 = gen_cn_buses_19.in_cn_proc_7_r;
            //    assign gen_cn_buses_19.in_cn_proc_8 = gen_cn_buses_19.in_cn_proc_8_r;
            //    assign gen_cn_buses_19.in_cn_proc_9 = gen_cn_buses_19.in_cn_proc_9_r;
            //    assign gen_cn_buses_19.in_cn_proc_10 = gen_cn_buses_19.in_cn_proc_10_r;
            //    assign gen_cn_buses_19.in_cn_proc_19 = gen_cn_buses_19.in_cn_proc_19_r;
            //end
        endcase
    endgenerate
    
//mod_name #(
//    .PARAM1(PARAM1_VALUE),
//    .PARAM2(PARAM2_VALUE),
//    ...
//) instance_label (
//    .port_1(port_1_wire), -> if input
//    .port_2(port_2_wire_or_reg) -> if output
//);

    generate
        case (DEGREE)
            4 :
            begin : gen_cn_procs_4
                cn_proc #(
                    .GROUP(3),
                    .WIDTH(WIDTH)
                ) cn_proc_3 (
                    .in_llr(gen_cn_buses_4.in_cn_proc_3),
                    .min_llr(gen_cn_buses_4.min_cn_proc_3),
                    .sgn_mul_llr(gen_cn_buses_4.sgn_mul_cn_proc_3)
                );
                cn_proc #(
                    .GROUP(4),
                    .WIDTH(WIDTH)
                ) cn_proc_4 (
                    .in_llr(gen_cn_buses_4.in_cn_proc_4),
                    .min_llr(gen_cn_buses_4.min_cn_proc_4),
                    .sgn_mul_llr(gen_cn_buses_4.sgn_mul_cn_proc_4)
                );
            end
            5:
            begin : gen_cn_procs_5
                cn_proc #(
                    .GROUP(3),
                    .WIDTH(WIDTH)
                ) cn_proc_3 (
                    .in_llr(gen_cn_buses_5.in_cn_proc_3),
                    .min_llr(gen_cn_buses_5.min_cn_proc_3),
                    .sgn_mul_llr(gen_cn_buses_5.sgn_mul_cn_proc_3)
                );
                cn_proc #(
                    .GROUP(4),
                    .WIDTH(WIDTH)
                ) cn_proc_4 (
                    .in_llr(gen_cn_buses_5.in_cn_proc_4),
                    .min_llr(gen_cn_buses_5.min_cn_proc_4),
                    .sgn_mul_llr(gen_cn_buses_5.sgn_mul_cn_proc_4)
                );
                cn_proc #(
                    .GROUP(5),
                    .WIDTH(WIDTH)
                ) cn_proc_5 (
                    .in_llr(gen_cn_buses_5.in_cn_proc_5),
                    .min_llr(gen_cn_buses_5.min_cn_proc_5),
                    .sgn_mul_llr(gen_cn_buses_5.sgn_mul_cn_proc_5)
                );
            end
            6:
            begin : gen_cn_procs_6
                cn_proc #(
                    .GROUP(3),
                    .WIDTH(WIDTH)
                ) cn_proc_3 (
                    .in_llr(gen_cn_buses_6.in_cn_proc_3),
                    .min_llr(gen_cn_buses_6.min_cn_proc_3),
                    .sgn_mul_llr(gen_cn_buses_6.sgn_mul_cn_proc_3)
                );
                cn_proc #(
                    .GROUP(4),
                    .WIDTH(WIDTH)
                ) cn_proc_4 (
                    .in_llr(gen_cn_buses_6.in_cn_proc_4),
                    .min_llr(gen_cn_buses_6.min_cn_proc_4),
                    .sgn_mul_llr(gen_cn_buses_6.sgn_mul_cn_proc_4)
                );
                cn_proc #(
                    .GROUP(5),
                    .WIDTH(WIDTH)
                ) cn_proc_5 (
                    .in_llr(gen_cn_buses_6.in_cn_proc_5),
                    .min_llr(gen_cn_buses_6.min_cn_proc_5),
                    .sgn_mul_llr(gen_cn_buses_6.sgn_mul_cn_proc_5)
                );
                cn_proc #(
                    .GROUP(6),
                    .WIDTH(WIDTH)
                ) cn_proc_6 (
                    .in_llr(gen_cn_buses_6.in_cn_proc_6),
                    .min_llr(gen_cn_buses_6.min_cn_proc_6),
                    .sgn_mul_llr(gen_cn_buses_6.sgn_mul_cn_proc_6)
                );
            end
            7:
            begin : gen_cn_procs_7
                cn_proc #(
                    .GROUP(3),
                    .WIDTH(WIDTH)
                ) cn_proc_3 (
                    .in_llr(gen_cn_buses_7.in_cn_proc_3),
                    .min_llr(gen_cn_buses_7.min_cn_proc_3),
                    .sgn_mul_llr(gen_cn_buses_7.sgn_mul_cn_proc_3)
                );
                cn_proc #(
                    .GROUP(4),
                    .WIDTH(WIDTH)
                ) cn_proc_4 (
                    .in_llr(gen_cn_buses_7.in_cn_proc_4),
                    .min_llr(gen_cn_buses_7.min_cn_proc_4),
                    .sgn_mul_llr(gen_cn_buses_7.sgn_mul_cn_proc_4)
                );
                cn_proc #(
                    .GROUP(5),
                    .WIDTH(WIDTH)
                ) cn_proc_5 (
                    .in_llr(gen_cn_buses_7.in_cn_proc_5),
                    .min_llr(gen_cn_buses_7.min_cn_proc_5),
                    .sgn_mul_llr(gen_cn_buses_7.sgn_mul_cn_proc_5)
                );
                cn_proc #(
                    .GROUP(6),
                    .WIDTH(WIDTH)
                ) cn_proc_6 (
                    .in_llr(gen_cn_buses_7.in_cn_proc_6),
                    .min_llr(gen_cn_buses_7.min_cn_proc_6),
                    .sgn_mul_llr(gen_cn_buses_7.sgn_mul_cn_proc_6)
                );
                cn_proc #(
                    .GROUP(7),
                    .WIDTH(WIDTH)
                ) cn_proc_7 (
                    .in_llr(gen_cn_buses_7.in_cn_proc_7),
                    .min_llr(gen_cn_buses_7.min_cn_proc_7),
                    .sgn_mul_llr(gen_cn_buses_7.sgn_mul_cn_proc_7)
                );
            end
            19:
            begin : gen_cn_procs_19
                cn_proc #(
                    .GROUP(3),
                    .WIDTH(WIDTH)
                ) cn_proc_3 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_3),
                    .min_llr(gen_cn_buses_19.min_cn_proc_3),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_3)
                );
                cn_proc #(
                    .GROUP(4),
                    .WIDTH(WIDTH)
                ) cn_proc_4 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_4),
                    .min_llr(gen_cn_buses_19.min_cn_proc_4),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_4)
                );
                cn_proc #(
                    .GROUP(5),
                    .WIDTH(WIDTH)
                ) cn_proc_5 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_5),
                    .min_llr(gen_cn_buses_19.min_cn_proc_5),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_5)
                );
                cn_proc #(
                    .GROUP(6),
                    .WIDTH(WIDTH)
                ) cn_proc_6 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_6),
                    .min_llr(gen_cn_buses_19.min_cn_proc_6),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_6)
                );
                cn_proc #(
                    .GROUP(7),
                    .WIDTH(WIDTH)
                ) cn_proc_7 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_7),
                    .min_llr(gen_cn_buses_19.min_cn_proc_7),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_7)
                );
                cn_proc #(
                    .GROUP(8),
                    .WIDTH(WIDTH)
                ) cn_proc_8 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_8),
                    .min_llr(gen_cn_buses_19.min_cn_proc_8),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_8)
                );
                cn_proc #(
                    .GROUP(9),
                    .WIDTH(WIDTH)
                ) cn_proc_9 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_9),
                    .min_llr(gen_cn_buses_19.min_cn_proc_9),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_9)
                );
                cn_proc #(
                    .GROUP(10),
                    .WIDTH(WIDTH)
                ) cn_proc_10 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_10),
                    .min_llr(gen_cn_buses_19.min_cn_proc_10),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_10)
                );
                cn_proc #(
                    .GROUP(19),
                    .WIDTH(WIDTH)
                ) cn_proc_19 (
                    .in_llr(gen_cn_buses_19.in_cn_proc_19),
                    .min_llr(gen_cn_buses_19.min_cn_proc_19),
                    .sgn_mul_llr(gen_cn_buses_19.sgn_mul_cn_proc_19)
                );
            end
        endcase
    endgenerate
    
    generate 
        case (DEGREE)
            4:
            begin : gen_min_mul_buses_4
                wire [WIDTH-1:0] out_demux_min_0;
                wire [WIDTH-1:0] out_demux_min_1;
                wire [WIDTH-1:0] out_demux_mul_0;
                wire [WIDTH-1:0] out_demux_mul_1;
                reg [WIDTH-1:0] out_demux_min_0_r;
                reg [WIDTH-1:0] out_demux_min_1_r;
                reg [WIDTH-1:0] out_demux_mul_0_r;
                reg [WIDTH-1:0] out_demux_mul_1_r;
            end
            5:
            begin : gen_min_mul_buses_5
                wire [WIDTH-1:0] out_demux_min_0;
                wire [WIDTH-1:0] out_demux_min_1;
                wire [WIDTH-1:0] out_demux_min_2;
                wire [WIDTH-1:0] out_demux_mul_0;
                wire [WIDTH-1:0] out_demux_mul_1;
                wire [WIDTH-1:0] out_demux_mul_2;
                reg [WIDTH-1:0] out_demux_min_0_r;
                reg [WIDTH-1:0] out_demux_min_1_r;
                reg [WIDTH-1:0] out_demux_min_2_r;
                reg [WIDTH-1:0] out_demux_mul_0_r;
                reg [WIDTH-1:0] out_demux_mul_1_r;
                reg [WIDTH-1:0] out_demux_mul_2_r;
            end
            6:
            begin : gen_min_mul_buses_6
                wire [WIDTH-1:0] out_demux_min_0;
                wire [WIDTH-1:0] out_demux_min_1;
                wire [WIDTH-1:0] out_demux_min_2;
                wire [WIDTH-1:0] out_demux_mul_0;
                wire [WIDTH-1:0] out_demux_mul_1;
                wire [WIDTH-1:0] out_demux_mul_2;
                reg [WIDTH-1:0] out_demux_min_0_r;
                reg [WIDTH-1:0] out_demux_min_1_r;
                reg [WIDTH-1:0] out_demux_min_2_r;
                reg [WIDTH-1:0] out_demux_mul_0_r;
                reg [WIDTH-1:0] out_demux_mul_1_r;
                reg [WIDTH-1:0] out_demux_mul_2_r;
            end
            7:
            begin : gen_min_mul_buses_7
                wire [WIDTH-1:0] out_demux_min_0;
                wire [WIDTH-1:0] out_demux_min_1;
                wire [WIDTH-1:0] out_demux_min_2;
                wire [WIDTH-1:0] out_demux_mul_0;
                wire [WIDTH-1:0] out_demux_mul_1;
                wire [WIDTH-1:0] out_demux_mul_2;
                reg [WIDTH-1:0] out_demux_min_0_r;
                reg [WIDTH-1:0] out_demux_min_1_r;
                reg [WIDTH-1:0] out_demux_min_2_r;
                reg [WIDTH-1:0] out_demux_mul_0_r;
                reg [WIDTH-1:0] out_demux_mul_1_r;
                reg [WIDTH-1:0] out_demux_mul_2_r;
            end
            19:
            begin : gen_min_mul_buses_19
                wire [WIDTH-1:0] out_demux_min_0;
                wire [WIDTH-1:0] out_demux_min_1;
                wire [WIDTH-1:0] out_demux_min_2;
                wire [WIDTH-1:0] out_demux_mul_0;
                wire [WIDTH-1:0] out_demux_mul_1;
                wire [WIDTH-1:0] out_demux_mul_2;
                //reg [WIDTH-1:0] out_demux_min_0_r;
                //reg [WIDTH-1:0] out_demux_min_1_r;
                //reg [WIDTH-1:0] out_demux_min_2_r;
                //reg [WIDTH-1:0] out_demux_mul_0_r;
                //reg [WIDTH-1:0] out_demux_mul_1_r;
                //reg [WIDTH-1:0] out_demux_mul_2_r;
            end
        endcase
    endgenerate

    generate 
        case (DEGREE)
            4:
            begin : gen_min_mul_buses_4_assignments_first
                case (sel_demux_min_0)
                    0:
                        assign gen_min_mul_buses_4.out_demux_min_0_r = gen_cn_buses_4.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_4.out_demux_min_0_r = gen_cn_buses_4.min_cn_proc_4;
                    default:
                        assign gen_min_mul_buses_4.out_demux_min_0_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_1)
                    0: 
                        assign gen_min_mul_buses_4.out_demux_min_1_r = gen_cn_buses_4.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_4.out_demux_min_1_r = gen_cn_buses_4.min_cn_proc_4;
                    default:
                        assign gen_min_mul_buses_4.out_demux_min_1_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_0)
                    0:
                        assign gen_min_mul_buses_4.out_demux_mul_0_r = gen_cn_buses_4.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_4.out_demux_mul_0_r = gen_cn_buses_4.sgn_mul_cn_proc_4;
                    default:
                        assign gen_min_mul_buses_4.out_demux_mul_0_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_1)
                    0:
                        assign gen_min_mul_buses_4.out_demux_mul_1_r = gen_cn_buses_4.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_4.out_demux_mul_1_r = gen_cn_buses_4.sgn_mul_cn_proc_4;
                    default:
                        assign gen_min_mul_buses_4.out_demux_mul_1_r = {(WIDTH){1'bx}};
                endcase
            end
            5:
            begin : gen_min_mul_buses_5_assignments_first
                case (sel_demux_min_0)
                    0:
                        assign gen_min_mul_buses_5.out_demux_min_0_r = gen_cn_buses_5.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_5.out_demux_min_0_r = gen_cn_buses_5.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_5.out_demux_min_0_r = gen_cn_buses_5.min_cn_proc_5;
                    default:
                        assign gen_min_mul_buses_5.out_demux_min_0_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_1)
                    0: 
                        assign gen_min_mul_buses_5.out_demux_min_1_r = gen_cn_buses_5.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_5.out_demux_min_1_r = gen_cn_buses_5.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_5.out_demux_min_1_r = gen_cn_buses_5.min_cn_proc_5;
                    default:
                        assign gen_min_mul_buses_5.out_demux_min_1_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_2)
                    0: 
                        assign gen_min_mul_buses_5.out_demux_min_2_r = gen_cn_buses_5.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_5.out_demux_min_2_r = gen_cn_buses_5.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_5.out_demux_min_2_r = gen_cn_buses_5.min_cn_proc_5;
                    default:
                        assign gen_min_mul_buses_5.out_demux_min_2_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_0)
                    0:
                        assign gen_min_mul_buses_5.out_demux_mul_0_r = gen_cn_buses_5.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_5.out_demux_mul_0_r = gen_cn_buses_5.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_5.out_demux_mul_0_r = gen_cn_buses_5.sgn_mul_cn_proc_5;
                    default:
                        assign gen_min_mul_buses_5.out_demux_mul_0_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_1)
                    0:
                        assign gen_min_mul_buses_5.out_demux_mul_1_r = gen_cn_buses_5.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_5.out_demux_mul_1_r = gen_cn_buses_5.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_5.out_demux_mul_1_r = gen_cn_buses_5.sgn_mul_cn_proc_5;
                    default:
                        assign gen_min_mul_buses_5.out_demux_mul_1_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_2)
                    0:
                        assign gen_min_mul_buses_5.out_demux_mul_2_r = gen_cn_buses_5.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_5.out_demux_mul_2_r = gen_cn_buses_5.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_5.out_demux_mul_2_r = gen_cn_buses_5.sgn_mul_cn_proc_5;
                    default:
                        assign gen_min_mul_buses_5.out_demux_mul_2_r = {(WIDTH){1'bx}};
                endcase
            end
            6:
            begin : gen_min_mul_buses_6_assignments_first 
                case (sel_demux_min_0)
                    0:
                        assign gen_min_mul_buses_6.out_demux_min_0_r = gen_cn_buses_6.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_6.out_demux_min_0_r = gen_cn_buses_6.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_6.out_demux_min_0_r = gen_cn_buses_6.min_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_6.out_demux_min_0_r = gen_cn_buses_6.min_cn_proc_6;
                    default:
                        assign gen_min_mul_buses_6.out_demux_min_0_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_1)
                    0: 
                        assign gen_min_mul_buses_6.out_demux_min_1_r = gen_cn_buses_6.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_6.out_demux_min_1_r = gen_cn_buses_6.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_6.out_demux_min_1_r = gen_cn_buses_6.min_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_6.out_demux_min_1_r = gen_cn_buses_6.min_cn_proc_6;
                    default:
                        assign gen_min_mul_buses_6.out_demux_min_1_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_2)
                    0: 
                        assign gen_min_mul_buses_6.out_demux_min_2_r = gen_cn_buses_6.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_6.out_demux_min_2_r = gen_cn_buses_6.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_6.out_demux_min_2_r = gen_cn_buses_6.min_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_6.out_demux_min_2_r = gen_cn_buses_6.min_cn_proc_6;
                    default:
                        assign gen_min_mul_buses_6.out_demux_min_2_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_0)
                    0:
                        assign gen_min_mul_buses_6.out_demux_mul_0_r = gen_cn_buses_6.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_6.out_demux_mul_0_r = gen_cn_buses_6.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_6.out_demux_mul_0_r = gen_cn_buses_6.sgn_mul_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_6.out_demux_mul_0_r = gen_cn_buses_6.sgn_mul_cn_proc_6;
                    default:
                        assign gen_min_mul_buses_6.out_demux_mul_0_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_1)
                    0:
                        assign gen_min_mul_buses_6.out_demux_mul_1_r = gen_cn_buses_6.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_6.out_demux_mul_1_r = gen_cn_buses_6.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_6.out_demux_mul_1_r = gen_cn_buses_6.sgn_mul_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_6.out_demux_mul_1_r = gen_cn_buses_6.sgn_mul_cn_proc_6;
                    default:
                        assign gen_min_mul_buses_6.out_demux_mul_1_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_2)
                    0:
                        assign gen_min_mul_buses_6.out_demux_mul_2_r = gen_cn_buses_6.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_6.out_demux_mul_2_r = gen_cn_buses_6.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_6.out_demux_mul_2_r = gen_cn_buses_6.sgn_mul_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_6.out_demux_mul_2_r = gen_cn_buses_6.sgn_mul_cn_proc_6;
                    default:
                        assign gen_min_mul_buses_6.out_demux_mul_2_r = {(WIDTH){1'bx}};
                endcase
            end
            7:
            begin : gen_min_mul_buses_7_assignments_first
                case (sel_demux_min_0)
                    0:
                        assign gen_min_mul_buses_7.out_demux_min_0_r = gen_cn_buses_7.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_7.out_demux_min_0_r = gen_cn_buses_7.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_7.out_demux_min_0_r = gen_cn_buses_7.min_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_7.out_demux_min_0_r = gen_cn_buses_7.min_cn_proc_6;
                    4:
                        assign gen_min_mul_buses_7.out_demux_min_0_r = gen_cn_buses_7.min_cn_proc_7;
                    default:
                        assign gen_min_mul_buses_7.out_demux_min_0_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_1)
                    0: 
                        assign gen_min_mul_buses_7.out_demux_min_1_r = gen_cn_buses_7.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_7.out_demux_min_1_r = gen_cn_buses_7.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_7.out_demux_min_1_r = gen_cn_buses_7.min_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_7.out_demux_min_1_r = gen_cn_buses_7.min_cn_proc_6;
                    4:
                        assign gen_min_mul_buses_7.out_demux_min_1_r = gen_cn_buses_7.min_cn_proc_7;
                    default:
                        assign gen_min_mul_buses_7.out_demux_min_1_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_2)
                    0: 
                        assign gen_min_mul_buses_7.out_demux_min_2_r = gen_cn_buses_7.min_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_7.out_demux_min_2_r = gen_cn_buses_7.min_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_7.out_demux_min_2_r = gen_cn_buses_7.min_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_7.out_demux_min_2_r = gen_cn_buses_7.min_cn_proc_6;
                    4:
                        assign gen_min_mul_buses_7.out_demux_min_2_r = gen_cn_buses_7.min_cn_proc_7;
                    default:
                        assign gen_min_mul_buses_7.out_demux_min_2_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_0)
                    0:
                        assign gen_min_mul_buses_7.out_demux_mul_0_r = gen_cn_buses_7.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_7.out_demux_mul_0_r = gen_cn_buses_7.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_7.out_demux_mul_0_r = gen_cn_buses_7.sgn_mul_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_7.out_demux_mul_0_r = gen_cn_buses_7.sgn_mul_cn_proc_6;
                    4:
                        assign gen_min_mul_buses_7.out_demux_mul_0_r = gen_cn_buses_7.sgn_mul_cn_proc_7;
                    default:
                        assign gen_min_mul_buses_7.out_demux_mul_0_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_1)
                    0:
                        assign gen_min_mul_buses_7.out_demux_mul_1_r = gen_cn_buses_7.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_7.out_demux_mul_1_r = gen_cn_buses_7.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_7.out_demux_mul_1_r = gen_cn_buses_7.sgn_mul_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_7.out_demux_mul_1_r = gen_cn_buses_7.sgn_mul_cn_proc_6;
                    4:
                        assign gen_min_mul_buses_7.out_demux_mul_1_r = gen_cn_buses_7.sgn_mul_cn_proc_7;
                    default:
                        assign gen_min_mul_buses_7.out_demux_mul_1_r = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_2)
                    0:
                        assign gen_min_mul_buses_7.out_demux_mul_2_r = gen_cn_buses_7.sgn_mul_cn_proc_3;
                    1:
                        assign gen_min_mul_buses_7.out_demux_mul_2_r = gen_cn_buses_7.sgn_mul_cn_proc_4;
                    2:
                        assign gen_min_mul_buses_7.out_demux_mul_2_r = gen_cn_buses_7.sgn_mul_cn_proc_5;
                    3:
                        assign gen_min_mul_buses_7.out_demux_mul_2_r = gen_cn_buses_7.sgn_mul_cn_proc_6;
                    4:
                        assign gen_min_mul_buses_7.out_demux_mul_2_r = gen_cn_buses_7.sgn_mul_cn_proc_7;
                    default:
                        assign gen_min_mul_buses_7.out_demux_mul_2_r = {(WIDTH){1'bx}};
                endcase
            end
            19:
            begin : gen_min_mul_buses_19_assignments_first
                case (sel_demux_min_0)
                    0:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_3;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_3;
                    1:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_4;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_4;
                    2:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_5;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_5;
                    3:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_6;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_6;
                    4:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_7;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_7;
                    5:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_8;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_8;
                    6:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_9;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_9;
                    7:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_10;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_10;
                    8:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = gen_cn_buses_19.min_cn_proc_19;
                        assign gen_min_mul_buses_19.out_demux_min_0 = gen_cn_buses_19.min_cn_proc_19;
                    default:
                        //assign gen_min_mul_buses_19.out_demux_min_0_r = {(WIDTH){1'bx}};
                        assign gen_min_mul_buses_19.out_demux_min_0 = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_1)
                    0: 
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_3;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_3;
                    1:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_4;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_4;
                    2:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_5;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_5;
                    3:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_6;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_6;
                    4:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_7;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_7;
                    5:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_8;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_8;
                    6:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_9;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_9;
                    7:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_10;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_10;
                    8:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = gen_cn_buses_19.min_cn_proc_19;
                        assign gen_min_mul_buses_19.out_demux_min_1 = gen_cn_buses_19.min_cn_proc_19;
                    default:
                        //assign gen_min_mul_buses_19.out_demux_min_1_r = {(WIDTH){1'bx}};
                        assign gen_min_mul_buses_19.out_demux_min_1 = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_min_2)
                    0: 
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_3;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_3;
                    1:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_4;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_4;
                    2:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_5;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_5;
                    3:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_6;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_6;
                    4:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_7;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_7;
                    5:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_8;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_8;
                    6:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_9;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_9;
                    7:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_10;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_10;
                    8:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = gen_cn_buses_19.min_cn_proc_19;
                        assign gen_min_mul_buses_19.out_demux_min_2 = gen_cn_buses_19.min_cn_proc_19;
                    default:
                        //assign gen_min_mul_buses_19.out_demux_min_2_r = {(WIDTH){1'bx}};
                        assign gen_min_mul_buses_19.out_demux_min_2 = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_0)
                    0:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_3;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_3;
                    1:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_4;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_4;
                    2:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_5;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_5;
                    3:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_6;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_6;
                    4:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_7;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_7;
                    5:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_8;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_8;
                    6:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_9;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_9;
                    7:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_10;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_10;
                    8:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = gen_cn_buses_19.sgn_mul_cn_proc_19;
                        assign gen_min_mul_buses_19.out_demux_mul_0 = gen_cn_buses_19.sgn_mul_cn_proc_19;
                    default:
                        //assign gen_min_mul_buses_19.out_demux_mul_0_r = {(WIDTH){1'bx}};
                        assign gen_min_mul_buses_19.out_demux_mul_0 = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_1)
                    0:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_3;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_3;
                    1:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_4;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_4;
                    2:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_5;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_5;
                    3:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_6;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_6;
                    4:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_7;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_7;
                    5:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_8;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_8;
                    6:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_9;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_9;
                    7:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_10;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_10;
                    8:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = gen_cn_buses_19.sgn_mul_cn_proc_19;
                        assign gen_min_mul_buses_19.out_demux_mul_1 = gen_cn_buses_19.sgn_mul_cn_proc_19;
                    default:
                        //assign gen_min_mul_buses_19.out_demux_mul_1_r = {(WIDTH){1'bx}};
                        assign gen_min_mul_buses_19.out_demux_mul_1 = {(WIDTH){1'bx}};
                endcase
                case (sel_demux_mul_2)
                    0:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_3;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_3;
                    1:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_4;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_4;
                    2:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_5;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_5;
                    3:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_6;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_6;
                    4:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_7;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_7;
                    5:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_8;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_8;
                    6:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_9;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_9;
                    7:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_10;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_10;
                    8:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = gen_cn_buses_19.sgn_mul_cn_proc_19;
                        assign gen_min_mul_buses_19.out_demux_mul_2 = gen_cn_buses_19.sgn_mul_cn_proc_19;
                    default:
                        //assign gen_min_mul_buses_19.out_demux_mul_2_r = {(WIDTH){1'bx}};
                        assign gen_min_mul_buses_19.out_demux_mul_2 = {(WIDTH){1'bx}};
                endcase
            end
        endcase
    endgenerate
    
    generate 
        case (DEGREE)
            4:
            begin : gen_min_mul_buses_4_assignments
                assign gen_min_mul_buses_4.out_demux_min_0 = gen_min_mul_buses_4.out_demux_min_0_r;
                assign gen_min_mul_buses_4.out_demux_min_1 = gen_min_mul_buses_4.out_demux_min_1_r;
                assign gen_min_mul_buses_4.out_demux_mul_0 = gen_min_mul_buses_4.out_demux_mul_0_r;
                assign gen_min_mul_buses_4.out_demux_mul_1 = gen_min_mul_buses_4.out_demux_mul_1_r;
            end
            5:
            begin : gen_min_mul_buses_5_assignments
                assign gen_min_mul_buses_5.out_demux_min_0 = gen_min_mul_buses_5.out_demux_min_0_r;
                assign gen_min_mul_buses_5.out_demux_min_1 = gen_min_mul_buses_5.out_demux_min_1_r;
                assign gen_min_mul_buses_5.out_demux_min_2 = gen_min_mul_buses_5.out_demux_min_2_r;
                assign gen_min_mul_buses_5.out_demux_mul_0 = gen_min_mul_buses_5.out_demux_mul_0_r;
                assign gen_min_mul_buses_5.out_demux_mul_1 = gen_min_mul_buses_5.out_demux_mul_1_r;
                assign gen_min_mul_buses_5.out_demux_mul_2 = gen_min_mul_buses_5.out_demux_mul_2_r;
            end
            6:
            begin : gen_min_mul_buses_6_assignments
                assign gen_min_mul_buses_6.out_demux_min_0 = gen_min_mul_buses_6.out_demux_min_0_r;
                assign gen_min_mul_buses_6.out_demux_min_1 = gen_min_mul_buses_6.out_demux_min_1_r;
                assign gen_min_mul_buses_6.out_demux_min_2 = gen_min_mul_buses_6.out_demux_min_2_r;
                assign gen_min_mul_buses_6.out_demux_mul_0 = gen_min_mul_buses_6.out_demux_mul_0_r;
                assign gen_min_mul_buses_6.out_demux_mul_1 = gen_min_mul_buses_6.out_demux_mul_1_r;
                assign gen_min_mul_buses_6.out_demux_mul_2 = gen_min_mul_buses_6.out_demux_mul_2_r;
            end
            7:
            begin : gen_min_mul_buses_7_assignments
                assign gen_min_mul_buses_7.out_demux_min_0 = gen_min_mul_buses_7.out_demux_min_0_r;
                assign gen_min_mul_buses_7.out_demux_min_1 = gen_min_mul_buses_7.out_demux_min_1_r;
                assign gen_min_mul_buses_7.out_demux_min_2 = gen_min_mul_buses_7.out_demux_min_2_r;
                assign gen_min_mul_buses_7.out_demux_mul_0 = gen_min_mul_buses_7.out_demux_mul_0_r;
                assign gen_min_mul_buses_7.out_demux_mul_1 = gen_min_mul_buses_7.out_demux_mul_1_r;
                assign gen_min_mul_buses_7.out_demux_mul_2 = gen_min_mul_buses_7.out_demux_mul_2_r;
            end
            //19:
            //begin : gen_min_mul_buses_19_assignments
            //    assign gen_min_mul_buses_19.out_demux_min_0 = gen_min_mul_buses_19.out_demux_min_0_r;
            //    assign gen_min_mul_buses_19.out_demux_min_1 = gen_min_mul_buses_19.out_demux_min_1_r;
            //    assign gen_min_mul_buses_19.out_demux_min_2 = gen_min_mul_buses_19.out_demux_min_2_r;
            //    assign gen_min_mul_buses_19.out_demux_mul_0 = gen_min_mul_buses_19.out_demux_mul_0_r;
            //    assign gen_min_mul_buses_19.out_demux_mul_1 = gen_min_mul_buses_19.out_demux_mul_1_r;
            //    assign gen_min_mul_buses_19.out_demux_mul_2 = gen_min_mul_buses_19.out_demux_mul_2_r;
            //end
        endcase
    endgenerate

    generate 
        case (DEGREE)
            4:
            begin : gen_out_buses_4
                wire [WIDTH-1:0] in_0_min_0;
                wire [WIDTH-1:0] in_1_min_0;
                wire [WIDTH-1:0] out_min_0;

                wire [WIDTH-1:0] in_0_demux_min_30;
                wire [WIDTH-1:0] in_1_demux_min_30;
                wire [WIDTH-1:0] out_demux_min_30;
                reg [WIDTH-1:0] out_demux_min_30_r;
                
                wire [WIDTH-1:0] in_0_mul_0;
                wire [WIDTH-1:0] in_1_mul_0;
                wire [WIDTH-1:0] out_mul_0;

                wire [WIDTH-1:0] in_0_demux_mul_30;
                wire [WIDTH-1:0] in_1_demux_mul_30;
                wire [WIDTH-1:0] out_demux_mul_30;
                reg [WIDTH-1:0] out_demux_mul_30_r;

                wire [WIDTH-1:0] in_0_mul_out;
                wire [WIDTH-1:0] in_1_mul_out;
                wire [WIDTH-1:0] out_mul_out;

                min_2x1 
                # (
                    .WIDTH(WIDTH) 
                ) min_0 (
                    .in0(in_0_min_0),
                    .in1(in_1_min_0),
                    .out(out_min_0)
                );

                mul_2x1
                # (
                    .WIDTH(WIDTH)
                ) mul_0 (
                    .in0(in_0_mul_0),
                    .in1(in_1_mul_0),
                    .out(out_mul_0)
                );

                mul_2x1
                # (
                    .WIDTH(WIDTH)
                ) mul_out (
                    .in0(in_0_mul_out),
                    .in1(in_1_mul_out),
                    .out(out_mul_out)
                );
            end
            5 , 6 , 7 , 19:
            begin : gen_out_buses_5_6_7_19
                wire [WIDTH-1:0] in_0_min_0;
                wire [WIDTH-1:0] in_1_min_0;
                wire [WIDTH-1:0] out_min_0;
                wire [WIDTH-1:0] in_0_min_1;
                wire [WIDTH-1:0] in_1_min_1;
                wire [WIDTH-1:0] out_min_1;
                wire [WIDTH-1:0] in_0_demux_min_30;
                wire [WIDTH-1:0] in_1_demux_min_30;
                wire [WIDTH-1:0] out_demux_min_30;
                //reg [WIDTH-1:0] out_demux_min_30_r;
                wire [WIDTH-1:0] in_0_demux_min_31;
                wire [WIDTH-1:0] in_1_demux_min_31;
                wire [WIDTH-1:0] out_demux_min_31;
                //reg [WIDTH-1:0] out_demux_min_31_r;
                
                wire [WIDTH-1:0] in_0_mul_0;
                wire [WIDTH-1:0] in_1_mul_0;
                wire [WIDTH-1:0] out_mul_0;
                wire [WIDTH-1:0] in_0_mul_1;
                wire [WIDTH-1:0] in_1_mul_1;
                wire [WIDTH-1:0] out_mul_1;
                wire [WIDTH-1:0] in_0_demux_mul_30;
                wire [WIDTH-1:0] in_1_demux_mul_30;
                wire [WIDTH-1:0] out_demux_mul_30;
                //reg [WIDTH-1:0] out_demux_mul_30_r;
                wire [WIDTH-1:0] in_0_demux_mul_31;
                wire [WIDTH-1:0] in_1_demux_mul_31;
                wire [WIDTH-1:0] out_demux_mul_31;
                //reg [WIDTH-1:0] out_demux_mul_31_r;

                wire [WIDTH-1:0] in_0_mul_out;
                wire [WIDTH-1:0] in_1_mul_out;
                wire [WIDTH-1:0] out_mul_out;
                
                min_2x1 
                # (
                    .WIDTH(WIDTH) 
                ) min_0 (
                    .in0(in_0_min_0),
                    .in1(in_1_min_0),
                    .out(out_min_0)
                );

                min_2x1 
                # (
                    .WIDTH(WIDTH) 
                ) min_1 (
                    .in0(in_0_min_1),
                    .in1(in_1_min_1),
                    .out(out_min_0)
                );

                mul_2x1
                # (
                    .WIDTH(WIDTH)
                ) mul_0 (
                    .in0(in_0_mul_0),
                    .in1(in_1_mul_0),
                    .out(out_mul_0)
                );

                mul_2x1
                # (
                    .WIDTH(WIDTH)
                ) mul_1 (
                    .in0(in_0_mul_1),
                    .in1(in_1_mul_1),
                    .out(out_mul_1)
                );

                mul_2x1
                # (
                    .WIDTH(WIDTH)
                ) mul_out (
                    .in0(in_0_mul_out),
                    .in1(in_1_mul_out),
                    .out(out_mul_out)
                );
            end
        endcase
    endgenerate
    
    //reg [WIDTH-1:0] out_llr_r;
    generate 
        case (DEGREE)
            4:
            begin : gen_out_buses_4_assignments
                assign gen_out_buses_4.in_0_min_0 = gen_min_mul_buses_4.out_demux_min_0;
                assign gen_out_buses_4.in_1_min_0 = gen_min_mul_buses_4.out_demux_min_1;
                assign gen_out_buses_4.in_0_demux_min_30 = gen_min_mul_buses_4.out_demux_min_0;
                assign gen_out_buses_4.in_1_demux_min_30 = gen_out_buses_4.out_min_0;

                assign gen_out_buses_4.in_0_mul_0 = gen_min_mul_buses_4.out_demux_mul_0;
                assign gen_out_buses_4.in_1_mul_0 = gen_min_mul_buses_4.out_demux_mul_1;
                assign gen_out_buses_4.in_0_demux_mul_30 = gen_min_mul_buses_4.out_demux_mul_0;
                assign gen_out_buses_4.in_1_demux_mul_30 = gen_out_buses_4.out_mul_0;

                assign gen_out_buses_4.in_0_mul_out = gen_out_buses_4.out_demux_min_30;
                assign gen_out_buses_4.in_1_mul_out = gen_out_buses_4.out_demux_mul_30;
                //assign out_llr_r = gen_out_buses_4.out_mul_out;
                assign out_llr = gen_out_buses_4.out_mul_out;
                
                case (sel_demux_min_30)
                    0:
                        assign gen_out_buses_4.out_demux_min_30_r = gen_out_buses_4.in_0_demux_min_30;
                    default:
                        assign gen_out_buses_4.out_demux_min_30_r = gen_out_buses_4.in_1_demux_min_30;
                endcase
                
                case (sel_demux_mul_30)
                    0:
                        assign gen_out_buses_4.out_demux_mul_30_r = gen_out_buses_4.in_0_demux_mul_30;
                    default:
                        assign gen_out_buses_4.out_demux_mul_30_r = gen_out_buses_4.in_1_demux_mul_30;
                endcase
            end
            5 , 6 , 7 , 19:
            begin : gen_out_buses_5_6_7_19_assignments
                case (DEGREE)
                    5:
                    begin : out_logic_assignments_5
                        assign gen_out_buses_5_6_7_19.in_0_min_0 = gen_min_mul_buses_5.out_demux_min_0;
                        assign gen_out_buses_5_6_7_19.in_1_min_0 = gen_min_mul_buses_5.out_demux_min_1;
                        assign gen_out_buses_5_6_7_19.in_0_demux_min_30 = gen_min_mul_buses_5.out_demux_min_0;
                        assign gen_out_buses_5_6_7_19.in_1_demux_min_30 = gen_out_buses_5_6_7_19.out_min_0;

                        assign gen_out_buses_5_6_7_19.in_0_min_1 = gen_out_buses_5_6_7_19.out_demux_min_30;
                        assign gen_out_buses_5_6_7_19.in_1_min_1 = gen_min_mul_buses_5.out_demux_min_2;
                        assign gen_out_buses_5_6_7_19.in_0_demux_min_31 = gen_out_buses_5_6_7_19.out_demux_min_30;
                        assign gen_out_buses_5_6_7_19.in_1_demux_min_31 = gen_out_buses_5_6_7_19.out_min_1;

                        assign gen_out_buses_5_6_7_19.in_0_mul_0 = gen_min_mul_buses_5.out_demux_mul_0;
                        assign gen_out_buses_5_6_7_19.in_1_mul_0 = gen_min_mul_buses_5.out_demux_mul_1;
                        assign gen_out_buses_5_6_7_19.in_0_demux_mul_30 = gen_min_mul_buses_5.out_demux_mul_0;
                        assign gen_out_buses_5_6_7_19.in_1_demux_mul_30 = gen_out_buses_5_6_7_19.out_mul_0;

                        assign gen_out_buses_5_6_7_19.in_0_mul_1 = gen_out_buses_5_6_7_19.out_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.in_1_mul_1 = gen_min_mul_buses_5.out_demux_mul_2;
                        assign gen_out_buses_5_6_7_19.in_0_demux_mul_31 = gen_out_buses_5_6_7_19.out_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.in_1_demux_mul_31 = gen_out_buses_5_6_7_19.out_mul_1;

                        assign gen_out_buses_5_6_7_19.in_0_mul_out = gen_out_buses_5_6_7_19.out_demux_min_31;
                        assign gen_out_buses_5_6_7_19.in_1_mul_out = gen_out_buses_5_6_7_19.out_demux_mul_31;
                        //assign out_llr_r = gen_out_buses_5_6_7_19.out_mul_out;
                        assign out_llr = gen_out_buses_5_6_7_19.out_mul_out;
                    end
                    6:
                    begin : out_logic_assignments_6
                        assign gen_out_buses_5_6_7_19.in_0_min_0 = gen_min_mul_buses_6.out_demux_min_0;
                        assign gen_out_buses_5_6_7_19.in_1_min_0 = gen_min_mul_buses_6.out_demux_min_1;
                        assign gen_out_buses_5_6_7_19.in_0_demux_min_30 = gen_min_mul_buses_6.out_demux_min_0;
                        assign gen_out_buses_5_6_7_19.in_1_demux_min_30 = gen_out_buses_5_6_7_19.out_min_0;

                        assign gen_out_buses_5_6_7_19.in_0_min_1 = gen_out_buses_5_6_7_19.out_demux_min_30;
                        assign gen_out_buses_5_6_7_19.in_1_min_1 = gen_min_mul_buses_6.out_demux_min_2;
                        assign gen_out_buses_5_6_7_19.in_0_demux_min_31 = gen_out_buses_5_6_7_19.out_demux_min_30;
                        assign gen_out_buses_5_6_7_19.in_1_demux_min_31 = gen_out_buses_5_6_7_19.out_min_1;

                        assign gen_out_buses_5_6_7_19.in_0_mul_0 = gen_min_mul_buses_6.out_demux_mul_0;
                        assign gen_out_buses_5_6_7_19.in_1_mul_0 = gen_min_mul_buses_6.out_demux_mul_1;
                        assign gen_out_buses_5_6_7_19.in_0_demux_mul_30 = gen_min_mul_buses_6.out_demux_mul_0;
                        assign gen_out_buses_5_6_7_19.in_1_demux_mul_30 = gen_out_buses_5_6_7_19.out_mul_0;

                        assign gen_out_buses_5_6_7_19.in_0_mul_1 = gen_out_buses_5_6_7_19.out_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.in_1_mul_1 = gen_min_mul_buses_6.out_demux_mul_2;
                        assign gen_out_buses_5_6_7_19.in_0_demux_mul_31 = gen_out_buses_5_6_7_19.out_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.in_1_demux_mul_31 = gen_out_buses_5_6_7_19.out_mul_1;

                        assign gen_out_buses_5_6_7_19.in_0_mul_out = gen_out_buses_5_6_7_19.out_demux_min_31;
                        assign gen_out_buses_5_6_7_19.in_1_mul_out = gen_out_buses_5_6_7_19.out_demux_mul_31;
                        //assign out_llr_r = gen_out_buses_5_6_7_19.out_mul_out;
                        assign out_llr = gen_out_buses_5_6_7_19.out_mul_out;
                    end
                    7:
                    begin : out_logic_assignments_7
                        assign gen_out_buses_5_6_7_19.in_0_min_0 = gen_min_mul_buses_7.out_demux_min_0;
                        assign gen_out_buses_5_6_7_19.in_1_min_0 = gen_min_mul_buses_7.out_demux_min_1;
                        assign gen_out_buses_5_6_7_19.in_0_demux_min_30 = gen_min_mul_buses_7.out_demux_min_0;
                        assign gen_out_buses_5_6_7_19.in_1_demux_min_30 = gen_out_buses_5_6_7_19.out_min_0;

                        assign gen_out_buses_5_6_7_19.in_0_min_1 = gen_out_buses_5_6_7_19.out_demux_min_30;
                        assign gen_out_buses_5_6_7_19.in_1_min_1 = gen_min_mul_buses_7.out_demux_min_2;
                        assign gen_out_buses_5_6_7_19.in_0_demux_min_31 = gen_out_buses_5_6_7_19.out_demux_min_30;
                        assign gen_out_buses_5_6_7_19.in_1_demux_min_31 = gen_out_buses_5_6_7_19.out_min_1;

                        assign gen_out_buses_5_6_7_19.in_0_mul_0 = gen_min_mul_buses_7.out_demux_mul_0;
                        assign gen_out_buses_5_6_7_19.in_1_mul_0 = gen_min_mul_buses_7.out_demux_mul_1;
                        assign gen_out_buses_5_6_7_19.in_0_demux_mul_30 = gen_min_mul_buses_7.out_demux_mul_0;
                        assign gen_out_buses_5_6_7_19.in_1_demux_mul_30 = gen_out_buses_5_6_7_19.out_mul_0;

                        assign gen_out_buses_5_6_7_19.in_0_mul_1 = gen_out_buses_5_6_7_19.out_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.in_1_mul_1 = gen_min_mul_buses_7.out_demux_mul_2;
                        assign gen_out_buses_5_6_7_19.in_0_demux_mul_31 = gen_out_buses_5_6_7_19.out_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.in_1_demux_mul_31 = gen_out_buses_5_6_7_19.out_mul_1;

                        assign gen_out_buses_5_6_7_19.in_0_mul_out = gen_out_buses_5_6_7_19.out_demux_min_31;
                        assign gen_out_buses_5_6_7_19.in_1_mul_out = gen_out_buses_5_6_7_19.out_demux_mul_31;
                        //assign out_llr_r = gen_out_buses_5_6_7_19.out_mul_out;
                        assign out_llr = gen_out_buses_5_6_7_19.out_mul_out;
                    end
                    19:
                    begin : out_logic_assignments_19
                        assign gen_out_buses_5_6_7_19.in_0_min_0 = gen_min_mul_buses_19.out_demux_min_0;
                        assign gen_out_buses_5_6_7_19.in_1_min_0 = gen_min_mul_buses_19.out_demux_min_1;
                        assign gen_out_buses_5_6_7_19.in_0_demux_min_30 = gen_min_mul_buses_19.out_demux_min_0;
                        assign gen_out_buses_5_6_7_19.in_1_demux_min_30 = gen_out_buses_5_6_7_19.out_min_0;

                        assign gen_out_buses_5_6_7_19.in_0_min_1 = gen_out_buses_5_6_7_19.out_demux_min_30;
                        assign gen_out_buses_5_6_7_19.in_1_min_1 = gen_min_mul_buses_19.out_demux_min_2;
                        assign gen_out_buses_5_6_7_19.in_0_demux_min_31 = gen_out_buses_5_6_7_19.out_demux_min_30;
                        assign gen_out_buses_5_6_7_19.in_1_demux_min_31 = gen_out_buses_5_6_7_19.out_min_1;

                        assign gen_out_buses_5_6_7_19.in_0_mul_0 = gen_min_mul_buses_19.out_demux_mul_0;
                        assign gen_out_buses_5_6_7_19.in_1_mul_0 = gen_min_mul_buses_19.out_demux_mul_1;
                        assign gen_out_buses_5_6_7_19.in_0_demux_mul_30 = gen_min_mul_buses_19.out_demux_mul_0;
                        assign gen_out_buses_5_6_7_19.in_1_demux_mul_30 = gen_out_buses_5_6_7_19.out_mul_0;

                        assign gen_out_buses_5_6_7_19.in_0_mul_1 = gen_out_buses_5_6_7_19.out_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.in_1_mul_1 = gen_min_mul_buses_19.out_demux_mul_2;
                        assign gen_out_buses_5_6_7_19.in_0_demux_mul_31 = gen_out_buses_5_6_7_19.out_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.in_1_demux_mul_31 = gen_out_buses_5_6_7_19.out_mul_1;

                        assign gen_out_buses_5_6_7_19.in_0_mul_out = gen_out_buses_5_6_7_19.out_demux_min_31;
                        assign gen_out_buses_5_6_7_19.in_1_mul_out = gen_out_buses_5_6_7_19.out_demux_mul_31;
                        //assign out_llr_r = gen_out_buses_5_6_7_19.out_mul_out;
                        assign out_llr = gen_out_buses_5_6_7_19.out_mul_out;
                    end
                endcase
                case (sel_demux_min_30)
                    0:
                        //assign gen_out_buses_5_6_7_19.out_demux_min_30_r = gen_out_buses_5_6_7_19.in_0_demux_min_30;
                        assign gen_out_buses_5_6_7_19.out_demux_min_30 = gen_out_buses_5_6_7_19.in_0_demux_min_30;
                    default:
                        //assign gen_out_buses_5_6_7_19.out_demux_min_30_r = gen_out_buses_5_6_7_19.in_1_demux_min_30;
                        assign gen_out_buses_5_6_7_19.out_demux_min_30 = gen_out_buses_5_6_7_19.in_1_demux_min_30;
                endcase

                case (sel_demux_min_31)
                    0:
                        //assign gen_out_buses_5_6_7_19.out_demux_min_31_r = gen_out_buses_5_6_7_19.in_0_demux_min_31;
                        assign gen_out_buses_5_6_7_19.out_demux_min_31 = gen_out_buses_5_6_7_19.in_0_demux_min_31;
                    default:
                        //assign gen_out_buses_5_6_7_19.out_demux_min_31_r = gen_out_buses_5_6_7_19.in_1_demux_min_31;
                        assign gen_out_buses_5_6_7_19.out_demux_min_31 = gen_out_buses_5_6_7_19.in_1_demux_min_31;
                endcase

                case (sel_demux_mul_30)
                    0:
                        //assign gen_out_buses_5_6_7_19.out_demux_mul_30_r = gen_out_buses_5_6_7_19.in_0_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.out_demux_mul_30 = gen_out_buses_5_6_7_19.in_0_demux_mul_30;
                    default:
                        //assign gen_out_buses_5_6_7_19.out_demux_mul_30_r = gen_out_buses_5_6_7_19.in_1_demux_mul_30;
                        assign gen_out_buses_5_6_7_19.out_demux_mul_30 = gen_out_buses_5_6_7_19.in_1_demux_mul_30;
                endcase

                case (sel_demux_mul_31)
                    0:
                        //assign gen_out_buses_5_6_7_19.out_demux_mul_31_r = gen_out_buses_5_6_7_19.in_0_demux_mul_31;
                        assign gen_out_buses_5_6_7_19.out_demux_mul_31 = gen_out_buses_5_6_7_19.in_0_demux_mul_31;
                    default:
                        //assign gen_out_buses_5_6_7_19.out_demux_mul_31_r = gen_out_buses_5_6_7_19.in_1_demux_mul_31;
                        assign gen_out_buses_5_6_7_19.out_demux_mul_31 = gen_out_buses_5_6_7_19.in_1_demux_mul_31;
                endcase
            end
            default:
                //assign out_llr_r = {(WIDTH){1'bx}}; //default
                assign out_llr = {(WIDTH){1'bx}}; //default
        endcase
    endgenerate

    generate 
        case (DEGREE)
            4:
            begin : gen_out_buses_4_assignments_final
                assign gen_out_buses_4.out_demux_min_30 = gen_out_buses_4.out_demux_min_30_r; 
                assign gen_out_buses_4.out_demux_mul_30 = gen_out_buses_4.out_demux_mul_30_r;
            end
            //5 , 6 , 7 , 19:
            //begin : gen_out_buses_5_6_7_19_assignments_final
            //    assign gen_out_buses_5_6_7_19.out_demux_min_30 = gen_out_buses_5_6_7_19.out_demux_min_30_r; 
            //    assign gen_out_buses_5_6_7_19.out_demux_min_31 = gen_out_buses_5_6_7_19.out_demux_min_31_r; 
            //    assign gen_out_buses_5_6_7_19.out_demux_mul_30 = gen_out_buses_5_6_7_19.out_demux_mul_30_r;
            //    assign gen_out_buses_5_6_7_19.out_demux_mul_31 = gen_out_buses_5_6_7_19.out_demux_mul_31_r;
            //end
        endcase
    endgenerate
    
    //assign out_llr = out_llr_r; 

endmodule
