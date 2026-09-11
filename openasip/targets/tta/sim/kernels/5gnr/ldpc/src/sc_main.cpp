/*
 * -----------------------------------------------------------------------------
 * SystemC host/orchestration program for the 5G NR LDPC TTA simulation.
 *
 * host
 *   |
 *   | EXT_MSG (uint8_t[])
 *   v
 * encoder/rate match TTA
 *   |
 *   | RM_ENC (uint8_t[])
 *   v
 * BPSK + AWGN
 *   |
 *   | LLR (float[])
 *   v
 * rate recover/decoder TTA
 *   |
 *   | MSG_CAP (uint8_t[])
 *   v
 * host
 * -----------------------------------------------------------------------------
 */

#include "systemc.h"
#include "tce_systemc.hh"

#include "defines.h"
#include "norm_dist.h"
#include "n0_table.h"
#include "payload.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#ifndef SHARED_MEMORY_SIZE
#define SHARED_MEMORY_SIZE (1024 * 1024)
#endif

/*
 * -----------------------------------------------------------------------------
 * Shared memory
 * -----------------------------------------------------------------------------
 */

class SharedMemory
{
public:
    SharedMemory()
    {
        std::memset(memory_, 0, sizeof(memory_));
    }

    unsigned char* data() { return memory_; }
    const unsigned char* data() const { return memory_; }

    unsigned size() const { return SHARED_MEMORY_SIZE; }

    bool valid(unsigned address, unsigned bytes = 1) const
    {
        return address < SHARED_MEMORY_SIZE &&
               bytes <= SHARED_MEMORY_SIZE - address;
    }

    unsigned read32(unsigned address) const
    {
        assert(valid(address, sizeof(unsigned)));

        unsigned value;
        std::memcpy(&value, memory_ + address, sizeof(value));
        return value;
    }

    void write32(unsigned address, unsigned value)
    {
        assert(valid(address, sizeof(unsigned)));
        std::memcpy(memory_ + address, &value, sizeof(value));
    }

    template <typename T>
    T* ptr(unsigned address, size_t count = 1)
    {
        assert(valid(address, sizeof(T) * count));
        return reinterpret_cast<T*>(memory_ + address);
    }

    template <typename T>
    const T* ptr(unsigned address, size_t count = 1) const
    {
        assert(valid(address, sizeof(T) * count));
        return reinterpret_cast<const T*>(memory_ + address);
    }

private:
    unsigned char memory_[SHARED_MEMORY_SIZE];
};

/*
 * -----------------------------------------------------------------------------
 * SystemC LSU model
 * -----------------------------------------------------------------------------
 */

TCE_SC_OPERATION_SIMULATOR(SharedLSUModel)
{
    SharedMemory* memory;

    TCE_SC_OPERATION_SIMULATOR_CTOR(SharedLSUModel)
    {
        memory = nullptr;
    }

    void setSharedMemory(SharedMemory* mem)
    {
        memory = mem;
    }

    TCE_SC_SIMULATE_CYCLE_START {}

    TCE_SC_SIMULATE_STAGE
    {
        assert(memory != nullptr);

        unsigned address = TCE_SC_UINT(1);

        if (address <= LAST_DMEM_ADDR || TCE_SC_OPSTAGE > 0)
            return false;

        if (TCE_SC_OPERATION.writesMemory())
        {
            unsigned value = TCE_SC_UINT(2);
            memory->write32(address, value);
            return true;
        }

        unsigned value = memory->read32(address);
        TCE_SC_OUTPUT(2) = value;
        return true;
    }
};

/*
 * -----------------------------------------------------------------------------
 * Host-side helpers
 * -----------------------------------------------------------------------------
 *
 * The shared-memory ABI uses uint8_t for bits:
 *
 *     0 -> false
 *     1 -> true
 *
 * Internally the host also uses uint8_t rather than std::vector<bool>, avoiding
 * the special packed representation of vector<bool>.
 */

template <size_t N>
std::vector<uint8_t> stringToBits(const char (&text)[N])
{
    std::vector<uint8_t> bits(MESSAGE_BITS_LENGTH, 0);

    for (size_t j = 0; j < MESSAGE_LENGTH; ++j)
    {
        const unsigned char c =
            static_cast<unsigned char>(text[j]);

        for (int i = 7; i >= 0; --i)
            bits[j * 8 + (7 - i)] = (c >> i) & 1U;
    }

    return bits;
}

template <typename BitVector>
std::array<char, MESSAGE_LENGTH> bitsToString(const BitVector& bits)
{
    std::array<char, MESSAGE_LENGTH> text{};

    for (size_t j = 0; j < MESSAGE_LENGTH; ++j)
    {
        unsigned char c = 0;

        for (int b = 0; b < 8; ++b)
            c = static_cast<unsigned char>((c << 1) | (bits[j * 8 + b] & 1U));

        text[j] = static_cast<char>(c);
    }

    return text;
}

std::vector<float> makeLinspace(float start, float end, size_t n)
{
    std::vector<float> result(n);

    if (n == 0)
        return result;

    if (n == 1)
    {
        result[0] = start;
        return result;
    }

    for (size_t i = 0; i < n; ++i)
        result[i] = start + (end - start) *
                    static_cast<float>(i) /
                    static_cast<float>(n - 1);

    return result;
}

/*
 * -----------------------------------------------------------------------------
 * Monte-Carlo LDPC simulation
 * -----------------------------------------------------------------------------
 *
 * No LDPC algorithm is executed by the host.
 *
 * The host only:
 *
 *   1. prepares EXT_MSG;
 *   2. starts/waits for encoder TTA;
 *   3. performs BPSK + AWGN;
 *   4. prepares LLR;
 *   5. starts/waits for decoder TTA;
 *   6. checks MSG_CAP.
 */

void example_run_LDPC(
    NormDistRandomEngine norm_dist_random_engine,
    sc_clock& clk,
    SharedMemory& shared)
{
    const std::vector<float> EsN0_dB(
        ESN0_DB_TABLE,
        ESN0_DB_TABLE + N0_TABLE_SIZE);

    const std::vector<float> N0(
        N0_TABLE,
        N0_TABLE + N0_TABLE_SIZE);

    assert(EsN0_dB.size() == N0.size());

    std::vector<float> ber(N0.size(), 0.0f);
    std::vector<float> bler(N0.size(), 0.0f);
    std::vector<unsigned> n_bit_errs(N0.size(), 0);
    std::vector<unsigned> n_blk_errs(N0.size(), 0);

    /*
     * The encoder TTA constructs the persistent nrLDPC object in shared
     * memory during startup. The host only obtains the resulting filler
     * length through the shared control register.
     */
    const size_t fillerLength =
        shared.read32(FILLER_LENGTH_ADDR);

    while (shared.read32(STARTUP_ADDR) == 0) 
        sc_start(clk.period());

    std::vector<uint8_t> fillers(fillerLength, 0);

    std::vector<uint8_t> complete_message =
        stringToBits(message);

    const unsigned paddingBits =
        INFO_BITS_LENGTH - MESSAGE_BITS_LENGTH;

    if (complete_message.size() > INFO_BITS_LENGTH)
    {
        std::printf("[Runtime Error] Message too large.\n");
        return;
    }

    std::printf(
        "[t_sim [s] = %.6f] Began simulated TTA CPU clock time count\n",
        sc_time_stamp().to_seconds());

    for (unsigned i = 0; i < N0.size(); ++i)
    {
        std::printf(
            "\nNow running EsN0: %.2f dB [%u of %lu]\n",
            EsN0_dB[i],
            i + 1,
            static_cast<unsigned long>(N0.size()));

        unsigned n_blks_done = 0;
        bool printedFirstFailure = false;

        const sc_time one_snr_elapsed_time_start =
            sc_time_stamp();

        while (n_blks_done < N_MAX_BLKS && n_blk_errs[i] < 100)
        {
            shared.write32(ENC_STATUS_ADDR, 1);
            shared.write32(DEC_STATUS_ADDR, 1);

            /*
             * -----------------------------------------------------------------
             * Information block
             * -----------------------------------------------------------------
             */

            std::vector<uint8_t> msg(INFO_BITS_LENGTH, 0);

            std::copy(
                complete_message.begin(),
                complete_message.end(),
                msg.begin());

            /*
             * msg + filler bits.
             *
             * The raw shared-memory representation is one byte per bit.
             */
            std::vector<uint8_t> extMsg;
            extMsg.reserve(msg.size() + fillers.size());
            extMsg.insert(extMsg.end(), msg.begin(), msg.end());
            extMsg.insert(extMsg.end(), fillers.begin(), fillers.end());

            assert(extMsg.size() <= MAX_INFO_NODE_BITS);

            /*
             * -----------------------------------------------------------------
             * Encoder TTA
             * -----------------------------------------------------------------
             */

            std::printf(
                "[t_sim [s] = %.6f][SNR_0%u] Started encode_core_prog\n",
                sc_time_stamp().to_seconds(),
                i);

            const sc_time encode_start =
                sc_time_stamp();

            shared.write32(
                SIM_TIME_ADDR,
                static_cast<unsigned>(
                    sc_time_stamp().to_seconds() * 1e6));

            shared.write32(SNR_G_ID_ADDR, i);
            shared.write32(BLK_G_ID_ADDR, n_blks_done + 1);

            /*
             * EXT_MSG_ADDR is a raw uint8_t buffer.
             */
            uint8_t* shared_extMsg =
                shared.ptr<uint8_t>(
                    EXT_MSG_ADDR,
                    extMsg.size());

            std::memcpy(
                shared_extMsg,
                extMsg.data(),
                extMsg.size());

            shared.write32(ENC_STATUS_ADDR, 0);

            while (shared.read32(ENC_STATUS_ADDR) == 0)
                sc_start(clk.period());

            /*
             * RM_ENC_ADDR is a raw uint8_t buffer.
             */
            const uint8_t* shared_rm_enc =
                shared.ptr<uint8_t>(
                    RM_ENC_ADDR,
                    CODE_WORD_BITS_LENGTH);

            std::vector<uint8_t> rm_enc(
                CODE_WORD_BITS_LENGTH);

            std::memcpy(
                rm_enc.data(),
                shared_rm_enc,
                CODE_WORD_BITS_LENGTH);

            const sc_time encode_end =
                sc_time_stamp();

            std::printf(
                "[t_sim [s] = %.6f][SNR_0%u] Ended encode_core_prog\n",
                sc_time_stamp().to_seconds(),
                i);

            std::printf(
                "[t_sim [s] = %.6f][SNR_0%u] "
                "encode elapsed time [s]: %.6f\n",
                sc_time_stamp().to_seconds(),
                i,
                (encode_end - encode_start).to_seconds());

            assert(rm_enc.size() == CODE_WORD_BITS_LENGTH);

            /*
             * -----------------------------------------------------------------
             * BPSK + AWGN
             * -----------------------------------------------------------------
             */

            std::vector<float> r(CODE_WORD_BITS_LENGTH);

            for (size_t j = 0; j < rm_enc.size(); ++j)
            {
                const float transmitted =
                    rm_enc[j] ? -1.0f : 1.0f;

                const float noise =
                    std::sqrt(N0[i] / 2.0f) *
                    norm_dist_random_engine();

                r[j] = transmitted + noise;
            }

            /*
             * -----------------------------------------------------------------
             * LLR
             * -----------------------------------------------------------------
             */

            std::vector<float> llr(CODE_WORD_BITS_LENGTH);

            for (size_t j = 0; j < r.size(); ++j)
                llr[j] = 4.0f * r[j] / N0[i];

            /*
             * -----------------------------------------------------------------
             * Decoder TTA
             * -----------------------------------------------------------------
             */

            std::printf(
                "[t_sim [s] = %.6f][SNR_0%u] Started decode\n",
                sc_time_stamp().to_seconds(),
                i);

            const sc_time decode_start =
                sc_time_stamp();

            /*
             * LLR_ADDR is a raw float buffer.
             */
            float* shared_llr =
                shared.ptr<float>(
                    LLR_ADDR,
                    llr.size());

            std::memcpy(
                shared_llr,
                llr.data(),
                llr.size() * sizeof(float));

            shared.write32(
                SIM_TIME_ADDR,
                static_cast<unsigned>(
                    sc_time_stamp().to_seconds() * 1e6));

            shared.write32(SNR_G_ID_ADDR, i);
            shared.write32(
                BLK_G_ID_ADDR,
                n_blks_done + 1);

            shared.write32(
                ACTUAL_CODEWORD_LENGTH_ADDR,
                static_cast<unsigned>(llr.size()));

            shared.write32(DEC_STATUS_ADDR, 0);

            while (shared.read32(DEC_STATUS_ADDR) == 0)
                sc_start(clk.period());

            /*
             * MSG_CAP_ADDR is a raw uint8_t buffer.
             */
            const uint8_t* shared_msg_cap =
                shared.ptr<uint8_t>(
                    MSG_CAP_ADDR,
                    MAX_INFO_NODE_BITS);

            std::vector<uint8_t> msg_cap(
                MAX_INFO_NODE_BITS);

            std::memcpy(
                msg_cap.data(),
                shared_msg_cap,
                MAX_INFO_NODE_BITS);

            const sc_time decode_end =
                sc_time_stamp();

            std::printf(
                "[t_sim [s] = %.6f][SNR_0%u] Ended decode\n",
                sc_time_stamp().to_seconds(),
                i);

            std::printf(
                "[t_sim [s] = %.6f][SNR_0%u] "
                "decode elapsed time [s]: %.6f\n",
                sc_time_stamp().to_seconds(),
                i,
                (decode_end - decode_start).to_seconds());

            /*
             * The TTA decoder returns MAX_INFO_NODE_BITS bytes in the shared
             * buffer. Only INFO_BITS_LENGTH bits are relevant here.
             */
            if (msg_cap.size() < INFO_BITS_LENGTH)
            {
                std::printf(
                    "[Runtime Error] Decoder returned only %lu bits.\n",
                    static_cast<unsigned long>(msg_cap.size()));

                return;
            }

            /*
             * -----------------------------------------------------------------
             * Error counting
             * -----------------------------------------------------------------
             */

            unsigned totalErrors = 0;
            unsigned messageErrors = 0;
            unsigned paddingErrors = 0;

            for (unsigned j = 0; j < INFO_BITS_LENGTH; ++j)
            {
                if ((msg[j] != 0) != (msg_cap[j] != 0))
                {
                    ++totalErrors;

                    if (j < MESSAGE_BITS_LENGTH)
                        ++messageErrors;
                    else
                        ++paddingErrors;
                }
            }

            /*
             * -----------------------------------------------------------------
             * First failure diagnostics
             * -----------------------------------------------------------------
             */

            if (totalErrors != 0)
            {
                n_bit_errs[i] += totalErrors;
                ++n_blk_errs[i];

                if (!printedFirstFailure)
                {
                    std::vector<uint8_t> decodedMessageBits(
                        MESSAGE_BITS_LENGTH);

                    std::copy(
                        msg_cap.begin(),
                        msg_cap.begin() + MESSAGE_BITS_LENGTH,
                        decodedMessageBits.begin());

                    const std::array<char, MESSAGE_LENGTH> decoded =
                        bitsToString(decodedMessageBits);

                    std::printf(
                        "\n=====================================================\n");

                    std::printf(
                        "First decoding failure at Es/N0 = %f dB\n",
                        EsN0_dB[i]);

                    std::printf(
                        "Block number: %u\n",
                        n_blks_done + 1);

                    std::printf(
                        "Total bit errors   : %u / %u\n",
                        totalErrors,
                        INFO_BITS_LENGTH);

                    std::printf(
                        "Message bit errors : %u / %u\n",
                        messageErrors,
                        MESSAGE_BITS_LENGTH);

                    std::printf(
                        "Padding bit errors : %u / %u\n",
                        paddingErrors,
                        paddingBits);

                    std::printf(
                        "BER(block)         : %f / %u\n",
                        static_cast<float>(totalErrors),
                        INFO_BITS_LENGTH);

                    std::printf(
                        "-----------------------------------------------------\n");

                    std::printf(
                        "Original : %s\n",
                        message);

                    std::printf(
                        "Decoded  : %.*s\n",
                        MESSAGE_LENGTH,
                        decoded.data());

                    if (messageErrors != 0)
                    {
                        std::printf(
                            "\nCharacter differences:\n");

                        for (size_t c = 0; c < MESSAGE_LENGTH; ++c)
                        {
                            if (message[c] != decoded[c])
                            {
                                std::printf(
                                    "  position %lu : '%c' -> '%c'\n",
                                    static_cast<unsigned long>(c),
                                    message[c],
                                    decoded[c]);
                            }
                        }
                    }
                    else
                    {
                        std::printf(
                            "\nMessage decoded correctly.\n");

                        std::printf(
                            "Errors occurred only in the "
                            "zero-padding region.\n");
                    }

                    std::printf(
                        "=====================================================\n\n");

                    printedFirstFailure = true;
                }
            }

            /*
             * -----------------------------------------------------------------
             * Block statistics
             * -----------------------------------------------------------------
             */

            ++n_blks_done;

            ber[i] =
                static_cast<float>(n_bit_errs[i]) /
                static_cast<float>(INFO_BITS_LENGTH) /
                static_cast<float>(n_blks_done);

            bler[i] =
                static_cast<float>(n_blk_errs[i]) /
                static_cast<float>(n_blks_done);

            if (n_blks_done % 10 == 0 || n_blks_done == 1)
            {
                const sc_time elapsed =
                    sc_time_stamp() -
                    one_snr_elapsed_time_start;

                std::printf(
                    "Elapsed time: %.1f seconds, "
                    "# tx blocks: %u, "
                    "# error blocks: %u, "
                    "ber: %.5f, "
                    "bler: %.5f\n",
                    elapsed.to_seconds(),
                    n_blks_done,
                    n_blk_errs[i],
                    ber[i],
                    bler[i]);
            }
        }

        /*
         * ---------------------------------------------------------------------
         * End-of-SNR report
         * ---------------------------------------------------------------------
         */

        const sc_time one_snr_elapsed_time_end =
            sc_time_stamp();

        std::printf(
            "Elapsed time: %.1f seconds, "
            "# tx blocks: %u, "
            "# error blocks: %u, "
            "ber: %.5f, "
            "bler: %.5f\n",
            (one_snr_elapsed_time_end -
             one_snr_elapsed_time_start).to_seconds(),
            n_blks_done,
            n_blk_errs[i],
            ber[i],
            bler[i]);
    }

    /*
     * -------------------------------------------------------------------------
     * Final results
     * -------------------------------------------------------------------------
     */

    std::printf("\nModulation: BPSK\n");

    std::printf(
        "[M,R] = [%d,%f]\n",
        CODE_WORD_BITS_LENGTH,
        CODE_RATE);

    std::printf("EsN0_db = [");
    for (float value : EsN0_dB)
        std::printf("%f ", value);
    std::printf("]\n");

    std::printf("BER = [");
    for (float value : ber)
        std::printf("%f ", value);
    std::printf("]\n");

    std::printf("BLER = [");
    for (float value : bler)
        std::printf("%f ", value);
    std::printf("]\n");
}

/*
 * -----------------------------------------------------------------------------
 * SystemC entry point
 * -----------------------------------------------------------------------------
 */

int sc_main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::fprintf(
            stderr,
            "Usage: %s <encode.adf> <encode.tpef> "
            "<decode.adf> <decode.tpef>\n",
            argv[0]);

        return EXIT_FAILURE;
    }

    const char* encode_adf = argv[1];
    const char* encode_tpef = argv[2];
    const char* decode_adf = argv[3];
    const char* decode_tpef = argv[4];

    std::printf("Hello, 5G-NR!\n");

    sc_clock clk(
        "clock",
        2,
        SC_US);

    sc_signal<bool> glock;

    SharedMemory shared;

    TTACore encoder_tta(
        "encoder_tta",
        encode_adf,
        encode_tpef);

    encoder_tta.clock(clk);
    encoder_tta.global_lock(glock);

    SharedLSUModel encoder_lsu("encoder_lsu");
    encoder_lsu.setSharedMemory(&shared);
    encoder_tta.setOperationSimulator("LSU", encoder_lsu);

    TTACore decoder_tta(
        "decoder_tta",
        decode_adf,
        decode_tpef);

    decoder_tta.clock(clk);
    decoder_tta.global_lock(glock);

    SharedLSUModel decoder_lsu("decoder_lsu");
    decoder_lsu.setSharedMemory(&shared);
    decoder_tta.setOperationSimulator("LSU", decoder_lsu);

    NormDistRandomEngine norm_dist_random_engine;

    example_run_LDPC(
        norm_dist_random_engine,
        clk,
        shared);

    return 0;
}
