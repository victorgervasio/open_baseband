module cn_proc
#(
    parameter GROUP = 6,          // Number of BNs per CN
    parameter WIDTH = 8           // Bit width (e.g., int8)
)
(
    input  signed [GROUP*WIDTH-1:0] in_llr,   // Flattened input vector
    // (GROUP*WIDTH-1):(GROUP-1)*WIDTH : [q(GROUP-1)(WIDTH-1),...,q(GROUP-1)1,q(GROUP-1)0]
    // ...
    // (2*WIDTH-1)    :WIDTH           : [q1(WIDTH-1)        ,...,q11        ,q10        ]
    // (WIDTH-1)      :0               : [q0(WIDTH-1)        ,...,q01        ,q00        ]

    output signed [GROUP*WIDTH-1:0] out_llr   // Flattened output vector
    // (GROUP*WIDTH-1):(GROUP-1)*WIDTH : [r(WIDTH-1)(GROUP-1),...,r1(GROUP-1),r0(GROUP-1)]
    // ...
    // (2*WIDTH-1)    :WIDTH           : [r(WIDTH-1)1        ,...,r11        ,r01        ]
    // (WIDTH-1)      :0               : [r(WIDTH-1)0        ,...,r10        ,r00        ]
);

    reg [WIDTH-1:0] abs_val [0:GROUP-1]; // to hold abs val of every qij
    // (GROUP*WIDTH-1):(GROUP-1)*WIDTH : [q(GROUP-1)(WIDTH-1),...,q(GROUP-1)1,q(GROUP-1)0]
    // ...
    // (2*WIDTH-1)    :WIDTH           : [q1(WIDTH-1)        ,...,q11        ,q10        ]
    // (WIDTH-1)      :0               : [q0(WIDTH-1)        ,...,q01        ,q00        ]
    integer i;
    
    // Calculates abs(qij)    
    always @(*) begin : calculate_abs_qij
        for (i = 0; i < GROUP; i = i + 1) begin
            if (in_llr[(i + 1)*WIDTH - 1] == 1'b1) // qij MSB == 1 (negative value)
                abs_val[i] = -in_llr[i*WIDTH +: WIDTH]; // index part-select
            else
                abs_val[i] = in_llr[i*WIDTH +: WIDTH]; // index part-select
        end
    end
    
    // Find min{abs(qij)}
    reg [WIDTH-1:0] min;
    reg [$clog2(GROUP)-1:0] min_idx; // also stores the index to check later if
                                     // min value of all BN's doesn't
                                     // correspond to BN getting the final
                                     // output (min(qij'), 'j diff. j)
    reg [WIDTH-1:0] min2; // if min_idx correspond to index of BN getting the
                          // final output, gives it the second min instead
    always @(*) begin : find_min_qij
        min = {WIDTH{1'b1}}; // max possible value for abs(qij)
        min2 = {WIDTH{1'b1}}; // max possible value for abs(qij)
        min_idx = {$clog2(GROUP){1'b0}};

        for (i = 0; i < GROUP; i = i + 1) begin
            if (abs_val[i] < min) begin
                min2 = min;
                min = abs_val[i];
                min_idx = i;
            end else if (abs_val[i] < min2) begin
                min2 = abs_val[i];
            end
        end
    end

    //find prod(sgn(qij))
    reg sign_cn  [0:GROUP-1];
    reg total_sign;
    always @(*) begin : find_total_sign
        total_sign = 1'b0;
        for (i = 0; i < GROUP; i = i + 1) begin
            sign_cn[i] = in_llr[(i + 1)*WIDTH - 1]; // MSB = sign, sign_cn[i] is used below
            total_sign = total_sign ^ sign_cn[i];
         end
    end


    //calculate rji = min(abs(qij'))*prod(sgn(qij)), j' diff. j
    reg [WIDTH-1:0] mag_out [0:GROUP-1];
    reg sign_out  [0:GROUP-1];
    reg [GROUP*WIDTH-1:0] llr_out;
    always @(*) begin : calculate_rji
        for (i = 0; i < GROUP; i = i + 1) begin
        
            if (i == min_idx) begin
                mag_out[i] = min2;
            end else begin
                mag_out[i] = min;
            end

            sign_out[i] = total_sign ^ sign_cn[i]; // xoring with itself anulates the element
                                                   // effect on the xor
                                                   // operation (0 xor 0 = 0,
                                                   // 1 xor 1 = 0, X xor 0 = X;
                                                   // here X is total_sign) 
            if (sign_out[i]) begin
                llr_out[i*WIDTH +: WIDTH] = -mag_out[i];
            end else begin
                llr_out[i*WIDTH +: WIDTH] =  mag_out[i];
            end
        end
    end

    assign out_llr = llr_out;

endmodule
