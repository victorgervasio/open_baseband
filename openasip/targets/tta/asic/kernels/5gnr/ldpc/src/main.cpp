#include "LDPC.h"
#include "norm_dist.h"
#include "n0_table.h"
using namespace std;

template <size_t N> etl::vector<bool,MESSAGE_BITS_LENGTH> stringToBits(const char (&text)[N]);
template <size_t N> etl::array<char, MESSAGE_LENGTH> bitsToString(const etl::vector<bool, N>& bits);
void example_run_LDPC(NormDistRandomEngine norm_dist_random_engine);

int main() {
    NormDistRandomEngine norm_dist_random_engine;
	example_run_LDPC(norm_dist_random_engine);
	return 0;
}

void example_run_LDPC(NormDistRandomEngine norm_dist_random_engine)
{
    const char message[MESSAGE_LENGTH] = "Hello NR LDPC!";
    etl::vector<bool, MESSAGE_BITS_LENGTH> complete_message = stringToBits(message);
    const unsigned paddingBits = INFO_BITS_LENGTH - MESSAGE_BITS_LENGTH;
    if (complete_message.size() > INFO_BITS_LENGTH) {
        return;
    }

	nrLDPC ldpc = nrLDPC(INFO_BITS_LENGTH, CODE_RATE);

    etl::vector<float, LIN_SPACE_NUM> EsN0_dB(ESN0_DB_TABLE,ESN0_DB_TABLE + N0_TABLE_SIZE);
    etl::vector<float, LIN_SPACE_NUM> N0(N0_TABLE,N0_TABLE + N0_TABLE_SIZE);

    etl::vector<float,LIN_SPACE_NUM>   ber(N0.size(), 0), bler(N0.size(), 0);
	etl::vector<unsigned,LIN_SPACE_NUM> n_bit_errs(N0.size(), 0), n_blk_errs(N0.size(), 0);
    etl::vector<bool,MAX_INFO_NODE_BITS> fillers(ldpc.getFillerLength(), 0); // For 'Hello NR LDPC!' -> ldpc.getFillerLength() = 200

	// loop each SNR
	for (unsigned i = 0; i < N0.size(); i++) {
        snr_g = i;
		unsigned n_blks_done = 0;

		while ((n_blks_done < N_MAX_BLKS) && (n_blk_errs[i] < 100)) {
            blk_g = n_blks_done;
			// generate random bit stream
            etl::vector<bool,INFO_BITS_LENGTH> msg(INFO_BITS_LENGTH,0);// = { 1,0,0,1,1,0,1,0,0,0,0,0,1,0,1,1,1,0,0,0,0,1,1,0,1,0,1,1,1,1,0,1,0,1 };
            etl::copy(complete_message.begin(), complete_message.end(), msg.begin());

			// add filler bits
            etl::vector<bool,MAX_INFO_NODE_BITS> extMsg(msg.begin(),msg.end()); // For 'Hello NR LDPC!' -> ldpc.getFillerLength() = 200
			extMsg.insert(extMsg.end(), fillers.begin(), fillers.end());

			// LDPC encoding
            etl::vector<bool,MAX_CODEWORD_LENGTH> enc = ldpc.encode(extMsg);
			
            assert(ldpc.checkSumCodeWord(enc));

			//rate matching
            etl::vector<bool,CODE_WORD_BITS_LENGTH> rm_enc = ldpc.rateMatch(enc, CODE_WORD_BITS_LENGTH);
            
			// BPSK + AWGN
            etl::vector<float,CODE_WORD_BITS_LENGTH> r; //r.reserve(M);
			for (auto e : rm_enc)
                r.push_back(1 - 2.0 * e + sqrtf(N0[i] / 2.0) * norm_dist_random_engine());

			// compute soft bits as LLR
            etl::vector<float,CODE_WORD_BITS_LENGTH> llr; //llr.reserve(M);
			for (auto e : r)
				llr.push_back(4.0 * e / N0[i]);

			// rate recovery
            etl::vector<float,MAX_CODEWORD_LENGTH> rr_llr = ldpc.rateRecover(llr);

			// scl decoding
            etl::vector<bool,MAX_INFO_NODE_BITS> msg_cap = ldpc.decode(rr_llr, N_MAX_ITER);
            
            msg_cap.resize(INFO_BITS_LENGTH);

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
            }

			n_blks_done += 1;

			ber[i] = n_bit_errs[i] * 1.0 / INFO_BITS_LENGTH / n_blks_done;
			bler[i] = n_blk_errs[i] * 1.0 / n_blks_done;

			if (n_blks_done % 10 == 0 || n_blks_done == 1) {
			}
		}
	}
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
