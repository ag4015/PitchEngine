
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

void process_frame(kiss_fft_cpx* input, float* mag, float* magPrev, float* phi_a, float* phi_s, float* phi_sPrev,
                   float* delta_t, float* delta_tPrev, float* delta_f, int hopA, int hopS, float shift, int bufLen, float var)
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
	float b_s = b_a * shift;

	for(uint16_t k = 0; k < bufLen; k++)
	{
		mag[k] = absc(&input[k]);
		current_phi_a = argc(&input[k]);
	}

	// STEP 1
	float tol     = 1e-4;
	float maxMag  = get_max(mag, bufLen);
	float maxPrev = get_max(magPrev, bufLen);
	float abstol  = tol * ((maxMag >= maxPrev) ? (maxMag) : (maxPrev));

	for(uint16_t k = 0; k < bufLen; k++)
	{

		// Time differentiation
		phi_diff = current_phi_a - phi_a[k];
		phi_a[k] = current_phi_a;
		deltaPhiPrime_t_back = phi_diff - (hopA * 2 * PI * k)/bufLen;
		deltaPhiPrimeMod_t_back = fmod(deltaPhiPrime_t_back + PI , 2 * PI) - PI;
		// deltaPhiPrimeMod_t_back = fmod(deltaPhiPrime_t_back, 2 * PI);
		delta_t[k] = deltaPhiPrimeMod_t_back/hopA + (2 * PI * k)/bufLen;

		// Backward frequency differentiation
		if (k > 0 && mag[k-1] > abstol)
		{
			phi_diff = current_phi_a - argc(&input[k - 1]);
			phi_diff = (phi_diff >= 0) ? phi_diff : -phi_diff;
			// delta_f_back = (1/b_a)*(fmod(phi_diff + PI, 2 * PI) - PI);
			delta_f_back = (1/b_a)*(fmod(phi_diff, 2 * PI));
		}
		else { delta_f_back = 0; }

		// Forward frequency differentiation
		if(k < bufLen - 1 && mag[k+1] > abstol)
		{
			phi_diff = argc(&input[k + 1]) - current_phi_a; 
			phi_diff = (phi_diff >= 0) ? phi_diff : -phi_diff;
			// delta_f_fwd = (1/b_a)*(fmod(phi_diff + PI, 2 * PI) - PI);
			delta_f_fwd = (1/b_a)*(fmod(phi_diff, 2 * PI));
		}
		else { delta_f_fwd = 0; }

		// Take the average of fwd and back or whichever is non-zero
		delta_f[k] = (delta_f_back && delta_f_fwd)
                   ? (0.5 * (delta_f_back + delta_f_fwd))
			   	   : (!delta_f_back ? delta_f_fwd : delta_f_back);

	}

	propagate_phase(delta_t, delta_tPrev, delta_f, mag, magPrev, phi_s, phi_sPrev, hopA, shift, bufLen, b_s, abstol);

	for(uint16_t k = 0; k < bufLen; k++)
	{
		expc(&input[k], &mag[k], &phi_s[k]);
	}

	// DUMP_ARRAY(mag, BUFLEN, "debugData/magxxxxx.csv" , count3, 10, 1, -1);
	// DUMP_ARRAY(phi_a, BUFLEN, "debugData/phi_axxxxx.csv" , count3, 10, 1, -1);
	// DUMP_ARRAY(phi_s, BUFLEN, "debugData/phi_sxxxxx.csv" , count3, 10, 1, -1);
	// DUMP_ARRAY(phi_sPrev, BUFLEN, "debugData/phi_sPrevxxxxx.csv" , count3, 10, 1, -1);
	count3++;
}

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
		if (current.n == 1) // STEP 15
		{
			if (setI.count(current.m + 1)) // STEP 16
			{
				// b_s/b_a = alpha
				phi_s[current.m + 1] = phi_s[current.m] + (b_s/2) * (delta_f[current.m] + delta_f[current.m + 1]); // STEP 17
				setI.erase(current.m + 1); // STEP 18
				h.push(Tuple(current.m + 1, 1)); // STEP 19
			}
			if (setI.count(current.m - 1)) // STEP 21
			{
				phi_s[current.m - 1] = phi_s[current.m] - (b_s/2) * (delta_f[current.m] + delta_f[current.m - 1]); // STEP 22
				setI.erase(current.m - 1); // STEP 23
				h.push(Tuple(current.m - 1, 1)); // STEP 24
			}
		}
	}
	for (int i = 0; i < BUFLEN; i++)
	{
		if(!setICopy.count(i))
		{
			delta_t[i] = 0;
			delta_f[i] = 0;
		}
	}
	DUMP_ARRAY(delta_f, BUFLEN, "debugData/delta_f.csv" , count3, -1, 1, -1);
	DUMP_ARRAY(delta_t, BUFLEN, "debugData/delta_t.csv" , count3, -1, 1, -1);
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

void strechFrame(float* output, float* input, int* cleanIdx, int hop, int frameNum, int outputIdx, int outputSize, int bufLen)
{
	int k;
	for (k = 0; k < hop; k++)
	{
		output[*cleanIdx] = 0;
		if (++(*cleanIdx) >= outputSize) *cleanIdx = 0;
	}

	// The indexing variable for output has to be circular.
	int t = outputIdx + hop * frameNum;
	if (t >= outputSize) t = 0;

	for (k = 0; k < bufLen; k++)
	{
		output[t] += input[k];
		if ((++t) >= (outputSize)) t = 0;
	}
}

void interpolate(float* outbuffer, float* vTime, int steps, float shift,
		             int vTimeIdx, float pOutBuffLastSample, int hopS, int bufLen)
{
	int k;
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
		int lowerIdx;
		int upperIdx;
		float delta_shift;
		for (k = vTimeIdx; k < vTimeIdx + BUFLEN; k++)
		{
			tShift = (k - vTimeIdx) * shift;

			lowerIdx = (int)(tShift + vTimeIdx);
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
