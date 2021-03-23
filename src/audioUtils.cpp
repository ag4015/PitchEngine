
#include "audioUtils.h"
#include "main.h"
#include <algorithm>
#include "Tuple.h"
#include <iostream>
#include <queue>
#include <set>
#include <complex.h>

int count3 = 0;
bool firstTime = true;

float absc(kiss_fft_cpx *a)
{
	return sqrt(pow(a->i, 2) + pow(a->r, 2));
}
float argc(kiss_fft_cpx *a)
{
	// float complex z;
	std::complex<float> z(a->r,a->i);
	return std::arg(z);
}
void expc(kiss_fft_cpx *a, float* mag, float* phase)
{
	a->r = std::real(std::polar(*mag, *phase));
	a->i = std::imag(std::polar(*mag, *phase));
}

void process_frame(buffer_data_t* bf, float var)
{
	float current_phi_a;
	float phi_diff;
	/*
	 Time differentiation variables.
	 Can't do forward differentiation because the algorithm is real time
	*/
	float deltaPhiPrime_t_back;
	float deltaPhiPrimeMod_t_back;

	// Frequency differentiation variables
	float delta_f_back;
	float delta_f_fwd;
	float b_a = 1;
	float b_s = b_a * bf->shift;

	for(uint16_t k = 0; k < bf->buflen; k++)
	{
		bf->mag[k] = absc(&bf->input[k]);
		current_phi_a = argc(&bf->input[k]);
	}

	// STEP 1
	float tol     = 1e-4;
	float maxMag  = get_max(bf->mag, bf->buflen);
	float maxPrev = get_max(bf->magPrev, bf->buflen);
	float abstol  = tol * ((maxMag >= maxPrev) ? (maxMag) : (maxPrev));

	for(uint16_t k = 0; k < bf->buflen; k++)
	{

		// Time differentiation
		phi_diff = current_phi_a - bf->phi_a[k];
		bf->phi_a[k] = current_phi_a;
		deltaPhiPrime_t_back = phi_diff - (bf->hopA * 2 * PI * k)/bf->buflen;
		deltaPhiPrimeMod_t_back = fmod(deltaPhiPrime_t_back + PI , 2 * PI) - PI;
		// deltaPhiPrimeMod_t_back = fmod(deltaPhiPrime_t_back, 2 * PI);
		bf->delta_t[k] = deltaPhiPrimeMod_t_back/bf->hopA + (2 * PI * k)/bf->buflen;

		// Backward frequency differentiation
		if (k > 0 && bf->mag[k-1] > abstol)
		{
			phi_diff = current_phi_a - argc(&bf->input[k - 1]);
			phi_diff = (phi_diff >= 0) ? phi_diff : -phi_diff;
			// delta_f_back = (1/b_a)*(fmod(phi_diff + PI, 2 * PI) - PI);
			delta_f_back = (1/b_a)*(fmod(phi_diff, 2 * PI));
		}
		else { delta_f_back = 0; }

		// Forward frequency differentiation
		if(k < bf->buflen - 1 && bf->mag[k+1] > abstol)
		{
			phi_diff = argc(&bf->input[k + 1]) - current_phi_a; 
			phi_diff = (phi_diff >= 0) ? phi_diff : -phi_diff;
			// delta_f_fwd = (1/b_a)*(fmod(phi_diff + PI, 2 * PI) - PI);
			delta_f_fwd = (1/b_a)*(fmod(phi_diff, 2 * PI));
		}
		else { delta_f_fwd = 0; }

		// Take the average of fwd and back or whichever is non-zero
		bf->delta_f[k] = (delta_f_back && delta_f_fwd)
                   ? (0.5f * (delta_f_back + delta_f_fwd))
			   	   : (!delta_f_back ? delta_f_fwd : delta_f_back);

	}

	propagate_phase(bf->delta_t, bf->delta_tPrev, bf->delta_f, bf->mag, bf->magPrev, bf->phi_s, bf->phi_sPrev, bf->hopA, bf->shift, bf->buflen, b_s, abstol);

	for(uint16_t k = 0; k < bf->buflen; k++)
	{
		expc(&bf->input[k], &bf->mag[k], &bf->phi_s[k]);
	}

	// DUMP_ARRAY(mag, BUFLEN, DEBUG_DIR "magxxxxx.csv" , count3, 10, 1, -1);
	// DUMP_ARRAY(phi_a, BUFLEN, DEBUG_DIR "phi_axxxxx.csv" , count3, 10, 1, -1);
	// DUMP_ARRAY(phi_s, BUFLEN, DEBUG_DIR "phi_sxxxxx.csv" , count3, 10, 1, -1);
	// DUMP_ARRAY(phi_sPrev, BUFLEN, DEBUG_DIR "phi_sPrevxxxxx.csv" , count3, 10, 1, -1);
	count3++;
}

// TODO: Create a struct with all these variables
void propagate_phase(float* delta_t, float* delta_tPrev, float* delta_f, float* mag, float* magPrev, float* phi_s, float* phi_sPrev, float hopA, float shift, int bufLen, float b_s, float abstol)
{
	// int count = already;
	std::set<uint16_t> setI, setICopy;
	std::vector<Tuple> container;
	container.reserve(1024);
	TupleCompareObject cmp(mag, magPrev);
	std::priority_queue<Tuple, std::vector<Tuple>, TupleCompareObject> h{cmp, std::move(container)}; // STEP 4

	for (int m = 0; m < bufLen; m++)
	{
		if (mag[m] > abstol)
		{ 
			setI.insert(m); // STEP 2
			h.push(Tuple(m, 0)); // STEP 5
		}
		else { phi_s[m] = (std::rand()/(float)RAND_MAX) * 2 * PI - PI; } // STEP 3
	}
	setICopy = setI;

	// PRINT_LOG2("I size: %lu\n", setI.size());

	while(!setI.empty()) // STEP 6
	{
		// STEP 7
		Tuple current = h.top();
		h.pop();

		if (current.n == 0) // STEP 8
		{
			if(setI.count(current.m)) // STEP 9
			{
				phi_s[current.m] = phi_sPrev[current.m] + (hopA/2)*(delta_tPrev[current.m] + delta_t[current.m]); // STEP 10
				setI.erase(current.m); // STEP 11
				h.push(Tuple(current.m, 1)); // STEP 12
			}
		}
		//if (current.n == 1) // STEP 15
		//{
		//	if (setI.count(current.m + 1)) // STEP 16
		//	{
		//		// b_s/b_a = alpha
		//		phi_s[current.m + 1] = phi_s[current.m] + (b_s/2) * (delta_f[current.m] + delta_f[current.m + 1]); // STEP 17
		//		setI.erase(current.m + 1); // STEP 18
		//		h.push(Tuple(current.m + 1, 1)); // STEP 19
		//	}
		//	if (setI.count(current.m - 1)) // STEP 21
		//	{
		//		phi_s[current.m - 1] = phi_s[current.m] - (b_s/2) * (delta_f[current.m] + delta_f[current.m - 1]); // STEP 22
		//		setI.erase(current.m - 1); // STEP 23
		//		h.push(Tuple(current.m - 1, 1)); // STEP 24
		//	}
		//}
	}
	for (int i = 0; i < BUFLEN; i++)
	{
		if(!setICopy.count(i))
		{
			delta_t[i] = 0;
			delta_f[i] = 0;
		}
	}
	DUMP_ARRAY(delta_f, BUFLEN, DEBUG_DIR "delta_f.csv" , count3, -1, 1, -1);
	DUMP_ARRAY(delta_t, BUFLEN, DEBUG_DIR "delta_t.csv" , count3, -1, 1, -1);
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
		*cleanIdx = *cleanIdx + 1;
		if (*cleanIdx >= outputSize) *cleanIdx = 0;
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

// TODO FIX POUTBUFFLASTSAMPLE
void interpolate(float* outbuffer, float* vTime, uint8_t steps, float shift,
	uint32_t vTimeIdx, float pOutBuffLastSample, uint32_t hopS, uint32_t bufLen)
{
	uint32_t k;
	if (steps == 12)
	{
		for (k = 0; k < BUFLEN; k++)
		{
			outbuffer[k] = vTime[vTimeIdx + k * 2];
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
		for (k = vTimeIdx; k < vTimeIdx + BUFLEN; k++)
		{
			tShift = (k - vTimeIdx) * shift;

			lowerIdx = (uint32_t)(tShift + vTimeIdx);
			upperIdx = lowerIdx + 1;
			if (lowerIdx == 0)
			{
				lower = vTime[lowerIdx + 1];
				upper = vTime[upperIdx + 1];
				outbuffer[k - vTimeIdx + 1] = lower * (1 - (tShift - (int)tShift)) + upper * (tShift - (int)tShift);
				delta_shift = (outbuffer[k - vTimeIdx + 1] - pOutBuffLastSample) / 2;
				outbuffer[k - vTimeIdx] = outbuffer[k - vTimeIdx + 1] - delta_shift;
			}
			if (upperIdx == 2*hopS*NUMFRAMES)
			{
				delta_shift = (shift*(lower - outbuffer[k - vTimeIdx - 1]))/(lowerIdx - tShift + shift);
				outbuffer[k - vTimeIdx] = delta_shift + outbuffer[k - vTimeIdx - 1];
				continue;
			}
			if (upperIdx == 2*hopS*NUMFRAMES + 1 && lowerIdx == 2*hopS*NUMFRAMES)
			{
				delta_shift = (shift*(lower - outbuffer[k - vTimeIdx - 1]))/(lowerIdx - tShift + shift);
				outbuffer[k - vTimeIdx] = delta_shift + outbuffer[k - vTimeIdx - 1];
				continue;
			}
			lower = vTime[lowerIdx];
			upper = vTime[upperIdx];
			outbuffer[k - vTimeIdx] = lower * (1 - (tShift - (int)tShift)) + upper * (tShift - (int)tShift);
		}
		pOutBuffLastSample = outbuffer[k - vTimeIdx - 1];
	}
}

float get_max(const float* in, int size)
{
	float max = 0;
	for (int k = 0; k < size; k++)
	{
		if (in[k] > max) { max = in[k]; }
	}
	return max;
}
