#include "LDPC.h"
#include <stdio.h> //OpenASIP supports it according to the manual
using namespace std;

void initializeLDPC(size_t infoLen, float codeRate);
void encode_blk(const etl::vector<bool,MAX_INFO_NODE_BITS>& extMsg, etl::vector<bool,CODE_WORD_BITS_LENGTH>& rm_enc);

int main() {
    // 1. DECLARATION: Map the structures directly onto the shared memory regions
    volatile const unsigned int* snr_g_id_ptr = SNR_G_ID_PTR;

    uint8_t* extMsg_raw = reinterpret_cast<uint8_t*>(EXT_MSG_ADDR);
    uint8_t* rmEnc_raw  = reinterpret_cast<uint8_t*>(RM_ENC_ADDR);
    etl::vector<bool, MAX_INFO_NODE_BITS> extMsg;
    etl::vector<bool,CODE_WORD_BITS_LENGTH> rm_enc;

    volatile unsigned int sim_time = 0;
    unsigned int step_time_start = 0;
    unsigned int step_time_end = 0;
    do {
        if (*STARTUP_PTR) { 
            initializeLDPC(INFO_BITS_LENGTH, CODE_RATE); 
            *FILLER_LENGTH_PTR = reinterpret_cast<nrLDPC*>(NRLDPC_ADDR)->getFillerLength();
            *STARTUP_PTR = 0;
        }
        while (*ENC_STATUS_PTR) {} // Wait for Top-Level to clear the flag to 0, which signals "New Data Ready to Encode"
        // 2. ATTRIBUTION/READING: Fetch the updated primitive values at the start of the loop
        unsigned int snr_g_id  = *snr_g_id_ptr;

        extMsg.clear();
        for (size_t i = 0; i < EXT_MSG_LENGTH; ++i) extMsg.push_back(extMsg_raw[i] != 0);

        _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
        printf("[t_sim [s] = %.6f][SNR_0%i] Starting encode_blk\n",sim_time/1e6,snr_g_id);
        _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
        // This will write directly into RM_ENC_PTR because rm_enc is a reference
        encode_blk(extMsg, rm_enc);
        for (size_t i = 0; i < rm_enc.size(); ++i) rmEnc_raw[i] = rm_enc[i] ? 1 : 0;
        *ENC_STATUS_PTR = 1; // Instantly notify Top-Level that writing is completed

        _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
        _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
        printf("[t_sim [s] = %.6f][SNR_0%i] Ended encode_blk\n",sim_time/1e6,snr_g_id);
        printf("[t_sim [s] = %.6f][SNR_0%i] encode_blk elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,(step_time_end - step_time_start)/1e6);
    } while(1);
	return 0;
}

void encode_blk(const etl::vector<bool, MAX_INFO_NODE_BITS>& extMsg, etl::vector<bool,CODE_WORD_BITS_LENGTH>& rm_enc)
{
    volatile const unsigned int* snr_g_id_ptr = SNR_G_ID_PTR;
    volatile unsigned int sim_time;
    unsigned int step_time_start = 0;
    unsigned int step_time_end = 0;
    // LDPC encoding
    volatile const unsigned int snr_g_id = *snr_g_id_ptr;
    nrLDPC* ldpc = reinterpret_cast<nrLDPC*>(NRLDPC_ADDR);
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i] Started encode\n",sim_time/1e6,snr_g_id);
    _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
    etl::vector<bool,MAX_CODEWORD_LENGTH> enc = ldpc->encode(extMsg);
    _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i] Ended encode\n",sim_time/1e6,snr_g_id);
    printf("[t_sim [s] = %.6f][SNR_0%i] encode elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,(step_time_end - step_time_start)/1e6);

    assert(ldpc->checkSumCodeWord(enc));

    //rate matching
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i] Started rate matching\n",sim_time/1e6,snr_g_id);
    _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
    rm_enc = ldpc->rateMatch(enc, CODE_WORD_BITS_LENGTH);
    _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i] Ended rate matching\n",sim_time/1e6,snr_g_id);
    printf("[t_sim [s] = %.6f][SNR_0%i] rate matching elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,(step_time_end - step_time_start)/1e6);
}

void initializeLDPC(size_t infoLen, float codeRate)
{
    nrLDPC* ldpc = new (reinterpret_cast<void*>(NRLDPC_ADDR))nrLDPC(infoLen, codeRate);
}
