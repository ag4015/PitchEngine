
#include "audioUtils.h"
#include <algorithm>
#include <vector>


float absc(kiss_fft_cpx *a)
{
	return sqrt(pow(a->i, 2) + pow(a->r, 2));
}
float argc(kiss_fft_cpx *a)
{
	return atan(a->i/a->r);
}
void expc(kiss_fft_cpx *a, float mag, float phase)
{
	a->r = mag*cos(phase);
	a->i = mag*sin(phase);
}

void process_frame(kiss_fft_cpx* input, float* mag, float* magPrev, float* phi_a, float* phi_s,
                   float* delta_t_back, float* delta_f_cent, int hopA, int hopS, int bufLen)
{
	float current_phi_a;
	float tol = 1e-6;

	float	phi_diff;
  /*
	 Time differentiation variables.
	 Can't do forward differentiation because the algorithm is real time
	*/
	float	deltaPhiPrime_t_back;
	float	deltaPhiPrimeMod_t_back;

	// Frequency differentiation variables
	float	deltaPhiPrimeMod_f_back;
	float	deltaPhiPrimeMod_f_fwd;
	float delta_f_back;
	float delta_f_fwd;

	for(uint16_t k = 0; k < bufLen; k++)
	{
		mag[k] = absc(&input[k]);

		current_phi_a = argc(&input[k]);

		// Time differentiation
		phi_diff = current_phi_a - phi_a[k];
		phi_a[k] = current_phi_a;
		deltaPhiPrime_t_back = phi_diff - (hopA * 2 * PI * k)/bufLen;
		deltaPhiPrimeMod_t_back = fmod(deltaPhiPrime_t_back + PI , 2 * PI) - PI;
		delta_t_back[k] = (2 * PI * k)/bufLen + deltaPhiPrimeMod_t_back/hopA;

		// Backward frequency differentiation
		if (k > 0)
		{
			phi_diff = argc(&input[k]) - argc(&input[k - 1]);
			deltaPhiPrimeMod_f_back = fmod(phi_diff + PI, 2 * PI) - PI;
			delta_f_back = deltaPhiPrimeMod_f_back/hopS;
		}
		else { delta_f_back = 0; }

		// Forward frequency differentiation
		if(k < bufLen - 1)
		{
			phi_diff = argc(&input[k + 1]) - argc(&input[k]);
			deltaPhiPrimeMod_f_fwd = fmod(phi_diff + PI, 2 * PI) - PI;
			delta_f_fwd = deltaPhiPrimeMod_f_fwd/hopS; // Might not be hopS?
		}
		else { delta_f_fwd = 0; }

		// Take the average of fwd and back or whichever is non-zero
		delta_f_cent[k] = (delta_f_back && delta_f_fwd)
                   ? (0.5 * (delta_f_back + delta_f_fwd))
									 : (!delta_f_back ? delta_f_fwd : delta_f_back);

		}
		
		propagate_phase(delta_t_back, delta_f_cent, mag, magPrev, phi_s, tol, bufLen);

		for(uint16_t k = 0; k < bufLen; k++)
		{
			phi_s[k] = phi_s[k] + hopS * delta_t_back[k];
			expc(&input[k], mag[k], phi_s[k]);
		}
}

void propagate_phase(float* delta_t, float* delta_f,float* mag, float* magPrev,float* phi_s, float tol, int bufLen)
{
	float maxMag  = get_max(mag, bufLen);
	float maxPrev = get_max(mag, bufLen);
	float abstol = tol * ((maxMag >= maxPrev) ? (maxMag) : (maxPrev));

	std::vector<uint16_t> set_I;
	set_I.reserve(bufLen);
	std::srand(1); // Don't know if the SoC knows about time so seed with 1

	for (uint16_t k = 0; k < bufLen; k++)
	{
		if (mag[k] > abstol) { set_I.push_back(k); }
		else
		{
			phi_s[k] = (std::rand()/RAND_MAX) * 2 * PI - PI;
		}
	}
	// auto cmpFunc = [&phi_s](auto &a, auto &b){ return phi_s[a] < phi_s[b]; };
}
bool cmp_in_heap(const uint16_t &a, const uint16_t &b, std::vector<uint16_t> &vec)
{
	return vec[a] < vec[b];
}

float get_max(float* in, int size)
{
	float max = 0;
	for (int k = 0; k < size; k++)
	{
		if (in[k] > max) { max = in[k]; }
	}
	return max;
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
