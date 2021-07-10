
#include "PVEngine.h"
#include "audioUtils.h"
#include "logger.h"
#include <utility>
#include <complex>
#include <algorithm>

PVEngine::~PVEngine()
{
}

PVEngine::PVEngine(buffer_data_t* bf, audio_data_t* audat) :
    bf_(bf),
    audat_(audat)
{
	inWinScale_ = sqrt(((bf->buflen / bf->hopA) / 2));
	outWinScale_ = sqrt(((bf->buflen / bf->hopS) / 2));
}

void PVEngine::process()
{
	for (uint8_t f = 0; f < audat_->numFrames; f++)
	{
		swap_ping_pong_buffer_data(bf_, audat_);

        /************ ANALYSIS STAGE ***********************/

		// Using mag as output buffer, nothing to do with magnitude
		overlapAdd(audat_->inbuffer, audat_->inframe, audat_->outframe, bf_->hopA, frameNum_);  

		// TODO: Need to fix this for floats
#ifdef RESET_BUFFER
		if (++reset_counter == 256)
		{
			reset_buffer_data_arrays(bf_);
			reset_counter = 0;
		}
#endif

		for (uint32_t k = 0; k < bf_->buflen; k++)
		{
			bf_->cpxIn[k].r = (audat_->outframe[k] * audat_->inwin[k]) / inWinScale_;
			bf_->cpxIn[k].i = 0;
		}

        /************ PROCESSING STAGE *********************/

		// DUMP_ARRAY_COMPLEX(bf_->cpxIn  , bf_->buflen, DEBUG_DIR "cpxIn.csv"  , counter_1,  5, sample_counter, -1);

		transform(bf_->cpxIn , bf_->cpxOut);

		processFrame();

		// DUMP_ARRAY_COMPLEX(bf_->cpxOut, bf_->buflen, DEBUG_DIR "cpxOut.csv"  , counter_1, 40, sample_counter , -1);
		// DUMP_ARRAY(audat_->inbuffer , bf_->buflen, DEBUG_DIR "inbuffer.csv", counter_1, -1, sample_counter , bf_->buflen);
		// DUMP_ARRAY(audat_->inwin    , bf_->buflen, DEBUG_DIR "inwin.csv"   , counter_1, -1, sample_counter , bf_->buflen);
		// DUMP_ARRAY(audat_->outwin   , bf_->buflen, DEBUG_DIR "outwin.csv"  , counter_1, -1, sample_counter , bf_->buflen);
		// DUMP_ARRAY(bf_->phi_a       , bf_->buflen, DEBUG_DIR "phi_a.csv"   , counter_1, -1, sample_counter , bf_->buflen);
		// DUMP_ARRAY(bf_->phi_s       , bf_->buflen, DEBUG_DIR "phi_s.csv"   , counter_1, -1, sample_counter , bf_->buflen);

		std::swap(bf_->cpxIn, bf_->cpxOut);

		inverseTransform(bf_->cpxIn , bf_->cpxOut);

		// DUMP_ARRAY(bf_->cpxIn       , bf_->buflen, DEBUG_DIR "cpxOut.csv"  , counter_1, 5, sample_counter, -1);

		for (uint32_t k = 0; k < bf_->buflen; k++)
		{
			audat_->outframe[k] = bf_->cpxOut[k].r * (audat_->outwin[k] / bf_->buflen) / outWinScale_;
		}

        /************ SYNTHESIS STAGE ***********************/

		strechFrame(audat_->outframe, audat_->vTime);

		// DUMP_ARRAY(audat_->vTime, audat_->numFrames*bf_->hopS*2, DEBUG_DIR "vTimeXXX.csv", counter_1, 40, sample_counter, -1);

		if ((++frameNum_) >= audat_->numFrames) frameNum_ = 0;

	}

    /************* LINEAR INTERPOLATION *****************/

	interpolate(audat_->vTime, audat_->outbuffer);

	// DUMP_ARRAY(audat_->outbuffer, bf_->buflen, DEBUG_DIR "outXXX.csv", counter_2, 10, audio_ptr, -1);

	vTimeIdx_ += audat_->numFrames * bf_->hopS;
	if ((vTimeIdx_) >= audat_->numFrames * bf_->hopS * 2) vTimeIdx_ = 0;

#ifdef DEBUG_DUMP
	counter_2++;
#endif

}

void PVEngine::processFrame()
{
#ifdef DEBUG_DUMP
	static int count = 0;
#endif

	for(uint32_t k = 0; k < bf_->buflen; k++)
	{
		bf_->mag[k] = std::abs(std::complex<my_float>{bf_->cpxOut[k].r, bf_->cpxOut[k].i});
		bf_->phi_aPrev[k] = bf_->phi_a[k];
		bf_->phi_a[k] = std::arg(std::complex<my_float>{bf_->cpxOut[k].r, bf_->cpxOut[k].i});
	}

	computeDifferenceStep();
	
	propagatePhase();

	std::complex<my_float> z[BUFLEN];

	for(uint16_t k = 0; k < bf_->buflen; k++)
	{
		z[k] = bf_->mag[k] * std::exp(std::complex<my_float>{0.f, bf_->phi_s[k]});
		bf_->cpxOut[k].r = std::real(z[k]);
		bf_->cpxOut[k].i = std::imag(z[k]);
	}

	DUMP_ARRAY(bf_->mag      , bf_->buflen, DEBUG_DIR "mag.csv"       , count, -1, 1, -1);
	DUMP_ARRAY(bf_->phi_a    , bf_->buflen, DEBUG_DIR "phi_a.csv"     , count, -1, 1, -1);
	DUMP_ARRAY(bf_->phi_s    , bf_->buflen, DEBUG_DIR "phi_s.csv"     , count, -1, 1, -1);
	DUMP_ARRAY(bf_->phi_sPrev, bf_->buflen, DEBUG_DIR "phi_sPrev.csv" , count, -1, 1, -1);

#ifdef DEBUG_DUMP
	count++;
#endif
}

void PVEngine::propagatePhase()
{
	for (uint32_t k = 0; k < bf_->buflen; k++)
	{
		bf_->phi_s[k] = bf_->phi_sPrev[k] + (bf_->hopS / 2) * (bf_->delta_t[k] + bf_->delta_tPrev[k]);
	}
}

void PVEngine::computeDifferenceStep()
{
	my_float phi_diff;
	const std::complex<my_float> i(0, 1);

	// Time differentiation variables.
	// Can't do forward differentiation because the algorithm is real time
	my_float deltaPhiPrime_t_back[BUFLEN];
	my_float deltaPhiPrimeMod_t_back[BUFLEN];

	// Time differentiation
	for(uint32_t k = 0; k < bf_->buflen; k++)
	{
		phi_diff = bf_->phi_a[k] - bf_->phi_aPrev[k];
		deltaPhiPrime_t_back[k] = phi_diff - (bf_->hopA * 2 * PI * k)/bf_->buflen;
		deltaPhiPrimeMod_t_back[k] = std::remainder(deltaPhiPrime_t_back[k], 2 * PI);
		bf_->delta_t[k] = deltaPhiPrimeMod_t_back[k]/bf_->hopA + (2 * PI * k)/bf_->buflen;
	}
}

void PVEngine::transform(cpx* input, cpx* output)
{
	kiss_fft(bf_->cfg, input, output);
}

void PVEngine::inverseTransform(cpx* input, cpx* output)
{
	kiss_fft(bf_->cfgInv, input, output);
}

void PVEngine::overlapAdd(my_float* input, my_float* frame, my_float* output, int hop, uint8_t frameNum)
{
	for (int k = 0; k < hop; k++)
	{
		frame[frameNum * hop + k] = input[frameNum * hop + k];
	}
	int frameNum2 = frameNum + 1;
	if (frameNum2 >= audat_->numFrames) frameNum2 = 0;
	for (uint8_t f2 = 0; f2 < audat_->numFrames; f2++)
	{
		for (int k = 0; k < hop; k++)
		{
			output[k + f2 * hop] = frame[frameNum2 * hop + k];
		}
		if (++frameNum2 >= audat_->numFrames) frameNum2 = 0;
	}
}

void PVEngine::strechFrame(my_float* input, my_float* output)
{
	uint32_t outputSize = audat_->numFrames * bf_->hopS * 2;
	for (uint32_t k = 0; k < bf_->hopS; k++)
	{
		output[audat_->cleanIdx] = 0;
		if (++(audat_->cleanIdx) >= outputSize) audat_->cleanIdx = 0;
	}

	// The indexing variable for output has to be circular.
	uint32_t t = vTimeIdx_ + bf_->hopS * frameNum_;
	if (t >= outputSize) t = 0;

	for (uint32_t k = 0; k < bf_->buflen; k++)
	{
		output[t] += input[k];
		if ((++t) >= (outputSize)) t = 0;
	}
}

// TODO: Last sample of buffer can be calculated better
void PVEngine::interpolate(my_float* input, my_float* output)
{
	uint32_t k;
	if (bf_->steps == 12)
	{
		for (k = 0; k < bf_->buflen; k++)
		{
			output[k] = input[vTimeIdx_ + k * 2];
		}
	}
	else
	{
		my_float tShift;
		my_float upper;
		my_float lower;
		uint32_t lowerIdx;
		uint32_t upperIdx;
		my_float delta_shift;
		for (k = vTimeIdx_; k < vTimeIdx_ + bf_->buflen; k++)
		{
			tShift = (k - vTimeIdx_) * bf_->shift;

			lowerIdx = (uint32_t)(tShift + vTimeIdx_);
			upperIdx = lowerIdx + 1;
			if (lowerIdx == 0)
			{
				lower = input[lowerIdx + 1];
				upper = input[upperIdx + 1];
				output[k - vTimeIdx_ + 1] = lower * (1 - (tShift - (int)tShift)) + upper * (tShift - (int)tShift);
				delta_shift = (output[k - vTimeIdx_ + 1] - pOutBuffLastSample_) / 2;
				output[k - vTimeIdx_] = output[k - vTimeIdx_ + 1] - delta_shift;
			}
			if (upperIdx == 2*bf_->hopS*audat_->numFrames)
			{
				delta_shift = (bf_->shift*(lower - output[k - vTimeIdx_ - 1]))/(lowerIdx - tShift + bf_->shift);
				output[k - vTimeIdx_] = delta_shift + output[k - vTimeIdx_ - 1];
				continue;
			}
			if (upperIdx == 2*bf_->hopS*audat_->numFrames + 1 && lowerIdx == 2*bf_->hopS*audat_->numFrames)
			{
				delta_shift = (bf_->shift*(lower - output[k - vTimeIdx_ - 1]))/(lowerIdx - tShift + bf_->shift);
				output[k - vTimeIdx_] = delta_shift + output[k - vTimeIdx_ - 1];
				continue;
			}
			lower = input[lowerIdx];
			upper = input[upperIdx];
			output[k - vTimeIdx_] = lower * (1 - (tShift - (int)tShift)) + upper * (tShift - (int)tShift);
		}
		pOutBuffLastSample_ = output[k - vTimeIdx_ - 1];
	}

}

