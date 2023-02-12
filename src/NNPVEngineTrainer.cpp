
#include "NNPVEngineTrainer.h"
#include "VariableDumper.h"
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
	Timer processTimer("process", timeUnit::MILISECONDS);

	for (int f = 0; f < numFrames_; f++)
	{
		swapPingPongPV();

        /************ ANALYSIS STAGE ***********************/

		createFrame(inbuffer_, inframe_, outframe_, hopA_, frameNum_);

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
	Timer processFrameTimer("processFrame", timeUnit::MILISECONDS);

	for (int k = 0; k < buflen_; k++)
	{
		// Computation of magnitude and phase
		mag_[k] = std::abs(std::complex<my_float>{cpxOut_[k].r, cpxOut_[k].i});
		phi_a_[k] = std::arg(std::complex<my_float>{cpxOut_[k].r, cpxOut_[k].i});
	}
	
	 DUMP_VAR("mag",   mag_,   buflen_/2 + 1);
	 DUMP_VAR("phi_a", phi_a_, buflen_/2);

}

