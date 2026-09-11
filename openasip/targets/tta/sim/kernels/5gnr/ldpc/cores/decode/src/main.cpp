#include "LDPC.h"
#include <stdio.h> //OpenASIP supports it according to the manual
using namespace std;

void decode_blk(etl::vector<float, CODE_WORD_BITS_LENGTH>& llr, etl::vector<bool, MAX_INFO_NODE_BITS>& msg_cap);

int main() {
    // 1. DECLARATION: Map the structures directly onto the shared memory regions
    volatile const unsigned int* snr_g_id_ptr = SNR_G_ID_PTR;
    
    float* llr_raw      = reinterpret_cast<float*>(LLR_ADDR);
    uint8_t* msgCap_raw = reinterpret_cast<uint8_t*>(MSG_CAP_ADDR);
    etl::vector<float, CODE_WORD_BITS_LENGTH> llr;
    etl::vector<bool, MAX_INFO_NODE_BITS> msg_cap;

    volatile unsigned int sim_time = 0;
    unsigned int step_time_start = 0;
    unsigned int step_time_end = 0;
    do {
        while (*DEC_STATUS_PTR) {} // Wait for Top-Level to clear the flag to 0, which signals "New Data Ready to Encode"
        // 2. ATTRIBUTION/READING: Fetch the updated primitive values at the start of the loop
        unsigned int snr_g_id = *snr_g_id_ptr; 
        
        llr.clear();
        for (size_t i = 0; i < *ACTUAL_CODEWORD_LENGTH_PTR; ++i) llr.push_back(llr_raw[i]);

        _TCE_RTC(1, sim_time); 
        printf("[t_sim [s] = %.6f][SNR_0%u] Starting decode_blk\n", sim_time / 1e6, snr_g_id);
        _TCE_RTC(1, step_time_start); 
        // This will write directly into MSG_CAP_PTR because msg_cap is a reference
        decode_blk(llr, msg_cap); 
        for (size_t i = 0; i < msg_cap.size(); ++i) msgCap_raw[i] = msg_cap[i] ? 1 : 0;
        *DEC_STATUS_PTR = 1; // Instantly notify Top-Level that writing is completed

        _TCE_RTC(1, step_time_end); 
        _TCE_RTC(1, sim_time); 
        printf("[t_sim [s] = %.6f][SNR_0%u] Ended decode_blk\n", sim_time / 1e6, snr_g_id);
        printf("[t_sim [s] = %.6f][SNR_0%u] decode_blk elapsed time [s]: %.6f\n", sim_time / 1e6, snr_g_id, (step_time_end - step_time_start) / 1e6);
    } while(1);
    return 0;
}

void decode_blk(etl::vector<float,CODE_WORD_BITS_LENGTH>& llr, etl::vector<bool,MAX_INFO_NODE_BITS>& msg_cap)
{
    volatile const unsigned int* snr_g_id_ptr = SNR_G_ID_PTR;
    volatile unsigned int sim_time;
    unsigned int step_time_start = 0;
    unsigned int step_time_end = 0;
    // rate recovery
    volatile const unsigned int snr_g_id = *snr_g_id_ptr;
    nrLDPC* ldpc = reinterpret_cast<nrLDPC*>(NRLDPC_ADDR);
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i] Started rate recover\n",sim_time/1e6,snr_g_id);
    _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
    etl::vector<float,MAX_CODEWORD_LENGTH> rr_llr = ldpc->rateRecover(llr);
    _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i] Ended rate recover\n",sim_time/1e6,snr_g_id);
    printf("[t_sim [s] = %.6f][SNR_0%i] rate recover elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,(step_time_end - step_time_start)/1e6);

    // scl decoding
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i] Started decode\n",sim_time/1e6,snr_g_id);
    _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
    msg_cap = ldpc->decode(rr_llr, N_MAX_ITER);
    _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i] Ended decode\n",sim_time/1e6,snr_g_id);
    printf("[t_sim [s] = %.6f][SNR_0%i] decode elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,(step_time_end - step_time_start)/1e6);
}
