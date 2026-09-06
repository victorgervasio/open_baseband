#include <etl/numeric.h> /*uint8_t, uint16_t*/

#ifndef LIN_SPACE_NUM
#define LIN_SPACE_NUM 9
#endif

#ifndef MESSAGE_LENGTH
#define MESSAGE_LENGTH 16
#endif

#ifndef MESSAGE_BITS_LENGTH
constexpr int MESSAGE_BITS_LENGTH = MESSAGE_LENGTH * 8; //112 
#endif

//#ifndef CODE_WORD_BITS_LENGTH
//#define CODE_WORD_BITS_LENGTH 1000
//#endif
#ifndef CODE_WORD_BITS_LENGTH
#define CODE_WORD_BITS_LENGTH 9000 // change until bler starts falling
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
#define N_MAX_BLKS 10000
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

#ifndef GLOBAL_VARS
#define GLOBAL_VARS

inline unsigned int snr_g = 0;
inline unsigned int blk_g = 0;

#endif
