module cn_proc 
#(
    parameter GROUP = 3,
    parameter WIDTH = 8
)
(
    input signed [(GROUP-1)*WIDTH-1:0] in_llr, // already without {qij': j' == j}
    output signed [WIDTH-1:0] min_llr,
    output signed [WIDTH-1:0] sgn_mul_llr
);
    
    integer i;

    reg [WIDTH-1:0] abs_val [0:GROUP-2];
    always @(*) begin : calculate_abs_qij
        for (i = 0; i < GROUP-1; i = i + 1) begin
            if (in_llr[(i + 1)*WIDTH - 1] == 1'b1)
                abs_val[i] = -in_llr[i*WIDTH +: WIDTH];
            else
                abs_val[i] = in_llr[i*WIDTH +: WIDTH];
        end
    end

    reg [WIDTH-1:0] min;
    always @(*) begin : find_min_qij
        min = {WIDTH{1'b1}};
        for (i = 0; i < GROUP-1; i = i + 1) begin
            if (abs_val[i] < min)
                min = abs_val[i];
            else
                min = min; // avoiding latches
        end
    end
    
    reg prod_sgn_llr;
    always @(*) begin: find_prod_sgn_llr
        prod_sgn_llr = 1'b0;
        for (i = 0; i < GROUP-1; i = i + 1) begin
            prod_sgn_llr = prod_sgn_llr ^ in_llr[(i + 1)*WIDTH - 1]; // MSB = sign 
        end
    end

    assign min_llr = min;
    assign sgn_mul_llr = prod_sgn_llr;
    
endmodule
