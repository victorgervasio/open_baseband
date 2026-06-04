`timescale 1ns/1ps

module tb_cn_proc;

    // -----------------------------------------
    // Parameters
    // -----------------------------------------
    parameter GROUP = 6;
    parameter WIDTH = 8;

    // -----------------------------------------
    // DUT I/O
    // -----------------------------------------
    reg  signed [GROUP*WIDTH-1:0] in_llr;
    wire signed [GROUP*WIDTH-1:0] out_llr;

    // -----------------------------------------
    // Instantiate DUT
    // -----------------------------------------
    cn_proc #(
        .GROUP(GROUP),
        .WIDTH(WIDTH)
    ) dut (
        .in_llr(in_llr),
        .out_llr(out_llr)
    );

    // -----------------------------------------
    // Reference model variables
    // -----------------------------------------
    integer i, j;

    reg [WIDTH-1:0] abs_val [0:GROUP-1];
    reg             sign_val[0:GROUP-1];

    reg [WIDTH-1:0] min1, min2;
    integer min1_idx;

    reg total_sign;

    reg signed [WIDTH-1:0] expected [0:GROUP-1];

    // temp variables (DECLARED HERE - Verilog 2001 compliant)
    reg [WIDTH-1:0] mag;
    reg sign;

    // -----------------------------------------
    // Reference model task
    // -----------------------------------------
    task compute_reference;
    begin
        // Extract abs + sign
        for (i = 0; i < GROUP; i = i + 1) begin
            if (in_llr[i*WIDTH + WIDTH-1] == 1'b1)
                abs_val[i] = -in_llr[i*WIDTH +: WIDTH];
            else
                abs_val[i] =  in_llr[i*WIDTH +: WIDTH];

            sign_val[i] = in_llr[i*WIDTH + WIDTH-1];
        end

        // Find min1 and min2
        min1 = {WIDTH{1'b1}};
        min2 = {WIDTH{1'b1}};
        min1_idx = 0;

        for (i = 0; i < GROUP; i = i + 1) begin
            if (abs_val[i] < min1) begin
                min2 = min1;
                min1 = abs_val[i];
                min1_idx = i;
            end
            else if (abs_val[i] < min2) begin
                min2 = abs_val[i];
            end
        end

        // Total sign (XOR of all signs)
        total_sign = 1'b0;
        for (i = 0; i < GROUP; i = i + 1)
            total_sign = total_sign ^ sign_val[i];

        // Compute expected outputs
        for (i = 0; i < GROUP; i = i + 1) begin
            if (i == min1_idx)
                mag = min2;
            else
                mag = min1;

            sign = total_sign ^ sign_val[i];

            if (sign == 1'b1)
                expected[i] = -mag;
            else
                expected[i] = mag;
        end
    end
    endtask

    // -----------------------------------------
    // Compare task
    // -----------------------------------------
    task check_output;
    begin
        for (j = 0; j < GROUP; j = j + 1) begin
            if (out_llr[j*WIDTH +: WIDTH] !== expected[j]) begin
                $display("MISMATCH index %0d: DUT=%0d EXPECTED=%0d",
                         j,
                         out_llr[j*WIDTH +: WIDTH],
                         expected[j]);
            end
            else begin
                $display("OK index %0d: %0d",
                         j,
                         out_llr[j*WIDTH +: WIDTH]);
            end
        end
        $display("-----------------------------------");
    end
    endtask

    // -----------------------------------------
    // Test sequence
    // -----------------------------------------
    initial begin
        $display("==== CN PROC TEST START ====");

        // Test 1
        in_llr = {
            8'sd3, -8'sd2, 8'sd7,
            -8'sd1, 8'sd4, -8'sd6
        };
        #5;
        compute_reference;
        check_output;

        // Test 2
        in_llr = {
            8'sd1, 8'sd2, 8'sd3,
            8'sd4, 8'sd5, 8'sd6
        };
        #5;
        compute_reference;
        check_output;

        // Test 3
        in_llr = {
            -8'sd1, -8'sd2, -8'sd3,
            -8'sd4, -8'sd5, -8'sd6
        };
        #5;
        compute_reference;
        check_output;

        // Test 4 (random)
        for (i = 0; i < GROUP; i = i + 1) begin
            in_llr[i*WIDTH +: WIDTH] = $random;
        end
        #5;
        compute_reference;
        check_output;

        $display("==== TEST END ====");
        $finish;
    end

endmodule
