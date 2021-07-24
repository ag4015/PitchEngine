
#include "DSPConfig.h"
#include "audioUtils.h"
#include "audioData.h"
#include "logger.h"
#include "Tuple.h"
#ifdef CONSTANT_Q_T
#include "ConstantQ.h"
#include "CQInverse.h"
#endif
#include <algorithm>
#include <iostream>
#include <queue>
#include <set>
#include <complex>
#include <random>
#include <chrono>

#define MAGNITUDE_TOLERANCE static_cast<my_float>(1e-6)
//#define SIMPLE_PV

void process_frame(buffer_data_t* bf)
{
#ifdef DEBUG_DUMP
	static int count = 0;
#endif
	//my_float current_phi_a;
	my_float phi_diff;
	const std::complex<my_float> i(0, 1);

	// Time differentiation variables.
	// Can't do forward differentiation because the algorithm is real time
	my_float deltaPhiPrime_t_back;
	my_float deltaPhiPrimeMod_t_back;

	// Frequency differentiation variables
	my_float delta_f_back;
	my_float delta_f_fwd;
	my_float b_a = 1; // b_a/b_s = shift; The value of b_a has no effect on the result
	my_float b_s = b_a * bf->shift;

	for(uint32_t k = 0; k < bf->buflen; k++)
	{
		bf->mag[k] = std::abs(std::complex<my_float>{bf->cpxOut[k].r, bf->cpxOut[k].i});
		bf->phi_aPrev[k] = bf->phi_a[k];
		bf->phi_a[k] = std::arg(std::complex<my_float>{bf->cpxOut[k].r, bf->cpxOut[k].i});
	}

	// STEP 1
	my_float tol = MAGNITUDE_TOLERANCE;
	my_float maxMag     = *std::max_element(bf->mag, bf->mag + bf->buflen);
	my_float abstol  = tol * ((maxMag >= bf->maxMagPrev) ? (maxMag) : (bf->maxMagPrev));
	bf->maxMagPrev = maxMag;

	for(uint32_t k = 0; k < bf->buflen; k++)
	{

		// Time differentiation
		phi_diff = bf->phi_a[k] - bf->phi_aPrev[k];
		deltaPhiPrime_t_back = phi_diff - ((my_float)bf->hopA * 2 * PI * k)/bf->buflen;
		deltaPhiPrimeMod_t_back = std::remainder(deltaPhiPrime_t_back, 2 * PI);
		bf->delta_t[k] = deltaPhiPrimeMod_t_back/bf->hopA + (2 * PI * k)/bf->buflen;

		// Backward frequency differentiation
		if (k > 0)
		{
			phi_diff = bf->phi_a[k] - bf->phi_a[k - 1];
			delta_f_back = (1 / b_a) * std::remainder(phi_diff, 2 * PI);
		}
		else { delta_f_back = 0; }

		// Forward frequency differentiation
		if(k < (bf->buflen - 1))
		{
			phi_diff = bf->phi_a[k + 1] - bf->phi_a[k]; 
			delta_f_fwd = (1 / b_a) * std::remainder(phi_diff, 2 * PI);
		}
		else { delta_f_fwd = 0; }

		// Take the average of fwd and back or whichever is non-zero
		bf->delta_f[k] = (delta_f_back && delta_f_fwd)
                   ? (0.5f * (delta_f_back + delta_f_fwd))
			   	   : (!delta_f_back ? delta_f_fwd : delta_f_back);

	}
	
#ifdef SIMPLE_PV
	for (uint32_t k = 0; k < bf->buflen; k++)
	{
		bf->phi_s[k] = bf->phi_sPrev[k] + (bf->hopS / 2) * (bf->delta_t[k] + bf->delta_tPrev[k]);
	}
#else
	auto initTime  = std::chrono::high_resolution_clock::now();
	propagate_phase(bf, b_s , abstol);
	auto finalTime = std::chrono::high_resolution_clock::now();
	auto exTime  = std::chrono::duration_cast<std::chrono::milliseconds>(finalTime - initTime);
	PRINT_LOG("Propagate phase execution time: %l ms.\n", exTime.count());
#endif
	std::complex<my_float> z;

	for(uint16_t k = 0; k < bf->buflen; k++)
	{
		z = bf->mag[k] * std::exp(std::complex<my_float>{0.f, bf->phi_s[k]});
		bf->cpxOut[k].r = std::real(z);
		bf->cpxOut[k].i = std::imag(z);
	}

//	DUMP_ARRAY(bf->mag      , bf->buflen, DEBUG_DIR "mag.csv"       , count, -1, 1, -1);
//	DUMP_ARRAY(bf->phi_a    , bf->buflen, DEBUG_DIR "phi_a.csv"     , count, -1, 1, -1);
//	DUMP_ARRAY(bf->phi_s    , bf->buflen, DEBUG_DIR "phi_s.csv"     , count, -1, 1, -1);
//	DUMP_ARRAY(bf->phi_sPrev, bf->buflen, DEBUG_DIR "phi_sPrev.csv" , count, -1, 1, -1);
//#ifdef DEBUG_DUMP
//	count++;
//#endif
}

void propagate_phase(buffer_data_t* bf, my_float b_s, my_float abstol)
{
#ifdef DEBUG_DUMP
	static int count = 0;
#endif
	std::set<uint16_t> setI;
	std::vector<Tuple> container;
	container.reserve(bf->buflen);
	TupleCompareObject cmp(bf->mag, bf->magPrev);
	std::priority_queue<Tuple, std::vector<Tuple>, TupleCompareObject<my_float> > h{cmp, std::move(container)}; // STEP 4

	for (uint16_t m = 0; m < bf->buflen; m++)
	{
		if (bf->mag[m] > abstol)
		{ 
			setI.insert(m); // STEP 2
			h.push(Tuple(m, 0)); // STEP 5
		}
		else // STEP 3
		{
			bf->phi_s[m] = (std::rand()/(my_float)RAND_MAX) * 2 * PI - PI;
		}
	}

	while(!setI.empty()) // STEP 6
	{
		// STEP 7
		Tuple current = h.top();
		h.pop();

		if (current.n == 0) // STEP 8
		{
			if(setI.count(current.m)) // STEP 9
			{
				bf->phi_s[current.m] = bf->phi_sPrev[current.m] + (bf->hopS/2)*(bf->delta_tPrev[current.m] + bf->delta_t[current.m]); // STEP 10
				setI.erase(current.m); // STEP 11
				h.push(Tuple(current.m, 1)); // STEP 12
			}
		}
		if (current.n == 1) // STEP 15
		{
			if (setI.count(current.m + 1)) // STEP 16
			{
				bf->phi_s[current.m + 1] = bf->phi_s[current.m] + (b_s/2) * (bf->delta_f[current.m] + bf->delta_f[current.m + 1]); // STEP 17
				setI.erase(current.m + 1); // STEP 18
				h.push(Tuple(current.m + 1, 1)); // STEP 19
			}
			if (setI.count(current.m - 1)) // STEP 21
			{
				bf->phi_s[current.m - 1] = bf->phi_s[current.m] - (b_s/2) * (bf->delta_f[current.m] + bf->delta_f[current.m - 1]); // STEP 22
				setI.erase(current.m - 1); // STEP 23
				h.push(Tuple(current.m - 1, 1)); // STEP 24
			}
		}
	}

	//DUMP_ARRAY(bf->delta_f, bf->buflen, DEBUG_DIR "bf->delta_f.csv" , count, -1, 1, -1);
	//DUMP_ARRAY(bf->delta_t, bf->buflen, DEBUG_DIR "bf->delta_t.csv" , count, -1, 1, -1);
#ifdef DEBUG_DUMP
	count++;
#endif
	return;
}

void overlapAdd(my_float* input, my_float* frame, my_float* output, int hop, uint8_t frameNum, int numFrames)
{
	for (int k = 0; k < hop; k++)
	{
		frame[frameNum * hop + k] = input[frameNum * hop + k];
	}
	int frameNum2 = frameNum + 1;
	if (frameNum2 >= numFrames) frameNum2 = 0;
	for (uint8_t f2 = 0; f2 < numFrames; f2++)
	{
		for (int k = 0; k < hop; k++)
		{
			output[k + f2 * hop] = frame[frameNum2 * hop + k];
		}
		if (++frameNum2 >= numFrames) frameNum2 = 0;
	}
}

void strechFrame(my_float* output, my_float* input, uint32_t* cleanIdx, uint32_t hop,
	uint8_t frameNum, uint32_t outputIdx, uint32_t outputSize, uint32_t bufLen)
{
	for (uint32_t k = 0; k < hop; k++)
	{
		output[*cleanIdx] = 0;
		if (++(*cleanIdx) >= outputSize) *cleanIdx = 0;
	}

	// The indexing variable for output has to be circular.
	uint32_t t = outputIdx + hop * frameNum;
	if (t >= outputSize) t = 0;

	for (uint32_t k = 0; k < bufLen; k++)
	{
		output[t] += input[k];
		if ((++t) >= (outputSize)) t = 0;
	}
}

// TODO: IMPROVE
void interpolate(buffer_data_t* bf, audio_data_t* audat, uint32_t vTimeIdx, my_float* pOutBuffLastSample)
{
	uint32_t k;
	if (bf->steps == 12)
	{
		for (k = 0; k < bf->buflen; k++)
		{
			audat->outbuffer[k] = audat->vTime[vTimeIdx + k * 2];
		}
	}
	else
	{
		my_float tShift;
		my_float upper;
		my_float lower = 0;
		uint32_t lowerIdx;
		uint32_t upperIdx;
		my_float delta_shift;
		for (k = vTimeIdx; k < vTimeIdx + bf->buflen; k++)
		{
			tShift = (k - vTimeIdx) * bf->shift;

			lowerIdx = (uint32_t)(tShift + vTimeIdx);
			upperIdx = lowerIdx + 1;
			if (lowerIdx == 0)
			{
				lower = audat->vTime[lowerIdx + 1];
				upper = audat->vTime[upperIdx + 1];
				audat->outbuffer[k - vTimeIdx + 1] = lower * (1 - (tShift - (int)tShift)) + upper * (tShift - (int)tShift);
				delta_shift = (audat->outbuffer[k - vTimeIdx + 1] - *pOutBuffLastSample) / 2;
				audat->outbuffer[k - vTimeIdx] = audat->outbuffer[k - vTimeIdx + 1] - delta_shift;
			}
			if (upperIdx == 2*bf->hopS*audat->numFrames)
			{
				delta_shift = (bf->shift*(lower - audat->outbuffer[k - vTimeIdx - 1]))/(lowerIdx - tShift + bf->shift);
				audat->outbuffer[k - vTimeIdx] = delta_shift + audat->outbuffer[k - vTimeIdx - 1];
				continue;
			}
			if (upperIdx == 2*bf->hopS*audat->numFrames + 1 && lowerIdx == 2*bf->hopS*audat->numFrames)
			{
				delta_shift = (bf->shift*(lower - audat->outbuffer[k - vTimeIdx - 1]))/(lowerIdx - tShift + bf->shift);
				audat->outbuffer[k - vTimeIdx] = delta_shift + audat->outbuffer[k - vTimeIdx - 1];
				continue;
			}
			lower = audat->vTime[lowerIdx];
			upper = audat->vTime[upperIdx];
			audat->outbuffer[k - vTimeIdx] = lower * (1 - (tShift - (int)tShift)) + upper * (tShift - (int)tShift);
		}
		*pOutBuffLastSample = audat->outbuffer[k - vTimeIdx - 1];
	}
}

void process_buffer(buffer_data_t* bf, audio_data_t* audat, uint8_t frameNum,
	uint32_t* vTimeIdx, my_float* pOutBuffLastSample)
{
#ifdef DEBUG_DUMP
	static int counter_1 = 0;
	static int counter_2 = 0;
	static int sample_counter = 0;
#endif
#ifdef RESET_BUFFER
	static int reset_counter = 0;
#endif


#ifdef CONSTANT_Q_T
	// Constant Q variable initialization
	uint32_t minFreq = 40;
	uint32_t maxFreq = 1000;
	uint8_t  bpo = 64;

	CQParameters params(audat->sampleRate, minFreq, maxFreq, bpo);
	ConstantQ cq(params);
	CQInverse cqi(params);
#endif

#ifdef USE_DOUBLE
	my_float inwinScale = sqrt(((bf->buflen / bf->hopA) / 2));
	my_float outwinScale = sqrt(((bf->buflen / bf->hopS) / 2));
#else
	my_float inwinScale  = sqrtf(((static_cast<my_float>(bf->buflen) / static_cast<my_float>(bf->hopA)) / 2));
	my_float outwinScale = sqrtf(((static_cast<my_float>(bf->buflen) / static_cast<my_float>(bf->hopS)) / 2));
#endif

	for (uint8_t f = 0; f < audat->numFrames; f++)
	{
		
		swap_ping_pong_buffer_data(bf, audat);

        /************ ANALYSIS STAGE ***********************/

		// Using mag as output buffer, nothing to do with magnitude
		overlapAdd(audat->inbuffer, audat->inframe, audat->outframe, bf->hopA, frameNum, audat->numFrames);  

		// TODO: Need to fix this for floats
#ifdef RESET_BUFFER
		if (++reset_counter == 256)
		{
			reset_buffer_data_arrays(bf);
			reset_counter = 0;
		}
#endif

//#ifdef CONSTANT_Q_T
//		std::vector<my_float> cqin;
//		cqin.reserve(bf->buflen);
//
//		for (uint32_t k = 0; k < bf->buflen; k++)
//		{
//			cqin.push_back((audat->outframe[k] * audat->inwin[k]) / inwinScale);
//		}
//		ConstantQ::ComplexBlock cqCoeff = cq.process(cqin);
//#else
		for (uint32_t k = 0; k < bf->buflen; k++)
		{
			bf->cpxIn[k].r = (audat->outframe[k] * audat->inwin[k]) / inwinScale;
			bf->cpxIn[k].i = 0;
		}
//#endif

        /************ PROCESSING STAGE *********************/

		//DUMP_ARRAY_COMPLEX(bf->cpxIn  , bf->buflen, DEBUG_DIR "cpxIn.csv"  , counter_1,  5, sample_counter, -1);

		kiss_fft( bf->cfg , bf->cpxIn , bf->cpxOut );

		//auto initTime  = std::chrono::high_resolution_clock::now();
		process_frame(bf);
		//auto finalTime = std::chrono::high_resolution_clock::now();
		//auto exTime  = std::chrono::duration_cast<std::chrono::milliseconds>(finalTime - initTime);
		//PRINT_LOG("Process frame execution time: %d ms.\n", exTime.count());

		//DUMP_ARRAY_COMPLEX(bf->cpxOut, bf->buflen, DEBUG_DIR "cpxOut.csv"  , counter_1, 40, sample_counter , -1);
		// DUMP_ARRAY(audat->inbuffer , bf->buflen, DEBUG_DIR "inbuffer.csv", counter_1, -1, sample_counter , bf->buflen);
		// DUMP_ARRAY(audat->inwin    , bf->buflen, DEBUG_DIR "inwin.csv"   , counter_1, -1, sample_counter , bf->buflen);
		// DUMP_ARRAY(audat->outwin   , bf->buflen, DEBUG_DIR "outwin.csv"  , counter_1, -1, sample_counter , bf->buflen);
		// DUMP_ARRAY(bf->phi_a       , bf->buflen, DEBUG_DIR "phi_a.csv"   , counter_1, -1, sample_counter , bf->buflen);
		// DUMP_ARRAY(bf->phi_s       , bf->buflen, DEBUG_DIR "phi_s.csv"   , counter_1, -1, sample_counter , bf->buflen);

		std::swap(bf->cpxIn, bf->cpxOut);

		kiss_fft( bf->cfgInv , bf->cpxIn, bf->cpxOut);

		// DUMP_ARRAY(bf->cpxIn       , bf->buflen, DEBUG_DIR "cpxOut.csv"  , counter_1, 5, sample_counter, -1);

//#ifdef CONSTANT_Q_T
//		for (uint32_t k = 0; k < bf->buflen && k < cqout.size(); k++)
//		{
//			audat->outframe[k] = cqout[k] * (audat->outwin[k] / bf->buflen) / outwinScale;
//		}
//#else
		for (uint32_t k = 0; k < bf->buflen; k++)
		{
			audat->outframe[k] = bf->cpxOut[k].r * (audat->outwin[k] / bf->buflen) / outwinScale;
		}
//#endif

        /************ SYNTHESIS STAGE ***********************/

		strechFrame(audat->vTime, audat->outframe, &audat->cleanIdx, bf->hopS, frameNum, *vTimeIdx, audat->numFrames * bf->hopS * 2, bf->buflen);

		// DUMP_ARRAY(audat->vTime, audat->numFrames*bf->hopS*2, DEBUG_DIR "vTimeXXX.csv", counter_1, 40, sample_counter, -1);

		if ((++frameNum) >= audat->numFrames) frameNum = 0;

#ifdef DEBUG_DUMP
		counter_1++;
#endif

	}

    /************* LINEAR INTERPOLATION *****************/

	interpolate(bf, audat, *vTimeIdx, pOutBuffLastSample);

	// DUMP_ARRAY(audat->outbuffer, bf->buflen, DEBUG_DIR "outXXX.csv", counter_2, 10, audio_ptr, -1);

	*vTimeIdx += audat->numFrames * bf->hopS;
	if ((*vTimeIdx) >= audat->numFrames * bf->hopS * 2) *vTimeIdx = 0;

#ifdef DEBUG_DUMP
	counter_2++;
#endif

}
