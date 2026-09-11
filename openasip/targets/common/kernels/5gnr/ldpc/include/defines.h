#include <etl/numeric.h> /*uint8_t, uint16_t*/

#ifndef DEFINES_H
#define DEFINES_H

#ifndef LIN_SPACE_NUM
#define LIN_SPACE_NUM 9
#endif

#ifndef MESSAGE_LENGTH
//#define MESSAGE_LENGTH 15 // including null terminator (\0)
#define MESSAGE_LENGTH 303 // including null terminator (\0)
#endif

#ifndef MESSAGE_BITS_LENGTH
constexpr int MESSAGE_BITS_LENGTH = MESSAGE_LENGTH * 8; //120
#endif

//#ifndef CODE_WORD_BITS_LENGTH
//#define CODE_WORD_BITS_LENGTH 1000
//#endif
#ifndef CODE_WORD_BITS_LENGTH
//#define CODE_WORD_BITS_LENGTH 9000 // change until bler starts falling
#define CODE_WORD_BITS_LENGTH 8000 // change until bler starts falling
#endif

#ifndef CODE_RATE_NUM
#define CODE_RATE_NUM 1
#endif

#ifndef CODE_RATE_DEN
#define CODE_RATE_DEN 3
#endif

#ifndef CODE_RATE
constexpr float CODE_RATE = static_cast<float>(CODE_RATE_NUM) / static_cast<float>(CODE_RATE_DEN);
#endif

#ifndef INFO_BITS_LENGTH
#define INFO_BITS_LENGTH 3000
#endif

#ifndef INFO_LENGTH
constexpr int INFO_LENGTH = INFO_BITS_LENGTH / 8; //41
#endif

#ifndef N_MAX_ITER
#define N_MAX_ITER 8 //number of decoders
#endif

#ifndef N_MAX_BLKS
//#define N_MAX_BLKS 10000
#define N_MAX_BLKS 10
#endif

#ifndef EXT_MSG_LENGTH
#define EXT_MSG_LENGTH 3200
#endif

constexpr size_t MAX_CB = 68; // max block columns (BG1 case)
constexpr size_t MAX_ROWS = 46; // max block rows (BG1 case)
constexpr size_t MAX_EDGES = 316; // max tanner graph edges (BG1 case)
constexpr size_t MAX_KB = 22; // max information block columns (BG1 case), the rest are parity columns (46)
constexpr size_t MAX_ZC = 384;
constexpr size_t MAX_PCM_ROWS = MAX_ROWS * MAX_ZC;  // max number of check nodes, 17664
constexpr size_t MAX_PCM_COLS = MAX_CB * MAX_ZC;  // max number of variable nodes, 26112
constexpr size_t MAX_CHECK_NODE_DEGREE = 19;
constexpr size_t MAX_CODEWORD_LENGTH = MAX_PCM_COLS; // max size for ldpc::encode() input, 26112
constexpr size_t MAX_INFO_NODE_BITS = MAX_KB * MAX_ZC;

//#ifndef GLOBAL_VARS
//#define GLOBAL_VARS
//
//inline unsigned int snr_g = 0;
//inline unsigned int blk_g = 0;
//
//#endif

#ifdef MULTICORE_TTA

/*
 * --------------------------------------------------------------------------
 * Shared memory address space
 * --------------------------------------------------------------------------
 *
*TTA local DMEM
*┌───────────────────────────────┐
*│ 0 ... 32767                   │
*└───────────────────────────────┘
*              │
*              ▼
*shared memory
*┌───────────────────────────────┐
*│ ENC_STATUS                    │ +0
*│ DEC_STATUS                    │ +4
*│ SIM_TIME                      │ +8
*│ SNR_G_ID                      │ +12
*├───────────────────────────────┤
*│ nrLDPC object                 │ +16
*│                               │
*│ NRLDPC_STORAGE_SIZE           │
*├───────────────────────────────┤
*│ EXT_MSG                       │
*│ MAX_INFO_NODE_BITS bytes      │
*├───────────────────────────────┤
*│ RM_ENC                        │
*│ CODE_WORD_BITS_LENGTH bytes   │
*├───────────────────────────────┤
*│ LLR                           │
*│ MAX_CODEWORD_LENGTH floats    │
*├───────────────────────────────┤
*│ MSG_CAP                       │
*│ MAX_INFO_NODE_BITS bytes      │
*└───────────────────────────────┘
 *
 * TTA local DMEM:
 *
 *     [0, LAST_DMEM_ADDR]
 *
 * Shared memory starts immediately afterwards.
 *
 * The first part contains control registers. The remaining regions are
 * ordinary raw byte/float buffers.
 */

#define LAST_DMEM_ADDR  (32 * 1024 - 1)
#define SHARED_MEM_BASE (LAST_DMEM_ADDR + 1)

/*
 * Control/status registers: pointer addresses
 *
 * 0 = processing/busy
 * 1 = idle/task finished
 */
#define ENC_STATUS_ADDR              (SHARED_MEM_BASE + 0 * 4)
#define DEC_STATUS_ADDR              (SHARED_MEM_BASE + 1 * 4)
#define SIM_TIME_ADDR                (SHARED_MEM_BASE + 2 * 4)
#define SNR_G_ID_ADDR                (SHARED_MEM_BASE + 3 * 4)
#define BLK_G_ID_ADDR                (SHARED_MEM_BASE + 4 * 4)
#define STARTUP_ADDR                 (SHARED_MEM_BASE + 5 * 4)
#define FILLER_LENGTH_ADDR           (SHARED_MEM_BASE + 6 * 4)
#define ACTUAL_CODEWORD_LENGTH_ADDR  (SHARED_MEM_BASE + 7 * 4)

#define ENC_STATUS_PTR ((volatile unsigned int*)ENC_STATUS_ADDR)
#define DEC_STATUS_PTR ((volatile unsigned int*)DEC_STATUS_ADDR)
#define SIM_TIME_PTR ((volatile unsigned int*)SIM_TIME_ADDR)
#define SNR_G_ID_PTR ((volatile unsigned int*)SNR_G_ID_ADDR)
#define BLK_G_ID_PTR ((volatile unsigned int*)BLK_G_ID_ADDR)
#define STARTUP_PTR ((volatile unsigned int*)STARTUP_ADDR)
#define FILLER_LENGTH_PTR ((volatile unsigned int*)FILLER_LENGTH_ADDR)
#define ACTUAL_CODEWORD_LENGTH_PTR ((volatile unsigned int*)ACTUAL_CODEWORD_LENGTH_ADDR)

/*
 * --------------------------------------------------------------------------
 * Persistent LDPC object
 * --------------------------------------------------------------------------
 *
 * Constructed exactly once by the encoder during startup.
 *
 * The decoder does NOT construct another nrLDPC object. It obtains the
 * existing object with:
 *
 *     nrLDPC* ldpc =
 *         reinterpret_cast<nrLDPC*>(NRLDPC_ADDR);
 */
#define NRLDPC_ADDR (SHARED_MEM_BASE + 8 * 4)


/*
 * --------------------------------------------------------------------------
 * Raw data buffers
 * --------------------------------------------------------------------------
 *
 * Keep these after NRLDPC_ADDR.
 *
 * NRLDPC_STORAGE_SIZE must be large enough for the actual nrLDPC object.
 * It is deliberately generous here; you can reduce it after measuring
 * sizeof(nrLDPC) in the TTA compilation environment.
 */
#define NRLDPC_STORAGE_SIZE  (16 * 1024)
#define EXT_MSG_ADDR (NRLDPC_ADDR + NRLDPC_STORAGE_SIZE)
#define RM_ENC_ADDR  (EXT_MSG_ADDR + MAX_INFO_NODE_BITS * sizeof(uint8_t))
#define LLR_ADDR     (RM_ENC_ADDR + CODE_WORD_BITS_LENGTH * sizeof(uint8_t))
#define MSG_CAP_ADDR (LLR_ADDR + MAX_CODEWORD_LENGTH * sizeof(float))

#else /* single-core TTA */

inline unsigned int snr_g = 0;
inline unsigned int blk_g = 0;

#endif

#endif //DEFINES_H
