
#include "audioUtils.h"


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

#ifdef PDEBUG
void process_frame(float* mag, kiss_fft_cpx* cpxOut, float* phase, float* deltaPhi, float* previousPhase,
                   float* deltaPhiPrime, float* deltaPhiPrimeMod, float* trueFreq, float* phaseCumulative,
									 int hopA, int hopS, int bufLen)
{
		COPY(mag[k], absc(&cpxOut[k]), bufLen);
		COPY(phase[k], argc(&cpxOut[k]), bufLen);
		COPY(deltaPhi[k], phase[k] - previousPhase[k], bufLen);
		COPY(previousPhase[k], phase[k], bufLen);
		COPY(deltaPhiPrime[k], deltaPhi[k] - (hopA * 2 * PI * k)/bufLen, bufLen);
		COPY(deltaPhiPrimeMod[k], fmod(deltaPhiPrime[k] + PI , 2 * PI) - PI, bufLen);
		COPY(trueFreq[k], (2 * PI * k)/bufLen + deltaPhiPrimeMod[k]/hopA, bufLen);
		COPY(phaseCumulative[k], phaseCumulative[k] + hopS * trueFreq[k], bufLen);
		for (uint16_t k = 0; k < bufLen; k++)
		{
			expc(&cpxOut[k], mag[k], phaseCumulative[k]);
		}
}
#else
void process_frame(kiss_fft_cpx* input, float* mag, float* phase, float phaseCumulative,
                   int hopA, int hopS, int bufLen)
{
	float   current_phase;
	float		deltaPhi;
	float		deltaPhiPrime;
	float		deltaPhiPrimeMod;
	float   trueFreq;

	for(uint16_t k = 0; k < bufLen; k++)
	{
		mag[k] = absc(&input[k]);

		current_phase = argc(&input[k]);
		deltaPhi = current_phase - phase[k];
		phase[k] = current_phase;
		deltaPhiPrime = deltaPhi - (hopA * 2 * PI * k)/bufLen;
		deltaPhiPrimeMod = fmod(deltaPhiPrime + PI , 2 * PI) - PI;
		trueFreq = (2 * PI * k)/bufLen + deltaPhiPrimeMod/hopA;
		phaseCumulative = phaseCumulative + hopS * trueFreq;
		expc(&input[k], mag[k], phaseCumulative);
	}	
}
#endif


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
