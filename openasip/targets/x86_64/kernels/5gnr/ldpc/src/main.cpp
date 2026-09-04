#include "LDPC.h"
#include <iostream>
#include <random>
#include <cstdio> // sprintf
#include "gen-random/norm_dist.h"
#include "gen-n0/n0_table.h"
using namespace std;

using std::chrono::duration_cast;
using std::chrono::steady_clock;
using std::chrono::milliseconds;

steady_clock::time_point sim_time; // for LDPC.cpp to access

vector<float> lin_space(float start, float ed, int num);
void example_run_LDPC();
template <size_t N> vector<bool> stringToBits(const char (&text)[N]);
array<char, MESSAGE_LENGTH> bitsToString(const vector<bool>& bits);
void example_run_LDPC(NormDistRandomEngine norm_dist_random_engine);
using namespace std;

int main() {
	printf("Hello, 5G-NR!\n");
    NormDistRandomEngine norm_dist_random_engine;
	example_run_LDPC(norm_dist_random_engine);
	return 0;
}

void example_run_LDPC(NormDistRandomEngine norm_dist_random_engine)
{
    steady_clock::time_point step_time_start;
    steady_clock::time_point step_time_end;
    steady_clock::time_point ten_blk_elapsed_time_start;
    steady_clock::time_point ten_blk_elapsed_time_end;
    steady_clock::time_point one_snr_elapsed_time_start;
    steady_clock::time_point one_snr_elapsed_time_end;

    sim_time = steady_clock::now(); // timing
    printf("[t_sim [s] = %.6f] Began simulated CPU clock time count\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3);

    const char message[MESSAGE_LENGTH] = "Hello NR LDPC!\0";
    std::vector<bool> complete_message = stringToBits(message);
    const unsigned paddingBits = INFO_BITS_LENGTH - MESSAGE_BITS_LENGTH;
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
	//vector<float> EsN0_dB = lin_space(-4, -2, 9);
    vector<float> EsN0_dB(ESN0_DB_TABLE,ESN0_DB_TABLE + N0_TABLE_SIZE);
    vector<float> N0(N0_TABLE,N0_TABLE + N0_TABLE_SIZE);
	//transform(EsN0_dB.begin(), EsN0_dB.end(), N0.begin(), [](const float& x) {return pow(10.0, -x / 10.0); });

	vector<float> ber(N0.size(), 0), bler(N0.size(), 0);
	vector<unsigned> n_bit_errs(N0.size(), 0), n_blk_errs(N0.size(), 0);
	vector<bool> fillers(ldpc.getFillerLength(), 0);
	// loop each SNR
	for (unsigned i = 0; i < N0.size(); i++) {
		//print progress
		//char str[100];
		//sprintf_s(str, "\nNow running EsN0: %.2f dB [%d of %lu]", EsN0_dB[i], i + 1, N0.size());
		printf("\nNow running EsN0: %.2f dB [%d of %lu]\n", EsN0_dB[i], i + 1, (unsigned long)N0.size());
		//unsigned print_len = 0;

		unsigned n_blks_done = 0;
		//clock_t tStart = steady_clock::now(); // timing

        bool printedFirstFailure = false;
        ten_blk_elapsed_time_start = steady_clock::now(); // timing
        one_snr_elapsed_time_start = steady_clock::now(); // timing
		while ((n_blks_done < N_MAX_BLKS) && (n_blk_errs[i] < 100)) {
            blk_g = n_blks_done;
			// generate random bit stream
            vector<bool> msg(INFO_BITS_LENGTH,0);// = { 1,0,0,1,1,0,1,0,0,0,0,0,1,0,1,1,1,0,0,0,0,1,1,0,1,0,1,1,1,1,0,1,0,1 };
			//msg.reserve(K);
			//for (unsigned j = 0; j < K; j++)
			//	msg.push_back(bern_dist(random_engine));

			// add filler bits
            vector<bool> extMsg(msg.begin(),msg.end()); // For 'Hello NR LDPC!' -> ldpc.getFillerLength() = 200
			extMsg.insert(extMsg.end(), fillers.begin(), fillers.end());

			// LDPC encoding
            printf("[t_sim [s] = %.6f][SNR_0%i] Started encode\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i);
            step_time_start = steady_clock::now();
			vector<bool> enc = ldpc.encode(extMsg);
            step_time_end = steady_clock::now();
            printf("[t_sim [s] = %.6f][SNR_0%i] Ended encode\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i);
            printf("[t_sim [s] = %.6f][SNR_0%i] encode elapsed time [s]: %.6f\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i,duration_cast<milliseconds>(step_time_end - step_time_start).count()/1e3);
			assert(ldpc.checkSumCodeWord(enc));

			//rate matching
            printf("[t_sim [s] = %.6f][SNR_0%i] Started rate matching\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i);
            step_time_start = steady_clock::now();
			vector<bool> rm_enc = ldpc.rateMatch(enc, CODE_WORD_BITS_LENGTH);
            step_time_end = steady_clock::now();
            printf("[t_sim [s] = %.6f][SNR_0%i] Ended rate matching\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i);
            printf("[t_sim [s] = %.6f][SNR_0%i] rate matching elapsed time [s]: %.6f\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i,duration_cast<milliseconds>(step_time_end - step_time_start).count()/1e3);

			// BPSK + AWGN
			vector<float> r; r.reserve(CODE_WORD_BITS_LENGTH);
			for (auto e : rm_enc)
                r.push_back(1 - 2.0 * e + sqrtf(N0[i] / 2.0) * norm_dist_random_engine());

			// compute soft bits as LLR
			vector<float> llr; llr.reserve(CODE_WORD_BITS_LENGTH);
			for (auto e : r)
				llr.push_back(4.0 * e / N0[i]);

			// rate recovery
            printf("[t_sim [s] = %.6f][SNR_0%i] Started rate recover\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i);
            step_time_start = steady_clock::now();
			vector<float> rr_llr = ldpc.rateRecover(llr);
            step_time_end = steady_clock::now();
            printf("[t_sim [s] = %.6f][SNR_0%i] Ended rate recover\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i);
            printf("[t_sim [s] = %.6f][SNR_0%i] rate recover elapsed time [s]: %.6f\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i,duration_cast<milliseconds>(step_time_end - step_time_start).count()/1e3);

			// scl decoding
            printf("[t_sim [s] = %.6f][SNR_0%i] Started decode\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i);
            step_time_start = steady_clock::now();
			vector<bool> msg_cap = ldpc.decode(rr_llr, N_MAX_ITER);
            step_time_end = steady_clock::now();
            printf("[t_sim [s] = %.6f][SNR_0%i] Ended decode\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i);
            printf("[t_sim [s] = %.6f][SNR_0%i] decode elapsed time [s]: %.6f\n",duration_cast<milliseconds>(steady_clock::now() - sim_time).count()/1e3,i,duration_cast<milliseconds>(step_time_end - step_time_start).count()/1e3);

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
                    vector<bool> decodedMessageBits(
                        msg_cap.begin(),
                        msg_cap.begin() + MESSAGE_BITS_LENGTH);

                    array<char,MESSAGE_LENGTH> decoded = bitsToString(decodedMessageBits);

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
				//print_len = sprintf(str, "Elapsed time: %.1f seconds, # tx blocks: %d,# error blocks:%d, ber: %.5f, bler %.5f", float(steady_clock::now() - tStart) / CLOCKS_PER_SEC, n_blks_done, n_blk_errs[i], ber[i], bler[i]);
				//cout << std::string(print_len, '\b') << str << flush;
                ten_blk_elapsed_time_end = steady_clock::now();
                printf(
                    "Elapsed time: %.1f seconds, # tx blocks: %u, "
                    "# error blocks: %u, ber: %.5f, bler: %.5f\n",
                    duration_cast<milliseconds>(ten_blk_elapsed_time_end - ten_blk_elapsed_time_start).count()/1e3,
                    n_blks_done,
                    n_blk_errs[i],
                    ber[i],
                    bler[i]
                );
			}
		}
		// print  progress when one SNR is finished
		//sprintf(str, "Elapsed time: %.1f seconds, # tx blocks: %d,# error blocks:%d, ber: %.5f, bler %.5f", static_cast<float>(steady_clock::now() - tStart) / CLOCKS_PER_SEC, n_blks_done, n_blk_errs[i], ber[i], bler[i]);
		//cout << std::string(print_len, '\b') << str << flush;
        one_snr_elapsed_time_end = steady_clock::now();
        printf(
            "Elapsed time: %.1f seconds, # tx blocks: %u, "
            "# error blocks: %u, ber: %.5f, bler: %.5f\n",
            duration_cast<milliseconds>(one_snr_elapsed_time_end - one_snr_elapsed_time_start).count()/1e3,
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

vector<float> lin_space(float start, float end, int num) {
	// catch rarely, throw often
	assert(num >= 2 && "The third parameter must be a positive integer >= 2!");

	int partitions = num - 1;
	vector<float> pts;
	// length of each segment
	float length = (end - start) / partitions;
	// first, not to change
	pts.push_back(start);
	for (int i = 1; i < num - 1; i++) {
		pts.push_back(start + i * length);
	}
	// last, not to change
	pts.push_back(end);
	return pts;
}

template <size_t N> vector<bool> stringToBits(const char (&text)[N])
{
    vector<bool> bits;
    bits.reserve(MESSAGE_BITS_LENGTH);

    for (size_t j = 0; j < MESSAGE_LENGTH; ++j) {
        unsigned char c = static_cast<unsigned char>(text[j]);

        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }

    return bits;
}

array<char, MESSAGE_LENGTH> bitsToString(const vector<bool>& bits)
{
    array<char, MESSAGE_LENGTH> text;

    for (size_t j = 0; j < MESSAGE_LENGTH; ++j)
    {
        unsigned char c = 0;

        for (int b = 0; b < 8; ++b)
            c = (c << 1) | bits[j * 8 + b];

        text[j] = static_cast<char>(c);
    }

    return text;
}
