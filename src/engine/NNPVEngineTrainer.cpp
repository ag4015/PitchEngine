
#include "NNPVEngineTrainer.h"
#include "DumperContainer.h"
#include <complex>

NNPVEngineTrainer::NNPVEngineTrainer(int steps, int buflen, int hopA)
	: PVEngine(steps, buflen, hopA)
{
	algorithmName_ = "NNPVEngineTrainer";
}

NNPVEngineTrainer::~NNPVEngineTrainer()
{
}

void NNPVEngineTrainer::process()
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

		if ((++frameNum_) >= numFrames_) frameNum_ = 0;

	}

}

void NNPVEngineTrainer::processFrame()
{
	CREATE_TIMER("processFrame", timeUnit::MICROSECONDS);

	for (int k = 0; k < buflen_; k++)
	{
		// Computation of magnitude and phase
		mag_[k] = std::abs(std::complex<my_float>{cpxOut_[k].r, cpxOut_[k].i});
		phi_a_[k] = std::arg(std::complex<my_float>{cpxOut_[k].r, cpxOut_[k].i});
	}
	
	DUMP_ARRAY(mag_      , "mag.csv");
	DUMP_ARRAY(phi_a_    , "phi_a.csv");

}

