#include "LDPC.h"
#include <math.h>   
#include <stdio.h>
using namespace  std;

nrLDPC::nrLDPC(size_t infoLen, float codeRate)
{
	mKBar = infoLen; //3000
	mR = codeRate; //1.0/3.0

	// select base graph based on 3GPP 38.212 7.2.2
	mBGn = selectBaseGraph(mKBar, mR);

	// select lifting size
	mZc = selectLiftSize(mKBar, mBGn);
    //printf("mZc: %i",mZc);
	// select shifting set
	mSetIdx = selectShiftSet(mZc);

	// systematic bits length and parity bits length
	if (mBGn == 1) {
		mK = 22 * mZc; mN = 68 * mZc;
	}
	else {
		mK = 10 * mZc;  mN = 52 * mZc;
	}

	// fillers length
	mF = mK - mKBar;

	// build up edges and shifts
	if (mBGn == 1) {
		//mEdges.reserve(316);
		for (unsigned i = 0; i < 316; i++) {
			mEdges.push_back(edge_t());
			mEdges[i] = { shiftTableBgn_1[i][0],shiftTableBgn_1[i][1],uint16_t(shiftTableBgn_1[i][mSetIdx + 2] % mZc) };
		}
	}
	else {
		//mEdges.reserve(197);
		for (unsigned i = 0; i < 197; i++) {
			mEdges.push_back(edge_t());
			mEdges[i] = { shiftTableBgn_2[i][0],shiftTableBgn_2[i][1], uint16_t(shiftTableBgn_2[i][mSetIdx + 2] % mZc) };
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
				   {302,307},{307,312},{312,316} };
	}
	else {
		mLayers = { {0,8},{8,18},{18,26},{26,36},{36,40},{40,46},{46,52},
				   {52,58},{58,62},{62,67},{67,72},{72,77},{77,81},{81,86},
				   {86,91},{91,95},{95,100},{100,105},{105,109},{109,113},
				   {113,117},{117,121},{121,124},{124,128},{128,132},{132,135},
				   {135,140},{140,143},{143,147},{147,150},{150,155},{155,158},
				   {158,162},{162,166},{166,170},{170,174},{174,178},{178,181},
				   {181,185},{185,189},{189,193},{193,197} };
	}
}

etl::vector<bool,MAX_CODEWORD_LENGTH> nrLDPC::encode(const etl::vector<bool,MAX_INFO_NODE_BITS>& msg)
{
	size_t  Kb, Cb, totLayers;
	if (mBGn == 1) {
		Kb = 22; Cb = 68; totLayers = 46;
	}
	else {
		Kb = 10; Cb = 52; totLayers = 42;
	}
    
    /*DEBUG*/
    //printf(
    //    "ENCODER: Kb=%i mZc=%i expected=%i msg.size=%i\n",
    //    Kb,
    //    mZc,
    //    Kb * mZc,
    //    msg.size()
    //);
    /*DEBUG*/
	assert(Kb * mZc == msg.size());

	// initialize encoded bits in nodes(vectors of size Zc)
	// the first Kb nodes correspond to information bits, the rest  parity bits
    etl::vector<etl::vector<bool,MAX_ZC>,MAX_CB> cWord(Cb);
	for (unsigned i = 0; i < Kb; i++) {
		cWord[i] = etl::vector<bool,MAX_ZC>(msg.begin() + i * mZc, msg.begin() + (i + 1) * mZc);
	}
	for (unsigned i = Kb; i < Cb; i++) {
		cWord[i] = etl::vector<bool,MAX_ZC>(mZc, 0);
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
    etl::rotate(cWord[Kb].begin(), cWord[Kb].begin() + (mZc - shiftP0), cWord[Kb].end());

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
    etl::vector<bool,MAX_CODEWORD_LENGTH> cWordVec;
	for (auto vec : cWord) {
		cWordVec.insert(cWordVec.end(), vec.begin(), vec.end());
	}
	return cWordVec;
}

etl::vector<bool, MAX_INFO_NODE_BITS> nrLDPC::decode(const etl::vector<float, MAX_CODEWORD_LENGTH>& softBitsIn, const unsigned nMaxIter)
{
	//------------------------------------------------------------------------------------------------------
	// [ref] Hocevar, D.E. "A reduced complexity decoder architecture via layered decoding of LDPC codes."
	// In IEEE Workshop on Signal Processing Systems, 2004. SIPS 2004.
	//------------------------------------------------------------------------------------------------------
    volatile unsigned int sim_time = 0;
    volatile unsigned int check_node_operation_start = 0;
    volatile unsigned int check_node_operation_end = 0;
    volatile unsigned int minSumMsgs_start = 0;
    volatile unsigned int minSumMsgs_end = 0;
    volatile unsigned int minSumMsgs_loop_start = 0;
    volatile unsigned int minSumMsgs_loop_end = 0;
    volatile unsigned int checkNodeOperation_core_start = 0;
    volatile unsigned int checkNodeOperation_core_end = 0;

    assert(softBitsIn.size() == mN);

    etl::vector<etl::vector<float, MAX_ZC>, MAX_CB> LLR(mN / mZc);
    for (unsigned i = 0; i < mN / mZc; i++) {
        LLR[i] = etl::vector<float, MAX_ZC>(softBitsIn.begin() + i * mZc, softBitsIn.begin() + (i + 1) * mZc);
    }

    unsigned nMaxLayer;
    if (mBGn == 1) {
        nMaxLayer = ((mKBar * CODE_RATE_DEN + CODE_RATE_NUM - 1) / CODE_RATE_NUM + mF + mZc - 1) / mZc - 20;
    } else {
        nMaxLayer = ((mKBar * CODE_RATE_DEN + CODE_RATE_NUM - 1) / CODE_RATE_NUM + mF + mZc - 1) / mZc - 8;
    }

    etl::vector<etl::vector<float, MAX_ZC>, MAX_EDGES> CtoVMsg(mEdges.size());
    int my_iter = 0;
    for (auto& e : CtoVMsg) {
        my_iter++;
        e = etl::vector<float,MAX_ZC>(mZc, 0);
    }

    unsigned nLayerEdges, edgeIdx, nShifts, vNodeIdx;
    for (unsigned iIter = 0; iIter < nMaxIter; iIter++) {
        for (unsigned iLayer = 0; iLayer < nMaxLayer; iLayer++) {
            nLayerEdges = mLayers[iLayer].edgeEnd - mLayers[iLayer].edgeStart;
            etl::vector<etl::vector<float, MAX_ZC>, MAX_CHECK_NODE_DEGREE> VtoCMsg(nLayerEdges);
            my_iter = 0;
            for (auto& e : VtoCMsg) {
                my_iter++;
                e = etl::vector<float,MAX_ZC>(mZc, 0);
            }
            for (unsigned iEdge = 0; iEdge < nLayerEdges; iEdge++) {
                edgeIdx = mLayers[iLayer].edgeStart + iEdge;
                vNodeIdx = mEdges[edgeIdx].vNodeIdx; nShifts = mEdges[edgeIdx].nShifts;
                LLR[vNodeIdx] = eleWiseMinus(LLR[vNodeIdx], CtoVMsg[edgeIdx]);
                VtoCMsg[iEdge] = LLR[vNodeIdx];
                VtoCMsg[iEdge] = circShift(VtoCMsg[iEdge], nShifts);
            }
            volatile const unsigned int* snr_g_id_ptr = SNR_G_ID_PTR;
            unsigned int snr_g_id = *snr_g_id_ptr;
            volatile const unsigned int* blk_g_id_ptr = BLK_G_ID_PTR;
            unsigned int blk_g_id = *blk_g_id_ptr;
            _TCE_RTC(1, sim_time);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] Started checkNodeOperation\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer);
            _TCE_RTC(1, check_node_operation_start);
            _TCE_RTC(1, minSumMsgs_start);
            etl::vector<etl::vector<float, MAX_ZC>, MAX_CHECK_NODE_DEGREE> minSumMsgs(nLayerEdges);
            _TCE_RTC(1, minSumMsgs_end);
            _TCE_RTC(1, minSumMsgs_loop_start);
            for (auto& e : minSumMsgs)
                e = etl::vector<float,MAX_ZC>(mZc, 0);
            _TCE_RTC(1, minSumMsgs_loop_end);
            _TCE_RTC(1, checkNodeOperation_core_start);
            checkNodeOperation(VtoCMsg, minSumMsgs);
            _TCE_RTC(1, checkNodeOperation_core_end);
            _TCE_RTC(1,check_node_operation_end);
            _TCE_RTC(1, sim_time);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] Ended checkNodeOperation\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] minSumMsgs elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(minSumMsgs_end - minSumMsgs_start)/1e6);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] minSumMsgs_loop elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(minSumMsgs_loop_end - minSumMsgs_loop_start)/1e6);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] checkNodeOperation_core elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(checkNodeOperation_core_end - checkNodeOperation_core_start)/1e6);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] checkNodeOperation elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(check_node_operation_end - check_node_operation_start)/1e6);
            for (unsigned iEdge = 0; iEdge < nLayerEdges; iEdge++) {
                edgeIdx = mLayers[iLayer].edgeStart + iEdge;
                vNodeIdx = mEdges[edgeIdx].vNodeIdx; nShifts = mEdges[edgeIdx].nShifts;
                CtoVMsg[edgeIdx] = circShift(minSumMsgs[iEdge], mZc - nShifts);
                LLR[vNodeIdx] = eleWisePlus(LLR[vNodeIdx], CtoVMsg[edgeIdx]);
            }
        }
    }
    etl::vector<float, MAX_CODEWORD_LENGTH> vecLLR;
    my_iter = 0;
    for (auto e : LLR) {
        my_iter++;
        vecLLR.insert(vecLLR.end(), e.begin(), e.end());
    }
    etl::vector<bool, MAX_INFO_NODE_BITS> decBits(mKBar, false);
    my_iter = 0;
    for (unsigned i = 0; i < mKBar; i++) {
        my_iter++;
        decBits[i] = (vecLLR[i] <= 0);
    }
    return decBits;
}

//etl::vector<etl::vector<float, MAX_ZC>, MAX_CHECK_NODE_DEGREE>
//nrLDPC::checkNodeOperation(const etl::vector<etl::vector<float, MAX_ZC>, MAX_CHECK_NODE_DEGREE>& msgIn)
//{
void nrLDPC::checkNodeOperation(
const etl::vector<etl::vector<float, MAX_ZC>, MAX_CHECK_NODE_DEGREE>& msgIn,
etl::vector<etl::vector<float, MAX_ZC>, MAX_CHECK_NODE_DEGREE>& msgOut)
{
    // ------------------------------------------------------------------------------------------------
	// [ref] Chen, Jinghu, R.M. Tanner, C. Jones, and Yan Li. "Improved min-sum decoding algorithms for
	// irregular LDPC codes." In Proceedings. International Symposium on Information Theory, 2005.
	//-------------------------------------------------------------------------------------------------
 
    const unsigned nEdges = msgIn.size();

    for (unsigned edge = 0; edge < nEdges; ++edge)
        msgOut[edge] = etl::vector<float, MAX_ZC>(mZc, 0.0f);

#ifdef NRLDPC_USE_DECODER_CUSTOM_FUS
    // ==========================================================
    // Custom Function Units accelerated implementation
    // ==========================================================

    float in[19];
    float out[19];
    float min_llr;
    bool sign_llr;

    for (unsigned z = 0; z < mZc; ++z) {
        for (unsigned edge = 0; edge < 19; ++edge)
            in[edge] = (edge < nEdges) ? msgIn[edge][z] : 0.0f;

        for (unsigned edge = 0; edge < nEdges; ++edge) {
            switch (nEdges) {
                case 3:
                    if (edge == 0) {
                        _OA_CN_CORE_3(in[1], in[2], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_3(in[0], in[2], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_3(in[0], in[1], min_llr, sign_llr);
                    }
                    break;

                case 4:
                    if (edge == 0) {
                        _OA_CN_CORE_4(in[1], in[2], in[3], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_4(in[0], in[2], in[3], min_llr, sign_llr);
                    } else if (edge == 2) {
                        _OA_CN_CORE_4(in[0], in[1], in[3], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_4(in[0], in[1], in[2], min_llr, sign_llr);
                    }
                    break;

                case 5:
                    if (edge == 0) {
                        _OA_CN_CORE_5(in[1], in[2], in[3], in[4], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_5(in[0], in[2], in[3], in[4], min_llr, sign_llr);
                    } else if (edge == 2) {
                        _OA_CN_CORE_5(in[0], in[1], in[3], in[4], min_llr, sign_llr);
                    } else if (edge == 3) {
                        _OA_CN_CORE_5(in[0], in[1], in[2], in[4], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_5(in[0], in[1], in[2], in[3], min_llr, sign_llr);
                    }
                    break;

                case 6:
                    if (edge == 0) {
                        _OA_CN_CORE_6(in[1], in[2], in[3], in[4], in[5], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_6(in[0], in[2], in[3], in[4], in[5], min_llr, sign_llr);
                    } else if (edge == 2) {
                        _OA_CN_CORE_6(in[0], in[1], in[3], in[4], in[5], min_llr, sign_llr);
                    } else if (edge == 3) {
                        _OA_CN_CORE_6(in[0], in[1], in[2], in[4], in[5], min_llr, sign_llr);
                    } else if (edge == 4) {
                        _OA_CN_CORE_6(in[0], in[1], in[2], in[3], in[5], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_6(in[0], in[1], in[2], in[3], in[4], min_llr, sign_llr);
                    }
                    break;

                case 7:
                    if (edge == 0) {
                        _OA_CN_CORE_7(in[1], in[2], in[3], in[4], in[5], in[6], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_7(in[0], in[2], in[3], in[4], in[5], in[6], min_llr, sign_llr);
                    } else if (edge == 2) {
                        _OA_CN_CORE_7(in[0], in[1], in[3], in[4], in[5], in[6], min_llr, sign_llr);
                    } else if (edge == 3) {
                        _OA_CN_CORE_7(in[0], in[1], in[2], in[4], in[5], in[6], min_llr, sign_llr);
                    } else if (edge == 4) {
                        _OA_CN_CORE_7(in[0], in[1], in[2], in[3], in[5], in[6], min_llr, sign_llr);
                    } else if (edge == 5) {
                        _OA_CN_CORE_7(in[0], in[1], in[2], in[3], in[4], in[6], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_7(in[0], in[1], in[2], in[3], in[4], in[5], min_llr, sign_llr);
                    }
                    break;

                case 8:
                    if (edge == 0) {
                        _OA_CN_CORE_8(in[1], in[2], in[3], in[4], in[5], in[6], in[7], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_8(in[0], in[2], in[3], in[4], in[5], in[6], in[7], min_llr, sign_llr);
                    } else if (edge == 2) {
                        _OA_CN_CORE_8(in[0], in[1], in[3], in[4], in[5], in[6], in[7], min_llr, sign_llr);
                    } else if (edge == 3) {
                        _OA_CN_CORE_8(in[0], in[1], in[2], in[4], in[5], in[6], in[7], min_llr, sign_llr);
                    } else if (edge == 4) {
                        _OA_CN_CORE_8(in[0], in[1], in[2], in[3], in[5], in[6], in[7], min_llr, sign_llr);
                    } else if (edge == 5) {
                        _OA_CN_CORE_8(in[0], in[1], in[2], in[3], in[4], in[6], in[7], min_llr, sign_llr);
                    } else if (edge == 6) {
                        _OA_CN_CORE_8(in[0], in[1], in[2], in[3], in[4], in[5], in[7], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_8(in[0], in[1], in[2], in[3], in[4], in[5], in[6], min_llr, sign_llr);
                    }
                    break;

                case 9:
                    if (edge == 0) {
                        _OA_CN_CORE_9(in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_9(in[0], in[2], in[3], in[4], in[5], in[6], in[7], in[8], min_llr, sign_llr);
                    } else if (edge == 2) {
                        _OA_CN_CORE_9(in[0], in[1], in[3], in[4], in[5], in[6], in[7], in[8], min_llr, sign_llr);
                    } else if (edge == 3) {
                        _OA_CN_CORE_9(in[0], in[1], in[2], in[4], in[5], in[6], in[7], in[8], min_llr, sign_llr);
                    } else if (edge == 4) {
                        _OA_CN_CORE_9(in[0], in[1], in[2], in[3], in[5], in[6], in[7], in[8], min_llr, sign_llr);
                    } else if (edge == 5) {
                        _OA_CN_CORE_9(in[0], in[1], in[2], in[3], in[4], in[6], in[7], in[8], min_llr, sign_llr);
                    } else if (edge == 6) {
                        _OA_CN_CORE_9(in[0], in[1], in[2], in[3], in[4], in[5], in[7], in[8], min_llr, sign_llr);
                    } else if (edge == 7) {
                        _OA_CN_CORE_9(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[8], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_9(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], min_llr, sign_llr);
                    }
                    break;

                case 10:
                    if (edge == 0) {
                        _OA_CN_CORE_10(in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_10(in[0], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], min_llr, sign_llr);
                    } else if (edge == 2) {
                        _OA_CN_CORE_10(in[0], in[1], in[3], in[4], in[5], in[6], in[7], in[8], in[9], min_llr, sign_llr);
                    } else if (edge == 3) {
                        _OA_CN_CORE_10(in[0], in[1], in[2], in[4], in[5], in[6], in[7], in[8], in[9], min_llr, sign_llr);
                    } else if (edge == 4) {
                        _OA_CN_CORE_10(in[0], in[1], in[2], in[3], in[5], in[6], in[7], in[8], in[9], min_llr, sign_llr);
                    } else if (edge == 5) {
                        _OA_CN_CORE_10(in[0], in[1], in[2], in[3], in[4], in[6], in[7], in[8], in[9], min_llr, sign_llr);
                    } else if (edge == 6) {
                        _OA_CN_CORE_10(in[0], in[1], in[2], in[3], in[4], in[5], in[7], in[8], in[9], min_llr, sign_llr);
                    } else if (edge == 7) {
                        _OA_CN_CORE_10(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[8], in[9], min_llr, sign_llr);
                    } else if (edge == 8) {
                        _OA_CN_CORE_10(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[9], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_10(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], min_llr, sign_llr);
                    }
                    break;

                case 19:
                    if (edge == 0) {
                        _OA_CN_CORE_19(in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 1) {
                        _OA_CN_CORE_19(in[0], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 2) {
                        _OA_CN_CORE_19(in[0], in[1], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 3) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 4) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 5) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 6) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 7) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 8) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 9) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 10) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[11], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 11) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[12], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 12) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[13], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 13) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[14], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 14) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[15], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 15) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[16], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 16) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[17], in[18], min_llr, sign_llr);
                    } else if (edge == 17) {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[18], min_llr, sign_llr);
                    } else {
                        _OA_CN_CORE_19(in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7], in[8], in[9], in[10], in[11], in[12], in[13], in[14], in[15], in[16], in[17], min_llr, sign_llr);
                    }
                    break;

                default:
                    assert(false && "Unsupported LDPC check-node degree");
                    min_llr = 0.0f;
                    sign_llr = false;
                    break;
            }

            min_llr = (min_llr > 0.5f) ? min_llr - 0.5f : 0.0f;
            out[edge] = sign_llr ? -min_llr : min_llr;
        }

        for (unsigned edge = 0; edge < nEdges; ++edge)
            msgOut[edge][z] = out[edge];
    }
#else
    // ==========================================================
    // Generic C++ implementation
    // ==========================================================
	unsigned nNodes = msgIn.size();
    assert(nNodes > 0 && nNodes <= MAX_CHECK_NODE_DEGREE);

    etl::vector<etl::vector<float, MAX_CHECK_NODE_DEGREE>, MAX_ZC> msgMat = transposeMat(msgIn);
    etl::vector<size_t,MAX_CHECK_NODE_DEGREE> sortedIdx(nNodes, 0);
	etl::vector<float,MAX_CHECK_NODE_DEGREE> sign(nNodes, 1.0);
	float min1, min2, parity;
	size_t min1Idx, min2Idx;

    for (unsigned i = 0; i < mZc; ++i) {
        sortedIdx = sort_indexes(msgMat[i]);

        min1Idx = sortedIdx[0];
        min2Idx = sortedIdx[1];

        min1 = fabs(msgMat[i][min1Idx]);
        min2 = fabs(msgMat[i][min2Idx]);

        min1 = (min1 > 0.5f) ? min1 - 0.5f : 0.0f;
        min2 = (min2 > 0.5f) ? min2 - 0.5f : 0.0f;

        etl::vector<float, MAX_CHECK_NODE_DEGREE> outZ(nNodes, min1);

        outZ[min1Idx] = min2;

        parity = 1.0f;

        for (unsigned j = 0; j < nNodes; ++j) {
            sign[j] = (msgMat[i][j] >= 0.0f) ? 1.0f : -1.0f;
            parity *= sign[j];
        }

        for (unsigned j = 0; j < nNodes; ++j) {
            outZ[j] *= parity * sign[j];
            msgOut[j][i] = outZ[j];
        }
    }

#endif
}

etl::vector<bool,CODE_WORD_BITS_LENGTH> nrLDPC::rateMatch(const etl::vector<bool,MAX_CODEWORD_LENGTH>& bitsIn, size_t nOfBitOut)
{
	if (mBGn == 1)
		assert(bitsIn.size() == 68 * mZc);
	else
		assert(bitsIn.size() == 52 * mZc);

    etl::vector<bool,MAX_CODEWORD_LENGTH> txBufferRing = bitsIn;

	// shortening by removing filling bits
	txBufferRing.erase(txBufferRing.begin() + mK - mF, txBufferRing.begin() + mK);

	// puncturing the first 2*Z
	txBufferRing.erase(txBufferRing.begin(), txBufferRing.begin() + 2 * mZc);

	// take nOfBitOut bits out of the ring
    etl::vector<bool,CODE_WORD_BITS_LENGTH> bitsOut(nOfBitOut);
	for (unsigned i = 0, j = 0; i < nOfBitOut; i++, j++) {
		bitsOut[i] = txBufferRing[i % txBufferRing.size()];
	}
	return bitsOut;
}
etl::vector<float,MAX_CODEWORD_LENGTH> nrLDPC::rateRecover(const etl::vector<float,CODE_WORD_BITS_LENGTH>& softBitsIn)
{
	unsigned rxRingLen = mN - 2 * mZc - mF;
    etl::vector<float, MAX_CODEWORD_LENGTH> rxBufferRing(rxRingLen, 0);

	// for received bits longer than the ring
	for (unsigned i = 0; i < softBitsIn.size(); i++) {
		rxBufferRing[i % rxRingLen] = rxBufferRing[i % rxRingLen] + softBitsIn[i];
	}

	// first 2*Zc with all 0
    etl::vector<float, MAX_CODEWORD_LENGTH> softBitsOut(2 * mZc, 0);
	// add information soft bits
	softBitsOut.insert(softBitsOut.end(), rxBufferRing.begin(), rxBufferRing.begin() + mKBar - 2 * mZc);
	//fillers
    etl::vector<float,MAX_INFO_NODE_BITS> fillers(mF, etl::numeric_limits<float>::infinity());
	softBitsOut.insert(softBitsOut.end(), fillers.begin(), fillers.end());
	// add parity soft bits
	softBitsOut.insert(softBitsOut.end(), rxBufferRing.begin() + mKBar - 2 * mZc, rxBufferRing.end());

	return softBitsOut;
}

uint8_t nrLDPC::selectBaseGraph(size_t KBar, float R) // for 'Hello NR LDPC!' -> KBar = 3000, R = 1.0/3.0
{
	// 3GPP 38.212 7.2.2 LDPC base graph selection
	if (KBar <= 292 || (KBar <= 3824 && R <= 0.67) || R <= 0.25) {
		assert(KBar <= 3840);
		return 2; // for 'Hello NR LDPC', that's the one.
	}
	else {
		assert(KBar <= 8448);
		return 1;
	}
}
uint16_t nrLDPC::selectLiftSize(size_t KBar, uint8_t BGn) // for 'Hello NR LDPC!' -> KBar = 3000, BGn = 2
{
	// select kb 3GPP 38.212 section 5.2.2
	uint16_t Kb;
	if (BGn == 1)
		Kb = 22;
	else {
		if (KBar > 640)
			Kb = 10; // for 'Hello NR LDPC!', that's the one.
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
			if (candiZc * Kb == KBar)
				return candiZc;
			else if (candiZc * Kb > KBar && candiZc < Zc)
				Zc = candiZc;
		}
	}
    //printf("final Zc: %i",Zc);
	return Zc;
}
uint8_t nrLDPC::selectShiftSet(uint16_t Zc)
{
	assert(Zc >= 2 && Zc <= 384);
	for (unsigned i = 0; i < 8; i++) {
		for (unsigned j = 0; j < 8; j++) {
			if (liftSizeTable[i][j] == Zc)
				return i;
		}
	}
	//cerr << " Zc is not valid!";
	fprintf(stderr,"Zc is not valid!");
	return -1;
}
etl::vector<etl::vector<bool,MAX_PCM_COLS>,MAX_PCM_ROWS> nrLDPC::makeParityCheckMatrix(uint8_t BGn, const uint16_t Zc)
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
    etl::vector<etl::vector<bool,MAX_PCM_COLS>,MAX_PCM_ROWS> H(numOfBlkRows);
	for (unsigned i = 0; i < numOfBlkRows * Zc; i++) {
		H[i] = etl::vector<bool,MAX_PCM_COLS>(numOfBlkCols * Zc, 0);
	}

	unsigned blkRowIdx, blkColIdx, shiftCoeff;
	// a shifted vecOne will be used to fill H
	// vecOne = [0,0,0,1] if Zc = 4;
    etl::vector<bool,MAX_ZC> vecOne(Zc, 0); //vecOne.back() = 1;
    vecOne.resize(Zc, false);
    vecOne.back() = true;

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
        etl::vector<bool,MAX_ZC> shiftVecOne = vecOne;
        etl::rotate(shiftVecOne.begin(), shiftVecOne.begin() + shiftCoeff, shiftVecOne.end());

		// shift right by 1 for each row in a block
		for (unsigned m = 0; m < Zc; m++) {
            etl::rotate(shiftVecOne.begin(), shiftVecOne.begin() + 1, shiftVecOne.end());
			for (unsigned n = 0; n < Zc; n++) {
				H[Zc * blkRowIdx + m][Zc * blkColIdx + n] = shiftVecOne[n];
			}
		}
	}

	return H;
}

template<typename T>
inline etl::vector<size_t,MAX_CHECK_NODE_DEGREE> nrLDPC::sort_indexes(const etl::vector<T,MAX_CHECK_NODE_DEGREE>& v)
{
	// initialize original index locations
    etl::vector<size_t,MAX_CHECK_NODE_DEGREE> idx(v.size());
    etl::iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	// using std::stable_sort instead of std::sort
	// to avoid unnecessary index re-orderings
	// when v contains elements of equal values
    etl::stable_sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return fabs(v[i1]) < fabs(v[i2]); });

	return idx;
}

template<size_t ROW_CAPACITY,size_t COL_CAPACITY> etl::vector<etl::vector<float, ROW_CAPACITY>,COL_CAPACITY> 
nrLDPC::transposeMat(const etl::vector<etl::vector<float, COL_CAPACITY>,ROW_CAPACITY>& mat)
{
	unsigned nRows = mat.size();
	unsigned nCols = mat[0].size();

    etl::vector<etl::vector<float,ROW_CAPACITY>,COL_CAPACITY> matOut(nCols);
	for (unsigned i = 0; i < nCols; i++) {
		matOut[i] = etl::vector<float,ROW_CAPACITY>(nRows, 0);
		for (unsigned j = 0; j < nRows; j++)
			matOut[i][j] = mat[j][i];
	}
	return matOut;
}

bool nrLDPC::checkSumCodeWord(etl::vector<bool,MAX_CODEWORD_LENGTH>& cw)
{
	unsigned nLyaers = mLayers.size();

    etl::vector<etl::vector<bool,MAX_ZC>,MAX_CB> cwMat(mN / mZc);
	for (unsigned i = 0; i < mN / mZc; i++) {
		cwMat[i] = etl::vector<bool,MAX_ZC>(cw.begin() + i * mZc, cw.begin() + (i + 1) * mZc);
	}

	for (unsigned i = 0; i < nLyaers; i++) {
        etl::vector<bool,MAX_ZC> checkNode(mZc, 0);
		etl::vector<bool,MAX_ZC> tmpWord(mZc, 0);
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
inline etl::vector<T,MAX_ZC> nrLDPC::circShift(const etl::vector<T,MAX_ZC>& vecIn, const unsigned nShifts) {
    etl::vector<T,MAX_ZC> vecOut = vecIn;
    etl::rotate(vecOut.begin(), vecOut.begin() + nShifts, vecOut.end());
	return vecOut;
};

inline etl::vector<bool,MAX_ZC> nrLDPC::eleWiseXor(const etl::vector<bool,MAX_ZC>& vec1, const etl::vector<bool,MAX_ZC>& vec2) {
    etl::vector<bool,MAX_ZC> vecOut = vec1;
	transform(vec1.begin(), vec1.end(), vec2.begin(), vecOut.begin(), etl::bit_xor<bool>{});
	return vecOut;
}

inline etl::vector<float,MAX_ZC> nrLDPC::eleWisePlus(const etl::vector<float,MAX_ZC>& vec1, const etl::vector<float,MAX_ZC>& vec2) {
    etl::vector<float,MAX_ZC> vecOut = vec1;
	transform(vec1.begin(), vec1.end(), vec2.begin(), vecOut.begin(), etl::plus<float>{});
	return vecOut;
}

inline etl::vector<float,MAX_ZC> nrLDPC::eleWiseMinus(const etl::vector<float,MAX_ZC>& vec1, const etl::vector<float,MAX_ZC>& vec2) {
    etl::vector<float,MAX_ZC> vecOut = vec1;
	transform(vec1.begin(), vec1.end(), vec2.begin(), vecOut.begin(), etl::minus<float>{});
	return vecOut;
}
