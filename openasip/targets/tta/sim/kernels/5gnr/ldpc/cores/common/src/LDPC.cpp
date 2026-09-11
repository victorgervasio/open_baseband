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
    printf(
        "ENCODER: Kb=%i mZc=%i expected=%i msg.size=%i\n",
        Kb,
        mZc,
        Kb * mZc,
        msg.size()
    );
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

	// initialize LLR in blocks(nodes), each node with Zc bits
    etl::vector<etl::vector<float, MAX_ZC>, MAX_CB> LLR(mN / mZc);
	for (unsigned i = 0; i < mN / mZc; i++) {
		LLR[i] = etl::vector<float, MAX_ZC>(softBitsIn.begin() + i * mZc, softBitsIn.begin() + (i + 1) * mZc);
	}

	// find how many parity nodes to use for decoding
	unsigned nMaxLayer;
	if (mBGn == 1) {
		// assume tx bits length =  ceil(kBar/R), alternatively can use all layers(slower)
		//nMaxLayer = ceil((ceil(mKBar / mR) + mF) / mZc) - 20;
        nMaxLayer = ((mKBar * CODE_RATE_DEN + CODE_RATE_NUM - 1) / CODE_RATE_NUM + mF + mZc - 1) / mZc - 20;
    } else {
		//nMaxLayer = ceil((ceil(mKBar / mR) + mF) / mZc) - 8;
        nMaxLayer = ((mKBar * CODE_RATE_DEN + CODE_RATE_NUM - 1) / CODE_RATE_NUM + mF + mZc - 1) / mZc - 8;
	}

	// initialize msg from check nodes to vector nodes, each edge correspond a message
    etl::vector<etl::vector<float, MAX_ZC>, MAX_EDGES> CtoVMsg(mEdges.size());
    int my_iter = 0;
	for (auto& e : CtoVMsg) {
        my_iter++;
		e = etl::vector<float,MAX_ZC>(mZc, 0);
	}
	// llr updates
	unsigned nLayerEdges, edgeIdx, nShifts, vNodeIdx;
	for (unsigned iIter = 0; iIter < nMaxIter; iIter++) {
		for (unsigned iLayer = 0; iLayer < nMaxLayer; iLayer++) {
			nLayerEdges = mLayers[iLayer].edgeEnd - mLayers[iLayer].edgeStart;
			// messages from variable nodes to check node
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
			//check node operation
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] Started checkNodeOperation\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer);
            _TCE_RTC(1, check_node_operation_start); // OpenASIP 2.0 doc (search for printf explanation)
            //etl::vector<etl::vector<float,MAX_ZC>,MAX_CHECK_NODE_DEGREE> minSumMsgs = checkNodeOperation(VtoCMsg);
            
            _TCE_RTC(1, minSumMsgs_start); // OpenASIP 2.0 doc (search for printf explanation)
            etl::vector<etl::vector<float, MAX_ZC>, MAX_CHECK_NODE_DEGREE> minSumMsgs(nLayerEdges);
            _TCE_RTC(1, minSumMsgs_end); // OpenASIP 2.0 doc (search for printf explanation)
            
            _TCE_RTC(1, minSumMsgs_loop_start); // OpenASIP 2.0 doc (search for printf explanation)
            for (auto& e : minSumMsgs)
                e = etl::vector<float, MAX_ZC>(mZc, 0);
            _TCE_RTC(1, minSumMsgs_loop_end); // OpenASIP 2.0 doc (search for printf explanation)
                                        
            _TCE_RTC(1, checkNodeOperation_core_start); // OpenASIP 2.0 doc (search for printf explanation)
            checkNodeOperation(VtoCMsg, minSumMsgs);
            _TCE_RTC(1, checkNodeOperation_core_end); // OpenASIP 2.0 doc (search for printf explanation)

            _TCE_RTC(1,check_node_operation_end);
            _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] Ended checkNodeOperation\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] minSumMsgs elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(minSumMsgs_end - minSumMsgs_start)/1e6);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] minSumMsgs_loop elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(minSumMsgs_loop_end - minSumMsgs_loop_start)/1e6);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] checkNodeOperation_core elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(checkNodeOperation_core_end - checkNodeOperation_core_start)/1e6);
            printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] checkNodeOperation elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(check_node_operation_end - check_node_operation_start)/1e6);

			//message from check node to varible nodes
			for (unsigned iEdge = 0; iEdge < nLayerEdges; iEdge++) {
				edgeIdx = mLayers[iLayer].edgeStart + iEdge;
				vNodeIdx = mEdges[edgeIdx].vNodeIdx; nShifts = mEdges[edgeIdx].nShifts;
				CtoVMsg[edgeIdx] = circShift(minSumMsgs[iEdge], mZc - nShifts);
				LLR[vNodeIdx] = eleWisePlus(LLR[vNodeIdx], CtoVMsg[edgeIdx]);
			}
		}
	}
	// flatten the 2-D vector LLR
    etl::vector<float, MAX_CODEWORD_LENGTH> vecLLR;
    my_iter = 0;
	for (auto e : LLR) {
        my_iter++;
		vecLLR.insert(vecLLR.end(), e.begin(), e.end());
	}
	//vecLLR.erase(vecLLR.end() - mF, vecLLR.end());

	// chose information bits
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
    volatile unsigned int check_node_operation_in_start = 0; 
    volatile unsigned int check_node_operation_in_end = 0; 
    unsigned int total_check_node_operation_in = 0;
    _TCE_RTC(1, check_node_operation_in_start); // OpenASIP 2.0 doc (search for printf explanation)

    volatile const unsigned int* snr_g_id_ptr = SNR_G_ID_PTR;
    unsigned int snr_g_id = *snr_g_id_ptr;
    volatile const unsigned int* blk_g_id_ptr = BLK_G_ID_PTR;
    unsigned int blk_g_id = *blk_g_id_ptr;

    volatile unsigned int timer_declararions_start = 0; 
    _TCE_RTC(1, timer_declararions_start); // OpenASIP 2.0 doc (search for printf explanation)
    volatile unsigned int timer_declararions_end = 0; 
    unsigned int total_timer_declarations = 0;

    volatile unsigned int sim_time = 0;

    volatile unsigned int nEdgesDec_start = 0;
    volatile unsigned int nEdgesDec_end = 0;
    unsigned int total_nEdgesDec = 0;

    volatile unsigned int infDec_start = 0;
    volatile unsigned int infDec_end = 0;
    unsigned int total_infDec = 0;

    //volatile unsigned int msgOutDec_start = 0;
    //volatile unsigned int msgOutDec_end = 0;
    //unsigned int total_msgOutDec = 0;

    volatile unsigned int loop_1_start = 0;
    volatile unsigned int loop_1_end = 0;
    unsigned int total_loop_1 = 0;

    volatile unsigned int inDec_start = 0;
    volatile unsigned int inDec_end = 0;
    unsigned int total_inDec = 0;

    volatile unsigned int outDec_start = 0;
    volatile unsigned int outDec_end = 0;
    unsigned int total_outDec = 0;

    volatile unsigned int in_init_loop_2_start = 0;
    volatile unsigned int in_init_loop_2_end = 0;
    unsigned int total_in_init_loop_2 = 0;

    volatile unsigned int out_init_loop_2_start = 0;
    volatile unsigned int out_init_loop_2_end = 0;
    unsigned int total_out_init_loop_2 = 0;

    volatile unsigned int oa_check_node_operation_start = 0;
    volatile unsigned int oa_check_node_operation_end = 0;
    unsigned int total_oa_check_node_operation = 0;

    volatile unsigned int loop_2_start = 0;
    volatile unsigned int loop_2_end = 0;
    unsigned int total_loop_2 = 0;
    _TCE_RTC(1, timer_declararions_end); // OpenASIP 2.0 doc (search for printf explanation)
    total_timer_declarations += (timer_declararions_end - timer_declararions_start);
    //=======================================================

    _TCE_RTC(1, nEdgesDec_start); // OpenASIP 2.0 doc (search for printf explanation)
    const unsigned nEdges = msgIn.size();
    _TCE_RTC(1, nEdgesDec_end); // OpenASIP 2.0 doc (search for printf explanation)
    total_nEdgesDec += (nEdgesDec_end - nEdgesDec_start);

    _TCE_RTC(1, infDec_start); // OpenASIP 2.0 doc (search for printf explanation)
    const float INF = etl::numeric_limits<float>::infinity();
    _TCE_RTC(1, infDec_end); // OpenASIP 2.0 doc (search for printf explanation)
    total_infDec += (infDec_end - infDec_start);

    //_TCE_RTC(1, msgOutDec_start); // OpenASIP 2.0 doc (search for printf explanation)
    //etl::vector<etl::vector<float, MAX_ZC>, MAX_CHECK_NODE_DEGREE>
    //    msgOut(nEdges);
    //_TCE_RTC(1, msgOutDec_end); // OpenASIP 2.0 doc (search for printf explanation)
    //total_msgOutDec += (msgOutDec_end - msgOutDec_start);

    _TCE_RTC(1, loop_1_start); // OpenASIP 2.0 doc (search for printf explanation)
    for (unsigned edge = 0; edge < nEdges; ++edge)
        msgOut[edge] = etl::vector<float, MAX_ZC>(mZc, 0.0f);
    _TCE_RTC(1, loop_1_end); // OpenASIP 2.0 doc (search for printf explanation)
    total_loop_1 += (loop_1_end - loop_1_start);

    for (unsigned z = 0; z < mZc; ++z) {

        _TCE_RTC(1, inDec_start); // OpenASIP 2.0 doc (search for printf explanation)
        float in[19];
        _TCE_RTC(1, inDec_end); // OpenASIP 2.0 doc (search for printf explanation)
        total_inDec += (inDec_end - inDec_start);

        _TCE_RTC(1, outDec_start); // OpenASIP 2.0 doc (search for printf explanation)
        float out[19];
        _TCE_RTC(1, outDec_end); // OpenASIP 2.0 doc (search for printf explanation)
        total_outDec += (outDec_end - outDec_start);

        for (unsigned edge = 0; edge < 19; ++edge) {
            _TCE_RTC(1, in_init_loop_2_start); // OpenASIP 2.0 doc (search for printf explanation)
            in[edge] = (edge < nEdges) ? msgIn[edge][z] : INF;
            _TCE_RTC(1, in_init_loop_2_end); // OpenASIP 2.0 doc (search for printf explanation)
            total_in_init_loop_2 += (in_init_loop_2_end - in_init_loop_2_start);

            _TCE_RTC(1, out_init_loop_2_start); // OpenASIP 2.0 doc (search for printf explanation)
            out[edge] = 0.0f;
            _TCE_RTC(1, out_init_loop_2_end); // OpenASIP 2.0 doc (search for printf explanation)
            total_out_init_loop_2 += (out_init_loop_2_end - out_init_loop_2_start);
        }

        //_TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
        //printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] Started checkNodeOperation\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer);
        _TCE_RTC(1, oa_check_node_operation_start); // OpenASIP 2.0 doc (search for printf explanation)
        _OA_CHECK_NODE(
            in[0],  in[1],  in[2],  in[3],  in[4],
            in[5],  in[6],  in[7],  in[8],  in[9],
            in[10], in[11], in[12], in[13], in[14],
            in[15], in[16], in[17], in[18],

            out[0],  out[1],  out[2],  out[3],  out[4],
            out[5],  out[6],  out[7],  out[8],  out[9],
            out[10], out[11], out[12], out[13], out[14],
            out[15], out[16], out[17], out[18]
        );
        _TCE_RTC(1,oa_check_node_operation_end);
        total_oa_check_node_operation += (oa_check_node_operation_end - oa_check_node_operation_start);
        //_TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
        //printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] Ended checkNodeOperation\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer);
        //printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode][iIter %i ; iLayer %i] checkNodeOperation elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,iIter,iLayer,(check_node_operation_end - check_node_operation_start)/1e6);

        _TCE_RTC(1, loop_2_start); // OpenASIP 2.0 doc (search for printf explanation)
        for (unsigned edge = 0; edge < nEdges; ++edge)
            msgOut[edge][z] = out[edge];
        _TCE_RTC(1, loop_2_end); // OpenASIP 2.0 doc (search for printf explanation)
        total_loop_2 += (loop_2_end - loop_2_start);
    }
    _TCE_RTC(1, sim_time); // OpenASIP 2.0 doc (search for printf explanation)
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total timer_declarations elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_timer_declarations/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total nEdgesDec elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_nEdgesDec/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total infDec elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_infDec/1e6);
    //printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total msgOutDec elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_msgOutDec/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total loop_1 elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_loop_1/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total inDec elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_inDec/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total outDec elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_outDec/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total in_init_loop_2 elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_in_init_loop_2/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total out_init_loop_2 elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_out_init_loop_2/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total _OA_CHECK_NODE elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_oa_check_node_operation/1e6);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total loop_2 elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_loop_2/1e6);

    _TCE_RTC(1, check_node_operation_in_end); // OpenASIP 2.0 doc (search for printf explanation)
    total_check_node_operation_in += (check_node_operation_in_end - check_node_operation_in_start);
    printf("[t_sim [s] = %.6f][SNR_0%i][Block %i][decode] total check_node_operation_in elapsed time [s]: %.6f\n",sim_time/1e6,snr_g_id,blk_g_id,total_check_node_operation_in/1e6);
    //return msgOut;
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
