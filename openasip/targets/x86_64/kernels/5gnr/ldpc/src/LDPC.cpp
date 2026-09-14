#include "LDPC.h"
#include <math.h>   
using namespace  std;

nrLDPC::nrLDPC(size_t infoLen, float codeRate)
{
	mKBar = infoLen; // 3000 = INFO_BITS_LENGTH(3000,defines.h) -> assume previous Code Block Segmentation (TS 38.212-j40, cap. 5.2.2)
	mR = codeRate; // (float)(1/3) = CODE_RATE(static_cast<float>(CODE_RATE_NUM(1,defines.h)) / static_cast<float>(CODE_RATE_DEN(3,defines.h)),defines.h)

	// select base graph based on 3GPP 38.212 7.2.2
	mBGn = selectBaseGraph(mKBar, mR); // 2 = f(INFO_BITS_LENGTH(3000,defines.h),CODE_RATE(static_cast<float>(CODE_RATE_NUM(1,defines.h)) / static_cast<float>(CODE_RATE_DEN(3,defines.h)),defines.h)

	// select lifting size
	mZc = selectLiftSize(mKBar, mBGn); // 320 = f(INFO_BITS_LENGTH(3000,defines.h),CODE_RATE(static_cast<float>(CODE_RATE_NUM(1,defines.h)) / static_cast<float>(CODE_RATE_DEN(3,defines.h)),defines.h)

	// select shifting set
	mSetIdx = selectShiftSet(mZc); // 2 = f(INFO_BITS_LENGTH(3000,defines.h),CODE_RATE(static_cast<float>(CODE_RATE_NUM(1,defines.h)) / static_cast<float>(CODE_RATE_DEN(3,defines.h)),defines.h)

	// systematic bits length and parity bits length
	if (mBGn == 1) {
		mK = 22 * mZc; mN = 68 * mZc;
	}
	else {
		mK = 10 * mZc;  mN = 52 * mZc; // mK = 3200 = f(INFO_BITS_LENGTH(3000,defines.h),CODE_RATE(static_cast<float>(CODE_RATE_NUM(1,defines.h)) / static_cast<float>(CODE_RATE_DEN(3,defines.h)),defines.h)
	}                                  // mN = 16640 = f(INFO_BITS_LENGTH(3000,defines.h),CODE_RATE(static_cast<float>(CODE_RATE_NUM(1,defines.h)) / static_cast<float>(CODE_RATE_DEN(3,defines.h)),defines.h)

	// fillers length
	mF = mK - mKBar; // mF = 200 = f(INFO_BITS_LENGTH(3000,defines.h),CODE_RATE(static_cast<float>(CODE_RATE_NUM(1,defines.h)) / static_cast<float>(CODE_RATE_DEN(3,defines.h)),defines.h)

	// build up edges and shifts
	if (mBGn == 1) {
		mEdges.reserve(316); // number of rows of 3GPP TS 38.212 - Table 5.3.2-2: LDPC base graph 1 and its parity check matrices, see nrLDPCTables.cpp
		for (unsigned i = 0; i < 316; i++) {
			mEdges.push_back(edge_t());
			mEdges[i] = { shiftTableBgn_1[i][0],shiftTableBgn_1[i][1],uint16_t(shiftTableBgn_1[i][mSetIdx + 2] % mZc) }; // mEdges[i] = { cNodeIdx , vNodeIdx, nShifts }, see LDPC.h
		}
	}
	else {
		mEdges.reserve(197); // number of rows of 3GPP TS 38.212 - Table 5.3.2-2: LDPC base graph 2 and its parity check matrices, see nrLDPCTables.cpp
		for (unsigned i = 0; i < 197; i++) {
			mEdges.push_back(edge_t());
			mEdges[i] = { shiftTableBgn_2[i][0],shiftTableBgn_2[i][1], uint16_t(shiftTableBgn_2[i][mSetIdx + 2] % mZc) }; // mEdges[i] = { cNodeIdx , vNodeIdx, nShifts }, see LDPC.h
		}
	}

	// build up layers
	if (mBGn == 1) {
		mLayers = { {0,19},{19,38},{38,57},{57,76},{76,79},{79,87},{87,96},{96,103},
				   {103,113},{113,122},{122,129},{129,137},{137,144},{144,150},{150,157},
				   {157,164},{164,170},{170,176},{176,182},{182,188},{188,194},{194,200},
				   {200,205},{205,210},{210,216},{216,221},{221,226},{226,230},{230,235},
				   {235,240},{240,245},{245,250},{250,255},{255,260},{260,265},{265,270},
				   {270,275},{275,279},{279,284},{284,289},{289,293},{293,298},{298,302},
				   {302,307},{307,312},{312,316} }; // mLayers[i] = { edgeStart, edgeEnd }, see LDPC.h
	}
	else {
		mLayers = { {0,8},{8,18},{18,26},{26,36},{36,40},{40,46},{46,52},
				   {52,58},{58,62},{62,67},{67,72},{72,77},{77,81},{81,86},
				   {86,91},{91,95},{95,100},{100,105},{105,109},{109,113},
				   {113,117},{117,121},{121,124},{124,128},{128,132},{132,135},
				   {135,140},{140,143},{143,147},{147,150},{150,155},{155,158},
				   {158,162},{162,166},{166,170},{170,174},{174,178},{178,181},
				   {181,185},{185,189},{189,193},{193,197} }; // mLayers[i] = { edgeStart, edgeEnd }, see LDPC.h
	}
}

vector<bool> nrLDPC::encode(const vector<bool>& msg)
{
	size_t  Kb, Cb, totLayers;
	if (mBGn == 1) {
		Kb = 22; Cb = 68; totLayers = 46;
	}
	else {
		Kb = 10; Cb = 52; totLayers = 42;
	}

	assert(Kb * mZc == msg.size());

	// initialize encoded bits in nodes(vectors of size Zc)
	// the first Kb nodes correspond to information bits, the rest  parity bits
	vector<vector<bool>> cWord(Cb);
	for (unsigned i = 0; i < Kb; i++) {
		cWord[i] = vector<bool>(msg.begin() + i * mZc, msg.begin() + (i + 1) * mZc);
	}
	for (unsigned i = Kb; i < Cb; i++) {
		cWord[i] = vector<bool>(mZc, 0);
	}

	uint16_t vNodeIdx, nShifts, shiftP0;

	// solve the first parity node P0
	for (unsigned i = 0; i < 4; i++) {
		for (unsigned edgeIdx = mLayers[i].edgeStart; edgeIdx < mLayers[i].edgeEnd; edgeIdx++) {
			vNodeIdx = mEdges[edgeIdx].vNodeIdx; nShifts = mEdges[edgeIdx].nShifts;
			// only count information bit nodes
			if (vNodeIdx < Kb) {
				cWord[Kb] = eleWiseXor(cWord[Kb], circShift(cWord[vNodeIdx], nShifts));
			}
			// find the shift coefficient of P0
			if (vNodeIdx == Kb && (i == 1 || i == 2)) {
				shiftP0 = nShifts;
			}
		}
	}
	// rotate back to get P0
	rotate(cWord[Kb].begin(), cWord[Kb].begin() + (mZc - shiftP0), cWord[Kb].end());

	// solve P1,P2,P3
	for (unsigned i = 0; i < 3; i++) {
		for (unsigned edgeIdx = mLayers[i].edgeStart; edgeIdx < mLayers[i].edgeEnd; edgeIdx++) {
			vNodeIdx = mEdges[edgeIdx].vNodeIdx; nShifts = mEdges[edgeIdx].nShifts;
			if (vNodeIdx <= Kb + i) {
				cWord[Kb + i + 1] = eleWiseXor(cWord[Kb + i + 1], circShift(cWord[vNodeIdx], nShifts));
			}
		}
	}
	// solve the rest parity node
	for (unsigned i = 4; i < totLayers; i++) {
		// not taking the last edge which corresponds the parity position
		for (unsigned edgeIdx = mLayers[i].edgeStart; edgeIdx < mLayers[i].edgeEnd - 1; edgeIdx++) {
			vNodeIdx = mEdges[edgeIdx].vNodeIdx; nShifts = mEdges[edgeIdx].nShifts;
			cWord[Kb + i] = eleWiseXor(cWord[Kb + i], circShift(cWord[vNodeIdx], nShifts));
		}
	}
	// flatten codeword and return
	vector<bool> cWordVec;
	for (auto vec : cWord) {
		cWordVec.insert(cWordVec.end(), vec.begin(), vec.end());
	}
	return cWordVec;
}
std::vector<bool> nrLDPC::decode(
    const std::vector<float>& softBitsIn,
    const unsigned nMaxIter)
{
    assert(softBitsIn.size() == mN);

    /*
     * ------------------------------------------------------------------------
     * RMSA state
     *
     * LLR[v]  = current variable-node LLR
     * LLR0[v] = original channel LLR
     *
     * CtoVMsg[e]    = old check-to-variable message
     * CtoVMsgNew[e] = newly calculated check-to-variable message
     *
     * Unlike the original layered decoder, ALL check nodes are processed
     * from the same old CtoVMsg state before the variable-node update.
     * ------------------------------------------------------------------------
     */

    const unsigned nNodes = mN / mZc;

    std::vector<std::vector<float>> LLR(nNodes);
    std::vector<std::vector<float>> LLR0(nNodes);

    for (unsigned i = 0; i < nNodes; ++i) {
        LLR[i] = std::vector<float>(
            softBitsIn.begin() + i * mZc,
            softBitsIn.begin() + (i + 1) * mZc
        );

        LLR0[i] = LLR[i];
    }

    /*
     * ------------------------------------------------------------------------
     * Number of layers used by the decoder.
     *
     * This is retained from the original implementation. The current RMSA
     * checkNodeOperation processes the layers represented by mLayers.
     * ------------------------------------------------------------------------
     */

    unsigned nMaxLayer;

    if (mBGn == 1) {
        nMaxLayer =
            (
                (
                    (mKBar * CODE_RATE_DEN + CODE_RATE_NUM - 1)
                    / CODE_RATE_NUM
                    + mF
                    + mZc - 1
                ) / mZc
            ) - 20;
    }
    else {
        nMaxLayer =
            (
                (
                    (mKBar * CODE_RATE_DEN + CODE_RATE_NUM - 1)
                    / CODE_RATE_NUM
                    + mF
                    + mZc - 1
                ) / mZc
            ) - 8;
    }

    /*
     * Currently the RMSA check-node operation works over mLayers.
     *
     * Keep the calculation above because it corresponds to the TTA
     * implementation and may be used later to limit the layer set.
     */
    (void)nMaxLayer;

    /*
     * ------------------------------------------------------------------------
     * Initialize C -> V messages.
     *
     * Initially:
     *
     *     Lambda_(c->v) = 0
     * ------------------------------------------------------------------------
     */

    std::vector<std::vector<float>> CtoVMsg(
        mEdges.size(),
        std::vector<float>(mZc, 0.0)
    );

    std::vector<std::vector<float>> CtoVMsgNew(
        mEdges.size(),
        std::vector<float>(mZc, 0.0)
    );

    /*
     * ------------------------------------------------------------------------
     * RMSA iterations
     * ------------------------------------------------------------------------
     */

    for (unsigned iIter = 0; iIter < nMaxIter; ++iIter) {

        /*
         * ================================================================
         * CHECK-NODE UPDATE
         *
         * V -> C:
         *
         *     VtoC[e] = LLR[v] - CtoVMsg[e]
         *
         * followed by the circulant shift associated with the edge.
         *
         * IMPORTANT:
         *
         * CtoVMsg is the OLD iteration state.
         * CtoVMsgNew receives the NEW state.
         *
         * Therefore no check node modifies LLR while another check node
         * is being processed.
         * ================================================================
         */

        std::vector<std::vector<float>> VtoCMsg(
            mEdges.size(),
            std::vector<float>(mZc, 0.0)
        );

        for (unsigned edgeIdx = 0;
             edgeIdx < mEdges.size();
             ++edgeIdx)
        {
            const unsigned vNodeIdx =
                mEdges[edgeIdx].vNodeIdx;

            const unsigned nShifts =
                mEdges[edgeIdx].nShifts;

            VtoCMsg[edgeIdx] =
                eleWiseMinus(
                    LLR[vNodeIdx],
                    CtoVMsg[edgeIdx]
                );

            VtoCMsg[edgeIdx] =
                circShift(
                    VtoCMsg[edgeIdx],
                    nShifts
                );
        }

        /*
         * ================================================================
         * CHECK-NODE PROCESSING
         * ================================================================
         */

        checkNodeOperation(
            VtoCMsg,
            CtoVMsgNew
        );

        /*
         * ================================================================
         * VARIABLE-NODE UPDATE
         *
         *     LLR[v] = LLR0[v] + sum_c CtoVMsgNew[c->v]
         *
         * Every variable node starts from the original channel LLR.
         * ================================================================
         */

        for (unsigned vNodeIdx = 0;
             vNodeIdx < LLR.size();
             ++vNodeIdx)
        {
            LLR[vNodeIdx] = LLR0[vNodeIdx];
        }

        for (unsigned edgeIdx = 0;
             edgeIdx < mEdges.size();
             ++edgeIdx)
        {
            const unsigned vNodeIdx =
                mEdges[edgeIdx].vNodeIdx;

            const unsigned nShifts =
                mEdges[edgeIdx].nShifts;

            std::vector<float> msg =
                circShift(
                    CtoVMsgNew[edgeIdx],
                    mZc - nShifts
                );

            LLR[vNodeIdx] =
                eleWisePlus(
                    LLR[vNodeIdx],
                    msg
                );
        }

        /*
         * ================================================================
         * NEW ITERATION STATE
         * ================================================================
         */

        CtoVMsg = CtoVMsgNew;
    }

    /*
     * ------------------------------------------------------------------------
     * Final hard decision.
     * ------------------------------------------------------------------------
     *
     * Flatten LLR and extract the information bits.
     *
     * Keep mKBar here because this is the existing x86_64 interface's
     * expected decoded information length.
     */

    std::vector<float> vecLLR;
    vecLLR.reserve(LLR.size() * mZc);

    for (const auto& node : LLR) {
        vecLLR.insert(
            vecLLR.end(),
            node.begin(),
            node.end()
        );
    }

    std::vector<bool> decBits(mKBar, false);

    for (unsigned i = 0; i < mKBar; ++i) {
        decBits[i] = (vecLLR[i] <= 0.0);
    }

    return decBits;
}
void nrLDPC::checkNodeOperation(
    const std::vector<std::vector<float>>& VtoCMsg,
    std::vector<std::vector<float>>& msgOut)
{
    assert(VtoCMsg.size() == mEdges.size());
    assert(msgOut.size() == mEdges.size());

    /*
     * ------------------------------------------------------------------------
     * CHECK-NODE PROCESSING
     * ------------------------------------------------------------------------
     */

    for (unsigned iLayer = 0;
         iLayer < mLayers.size();
         ++iLayer)
    {
        const unsigned edgeStart =
            mLayers[iLayer].edgeStart;

        const unsigned edgeEnd =
            mLayers[iLayer].edgeEnd;

        const unsigned nNodes =
            edgeEnd - edgeStart;

        assert(nNodes >= 2);
        assert(nNodes <= MAX_CHECK_NODE_DEGREE);

        for (unsigned z = 0;
             z < mZc;
             ++z)
        {
            /*
             * ------------------------------------------------------------
             * Find minimum, second minimum and total parity.
             * ------------------------------------------------------------
             */

            float min1 =
                std::numeric_limits<float>::max();

            float min2 =
                std::numeric_limits<float>::max();

            unsigned min1Idx = 0;

            float parity = 1.0;

            for (unsigned j = 0;
                 j < nNodes;
                 ++j)
            {
                const float value =
                    VtoCMsg[edgeStart + j][z];

                const float absValue =
                    std::fabs(value);

                if (absValue < min1)
                {
                    min2 = min1;
                    min1 = absValue;
                    min1Idx = j;
                }
                else if (absValue < min2)
                {
                    min2 = absValue;
                }

                if (value < 0.0)
                {
                    parity = -parity;
                }
            }

            /*
             * ------------------------------------------------------------
             * Generate extrinsic messages.
             *
             * No 0.5 correction here.
             *
             * The correction is applied below, after all CN outputs
             * have been generated.
             * ------------------------------------------------------------
             */

            for (unsigned j = 0;
                 j < nNodes;
                 ++j)
            {
                const float value =
                    VtoCMsg[edgeStart + j][z];

                const float sign =
                    (value < 0.0)
                        ? -1.0
                        : 1.0;

                const float magnitude =
                    (j == min1Idx)
                        ? min2
                        : min1;

                msgOut[edgeStart + j][z] =
                    magnitude *
                    parity *
                    sign;
            }
        }
    }

    /*
     * ------------------------------------------------------------------------
     * Offset-min-sum correction + inverse circulant shift.
     * ------------------------------------------------------------------------
     */

    for (unsigned iLayer = 0;
         iLayer < mLayers.size();
         ++iLayer)
    {
        const unsigned edgeStart =
            mLayers[iLayer].edgeStart;

        const unsigned edgeEnd =
            mLayers[iLayer].edgeEnd;

        const unsigned nNodes =
            edgeEnd - edgeStart;

        for (unsigned iEdge = 0;
             iEdge < nNodes;
             ++iEdge)
        {
            const unsigned edgeIdx =
                edgeStart + iEdge;

            const unsigned nShifts =
                mEdges[edgeIdx].nShifts;

            std::vector<float> msg(mZc);

            //for (unsigned z = 0;
            //     z < mZc;
            //     ++z)
            //{
            //    const float value =
            //        msgOut[edgeIdx][z];

            //    float magnitude =
            //        std::fabs(value);

            //    if (magnitude > 0.5)
            //    {
            //        magnitude -= 0.5;
            //    }
            //    else
            //    {
            //        magnitude = 0.0;
            //    }

            //    msg[z] =
            //        (value < 0.0)
            //            ? -magnitude
            //            : magnitude;
            //}

            msgOut[edgeIdx] =
                circShift(
                    msg,
                    mZc - nShifts
                );
        }
    }
}
vector<bool> nrLDPC::rateMatch(const vector<bool>& bitsIn, size_t nOfBitOut)
{
	if (mBGn == 1)
		assert(bitsIn.size() == 68 * mZc);
	else
		assert(bitsIn.size() == 52 * mZc);

	vector<bool> txBufferRing = bitsIn;

	// shortening by removing filling bits
	txBufferRing.erase(txBufferRing.begin() + mK - mF, txBufferRing.begin() + mK);

	// puncturing the first 2*Z
	txBufferRing.erase(txBufferRing.begin(), txBufferRing.begin() + 2 * mZc);

	// take nOfBitOut bits out of the ring
	vector<bool> bitsOut(nOfBitOut);
	for (unsigned i = 0, j = 0; i < nOfBitOut; i++, j++) {
		bitsOut[i] = txBufferRing[i % txBufferRing.size()];
	}
	return bitsOut;
}
vector<float> nrLDPC::rateRecover(const vector<float>& softBitsIn)
{
	unsigned rxRingLen = mN - 2 * mZc - mF;
	vector<float> rxBufferRing(rxRingLen, 0);

	// for received bits longer than the ring
	for (unsigned i = 0; i < softBitsIn.size(); i++) {
		rxBufferRing[i % rxRingLen] = rxBufferRing[i % rxRingLen] + softBitsIn[i];
	}

	// first 2*Zc with all 0
	vector<float> softBitsOut(2 * mZc, 0.0);
	// add information soft bits
	softBitsOut.insert(softBitsOut.end(), rxBufferRing.begin(), rxBufferRing.begin() + mKBar - 2 * mZc);
	//fillers
	vector<float> fillers(mF, numeric_limits<float>::infinity());
	softBitsOut.insert(softBitsOut.end(), fillers.begin(), fillers.end());
	// add parity soft bits
	softBitsOut.insert(softBitsOut.end(), rxBufferRing.begin() + mKBar - 2 * mZc, rxBufferRing.end());

	return softBitsOut;
}

uint8_t nrLDPC::selectBaseGraph(size_t KBar, float R)
{
	// 3GPP 38.212 7.2.2 LDPC base graph selection
	if (KBar <= 292 || (KBar <= 3824 && R <= 0.67) || R <= 0.25) {
		assert(KBar <= 3840);
		return 2;
	}
	else {
		assert(KBar <= 8448);
		return 1;
	}
}
uint16_t nrLDPC::selectLiftSize(size_t KBar, uint8_t BGn)
{
	// select Kb 3GPP 38.212 section 5.2.2
	uint16_t Kb;
	if (BGn == 1)
		Kb = 22;
	else {
		if (KBar > 640)
			Kb = 10;
		else if (KBar > 560)
			Kb = 9;
		else if (KBar > 192)
			Kb = 8;
		else
			Kb = 6;
	}

	// select lifting size and shifting set index based on 3GPP Table 5.3.2-1
	uint16_t  Zc = 384;
	uint16_t candiZc;
	for (unsigned i = 0; i < 8; i++) {
		for (unsigned j = 0; j < 8; j++) {
			candiZc = liftSizeTable[i][j];
			if (candiZc * Kb == KBar) // found min(Z) such that Kb*Z >= KBar (TS 38212-j40, cap. 5.2.2)
				return candiZc;
			else if (candiZc * Kb > KBar && candiZc < Zc)
				Zc = candiZc;
		}
	}
	return Zc; // min(Z) such that Kb*Z >= KBar (TS 38212-j40, cap. 5.2.2)
}
uint8_t nrLDPC::selectShiftSet(uint16_t Zc)
// find the set with index (mSetIdx, i_LS in spec) in Table 5.3.2-1 (see nrLDPCTables.cpp) witch contains mZc (Zc in spec) 
{
	assert(Zc >= 2 && Zc <= 384); // min(Zc) and max(Zc) (TS 38212-j40, Table 5.3.2-1: Sets of LDPC lifting size Z)
	for (unsigned i = 0; i < 8; i++) {
		for (unsigned j = 0; j < 8; j++) {
			if (liftSizeTable[i][j] == Zc)
				return i;
		}
	}
	cerr << " Zc is not valid!";
	return -1;
}
vector<vector<bool>> nrLDPC::makeParityCheckMatrix(uint8_t BGn, const uint16_t Zc)
{
	uint8_t setIdx = selectShiftSet(Zc);
	unsigned numOfBlkRows, numOfBlkCols, numOfShifts;
	if (BGn == 1) {
		numOfBlkRows = 46; numOfBlkCols = 68;
		numOfShifts = 316;
	}
	else {
		numOfBlkRows = 42; numOfBlkCols = 52;
		numOfShifts = 197;
	}

	// initialize parity check matrix (PCM)
	vector<vector<bool>> H(numOfBlkRows);
	for (unsigned i = 0; i < numOfBlkRows * Zc; i++) {
		H[i] = vector<bool>(numOfBlkCols * Zc, 0);
	}

	unsigned blkRowIdx, blkColIdx, shiftCoeff;
	// a shifted vecOne will be used to fill H
	// vecOne = [0,0,0,1] if Zc = 4;
	vector<bool> vecOne(Zc, 0); vecOne.back() = 1;

	for (unsigned i = 0; i < numOfShifts; i++) {
		if (BGn == 1) {
			blkRowIdx = shiftTableBgn_1[i][0];
			blkColIdx = shiftTableBgn_1[i][1];
			shiftCoeff = shiftTableBgn_1[i][2 + setIdx];
		}
		else {
			blkRowIdx = shiftTableBgn_2[i][0];
			blkColIdx = shiftTableBgn_2[i][1];
			shiftCoeff = shiftTableBgn_2[i][2 + setIdx];
		}

		// shift vector one
		vector<bool> shiftVecOne = vecOne;
		rotate(shiftVecOne.begin(), shiftVecOne.begin() + shiftCoeff, shiftVecOne.end());

		// shift right by 1 for each row in a block
		for (unsigned m = 0; m < Zc; m++) {
			rotate(shiftVecOne.begin(), shiftVecOne.begin() + 1, shiftVecOne.end());
			for (unsigned n = 0; n < Zc; n++) {
				H[Zc * blkRowIdx + m][Zc * blkColIdx + n] = shiftVecOne[n];
			}
		}
	}

	return H;
}

template<typename T>
inline vector<size_t> nrLDPC::sort_indexes(const vector<T>& v)
{
	// initialize original index locations
	vector<size_t> idx(v.size());
	iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	// using std::stable_sort instead of std::sort
	// to avoid unnecessary index re-orderings
	// when v contains elements of equal values
	stable_sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return abs(v[i1]) < abs(v[i2]); });

	return idx;
}

//template<typename T>
vector<vector<float>> nrLDPC::transposeMat(const vector<vector<float>>& mat)
{
	unsigned nRows = mat.size();
	unsigned nCols = mat[0].size();

	vector<vector<float>> matOut(nCols);
	for (unsigned i = 0; i < nCols; i++) {
		matOut[i] = vector<float>(nRows, 0);
		for (unsigned j = 0; j < nRows; j++)
			matOut[i][j] = mat[j][i];
	}
	return matOut;
}

bool nrLDPC::checkSumCodeWord(vector<bool>& cw)
{
	unsigned nLyaers = mLayers.size();

	vector<vector<bool>> cwMat(mN / mZc);
	for (unsigned i = 0; i < mN / mZc; i++) {
		cwMat[i] = vector<bool>(cw.begin() + i * mZc, cw.begin() + (i + 1) * mZc);
	}

	for (unsigned i = 0; i < nLyaers; i++) {
		vector<bool> checkNode(mZc, 0);
		vector<bool> tmpWord(mZc, 0);
		for (unsigned edgeIdx = mLayers[i].edgeStart; edgeIdx < mLayers[i].edgeEnd; edgeIdx++) {
			unsigned vNodeIdx = mEdges[edgeIdx].vNodeIdx; unsigned nShifts = mEdges[edgeIdx].nShifts;
			checkNode = eleWiseXor(checkNode, circShift(cwMat[vNodeIdx], nShifts));
		}
		for (unsigned j = 0; j < mZc; j++) {
			if (checkNode[j])
				return false;
		}
	}

	return true;
}

template<typename T>
inline vector<T> nrLDPC::circShift(const std::vector<T>& vecIn, const unsigned nShifts) {
	vector<T> vecOut = vecIn;
	rotate(vecOut.begin(), vecOut.begin() + nShifts, vecOut.end());
	return vecOut;
};

inline vector<bool>nrLDPC::eleWiseXor(const vector<bool>& vec1, const vector<bool>& vec2) {
	vector<bool> vecOut = vec1;
	transform(vec1.begin(), vec1.end(), vec2.begin(), vecOut.begin(), bit_xor<>{});
	return vecOut;
}

inline vector<float>nrLDPC::eleWisePlus(const vector<float>& vec1, const vector<float>& vec2) {
	vector<float> vecOut = vec1;
	transform(vec1.begin(), vec1.end(), vec2.begin(), vecOut.begin(), plus<>{});
	return vecOut;
}

inline vector<float>nrLDPC::eleWiseMinus(const vector<float>& vec1, const vector<float>& vec2) {
	vector<float> vecOut = vec1;
	transform(vec1.begin(), vec1.end(), vec2.begin(), vecOut.begin(), minus<>{});
	return vecOut;
}
