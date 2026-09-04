//---------------------------------------------------------------------------------------------
// 5G NR LDPC encoding/decoding C++ encapsulation&implementation
// supports rate matching and rate recovery
// Dr J Mao 2021 Oct
// juquan.justin.mao@gmail.com
//--------------------------------------------------------------------------------------------

#ifndef NR_LDPC_H
#define NR_LDPC_H
#include "defines.h"

#include <stdint.h> /*uint8_t, uint16_t*/
#include<etl/numeric.h> /*size_t*/
#include<etl/vector.h>
#include <assert.h>
#include <etl/array.h>
#include <etl/algorithm.h> /* sort, transform, rotate*/
#include <etl/functional.h> /*xor*/

#include <etl/limits.h>
#include <etl/memory.h>
#include <etl/utility.h>

// 3GPP LDPC related tables
extern const uint16_t liftSizeTable[8][8];
extern const uint16_t shiftTableBgn_1[316][10];
extern const uint16_t shiftTableBgn_2[197][10];

//an edge is a connection from a check node to a variable node in a tanner graph
// along with the corresponding shift coefficient
struct edge_t {
	uint16_t cNodeIdx;
	uint16_t vNodeIdx;
	uint16_t nShifts;
};

// a layer includes all the connections(edges) to a check node in a tanner graph
struct layer_t {
	uint16_t edgeStart;
	uint16_t edgeEnd;
};

class nrLDPC {
public:
	size_t mKBar;          // info length excluding fillers
	size_t mK;             // info length including fillers
	float      mR;             // code rate
	uint8_t     mBGn;           // base graph identifier
	uint16_t    mZc;            // lifting size
	uint8_t     mSetIdx;        // the set that shift coefficients belongs to 0-7
	size_t mF;             // number of fillers to make info length is 22*Zc 0r 10*Zc
	size_t mN;             // mother code word length
	// edges are sorted by check node idx and variable node idx
    etl::vector<edge_t, MAX_EDGES> mEdges;
	// the i-th layer stores all the connections(edges) to the i-th check node in a tanner graph
    etl::vector<layer_t, MAX_ROWS> mLayers;

public:
	nrLDPC(size_t infoLen, float codeRate);
	// encoding
    etl::vector<bool,MAX_CODEWORD_LENGTH> encode(const etl::vector<bool,MAX_INFO_NODE_BITS>& msg);
	// decoding
    etl::vector<bool, MAX_INFO_NODE_BITS> decode(const etl::vector<float, MAX_CODEWORD_LENGTH>& softBitsIn, const unsigned nMaxIter);
	// rate matching
	etl::vector<bool,CODE_WORD_BITS_LENGTH> rateMatch(const etl::vector<bool,MAX_CODEWORD_LENGTH>& bitsIn, size_t nOfBitOut);
	// rate recovery
	etl::vector<float,MAX_CODEWORD_LENGTH> rateRecover(const etl::vector<float,CODE_WORD_BITS_LENGTH>& softBitsIn);
	// checkSum a code word
	bool checkSumCodeWord(etl::vector<bool,MAX_CODEWORD_LENGTH>& cw);
	// filler length
	size_t getFillerLength() { return mF; }

public:
	static uint8_t selectBaseGraph(size_t KBar, float R);
	static uint16_t selectLiftSize(size_t KBar, uint8_t BGn);
	static uint8_t selectShiftSet(uint16_t Zc);
	static etl::vector<etl::vector<bool,MAX_PCM_COLS>,MAX_PCM_ROWS> makeParityCheckMatrix(uint8_t BGn, uint16_t Zc);

private:
	// core decoding module
    etl::vector<etl::vector<float, MAX_ZC>,MAX_CHECK_NODE_DEGREE> checkNodeOperation(const etl::vector<etl::vector<float, MAX_ZC>,MAX_CHECK_NODE_DEGREE>& msgIn);
	template<typename T>
	inline etl::vector<size_t,MAX_CHECK_NODE_DEGREE> sort_indexes(const etl::vector<T,MAX_CHECK_NODE_DEGREE>& v);
    template<size_t ROW_CAPACITY,size_t COL_CAPACITY> etl::vector<etl::vector<float, ROW_CAPACITY>,COL_CAPACITY>
    transposeMat(const etl::vector<etl::vector<float, COL_CAPACITY>,ROW_CAPACITY>& mat);

	// wrapper functions to enhance readability
	template<typename T>
	inline etl::vector<T,MAX_ZC> circShift(const etl::vector<T,MAX_ZC>& vecIn, const unsigned nShifts);
	inline etl::vector<bool,MAX_ZC> eleWiseXor(const etl::vector<bool,MAX_ZC>& vec1, const etl::vector<bool,MAX_ZC>& vec2);
    inline etl::vector<float,MAX_ZC> eleWisePlus(const etl::vector<float,MAX_ZC>& vec1, const etl::vector<float,MAX_ZC>& vec2);
    inline etl::vector<float,MAX_ZC> eleWiseMinus(const etl::vector<float,MAX_ZC>& vec1, const etl::vector<float,MAX_ZC>& vec2);
};

#endif
