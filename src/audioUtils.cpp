
#include "DSPConfig.h"
#include "audioUtils.h"
#include "audioData.h"
#include "logger.h"
#include <algorithm>
#include "Tuple.h"
#include <iostream>
#include <queue>
#include <set>
#include <complex.h>

#define MAGNITUDE_TOLERANCE 1e-4
#define SIMPLE_PV

void process_frame(buffer_data_t* bf, audio_data_t* audat, float var)
{
#ifdef DEBUG_DUMP
	static int count = 0;
#endif
	//float current_phi_a;
	float phi_diff;
	const std::complex<float> i(0, 1);

	// Time differentiation variables.
	// Can't do forward differentiation because the algorithm is real time
	float deltaPhiPrime_t_back[1024];
	float deltaPhiPrimeMod_t_back[1024];

	// Frequency differentiation variables
	float delta_f_back;
	float delta_f_fwd;
	float b_a = 1;
	float b_s = b_a * bf->shift;

	for(uint16_t k = 0; k < bf->buflen; k++)
	{
		bf->phi_sPrev[k] = bf->phi_s[k];
		bf->mag[k] = std::abs(std::complex<float>{bf->cpxOut[k].r, bf->cpxOut[k].i});
		bf->phi_aPrev[k] = bf->phi_a[k];
		bf->phi_a[k] = std::arg(std::complex<float>{bf->cpxOut[k].r, bf->cpxOut[k].i});
	}

	// STEP 1
	float tol = MAGNITUDE_TOLERANCE;
	float maxMag     = *std::max_element(bf->mag, bf->mag + bf->buflen);
	float abstol  = tol * ((maxMag >= bf->maxMagPrev) ? (maxMag) : (bf->maxMagPrev));
	bf->maxMagPrev = maxMag;

	for(uint16_t k = 0; k < bf->buflen; k++)
	{

		// Time differentiation
		phi_diff = bf->phi_a[k] - bf->phi_aPrev[k];
		bf->phi_a[k] = bf->phi_a[k];
		deltaPhiPrime_t_back[k] = phi_diff - (bf->hopA * 2 * PI * k)/bf->buflen;
		deltaPhiPrimeMod_t_back[k] = std::remainder(deltaPhiPrime_t_back[k], 2 * PI);
		bf->delta_t[k] = deltaPhiPrimeMod_t_back[k]/bf->hopA + (2 * PI * k)/bf->buflen;

		// Backward frequency differentiation
		if (k > 0 && bf->mag[k-1] > abstol)
		{
			phi_diff = bf->phi_a[k] - std::arg(std::complex<float>{bf->cpxOut[k - 1].r, bf->cpxOut[k - 1].i});
			delta_f_back = (1/b_a)*(fmod(phi_diff + PI, 2 * PI) - PI);
		}
		else { delta_f_back = 0; }

		// Forward frequency differentiation
		if(k < (bf->buflen - 1) && bf->mag[k+1] > abstol)
		{
			phi_diff = std::arg(std::complex<float>{bf->cpxOut[k + 1].r, bf->cpxOut[k + 1].i}) - bf->phi_a[k]; 
			delta_f_fwd = (1/b_a)*(fmod(phi_diff + PI, 2 * PI) - PI);
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
		bf->phi_s[k] = bf->phi_s[k] + bf->hopS * bf->delta_t[k];
	}
#else
	propagate_phase(bf, audat, b_s, abstol);
#endif
	std::complex<float> z[1024];

	for(uint16_t k = 0; k < bf->buflen; k++)
	{
		z[k] = bf->mag[k] * std::exp(std::complex{0.f, bf->phi_s[k]});
		//std::complex z = bf->mag[k] * std::exp(i * bf->phi_s[k]);
		bf->cpxOut[k].r = std::real(z[k]);
		bf->cpxOut[k].i = std::imag(z[k]);
	}

	DUMP_ARRAY(bf->mag      , bf->buflen, DEBUG_DIR "magxxxxx.csv"       , count, 10, 1, -1);
	DUMP_ARRAY(bf->phi_a    , bf->buflen, DEBUG_DIR "phi_axxxxx.csv"     , count, 10, 1, -1);
	DUMP_ARRAY(bf->phi_s    , bf->buflen, DEBUG_DIR "phi_sxxxxx.csv"     , count, 10, 1, -1);
	DUMP_ARRAY(bf->phi_sPrev, bf->buflen, DEBUG_DIR "phi_sPrevxxxxx.csv" , count, 10, 1, -1);
#ifdef DEBUG_DUMP
	count++;
#endif
}

void propagate_phase(buffer_data_t* bf, audio_data_t* audat, float b_s, float abstol)
{
#ifdef DEBUG_DUMP
	static int count = 0;
#endif
	std::set<uint16_t> setI, setICopy;
	std::vector<Tuple> container;
	container.reserve(1024);
	TupleCompareObject cmp(bf->mag, bf->magPrev);
	std::priority_queue<Tuple, std::vector<Tuple>, TupleCompareObject> h{cmp, std::move(container)}; // STEP 4

	for (uint32_t m = 0; m < bf->buflen; m++)
	{
		if (bf->mag[m] > abstol)
		{ 
			setI.insert(m); // STEP 2
			h.push(Tuple(m, 0)); // STEP 5
		}
		else { bf->phi_s[m] = (std::rand()/(float)RAND_MAX) * 2 * PI - PI; } // STEP 3
	}
	setICopy = setI;

	while(!setI.empty()) // STEP 6
	{
		// STEP 7
		Tuple current = h.top();
		h.pop();

		if (current.n == 0) // STEP 8
		{
			if(setI.count(current.m)) // STEP 9
			{
				bf->phi_s[current.m] = bf->phi_sPrev[current.m] + (bf->hopA/2)*(bf->delta_tPrev[current.m] + bf->delta_t[current.m]); // STEP 10
				setI.erase(current.m); // STEP 11
				h.push(Tuple(current.m, 1)); // STEP 12
			}
		}
		if (current.n == 1) // STEP 15
		{
			if (setI.count(current.m + 1)) // STEP 16
			{
				// b_s/b_a = alpha
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
	for (uint32_t i = 0; i < bf->buflen; i++)
	{
		if(!setICopy.count(i))
		{
			bf->delta_t[i] = 0;
			bf->delta_f[i] = 0;
		}
	}
	DUMP_ARRAY(bf->delta_f, bf->buflen, DEBUG_DIR "bf->delta_f.csv" , count, -1, 1, -1);
	DUMP_ARRAY(bf->delta_t, bf->buflen, DEBUG_DIR "bf->delta_t.csv" , count, -1, 1, -1);
#ifdef DEBUG_DUMP
	count++;
#endif
	return;
}

void overlapAdd(float* input, float* frame, float* output, int hop, uint8_t frameNum, int numFrames)
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

void strechFrame(float* output, float* input, uint32_t* cleanIdx, uint32_t hop,
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
//void strechFrame(float* output, float* input, int* cleanIdx, int hop, int frameNum, int outputIdx, int outputSize, int bufLen)
//{
//	int k;
//	for (k = 0; k < hop; k++)
//	{
//		output[*cleanIdx] = 0;
//		if (++(*cleanIdx) >= outputSize) *cleanIdx = 0;
//	}
//
//	// The indexing variable for output has to be circular.
//	int t = outputIdx + hop * frameNum;
//	if (t >= outputSize) t = 0;
//
//	for (k = 0; k < bufLen; k++)
//	{
//		output[t] += input[k];
//		if ((++t) >= (outputSize)) t = 0;
//	}
//}

// TODO FIX POUTBUFFLASTSAMPLE
void interpolate(buffer_data_t* bf, audio_data_t* audat, uint32_t vTimeIdx, float pOutBuffLastSample)
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
		float tShift;
		float upper;
		float lower;
		uint32_t lowerIdx;
		uint32_t upperIdx;
		float delta_shift;
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
				delta_shift = (audat->outbuffer[k - vTimeIdx + 1] - pOutBuffLastSample) / 2;
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
		pOutBuffLastSample = audat->outbuffer[k - vTimeIdx - 1];
	}
}

void process_buffer(buffer_data_t* bf, audio_data_t* audat, uint8_t frameNum,
	uint32_t audio_ptr, uint32_t* vTimeIdx, uint32_t* cleanIdx, float pOutBuffLastSample, float var)
{
#ifdef DEBUG_DUMP
	static int counter_1 = 0;
	static int counter_2 = 0;
	static int sample_counter = 0;
#endif
	static int reset_counter = 0;

	float inwinScale = sqrt(((bf->buflen / bf->hopA) / 2));
	float outwinScale = sqrt(((bf->buflen / bf->hopS) / 2));

	for (uint8_t f = 0; f < audat->numFrames; f++)
	{
		
		swap_ping_pong_buffer_data(bf, audat);

        /************ ANALYSIS STAGE ***********************/

		// Using mag as output buffer, nothing to do with magnitude
		overlapAdd(audat->inbuffer, audat->inframe, audat->outframe, bf->hopA, frameNum, audat->numFrames);  

		// TODO: Need to fix this
		if (++reset_counter == 256)
		{
			reset_buffer_data_arrays(bf);
			reset_counter = 0;
		}

		for (uint32_t k = 0; k < bf->buflen; k++)
		{
			bf->cpxIn[k].r = (audat->outframe[k] * audat->inwin[k]) / inwinScale;
			bf->cpxIn[k].i = 0;
		}

        /************ PROCESSING STAGE *********************/

		DUMP_ARRAY_COMPLEX(bf->cpxIn  , bf->buflen, DEBUG_DIR "cpxIn.csv"  , counter_1,  5, sample_counter, -1);

		kiss_fft( bf->cfg , bf->cpxIn , bf->cpxOut );

		process_frame(bf, audat, var);

		DUMP_ARRAY_COMPLEX(bf->cpxOut, bf->buflen, DEBUG_DIR "cpxOut.csv"  , counter_1, 40, sample_counter , -1);
		// DUMP_ARRAY(audat->inbuffer , bf->buflen, DEBUG_DIR "inbuffer.csv", counter_1, -1, sample_counter , bf->buflen);
		// DUMP_ARRAY(audat->inwin    , bf->buflen, DEBUG_DIR "inwin.csv"   , counter_1, -1, sample_counter , bf->buflen);
		// DUMP_ARRAY(audat->outwin   , bf->buflen, DEBUG_DIR "outwin.csv"  , counter_1, -1, sample_counter , bf->buflen);
		// DUMP_ARRAY(bf->phi_a       , bf->buflen, DEBUG_DIR "phi_a.csv"   , counter_1, -1, sample_counter , bf->buflen);
		// DUMP_ARRAY(bf->phi_s       , bf->buflen, DEBUG_DIR "phi_s.csv"   , counter_1, -1, sample_counter , bf->buflen);

		std::swap(bf->cpxIn, bf->cpxOut);

		kiss_fft( bf->cfgInv , bf->cpxIn, bf->cpxOut);

		// DUMP_ARRAY(bf->cpxIn       , bf->buflen, DEBUG_DIR "cpxOut.csv"  , counter_1, 5, sample_counter, -1);

		for (uint32_t k = 0; k < bf->buflen; k++)
		{
			audat->outframe[k] = bf->cpxOut[k].r * (audat->outwin[k] / bf->buflen) / outwinScale;
		}

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
