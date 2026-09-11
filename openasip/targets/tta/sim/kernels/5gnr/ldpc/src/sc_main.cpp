/*
 * -----------------------------------------------------------------------------
 * SystemC system-level simulation for the 5G NR LDPC multicore TTA system.
 *
 * This follows the structure of the OpenASIP Manual Appendix B example:
 *
 *     clock
 *       |
 *       +--------------------+
 *       |                    |
 *   encoder TTA          decoder TTA
 *       |                    |
 *     LSUModel             LSUModel
 *       |                    |
 *       +---------+----------+
 *                 |
 *           SharedMemory
 *
 * Unlike Appendix B, which models two individual memory-mapped registers,
 * this implementation provides one shared memory address space containing:
 *
 *   - control/status registers;
 *   - persistent nrLDPC storage;
 *   - EXT_MSG;
 *   - RM_ENC;
 *   - LLR;
 *   - MSG_CAP.
 *
 * The TTA cores themselves retain their normal local DMEM. The LSU model
 * overrides accesses above LAST_DMEM_ADDR and redirects those accesses to
 * SharedMemory.
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
 *
 * The SharedMemory object lives in the SystemC host process.
 *
 * Address 0 ... LAST_DMEM_ADDR:
 *     reserved for the TTA's local DMEM and therefore never handled here.
 *
 * Address SHARED_MEM_BASE and above:
 *     handled by LSUModel and mapped into this byte array.
 *
 * The array intentionally represents the complete simulated address space,
 * i.e. memory_[0] corresponds to simulated address 0.
 * -----------------------------------------------------------------------------
 */

static unsigned lsuWidth(const std::string& op)
{
    if (op == "ld8"  ||
        op == "ldu8" ||
        op == "st8")
    {
        return 1;
    }

    if (op == "ld16"  ||
        op == "ldu16" ||
        op == "st16")
    {
        return 2;
    }

    if (op == "ld32" ||
        op == "st32")
    {
        return 4;
    }

    return 0;
}

class SharedMemory
{
public:

    SharedMemory()
    {
        std::memset(memory_, 0, sizeof(memory_));
    }

    unsigned char* data()
    {
        return memory_;
    }

    const unsigned char* data() const
    {
        return memory_;
    }

    unsigned size() const
    {
        return SHARED_MEMORY_SIZE;
    }

    bool valid(unsigned address, unsigned bytes = 1) const
    {
        if (address < SHARED_MEM_BASE)
            return false;

        const unsigned offset =
            address - SHARED_MEM_BASE;

        return offset < SHARED_MEMORY_SIZE &&
               bytes <= SHARED_MEMORY_SIZE - offset;
    }

    uint8_t read8(unsigned address) const
    {
        assert(valid(address, sizeof(uint8_t)));

        const unsigned offset =
            address - SHARED_MEM_BASE;

        uint8_t value;

        std::memcpy(
            &value,
            memory_ + offset,
            sizeof(value));

        return value;
    }

    void write8(unsigned address, uint8_t value)
    {
        assert(valid(address, sizeof(uint8_t)));

        const unsigned offset =
            address - SHARED_MEM_BASE;

        std::memcpy(
            memory_ + offset,
            &value,
            sizeof(value));
    }

    uint16_t read16(unsigned address) const
    {
        assert(valid(address, sizeof(uint16_t)));

        const unsigned offset =
            address - SHARED_MEM_BASE;

        uint16_t value;

        std::memcpy(
            &value,
            memory_ + offset,
            sizeof(value));

        return value;
    }

    void write16(unsigned address, uint16_t value)
    {
        assert(valid(address, sizeof(uint16_t)));

        const unsigned offset =
            address - SHARED_MEM_BASE;

        std::memcpy(
            memory_ + offset,
            &value,
            sizeof(value));
    }

    uint32_t read32(unsigned address) const
    {
        assert(valid(address, sizeof(uint32_t)));

        const unsigned offset =
            address - SHARED_MEM_BASE;

        uint32_t value;

        std::memcpy(
            &value,
            memory_ + offset,
            sizeof(value));

        return value;
    }

    void write32(unsigned address, uint32_t value)
    {
        assert(valid(address, sizeof(uint32_t)));

        const unsigned offset =
            address - SHARED_MEM_BASE;

        std::memcpy(
            memory_ + offset,
            &value,
            sizeof(value));
    }

    template <typename T>
    T* ptr(unsigned address, size_t count = 1)
    {
        assert(valid(address, sizeof(T) * count));

        const unsigned offset =
            address - SHARED_MEM_BASE;

        return reinterpret_cast<T*>(
            memory_ + offset);
    }

    template <typename T>
    const T* ptr(unsigned address, size_t count = 1) const
    {
        assert(valid(address, sizeof(T) * count));

        const unsigned offset =
            address - SHARED_MEM_BASE;

        return reinterpret_cast<const T*>(
            memory_ + offset);
    }

private:

    unsigned char memory_[SHARED_MEMORY_SIZE];
};


/*
 * -----------------------------------------------------------------------------
 * LSU operation simulation model
 * -----------------------------------------------------------------------------
 *
 * This follows the OpenASIP Manual Appendix B LSUModel as closely as possible.
 *
 * The important TCE semantics are:
 *
 *     TCE_SC_UINT(1)
 *         operand 1 of a basic load/store is the memory address.
 *
 *     TCE_SC_UINT(2)
 *         operand 2 of a store is the value to write.
 *
 *     TCE_SC_OUTPUT(2)
 *         operand 2 of a load is the first result operand.
 *
 *     return false
 *         do not override the normal TTA LSU simulation.
 *
 *     return true
 *         the custom LSU simulation handled the operation.
 *
 * Thus:
 *
 *     address <= LAST_DMEM_ADDR
 *         -> normal TTA local DMEM
 *
 *     address > LAST_DMEM_ADDR
 *         -> SharedMemory
 * -----------------------------------------------------------------------------
 */

TCE_SC_OPERATION_SIMULATOR(LSUModel)
{
    SharedMemory* shared_memory;

    TCE_SC_OPERATION_SIMULATOR_CTOR(LSUModel)
    {
        shared_memory = nullptr;
    }

    TCE_SC_SIMULATE_CYCLE_START
    {
        /*
         * No per-cycle state is required by this memory model.
         *
         * Appendix B uses this hook to reset its register update signal.
         * We do not have such a signal because SharedMemory is directly
         * updated by the operation model.
         */
    }

    /*DEBUG*/
    TCE_SC_SIMULATE_STAGE
    {
        unsigned address = TCE_SC_UINT(1);
        bool writes = TCE_SC_OPERATION.writesMemory();

        /*
         * Only intercept addresses belonging to our simulated shared-memory
         * address space.
         *
         * Do NOT intercept arbitrary addresses above LAST_DMEM_ADDR.
         *
         * Addresses such as 0xfffde958 are generated by normal TTA code
         * and must remain under the normal TTA LSU simulation.
         */
        if (address < SHARED_MEM_BASE ||
            address >= SHARED_MEM_END)
        {
            return false;
        }

        if (shared_memory == nullptr)
        {
            return false;
        }

        /*
         * Get the actual operation name.
         */
        TCEString operationName =
            TCE_SC_OPERATION.name();

        std::string opName =
            operationName;

        /*
         * -------------------------------------------------------------------------
         * NRLDPC OBJECT DEBUG REGION
         * -------------------------------------------------------------------------
         *
         * NRLDPC_ADDR is the base address of the placement-new nrLDPC object.
         *
         * Print only accesses falling inside the nrLDPC object storage region,
         * rather than printing every shared-memory access. This keeps the
         * diagnostic output manageable while allowing us to see all constructor
         * and getFillerLength() accesses.
         */
        const unsigned nrldpc_begin =
            NRLDPC_ADDR;

        const unsigned nrldpc_end =
            NRLDPC_ADDR + NRLDPC_STORAGE_SIZE;

        const bool nrldpc_access =
            (address >= nrldpc_begin &&
             address < nrldpc_end);

        /*
         * DEBUG:
         * Print the operation BEFORE attempting to determine its width.
         *
         * Unlike the previous diagnostic, this prints every access within the
         * nrLDPC object, not only accesses exactly at NRLDPC_ADDR.
         */
        //if (nrldpc_access)
        //{
        //    fprintf(
        //        stderr,
        //        "[NRLDPC LSU DEBUG] op='%s' stage=%u "
        //        "addr=0x%08x offset=+0x%04x direction=%s\n",
        //        opName.c_str(),
        //        TCE_SC_OPSTAGE,
        //        address,
        //        address - NRLDPC_ADDR,
        //        writes ? "write" : "read");
        //}

        /*
         * Determine access width from the actual operation.
         */
        unsigned width = 0;

        if (opName == "ld8" ||
            opName == "ldu8" ||
            opName == "st8" ||
            opName == "LD8" ||
            opName == "LDU8" ||
            opName == "ST8")
        {
            width = 1;
        }
        else if (opName == "ld16" ||
                 opName == "ldu16" ||
                 opName == "st16" ||
                 opName == "LD16" ||
                 opName == "LDU16" ||
                 opName == "ST16")
        {
            width = 2;
        }
        else if (opName == "ld32" ||
                 opName == "st32" ||
                 opName == "LD32" ||
                 opName == "ST32")
        {
            width = 4;
        }

        /*
         * IMPORTANT:
         *
         * Do not silently fall back here while debugging.
         *
         * If width == 0, the diagnostic tells us the actual operation
         * name that OpenASIP is presenting to the simulator.
         */
        if (width == 0)
        {
            return false;
        }

        /*
         * Store:
         *
         *     stage 0 = actual memory write
         */
        if (writes)
        {
            if (TCE_SC_OPSTAGE != 0)
            {
                return false;
            }
        }
        /*
         * Load:
         *
         *     stage 2 = result becomes available
         */
        else
        {
            if (TCE_SC_OPSTAGE != 2)
            {
                return false;
            }
        }

        if (!shared_memory->valid(address, width))
        {
            return false;
        }

        /*
         * -------------------------------------------------------------------------
         * WRITE
         * -------------------------------------------------------------------------
         */

        if (writes)
        {
            uint32_t value =
                TCE_SC_UINT(2);

            switch (width)
            {
            case 1:
                shared_memory->write8(
                    address,
                    static_cast<uint8_t>(value));
                break;

            case 2:
                shared_memory->write16(
                    address,
                    static_cast<uint16_t>(value));
                break;

            case 4:
                shared_memory->write32(
                    address,
                    static_cast<uint32_t>(value));
                break;

            default:
                return false;
            }

            return true;
        }

        /*
         * -------------------------------------------------------------------------
         * READ
         * -------------------------------------------------------------------------
         */

        uint32_t value = 0;

        switch (width)
        {
        case 1:
            value =
                shared_memory->read8(address);
            break;

        case 2:
            value =
                shared_memory->read16(address);
            break;

        case 4:
            value =
                shared_memory->read32(address);
            break;

        default:
            return false;
        }

        TCE_SC_OUTPUT(2) = value;

        return true;
    }
    /*DEBUG*/

    /*
     * This is application-specific plumbing, not part of the TCE API.
     *
     * sc_main() uses it to connect this LSU model to the single shared
     * SharedMemory object.
     */
    void setSharedMemory(SharedMemory* memory)
    {
        shared_memory = memory;
    }
};


/*
 * -----------------------------------------------------------------------------
 * Host-side helpers
 * -----------------------------------------------------------------------------
 */

template <size_t N>
std::vector<uint8_t> stringToBits(const char (&text)[N])
{
    std::vector<uint8_t> bits(
        MESSAGE_BITS_LENGTH,
        0);

    for (size_t j = 0; j < MESSAGE_LENGTH; ++j)
    {
        const unsigned char c =
            static_cast<unsigned char>(text[j]);

        for (int i = 7; i >= 0; --i)
        {
            bits[j * 8 + (7 - i)] =
                (c >> i) & 1U;
        }
    }

    return bits;
}


template <typename BitVector>
std::array<char, MESSAGE_LENGTH>
bitsToString(const BitVector& bits)
{
    std::array<char, MESSAGE_LENGTH> text{};

    for (size_t j = 0; j < MESSAGE_LENGTH; ++j)
    {
        unsigned char c = 0;

        for (int b = 0; b < 8; ++b)
        {
            c = static_cast<unsigned char>(
                (c << 1) |
                (bits[j * 8 + b] & 1U));
        }

        text[j] = static_cast<char>(c);
    }

    return text;
}


/*
 * -----------------------------------------------------------------------------
 * Monte-Carlo LDPC simulation
 * -----------------------------------------------------------------------------
 *
 * The host does NOT execute LDPC encoding or decoding.
 *
 * It only:
 *
 *     1. writes EXT_MSG;
 *     2. starts encoder TTA;
 *     3. waits for encoder TTA;
 *     4. reads RM_ENC;
 *     5. performs BPSK + AWGN;
 *     6. writes LLR;
 *     7. starts decoder TTA;
 *     8. waits for decoder TTA;
 *     9. reads MSG_CAP;
 *    10. performs error counting.
 *
 * This is the same system-level orchestration role that the SystemC process
 * plays in the OpenASIP Manual's multicore example.
 * -----------------------------------------------------------------------------
 */

void example_run_LDPC(
    NormDistRandomEngine norm_dist_random_engine,
    sc_clock& clk,
    SharedMemory& shared,
    const char* encode_adf,
    const char* encode_tpef,
    const char* decode_adf,
    const char* decode_tpef)
{
    (void)encode_adf;
    (void)encode_tpef;
    (void)decode_adf;
    (void)decode_tpef;

    const std::vector<float> EsN0_dB(
        ESN0_DB_TABLE,
        ESN0_DB_TABLE + N0_TABLE_SIZE);

    const std::vector<float> N0(
        N0_TABLE,
        N0_TABLE + N0_TABLE_SIZE);

    assert(EsN0_dB.size() == N0.size());

    std::vector<float> ber(
        N0.size(),
        0.0f);

    std::vector<float> bler(
        N0.size(),
        0.0f);

    std::vector<unsigned> n_bit_errs(
        N0.size(),
        0);

    std::vector<unsigned> n_blk_errs(
        N0.size(),
        0);

    /*
     * -------------------------------------------------------------------------
     * Wait for encoder startup
     * -------------------------------------------------------------------------
     *
     * The encoder constructs the persistent nrLDPC object before setting
     *
     *     STARTUP = 1
     *
     * The decoder waits for the same signal.
     */
    while (shared.read32(STARTUP_ADDR) == 0)
    {
        wait(clk.period());
    }

    //printf(
    //    "[HOST] Encoder signaled STARTUP\n");

    //printf(
    //    "[HOST] FILLER_LENGTH raw = %u\n",
    //    shared.read32(FILLER_LENGTH_ADDR));

    //printf(
    //    "[HOST] NRLDPC bytes:\n");

    //for (unsigned i = 0; i < 64; ++i)
    //{
    //    printf(
    //        "%02x%s",
    //        shared.read8(NRLDPC_ADDR + i),
    //        ((i + 1) % 16 == 0) ? "\n" : " ");
    //}

    const size_t fillerLength =
        shared.read32(FILLER_LENGTH_ADDR);

    std::vector<uint8_t> fillers(
        fillerLength,
        0);

    std::vector<uint8_t> complete_message =
        stringToBits(message);

    const unsigned paddingBits =
        INFO_BITS_LENGTH - MESSAGE_BITS_LENGTH;

    if (complete_message.size() > INFO_BITS_LENGTH)
    {
        std::printf(
            "[Runtime Error] Message too large.\n");

        return;
    }

    std::printf(
        "[t_sim [s] = %.6f] "
        "Began simulated TTA CPU clock time count\n",
        sc_time_stamp().to_seconds());

    /*
     * -------------------------------------------------------------------------
     * SNR loop
     * -------------------------------------------------------------------------
     */

    for (unsigned i = 0;
         i < N0.size();
         ++i)
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

        /*
         * ---------------------------------------------------------------------
         * Block loop
         * ---------------------------------------------------------------------
         */

        while (n_blks_done < N_MAX_BLKS &&
               n_blk_errs[i] < N_MAX_BLKS_ERRS)
        {
            /*
             * -----------------------------------------------------------------
             * Information block
             * -----------------------------------------------------------------
             */

            std::vector<uint8_t> msg(
                INFO_BITS_LENGTH,
                0);

            std::copy(
                complete_message.begin(),
                complete_message.end(),
                msg.begin());

            /*
             * Append filler bits.
             */
            std::vector<uint8_t> extMsg;

            extMsg.reserve(
                msg.size() +
                fillers.size());

            extMsg.insert(
                extMsg.end(),
                msg.begin(),
                msg.end());

            extMsg.insert(
                extMsg.end(),
                fillers.begin(),
                fillers.end());

            assert(
                extMsg.size() <=
                MAX_INFO_NODE_BITS);

            /*
             * -----------------------------------------------------------------
             * Encoder TTA
             * -----------------------------------------------------------------
             */

            std::printf(
                "[t_sim [s] = %.6f]"
                "[SNR_0%u] "
                "Started encode_core_prog\n",
                sc_time_stamp().to_seconds(),
                i);

            const sc_time encode_start =
                sc_time_stamp();

            shared.write32(
                SIM_TIME_ADDR,
                static_cast<unsigned>(
                    sc_time_stamp().to_seconds() * 1e6));

            shared.write32(
                SNR_G_ID_ADDR,
                i);

            shared.write32(
                BLK_G_ID_ADDR,
                n_blks_done + 1);

            /*
             * Copy EXT_MSG into the simulated shared address space.
             */
            uint8_t* shared_extMsg =
                shared.ptr<uint8_t>(
                    EXT_MSG_ADDR,
                    extMsg.size());

            std::memcpy(
                shared_extMsg,
                extMsg.data(),
                extMsg.size());

            /*
             * 0 means:
             *
             *     "new input is available; encoder may process it"
             */
            shared.write32(
                ENC_STATUS_ADDR,
                0);

            //std::printf(
            //    "[t_sim (host) [s] = %.6f]"
            //    "[SNR_0%u] "
            //    "Host requested encode: "
            //    "ENC_STATUS=%u "
            //    "DEC_STATUS=%u "
            //    "STARTUP=%u\n",
            //    sc_time_stamp().to_seconds(),
            //    i,
            //    shared.read32(ENC_STATUS_ADDR),
            //    shared.read32(DEC_STATUS_ADDR),
            //    shared.read32(STARTUP_ADDR));

            /*
             * -----------------------------------------------------------------
             * Let both TTAs execute.
             * -----------------------------------------------------------------
             *
             * The encoder TTA is continuously running its program.
             *
             * wait() suspends this SystemC host process for one clock period,
             * allowing the encoder and decoder TTA processes to execute.
             *
             * The encoder eventually observes ENC_STATUS == 0, performs
             * encode + rate matching, writes RM_ENC, and writes ENC_STATUS = 1.
             */

            while (shared.read32(ENC_STATUS_ADDR) == 0)
            {
                wait(clk.period());
            }

            /*
             * -----------------------------------------------------------------
             * Read encoder output.
             * -----------------------------------------------------------------
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
                "[t_sim [s] = %.6f]"
                "[SNR_0%u] "
                "Ended encode_core_prog\n",
                sc_time_stamp().to_seconds(),
                i);

            std::printf(
                "[t_sim [s] = %.6f]"
                "[SNR_0%u] "
                "encode elapsed time [s]: %.6f\n",
                sc_time_stamp().to_seconds(),
                i,
                (encode_end - encode_start).to_seconds());

            /*
             * -----------------------------------------------------------------
             * BPSK + AWGN
             * -----------------------------------------------------------------
             */

            std::vector<float> r(
                CODE_WORD_BITS_LENGTH);

            for (size_t j = 0;
                 j < rm_enc.size();
                 ++j)
            {
                const float transmitted =
                    rm_enc[j]
                        ? -1.0f
                        : 1.0f;

                const float noise =
                    std::sqrt(N0[i] / 2.0f) *
                    norm_dist_random_engine();

                r[j] =
                    transmitted +
                    noise;
            }

            /*
             * -----------------------------------------------------------------
             * LLR
             * -----------------------------------------------------------------
             */

            std::vector<float> llr(
                CODE_WORD_BITS_LENGTH);

            for (size_t j = 0;
                 j < r.size();
                 ++j)
            {
                llr[j] =
                    4.0f *
                    r[j] /
                    N0[i];
            }

            /*
             * -----------------------------------------------------------------
             * Decoder TTA
             * -----------------------------------------------------------------
             */

            std::printf(
                "[t_sim [s] = %.6f]"
                "[SNR_0%u] "
                "Started decode\n",
                sc_time_stamp().to_seconds(),
                i);

            const sc_time decode_start =
                sc_time_stamp();

            /*
             * Copy LLR into shared memory.
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

            shared.write32(
                SNR_G_ID_ADDR,
                i);

            shared.write32(
                BLK_G_ID_ADDR,
                n_blks_done + 1);

            shared.write32(
                ACTUAL_CODEWORD_LENGTH_ADDR,
                static_cast<unsigned>(
                    llr.size()));

            /*
             * 0 means:
             *
             *     "new LLR input is available; decoder may process it"
             */
            shared.write32(
                DEC_STATUS_ADDR,
                0);

            /*
             * Wait for decoder TTA.
             */
            while (shared.read32(DEC_STATUS_ADDR) == 0)
            {
                wait(clk.period());
            }

            /*
             * -----------------------------------------------------------------
             * Read decoder output.
             * -----------------------------------------------------------------
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
                "[t_sim [s] = %.6f]"
                "[SNR_0%u] "
                "Ended decode\n",
                sc_time_stamp().to_seconds(),
                i);

            std::printf(
                "[t_sim [s] = %.6f]"
                "[SNR_0%u] "
                "decode elapsed time [s]: %.6f\n",
                sc_time_stamp().to_seconds(),
                i,
                (decode_end - decode_start).to_seconds());

            /*
             * -----------------------------------------------------------------
             * Error counting
             * -----------------------------------------------------------------
             */

            unsigned totalErrors = 0;
            unsigned messageErrors = 0;
            unsigned paddingErrors = 0;

            for (unsigned j = 0;
                 j < INFO_BITS_LENGTH;
                 ++j)
            {
                if ((msg[j] != 0) !=
                    (msg_cap[j] != 0))
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
                    std::vector<uint8_t>
                        decodedMessageBits(
                            MESSAGE_BITS_LENGTH);

                    std::copy(
                        msg_cap.begin(),
                        msg_cap.begin() +
                            MESSAGE_BITS_LENGTH,
                        decodedMessageBits.begin());

                    const std::array<char, MESSAGE_LENGTH>
                        decoded =
                            bitsToString(
                                decodedMessageBits);

                    std::printf(
                        "\n=====================================================\n");

                    std::printf(
                        "First decoding failure at "
                        "Es/N0 = %f dB\n",
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

                        for (size_t c = 0;
                             c < MESSAGE_LENGTH;
                             ++c)
                        {
                            if (message[c] != decoded[c])
                            {
                                std::printf(
                                    "  position %lu : "
                                    "'%c' -> '%c'\n",
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
                            "Errors occurred only in "
                            "the zero-padding region.\n");
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
                static_cast<float>(
                    n_bit_errs[i]) /
                static_cast<float>(
                    INFO_BITS_LENGTH) /
                static_cast<float>(
                    n_blks_done);

            bler[i] =
                static_cast<float>(
                    n_blk_errs[i]) /
                static_cast<float>(
                    n_blks_done);

            if (n_blks_done % 10 == 0 ||
                n_blks_done == 1)
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

    std::printf(
        "\nModulation: BPSK\n");

    std::printf(
        "[M,R] = [%d,%f]\n",
        CODE_WORD_BITS_LENGTH,
        CODE_RATE);

    std::printf(
        "EsN0_db = [");

    for (float value : EsN0_dB)
        std::printf(
            "%f ",
            value);

    std::printf(
        "]\n");

    std::printf(
        "BER = [");

    for (float value : ber)
        std::printf(
            "%f ",
            value);

    std::printf(
        "]\n");

    std::printf(
        "BLER = [");

    for (float value : bler)
        std::printf(
            "%f ",
            value);

    std::printf(
        "]\n");
}


/*
 * -----------------------------------------------------------------------------
 * SystemC host controller
 * -----------------------------------------------------------------------------
 *
 * example_run_LDPC() remains a normal C++ function so that its existing
 * structure and logic remain unchanged.
 *
 * The important difference is that it is now called from an SC_THREAD.
 *
 * Therefore wait() inside example_run_LDPC() is legal and suspends only this
 * host process, allowing the encoder and decoder TTA processes to continue
 * executing during the wait.
 * -----------------------------------------------------------------------------
 */

SC_MODULE(LDPCHost)
{
    NormDistRandomEngine& norm_dist_random_engine;
    sc_clock& clk;
    SharedMemory& shared;

    const char* encode_adf;
    const char* encode_tpef;
    const char* decode_adf;
    const char* decode_tpef;

    SC_HAS_PROCESS(LDPCHost);

    LDPCHost(
        sc_module_name name,
        NormDistRandomEngine& norm_dist_random_engine_,
        sc_clock& clk_,
        SharedMemory& shared_,
        const char* encode_adf_,
        const char* encode_tpef_,
        const char* decode_adf_,
        const char* decode_tpef_)
        :
        sc_module(name),
        norm_dist_random_engine(norm_dist_random_engine_),
        clk(clk_),
        shared(shared_),
        encode_adf(encode_adf_),
        encode_tpef(encode_tpef_),
        decode_adf(decode_adf_),
        decode_tpef(decode_tpef_)
    {
        SC_THREAD(run);
    }

    void run()
    {
        example_run_LDPC(
            norm_dist_random_engine,
            clk,
            shared,
            encode_adf,
            encode_tpef,
            decode_adf,
            decode_tpef);

        /*
         * The TTA programs themselves contain permanent loops.
         *
         * Therefore the SystemC simulation would otherwise continue forever.
         *
         * The host controller is the process that knows when the complete
         * Monte-Carlo experiment has finished, so it requests simulation stop.
         */
        sc_stop();
    }
};


/*
 * -----------------------------------------------------------------------------
 * SystemC entry point
 * -----------------------------------------------------------------------------
 *
 * This intentionally follows Appendix B:
 *
 *     1. create clock;
 *     2. create global lock;
 *     3. create shared system-level state;
 *     4. instantiate TTA core;
 *     5. connect clock/global lock;
 *     6. instantiate LSU simulation model;
 *     7. attach LSU model to TTA core;
 *     8. connect LSU model to shared state;
 *     9. instantiate second TTA;
 *    10. repeat LSU connection;
 *    11. instantiate the host SystemC process;
 *    12. start the complete SystemC simulation.
 * -----------------------------------------------------------------------------
 */

int sc_main(
    int argc,
    char* argv[])
{
    /*
     * The architecture and TPEF names are supplied by the build system.
     *
     * Keep these declarations consistent with the existing build-generated
     * configuration in your project.
     */
    if (argc != 5)
    {
        std::fprintf(
            stderr,
            "Usage: %s "
            "<encode.adf> <encode.tpef> "
            "<decode.adf> <decode.tpef>\n",
            argv[0]);

        return EXIT_FAILURE;
    }

    const char* encode_adf  = argv[1];
    const char* encode_tpef = argv[2];
    const char* decode_adf  = argv[3];
    const char* decode_tpef = argv[4];

    /*
     * 50 MHz TTA/SystemC clock.
     *
     * Appendix B uses:
     *
     *     sc_clock clk("clock", 1, SC_US);
     *
     * We retain your existing 2 us period here.
     */
    sc_clock clk(
        "clock",
        2,
        SC_US);

    /*
     * Global lock shared by both TTA cores.
     */
    sc_signal<bool> glock;

    /*
     * One system-level shared memory object.
     *
     * Both TTA LSU models will refer to this same object.
     */
    SharedMemory shared;


    /*
     * -------------------------------------------------------------------------
     * Initial shared state
     * -------------------------------------------------------------------------
     */

    shared.write32(
        STARTUP_ADDR,
        0);

    /*
     * Status convention:
     *
     *     0 = request/pending
     *     1 = completed/idle
     *
     * Encoder and decoder initially have no pending work.
     */
    shared.write32(
        ENC_STATUS_ADDR,
        1);

    shared.write32(
        DEC_STATUS_ADDR,
        1);


    /*
     * -------------------------------------------------------------------------
     * Encoder TTA
     * -------------------------------------------------------------------------
     */

    TTACore encoder_tta(
        "encoder_tta",
        encode_adf,
        encode_tpef);

    encoder_tta.clock(
        clk);

    encoder_tta.global_lock(
        glock);


    /*
     * Encoder LSU model.
     *
     * This corresponds directly to Appendix B:
     *
     *     LSUModel lsu1("LSU1");
     *     sender_tta.setOperationSimulator("LSU", lsu1);
     */
    LSUModel lsu1(
        "LSU1");

    encoder_tta.setOperationSimulator(
        "lsu",
        lsu1);
    encoder_tta.setOperationSimulator(
        "lsu_1",
        lsu1);
    encoder_tta.setOperationSimulator(
        "lsu_1_1",
        lsu1);

    /*
     * Application-specific connection to our shared memory.
     */
    lsu1.setSharedMemory(
        &shared);


    /*
     * -------------------------------------------------------------------------
     * Decoder TTA
     * -------------------------------------------------------------------------
     */

    TTACore decoder_tta(
        "decoder_tta",
        decode_adf,
        decode_tpef);

    decoder_tta.clock(
        clk);

    decoder_tta.global_lock(
        glock);


    /*
     * Decoder LSU model.
     *
     * Again this follows Appendix B:
     *
     *     LSUModel lsu2("LSU2");
     *     recv_tta.setOperationSimulator("LSU", lsu2);
     */
    LSUModel lsu2(
        "LSU2");

    decoder_tta.setOperationSimulator(
        "lsu",
        lsu2);
    decoder_tta.setOperationSimulator(
        "lsu_1",
        lsu2);
    decoder_tta.setOperationSimulator(
        "lsu_1_1",
        lsu2);

    /*
     * Both LSU models point to the SAME shared-memory object.
     */
    lsu2.setSharedMemory(
        &shared);


    /*
     * -------------------------------------------------------------------------
     * Host-side SystemC process
     * -------------------------------------------------------------------------
     *
     * This must be instantiated BEFORE sc_start().
     *
     * Its SC_THREAD will execute example_run_LDPC() concurrently with both
     * TTA cores.
     */
    NormDistRandomEngine norm_dist_random_engine;

    LDPCHost host(
        "host",
        norm_dist_random_engine,
        clk,
        shared,
        encode_adf,
        encode_tpef,
        decode_adf,
        decode_tpef);


    /*
     * -------------------------------------------------------------------------
     * Diagnostics before starting the actual workload
     * -------------------------------------------------------------------------
     */

    std::fprintf(
        stderr,
        "[HOST BEFORE SIM] "
        "ENC=%u "
        "DEC=%u "
        "STARTUP=%u\n",
        shared.read32(ENC_STATUS_ADDR),
        shared.read32(DEC_STATUS_ADDR),
        shared.read32(STARTUP_ADDR));

    std::fflush(stderr);


    /*
     * -------------------------------------------------------------------------
     * Start the complete SystemC simulation.
     * -------------------------------------------------------------------------
     *
     * IMPORTANT:
     *
     * There is intentionally NO fixed:
     *
     *     sc_time runtime(1, SC_MS);
     *     sc_start(runtime);
     *
     * here.
     *
     * The encoder and decoder TTA programs contain permanent loops, so a
     * fixed runtime would eventually terminate the simulation regardless of
     * whether the LDPC experiment had completed.
     *
     * Instead, the host SC_THREAD runs example_run_LDPC(), and calls
     * sc_stop() when the entire experiment has finished.
     */
    sc_start();


    return EXIT_SUCCESS;
}
