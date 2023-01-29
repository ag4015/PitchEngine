
#include "StrechEngine.h"
#include "VariableDumper.h"

#include <complex>


StrechEngine::StrechEngine(int steps, int buflen, int hopA)
	: PVEngine(steps, buflen, hopA)
{
	algorithmName_ = "Strecher";
}

StrechEngine::~StrechEngine()
{
}

void StrechEngine::processFrame()
{
	// CREATE_TIMER("processFrame", timeUnit::MICROSECONDS);

	computeDifferenceStep();
	
	std::complex<my_float> z;

	for(int k = 0; k < buflen_; k++)
	{
		z = mag_[k] * std::exp(std::complex<my_float>{0.f, phi_a_[k]});
		cpxOut_[k].r = std::real(z);
		cpxOut_[k].i = std::imag(z);
	}
	 DUMP_VAR("mag",   mag_,   buflen_/2 + 1);
	 DUMP_VAR("phi_a", phi_a_, buflen_/2);
}

