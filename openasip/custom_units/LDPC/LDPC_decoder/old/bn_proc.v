module bn_proc
#(
    parameter GROUP = 6,           // Number of CNs per BN (degree d_v)
    parameter WIDTH = 8            // Bit width (int8)
)
(
    input  signed [GROUP*WIDTH-1:0] in_llr,    // Flattened input per BN
    // (GROUP*WIDTH-1):(GROUP-1)*WIDTH : [q(GROUP-1)(WIDTH-1),...,q(GROUP-1)1,q(GROUP-1)0]
    // ...
    // (2*WIDTH-1)    :WIDTH           : [q1(WIDTH-1)        ,...,q11        ,q10        ]
    // (WIDTH-1)      :0               : [q0(WIDTH-1)        ,...,q01        ,q00        ]
    
    output signed [GROUP*WIDTH-1:0] out_llr   // Flattened output per BN
    // (GROUP*WIDTH-1):(GROUP-1)*WIDTH : [r(WIDTH-1)(GROUP-1),...,r1(GROUP-1),r0(GROUP-1)]
    // ...
    // (2*WIDTH-1)    :WIDTH           : [r(WIDTH-1)1        ,...,r11        ,r01        ]
    // (WIDTH-1)      :0               : [r(WIDTH-1)0        ,...,r10        ,r00        ]
);

    // Internal arrays
    reg signed [WIDTH-1:0] lambda [0:GROUP-1];          // lambda per BN (from input)
    // (GROUP*WIDTH-1):(GROUP-1)*WIDTH : [q(GROUP-1)(WIDTH-1),...,q(GROUP-1)1,q(GROUP-1)0]
    // ...
    // (2*WIDTH-1)    :WIDTH           : [q1(WIDTH-1)        ,...,q11        ,q10        ]
    // (WIDTH-1)      :0               : [q0(WIDTH-1)        ,...,q01        ,q00        ]
    
    reg signed [WIDTH-1:0] r [0:GROUP-1][0:GROUP-2];
    reg signed [WIDTH*2-1:0] total_sum [0:GROUP-1];
    
    integer i, j;
    
    //----------------------------------------
    // Unpack inputs using generate
    //----------------------------------------
    genvar g_i, g_j;
    
    generate
        for (g_j = 0; g_j < GROUP; g_j = g_j + 1) begin : unpack_gen
            // Lambda is first element for each BN
            always @(*) begin
                lambda[g_j] = in_llr[g_j*GROUP*WIDTH +: WIDTH];
            end
            
            // CN messages are the remaining GROUP-1 elements
            for (g_i = 0; g_i < GROUP-1; g_i = g_i + 1) begin : unpack_r
                always @(*) begin
                    r[g_j][g_i] = in_llr[g_j*GROUP*WIDTH + (g_i+1)*WIDTH +: WIDTH];
                end
            end
        end
    endgenerate
    
    //----------------------------------------
    // Calculate total sum per BN (lambda + all CN messages)
    //----------------------------------------
    always @(*) begin : calc_total_sum
        for (j = 0; j < GROUP; j = j + 1) begin
            // Start with lambda
            total_sum[j] = {{WIDTH{lambda[j][WIDTH-1]}}, lambda[j]};
            
            // Add all CN messages
            for (i = 0; i < GROUP-1; i = i + 1) begin
                total_sum[j] = total_sum[j] + {{WIDTH{r[j][i][WIDTH-1]}}, r[j][i]};
            end
        end
    end
    
//    //----------------------------------------
//    // Calculate sum excluding each CN message
//    // For output to CN i: total_sum - r_i
//    //----------------------------------------
//    always @(*) begin : calc_excluding
//        for (j = 0; j < GROUP; j = j + 1) begin
//            for (i = 0; i < GROUP-1; i = i + 1) begin
//                sum_excluding[j][i] = total_sum[j] - {{WIDTH{r[j][i][WIDTH-1]}}, r[j][i]};
//            end
//        end
//    end
    
    //----------------------------------------
    // Saturation function
    //----------------------------------------
    function signed [WIDTH-1:0] saturate;
        input signed [WIDTH*2-1:0] value;
        reg [WIDTH*2-1:0] max_pos;
        reg [WIDTH*2-1:0] max_neg;
        reg [WIDTH*2-1:0] neg_max;
        begin
            max_pos = (1 << WIDTH) - 1;
            max_neg = (1 << (WIDTH-1));
            neg_max = -max_neg;
            
            if (value > max_pos)
                saturate = max_pos[WIDTH-1:0];
            else if (value < -max_neg)
                saturate = neg_max[WIDTH-1:0];
            else
                saturate = value[WIDTH-1:0];
        end
    endfunction

    wire signed [WIDTH-1:0] q_out_flat [0:GROUP*(GROUP-1)-1];
   //----------------------------------------
    // Calculate outputs using continuous assignment
    //----------------------------------------
    generate
        for (g_j = 0; g_j < GROUP; g_j = g_j + 1) begin : calc_gen
            for (g_i = 0; g_i < GROUP-1; g_i = g_i + 1) begin : calc_out
                assign q_out_flat[g_j*(GROUP-1) + g_i] = saturate(total_sum[g_j] - {{WIDTH{r[g_j][g_i][WIDTH-1]}}, r[g_j][g_i]});
            end
        end
    endgenerate
    
    //----------------------------------------
    // Pack outputs using generate
    //----------------------------------------
    generate
        for (g_j = 0; g_j < GROUP; g_j = g_j + 1) begin : pack_gen
            for (g_i = 0; g_i < GROUP-1; g_i = g_i + 1) begin : pack_out
                assign out_llr[(g_j*(GROUP-1) + g_i)*WIDTH +: WIDTH] = q_out_flat[g_j*(GROUP-1) + g_i];
            end
        end
    endgenerate

endmodule
