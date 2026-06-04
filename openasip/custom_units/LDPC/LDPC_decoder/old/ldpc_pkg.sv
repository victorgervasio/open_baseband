// ldpc_pkg.sv - Libero-compatible version
package ldpc_pkg;
    
    // Function to get Zc value from 3GPP TS 38.212 Table 5.3.2-1
    function automatic int get_zc(input int i_ls, input int idx);
        begin
            get_zc = 0;
            case (i_ls)
                0: case (idx)
                    0: get_zc = 2;
                    1: get_zc = 4;
                    2: get_zc = 8;
                    3: get_zc = 16;
                    4: get_zc = 32;
                    5: get_zc = 64;
                    6: get_zc = 128;
                    7: get_zc = 256;
                endcase
                1: case (idx)
                    0: get_zc = 3;
                    1: get_zc = 6;
                    2: get_zc = 12;
                    3: get_zc = 24;
                    4: get_zc = 48;
                    5: get_zc = 96;
                    6: get_zc = 192;
                    7: get_zc = 384;
                endcase
                2: case (idx)
                    0: get_zc = 5;
                    1: get_zc = 10;
                    2: get_zc = 20;
                    3: get_zc = 40;
                    4: get_zc = 80;
                    5: get_zc = 160;
                    6: get_zc = 320;
                endcase
                3: case (idx)
                    0: get_zc = 7;
                    1: get_zc = 14;
                    2: get_zc = 28;
                    3: get_zc = 56;
                    4: get_zc = 112;
                    5: get_zc = 224;
                endcase
                4: case (idx)
                    0: get_zc = 9;
                    1: get_zc = 18;
                    2: get_zc = 36;
                    3: get_zc = 72;
                    4: get_zc = 144;
                    5: get_zc = 288;
                endcase
                5: case (idx)
                    0: get_zc = 11;
                    1: get_zc = 22;
                    2: get_zc = 44;
                    3: get_zc = 88;
                    4: get_zc = 176;
                    5: get_zc = 352;
                endcase
                6: case (idx)
                    0: get_zc = 13;
                    1: get_zc = 26;
                    2: get_zc = 52;
                    3: get_zc = 104;
                    4: get_zc = 208;
                endcase
                7: case (idx)
                    0: get_zc = 15;
                    1: get_zc = 30;
                    2: get_zc = 60;
                    3: get_zc = 120;
                    4: get_zc = 240;
                endcase
            endcase
        end
    endfunction
    
    function automatic int get_zc_count(input int i_ls);
        begin
            case (i_ls)
                0,1:   get_zc_count = 8;
                2:     get_zc_count = 7;
                3,4,5: get_zc_count = 6;
                6,7:   get_zc_count = 5;
                default: get_zc_count = 0;
            endcase
        end
    endfunction
    
endpackage
