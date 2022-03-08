
#include "PVEngine.h"
#include "logger.h"
#include "DumperContainer.h"
#include <utility>
#include <complex>
#include <algorithm>

#ifdef RESET_BUFFER
#define RESET_PV() if (++reset_counter == 256) { resetDataPV(); reset_counter = 0; }
#else
#define RESET_PV()
#endif

PVEngine::PVEngine(int steps, int buflen_, int hopA)
	: PitchEngine(buflen_, steps)
	, hopA_(hopA)
	, frameNum_(0)
	, vTimeIdx_(0)
	, pOutBuffLastSample_(0)
{
	algorithmName_ = "Phase Vocoder";

	hopS_      = static_cast<int>(ROUND(hopA_ * alpha_));
	numFrames_ = static_cast<int>(buflen_ / hopA_);
	vTimeSize_ = hopS_ * numFrames_ * 2;
	cleanIdx_ = hopS_ * numFrames_;

	allocateMemoryPV();

	initializeTransformer();

	// Initialize the magnitude pointers
	mag_ = mag_ping_;
	magPrev_ = mag_pong_;

	// Initialize the analysis phase pointers
	phi_s_ = phi_s_ping_;
	phi_sPrev_ = phi_s_pong_;

	// Initialize the synthesis phase pointers
	phi_s_ = phi_s_ping_;
	phi_sPrev_ = phi_s_pong_;

	// Initialize the time phase derivative pointers
	delta_t_     = delta_t_ping_;
	delta_tPrev_ = delta_t_pong_;

	inWinScale_  = SQRT(((buflen_ / hopA_) / 2));
	outWinScale_ = SQRT(((buflen_ / hopS_) / 2));

	// Initialize input and output window functions
	for(int k = 0; k < buflen_; k++)
	{
		inwin_[k]  = WINCONST * (1 - COS(2 * PI * k / buflen_));
		outwin_[k] = WINCONST * (1 - COS(2 * PI * k / buflen_));
	}
}

PVEngine::~PVEngine()
{
	freeMemoryPV();
}

void PVEngine::process()
{
	CREATE_TIMER("process", timeUnit::MILISECONDS);

	for (int f = 0; f < numFrames_; f++)
	{
		swapPingPongPV();

        /************ ANALYSIS STAGE ***********************/

		createFrame(inbuffer_, inframe_, outframe_, hopA_, frameNum_);
		DUMP_ARRAY(inframe_,  "inframe.csv");
		DUMP_ARRAY(outframe_, "outframe.csv");

		// TODO: Need to fix this for floats
		RESET_PV();

		for (int k = 0; k < buflen_; k++)
		{
			cpxIn_[k].r = (outframe_[k] * inwin_[k]) / inWinScale_;
			cpxIn_[k].i = 0;
		}

        /************ PROCESSING STAGE *********************/

		transform(cpxIn_, cpxOut_);

		processFrame();

		DUMP_ARRAY(inbuffer_, "inbuffer.csv");
		DUMP_ARRAY(inwin_   , "inwin.csv");
		DUMP_ARRAY(outwin_  , "outwin.csv");

		std::swap(cpxIn_, cpxOut_);

		inverseTransform(cpxIn_ , cpxOut_);

		DUMP_ARRAY(cpxOut_, "cpxOut.csv");

		for (int k = 0; k < buflen_; k++)
		{
			outframe_[k] = cpxOut_[k].r * (outwin_[k] / buflen_) / outWinScale_;
		}

		DUMP_ARRAY(outframe_, "outframe.csv");

        /************ SYNTHESIS STAGE ***********************/

		strechFrame(outframe_, vTime_);

		DUMP_ARRAY(vTime_, "vTime.csv");

		if ((++frameNum_) >= numFrames_) frameNum_ = 0;

	}

    /************* LINEAR INTERPOLATION *****************/

	interpolate(vTime_, outbuffer_);

    DUMP_ARRAY(outbuffer_, "outbuffer.csv");

	vTimeIdx_ += numFrames_ * hopS_;
	if ((vTimeIdx_) >= numFrames_ * hopS_ * 2) vTimeIdx_ = 0;

}

void PVEngine::processFrame()
{
	CREATE_TIMER("processFrame", timeUnit::MICROSECONDS);

	computeDifferenceStep();
	
	propagatePhase();

	std::complex<my_float> z;

	for(int k = 0; k < buflen_; k++)
	{
		z = mag_[k] * std::exp(std::complex<my_float>{0.f, phi_s_[k]});
		cpxOut_[k].r = std::real(z);
		cpxOut_[k].i = std::imag(z);
	}

	DUMP_ARRAY(mag_      , "mag.csv");
	DUMP_ARRAY(phi_a_    , "phi_a.csv");
	DUMP_ARRAY(phi_s_    , "phi_s.csv");

}

void PVEngine::computeDifferenceStep()
{
	my_float phi_diff;
	const std::complex<my_float> i(0, 1);

	// Time differentiation variables.
	// Can't do forward differentiation because the algorithm is real time
	my_float deltaPhiPrime_t_back;
	my_float deltaPhiPrimeMod_t_back;

	// Time differentiation
	for(int k = 0; k < buflen_; k++)
	{
		// Computation of magnitude and phase
		mag_[k] = std::abs(std::complex<my_float>{cpxOut_[k].r, cpxOut_[k].i});
		my_float phi_aPrev_ = phi_a_[k];
		phi_a_[k] = std::arg(std::complex<my_float>{cpxOut_[k].r, cpxOut_[k].i});

		// Time differentiation
		phi_diff = phi_a_[k] - phi_aPrev_;
		deltaPhiPrime_t_back = phi_diff - ((my_float)hopA_ * 2 * PI * k)/buflen_;
		deltaPhiPrimeMod_t_back = std::remainder(deltaPhiPrime_t_back, 2 * PI);
		delta_t_[k] = deltaPhiPrimeMod_t_back/hopA_ + (2 * PI * k)/buflen_;
	}

	DUMP_ARRAY(delta_t_, "delta_t.csv");

}

void PVEngine::propagatePhase()
{
	for (int k = 0; k < buflen_; k++)
	{
		phi_s_[k] = phi_sPrev_[k] + (hopS_ / 2) * (delta_t_[k] + delta_tPrev_[k]);
	}
}

void PVEngine::transform(cpx* input, cpx* output)
{
	DUMP_ARRAY(cpxIn_, "cpxIn.csv");
	CREATE_TIMER("fwd_fft", timeUnit::MICROSECONDS);
	kiss_fft(cfg_, input, output); // TODO Change to kiss_fftr, it's faster
	END_TIMER("fwd_fft");
	DUMP_ARRAY(cpxOut_, "cpxOut.csv");
}

void PVEngine::inverseTransform(cpx* input, cpx* output)
{
	CREATE_TIMER("inv_fft", timeUnit::MICROSECONDS);
	kiss_fft(cfgInv_, input, output);
}

void PVEngine::createFrame(my_float* input, my_float* frame, my_float* output, int hop, int frameNum)
{
	CREATE_TIMER("createFrame", timeUnit::MICROSECONDS);

	for (int k = 0; k < hop; k++)
	{
		frame[frameNum * hop + k] = input[frameNum * hop + k];
	}

	int frameNum2 = frameNum + 1;
	if (frameNum2 >= numFrames_)
	{
		frameNum2 = 0;
	}

	for (int f2 = 0; f2 < numFrames_; f2++)
	{
		for (int k = 0; k < hop; k++)
		{
			output[k + f2 * hop] = frame[frameNum2 * hop + k];
		}
		if (++frameNum2 >= numFrames_)
		{
			frameNum2 = 0;
		}
	}

	return;
}

void PVEngine::strechFrame(my_float* input, my_float* output)
{
	int outputSize = numFrames_ * hopS_ * 2;
	for (int k = 0; k < hopS_; k++)
	{
		output[cleanIdx_] = 0;
		if (++(cleanIdx_) >= outputSize) cleanIdx_ = 0;
	}

	// The indexing variable for output has to be circular.
	int t = vTimeIdx_ + hopS_ * frameNum_;
	if (t >= outputSize) t = 0;

	for (int k = 0; k < buflen_; k++)
	{
		output[t] += input[k];
		if ((++t) >= (outputSize)) t = 0;
	}
}

void PVEngine::interpolate(my_float* input, my_float* output)
{
	int k;
	// If interpolating one octave up, just take every two samples
	if (steps_ == 12)
	{
		for (k = 0; k < buflen_; k++)
		{
			output[k] = input[vTimeIdx_ + k * 2];
		}
	}
	else
	{
		my_float totalShift;
		int totalShiftInt;
		my_float upper;
		my_float lower = 0;
		int lowerIdx;
		int upperIdx;
		my_float delta_shift;
		for (k = vTimeIdx_; k < vTimeIdx_ + buflen_; k++)
		{
			totalShift = (k - vTimeIdx_) * alpha_;
			totalShiftInt = static_cast<int>(totalShift);

			lowerIdx = static_cast<int>(totalShift + vTimeIdx_);
			upperIdx = lowerIdx + 1;
			if (lowerIdx == 0)
			{
				lower = input[lowerIdx];
				upper = input[upperIdx + 1];
				output[k - vTimeIdx_ + 1] = lower * (1 - (totalShift - totalShiftInt)) + upper * (totalShift - totalShiftInt);
				delta_shift = (output[k - vTimeIdx_ + 1] - pOutBuffLastSample_) / 2;
				output[k - vTimeIdx_] = output[k - vTimeIdx_ + 1] - delta_shift;
			}
			if (upperIdx == 2*hopS_*numFrames_)
			{
				delta_shift = output[k - vTimeIdx_ - 1] - output[k - vTimeIdx_ - 2];
				output[k - vTimeIdx_] = delta_shift + output[k - vTimeIdx_ - 1];
				continue;
			}
			if (upperIdx == 2*hopS_*numFrames_ + 1 && lowerIdx == 2*hopS_*numFrames_)
			{
				delta_shift = (alpha_*(lower - output[k - vTimeIdx_ - 1]))/(lowerIdx - totalShift + alpha_);
				output[k - vTimeIdx_] = delta_shift + output[k - vTimeIdx_ - 1];
				continue;
			}
			lower = input[lowerIdx];
			upper = input[upperIdx];
			output[k - vTimeIdx_] = lower * (1 - (totalShift - totalShiftInt)) + upper * (totalShift - totalShiftInt);
		}
		pOutBuffLastSample_ = output[k - vTimeIdx_ - 1];
	}

}

void PVEngine::swapPingPongPV()
{
	// Update magnitude pointers
	magPrev_ = mag_;
	mag_ = (mag_ == mag_ping_) ? mag_pong_ : mag_ping_;

	// Phase pointers are not updated
	phi_sPrev_ = phi_s_;
	phi_s_ = (phi_s_ == phi_s_ping_) ? phi_s_pong_ : phi_s_ping_;

	// Update time phase derivative pointers
	delta_tPrev_ = delta_t_;
	delta_t_ = (delta_t_ == delta_t_ping_) ? delta_t_pong_ : delta_t_ping_;
}

void PVEngine::resetData()
{
	for (int k = 0; k < buflen_; k++)
	{
		cpxIn_[k].r     = 0;
		cpxIn_[k].i     = 0;
		cpxOut_[k].r    = 0;
		cpxOut_[k].i    = 0;
		mag_[k]         = 0;
		magPrev_[k]     = 0;
		phi_a_[k]       = 0;
		phi_s_[k]       = 0;
		phi_sPrev_[k]   = 0;
		delta_t_[k]     = 0;
		delta_tPrev_[k] = 0;
	}
}

void PVEngine::initializeTransformer()
{
	cfg_    = kiss_fft_alloc(buflen_, 0, 0, 0);
	cfgInv_ = kiss_fft_alloc(buflen_, 1, 0, 0);
	cpxIn_  = (kiss_fft_cpx*) calloc(buflen_, sizeof(kiss_fft_cpx));
	cpxOut_ = (kiss_fft_cpx*) calloc(buflen_, sizeof(kiss_fft_cpx));
}

void PVEngine::destroyTransformer()
{
	kiss_fft_free(cfg_);
	kiss_fft_free(cfgInv_);
	kiss_fft_free(cpxIn_);
	kiss_fft_free(cpxOut_);
}

void PVEngine::allocateMemoryPV()
{
	phi_a_        = new my_float[buflen_]();
	vTime_        = new my_float[vTimeSize_]();
	inframe_      = new my_float[buflen_]();
	outframe_     = new my_float[buflen_]();
	inwin_        = new my_float[buflen_]();
	outwin_       = new my_float[buflen_]();
	phi_s_ping_   = new my_float[buflen_]();
	phi_s_pong_   = new my_float[buflen_]();
	delta_t_ping_ = new my_float[buflen_]();
	delta_t_pong_ = new my_float[buflen_]();
	mag_ping_     = new my_float[buflen_]();
	mag_pong_     = new my_float[buflen_]();
}

void PVEngine::freeMemoryPV()
{
	delete[] phi_a_;
	delete[] vTime_;
	delete[] inframe_;
	delete[] outframe_;
	delete[] inwin_;
	delete[] outwin_;
	delete[] phi_s_ping_;
	delete[] phi_s_pong_;
	delete[] delta_t_ping_;
	delete[] delta_t_pong_;
	delete[] mag_ping_;
	delete[] mag_pong_;
}

