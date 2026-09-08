#include "LDPC.h"
//#include <etl/random.h>
//#include <etl/chrono.h> //timing
//#include <time.h>
//#include <cstdio> // sprintf
#include <stdio.h> //OpenASIP supports it according to the manual
#include "norm_dist.h"
#include "n0_table.h"
using namespace std;

//using etl::chrono::duration_cast;
//using etl::chrono::steady_clock;
//using etl::chrono::milliseconds;

//etl::vector<float,LIN_SPACE_NUM> lin_space(float start, float ed);
template <size_t N> etl::vector<bool,MESSAGE_BITS_LENGTH> stringToBits(const char (&text)[N]);
template <size_t N> etl::array<char, MESSAGE_LENGTH> bitsToString(const etl::vector<bool, N>& bits);
void example_run_LDPC(NormDistRandomEngine norm_dist_random_engine);
using namespace std;

int main() {
	//std::cout << "Hello, 5G-NR!" << std::endl;
	printf("Hello, 5G-NR!\n");
    NormDistRandomEngine norm_dist_random_engine;
	example_run_LDPC(norm_dist_random_engine);
	return 0;
}

void example_run_LDPC(NormDistRandomEngine norm_dist_random_engine)
{
    volatile unsigned int sim_time = 0;
    volatile unsigned int step_time_start = 0;
    volatile unsigned int step_time_end = 0;
    volatile unsigned int ten_blk_elapsed_time_start = 0;
    volatile unsigned int ten_blk_elapsed_time_end = 0;
    volatile unsigned int one_snr_elapsed_time_start = 0;
    volatile unsigned int one_snr_elapsed_time_end = 0;
    /* Clear RTC and sim_time */
    _TCE_RTC(0, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    sim_time = 0;
    printf("[t_sim [s] = %.6f] Began simulated TTA CPU clock time count\n",sim_time/1e6);

    const char message[MESSAGE_LENGTH] = "Hello NR LDPC!";
    etl::vector<bool, MESSAGE_BITS_LENGTH> complete_message = stringToBits(message);
    const unsigned paddingBits = INFO_BITS_LENGTH - MESSAGE_BITS_LENGTH;
    //if (complete_message.size() > INFO_BITS_LENGTH) throw "[Runtime Error] Message too large."; aparently, openasip tte-llvm target won't support exception handling
    if (complete_message.size() > INFO_BITS_LENGTH) {
        printf("[Runtime Error] Message too large.\n");
        return;
    }

	// instantiates a POLAR object
	nrLDPC ldpc = nrLDPC(INFO_BITS_LENGTH, CODE_RATE);

	// random engines
    //default_random_engine random_engine;

	//bernoulli_distribution  bern_dist;
    //normal_distribution<float> norm_dist(0, 1);

	// Running parameters
    //etl::vector<float,LIN_SPACE_NUM> EsN0_dB = lin_space(-4, -2);
    etl::vector<float, LIN_SPACE_NUM> EsN0_dB(ESN0_DB_TABLE,ESN0_DB_TABLE + N0_TABLE_SIZE);
	//etl::vector<float,LIN_SPACE_NUM> N0(LIN_SPACE_NUM,0);
    etl::vector<float, LIN_SPACE_NUM> N0(N0_TABLE,N0_TABLE + N0_TABLE_SIZE);
    //etl::transform(EsN0_dB.begin(), EsN0_dB.end(), N0.begin(), [](const float& x) {return powf(10.0, -x / 10.0); });
    //etl::transform(EsN0_dB.begin(), EsN0_dB.end(), N0.begin(), [](const float& x) {return N0_TABLE[static_cast<int>(x)]; }); //pre-computated noise points
    //etl::transform(EsN0_dB.begin(), EsN0_dB.end(), N0.begin(), [](const float& x) {return powf(10.0, 10.0); });
    //etl::transform(EsN0_dB.begin(), EsN0_dB.end(), N0.begin(), [](const float& x) {return pow(10.0, -1.0 * x / 10.0); });
    //etl::transform(EsN0_dB.begin(), EsN0_dB.end(), N0.begin(), [](const float& x) {return 1.0; });

    etl::vector<float,LIN_SPACE_NUM>   ber(N0.size(), 0), bler(N0.size(), 0);
	etl::vector<unsigned,LIN_SPACE_NUM> n_bit_errs(N0.size(), 0), n_blk_errs(N0.size(), 0);
    etl::vector<bool,MAX_INFO_NODE_BITS> fillers(ldpc.getFillerLength(), 0); // For 'Hello NR LDPC!' -> ldpc.getFillerLength() = 200

	// loop each SNR
	for (unsigned i = 0; i < N0.size(); i++) {
        snr_g = i;
		//print progress
		//char str[100];
		//sprintf_s(str, "\nNow running EsN0: %.2f dB [%d of %lu]", EsN0_dB[i], i + 1, N0.size());
		printf("\nNow running EsN0: %.2f dB [%d of %lu]\n", EsN0_dB[i], i + 1, (unsigned long)N0.size());
		//cout << str << endl;
		//unsigned print_len = 0;

		unsigned n_blks_done = 0;
		//clock_t tStart = clock(); // timing
        //clock_t stepStart;
        

        bool printedFirstFailure = false;
        _TCE_RTC(1, ten_blk_elapsed_time_start); // OpenASIP 2.0 doc (search for printf explanation)
        _TCE_RTC(1, one_snr_elapsed_time_start); // OpenASIP 2.0 doc (search for printf explanation)
		while ((n_blks_done < N_MAX_BLKS) && (n_blk_errs[i] < 100)) {
            blk_g = n_blks_done;
			// generate random bit stream
            etl::vector<bool,INFO_BITS_LENGTH> msg(INFO_BITS_LENGTH,0);// = { 1,0,0,1,1,0,1,0,0,0,0,0,1,0,1,1,1,0,0,0,0,1,1,0,1,0,1,1,1,1,0,1,0,1 };
            etl::copy(complete_message.begin(), complete_message.end(), msg.begin());

			//msg.reserve(K);
			//for (unsigned j = 0; j < K; j++)
			//  msg.push_back(bern_dist(random_engine));

			// add filler bits
            etl::vector<bool,MAX_INFO_NODE_BITS> extMsg(msg.begin(),msg.end()); // For 'Hello NR LDPC!' -> ldpc.getFillerLength() = 200
			extMsg.insert(extMsg.end(), fillers.begin(), fillers.end());

			// LDPC encoding
            //printf("[SNR_0%i_t_sim [s] = %.5f] Started encode\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC);
            //stepStart = clock();
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i] Started encode\n",sim_time/1e6,i);
            _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
            etl::vector<bool,MAX_CODEWORD_LENGTH> enc = ldpc.encode(extMsg);
            _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
            //printf("[SNR_0%i_t_sim [s] = %.5f] Ended encode\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC);
            //printf("[SNR_0%i_t_sim [s] = %.5f] encode elapsed time: %.5f\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC,static_cast<float>(clock() - stepStart) / CLOCKS_PER_SEC);
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i] Ended encode\n",sim_time/1e6,i);
            printf("[t_sim [s] = %.6f][SNR_0%i] encode elapsed time [s]: %.6f\n",sim_time/1e6,i,(step_time_end - step_time_start)/1e6);
            /*DEBUG*/
            //etl::vector<float, MAX_CODEWORD_LENGTH> perfect_llr;
            //for (auto bit : enc) {
            //    perfect_llr.push_back(bit ? -10.0 : 10.0);
            //}
            /*DEBUG*/
			assert(ldpc.checkSumCodeWord(enc));

			//rate matching
            //printf("[SNR_0%i_t_sim = %.5f] Started rate matching\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC);
            //stepStart = clock();
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i] Started rate matching\n",sim_time/1e6,i);
            _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
            etl::vector<bool,CODE_WORD_BITS_LENGTH> rm_enc = ldpc.rateMatch(enc, CODE_WORD_BITS_LENGTH);
            _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i] Ended rate matching\n",sim_time/1e6,i);
            printf("[t_sim [s] = %.6f][SNR_0%i] rate matching elapsed time [s]: %.6f\n",sim_time/1e6,i,(step_time_end - step_time_start)/1e6);
            //printf("[SNR_0%i_t_sim = %.5f] Ended rate matching\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC);
            //printf("[SNR_0%i_t_sim = %.5f] rate matching elapsed time: %.5f\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC,static_cast<float>(clock() - stepStart) / CLOCKS_PER_SEC);
            
			// BPSK + AWGN
            etl::vector<float,CODE_WORD_BITS_LENGTH> r; //r.reserve(M);
			for (auto e : rm_enc)
				//r.push_back(1 - 2.0 * e + sqrt(N0[i] / 2.0) * norm_dist(random_engine));
                r.push_back(1 - 2.0 * e + sqrtf(N0[i] / 2.0) * norm_dist_random_engine());
                /*DEBUG*/
                //r.push_back(1.0 - 2.0 * e);
                /*DEBUG*/

			// compute soft bits as LLR
            etl::vector<float,CODE_WORD_BITS_LENGTH> llr; //llr.reserve(M);
			for (auto e : r)
				llr.push_back(4.0 * e / N0[i]);

            /*DEBUG*/
            // PERFECT LLR AFTER RATE MATCHING
            //etl::vector<float,CODE_WORD_BITS_LENGTH> llr;

            //for (auto bit : rm_enc) {
            //    llr.push_back(bit ? -10.0 : 10.0);
            //}
            /*DEBUG*/

			// rate recovery
            //printf("[SNR_0%i_t_sim = %.5f] Started rate recovery\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC);
            //stepStart = clock();
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i] Started rate recover\n",sim_time/1e6,i);
            _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
            etl::vector<float,MAX_CODEWORD_LENGTH> rr_llr = ldpc.rateRecover(llr);
            _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i] Ended rate recover\n",sim_time/1e6,i);
            printf("[t_sim [s] = %.6f][SNR_0%i] rate recover elapsed time [s]: %.6f\n",sim_time/1e6,i,(step_time_end - step_time_start)/1e6);
            //printf("[SNR_0%i_t_sim = %.5f] Ended rate recovery\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC);
            //printf("[SNR_0%i_t_sim = %.5f] rate rate recovery elapsed time: %.5f\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC,static_cast<float>(clock() - stepStart) / CLOCKS_PER_SEC);

			// scl decoding
            //printf("[SNR_0%i_t_sim = %.5f] Started docode\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC);
            //stepStart = clock();
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i] Started decode\n",sim_time/1e6,i);
            _TCE_RTC(1, step_time_start); // OpenASIP 2.0 doc (search for printf explanation)
            etl::vector<bool,MAX_INFO_NODE_BITS> msg_cap = ldpc.decode(rr_llr, N_MAX_ITER);
            _TCE_RTC(1, step_time_end); // OpenASIP 2.0 doc (search for printf explanation)
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i] Ended decode\n",sim_time/1e6,i);
            printf("[t_sim [s] = %.6f][SNR_0%i] decode elapsed time [s]: %.6f\n",sim_time/1e6,i,(step_time_end - step_time_start)/1e6);
            //printf("[SNR_0%i_t_sim = %.5f] Ended decode\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC);
            //printf("[SNR_0%i_t_sim = %.5f] decode elapsed time: %.5f\n",i,static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC,static_cast<float>(clock() - stepStart) / CLOCKS_PER_SEC);
            /*DEBUG*/
            //etl::vector<bool, MAX_INFO_NODE_BITS> msg_cap = ldpc.decode(perfect_llr, N_MAX_ITER);
            /*DEBUG*/
            msg_cap.resize(INFO_BITS_LENGTH);

            //etl::array<char,MESSAGE_LENGTH> decoded = bitsToString(msg_cap);

			// count errors
            unsigned totalErrors   = 0;
            unsigned messageErrors = 0;
            unsigned paddingErrors = 0;

            for (unsigned j = 0; j < INFO_BITS_LENGTH; ++j)
            {
                if (msg[j] != msg_cap[j])
                {
                    ++totalErrors;

                    if (j < MESSAGE_BITS_LENGTH)
                        ++messageErrors;
                    else
                        ++paddingErrors;
                }
            }

            if (totalErrors)
            {
                n_bit_errs[i] += totalErrors;
                n_blk_errs[i]++;

                if (!printedFirstFailure)
                {
                    // Convert only the message bits back to ASCII
                    etl::vector<bool,MESSAGE_BITS_LENGTH> decodedMessageBits(
                        msg_cap.begin(),
                        msg_cap.begin() + MESSAGE_BITS_LENGTH);

                    etl::array<char,MESSAGE_LENGTH> decoded = bitsToString(decodedMessageBits);

                    //std::cout << "\n=====================================================\n";
                    printf("\n=====================================================\n");

                    //std::cout << "First decoding failure at Es/N0 = "
                    //          << EsN0_dB[i] << " dB\n";
                    printf("First decoding failure at Es/N0 = %f dB\n",EsN0_dB[i]);

                    //std::cout << "Block number : " << (n_blks_done + 1) << '\n';
                    printf("Block number: %d\n",n_blks_done + 1);

                    //std::cout << "Total bit errors   : "
                    //          << totalErrors << " / " << INFO_BITS_LENGTH << '\n';
                    printf("Total bit errors   : %d / %d\n",totalErrors,INFO_BITS_LENGTH);

                    //std::cout << "Message bit errors : "
                    //          << messageErrors << " / " << MESSAGE_BITS_LENGTH << '\n';
                    printf("Message bit errors : %d / %d\n",messageErrors,MESSAGE_BITS_LENGTH);

                    //std::cout << "Padding bit errors : "
                    //          << paddingErrors << " / " << paddingBits << '\n';
                    printf("Padding bit errors : %d / %d\n",paddingErrors,paddingBits); 

                    //std::cout << "BER(block)         : "
                    //          << static_cast<float>(totalErrors) / INFO_BITS_LENGTH << '\n';
                    printf("BER(block)         : %f / %d\n",static_cast<float>(totalErrors),INFO_BITS_LENGTH); 

                    //std::cout << "-----------------------------------------------------\n";
                    //std::cout << "Original : " << message << '\n';
                    //std::cout << "Decoded  : " << decoded << '\n';
                    printf("-----------------------------------------------------\n") ;
                    printf("Original : %s\n",message);
                    printf("Decoded : %.*s\n",MESSAGE_LENGTH,decoded.data());

                    if (messageErrors)
                    {
                        //std::cout << "\nCharacter differences:\n";
                        printf("\nCharacter differences:\n");

                        for (size_t c = 0; c < MESSAGE_LENGTH; ++c)
                        {
                            if (message[c] != decoded[c])
                            {
                                int position = c;
                                //std::cout << "  position " << c
                                //          << " : '" << message[c]
                                //          << "' -> '" << decoded[c] << "'\n";
                                printf("  position %d : '%c' -> '%c'\n",position,message[c],decoded.data()[c]);
                            }
                        }
                    }
                    else
                    {
                        //std::cout << "\nMessage decoded correctly.\n";
                        //std::cout << "Errors occurred only in the zero-padding region.\n";
                        printf("\nMessage decoded correctly.\n") ;
                        printf("Errors occurred only in the zero-padding region.\n");
                    }

                    //std::cout << "=====================================================\n\n";
                    printf("=====================================================\n\n"); 

                    printedFirstFailure = true;
                }
            }

			n_blks_done += 1;

			ber[i] = n_bit_errs[i] * 1.0 / INFO_BITS_LENGTH / n_blks_done;
			bler[i] = n_blk_errs[i] * 1.0 / n_blks_done;

			// print progress for every 10 blocks
			if (n_blks_done % 10 == 0 || n_blks_done == 1) {
				//print_len = sprintf(str, "Elapsed time: %.1f seconds, # tx blocks: %d,# error blocks:%d, ber: %.5f, bler %.5f", float(clock() - tStart) / CLOCKS_PER_SEC, n_blks_done, n_blk_errs[i], ber[i], bler[i]);
				//cout << std::string(print_len, '\b') << str << flush;
                _TCE_RTC(1, ten_blk_elapsed_time_end); // OpenASIP 2.0 doc (search for printf explanation)
                printf(
                    "Elapsed time: %.1f seconds, # tx blocks: %u, "
                    "# error blocks: %u, ber: %.5f, bler: %.5f\n",
                    (ten_blk_elapsed_time_end - ten_blk_elapsed_time_start)/1e6,
                    n_blks_done,
                    n_blk_errs[i],
                    ber[i],
                    bler[i]
                );
			}
		}
		// print  progress when one SNR is finished
		//sprintf(str, "Elapsed time: %.1f seconds, # tx blocks: %d,# error blocks:%d, ber: %.5f, bler %.5f", static_cast<float>(clock() - tStart) / CLOCKS_PER_SEC, n_blks_done, n_blk_errs[i], ber[i], bler[i]);
		//cout << std::string(print_len, '\b') << str << flush;
        _TCE_RTC(1, one_snr_elapsed_time_end); // OpenASIP 2.0 doc (search for printf explanation)
        printf(
            "Elapsed time: %.1f seconds, # tx blocks: %u, "
            "# error blocks: %u, ber: %.5f, bler: %.5f\n",
            (one_snr_elapsed_time_end - one_snr_elapsed_time_start)/1e6,
            n_blks_done,
            n_blk_errs[i],
            ber[i],
            bler[i]
        );
	}

	// print simulation result
	//cout << endl;
    printf("\n");

	//cout << "Modulation:" << "BPSK" << endl;
    printf("Modulation: BPSK"); 

	//cout << "[M,R] = [ " << CODE_WORD_BITS_LENGTH << "," << CODE_RATE << "]" << endl;
    printf("[M,R] = [%d,%f]\n",CODE_WORD_BITS_LENGTH,CODE_RATE);

	//cout << "EsN0_dB = [";
    printf("EsN0_db = [");
    
	for (auto e : EsN0_dB)
		//cout << e << " ";
        printf("%f",e);
	//cout << "]" << endl;
    printf("]\n");

	//cout << "BER = [";
    printf("BER = [");
	for (auto e : ber)
		//cout << e << " ";
        printf("%f ",e);
	//cout << "]" << endl;
    printf("]\n");

	//cout << "BLER = [";
    printf("BLER = [");
	for (auto e : bler)
		//cout << e << " ";
        printf("%f ",e);
	//cout << "]" << endl;
    printf("]\n");
}

etl::vector<float,LIN_SPACE_NUM> lin_space(float start, float end) {
	// catch rarely, throw often
	assert(LIN_SPACE_NUM >= 2 && "LIN_SPACE_NUM must be a positive integer >= 2!");

	int partitions = LIN_SPACE_NUM - 1; // 9 - 1 = 8
    etl::vector<float,LIN_SPACE_NUM> pts;
	// length of each segment
	float length = (end - start) / partitions; // (-2 -(-4)) / 8 = 0.25
	// first, not to change
	pts.push_back(start); // -4
	for (int i = 1; i < LIN_SPACE_NUM - 1; i++) {
		pts.push_back(start + i * length); // -3.75,-3.5,-3.25,-3,-2.75,-2.5,-2.25
	}
	// last, not to change
	pts.push_back(end); // -2
	return pts;
}

template <size_t N>
etl::vector<bool,MESSAGE_BITS_LENGTH> stringToBits(const char (&text)[N])
{
    etl::vector<bool, MESSAGE_BITS_LENGTH> bits;

    for (size_t j = 0; j < MESSAGE_LENGTH; ++j) {
        unsigned char c = static_cast<unsigned char>(text[j]);

        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }

    return bits;
}

template <size_t N> etl::array<char, MESSAGE_LENGTH>
bitsToString(const etl::vector<bool, N>& bits)
{
    //etl::array<char, MESSAGE_LENGTH> text{};
    etl::array<char, MESSAGE_LENGTH> text;

    for (size_t j = 0; j < MESSAGE_LENGTH; ++j)
    {
        unsigned char c = 0;

        for (int b = 0; b < 8; ++b)
            c = (c << 1) | bits[j * 8 + b];

        text[j] = static_cast<char>(c);
    }

    return text;
}
