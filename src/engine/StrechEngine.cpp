
#include "StrechEngine.h"
#include "DumperContainer.h"

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
	CREATE_TIMER("processFrame", timeUnit::MICROSECONDS);

	computeDifferenceStep();
	
	std::complex<my_float> z;

	for(int k = 0; k < buflen_; k++)
	{
		z = mag_[k] * std::exp(std::complex<my_float>{0.f, phi_a_[k]});
		cpxOut_[k].r = std::real(z);
		cpxOut_[k].i = std::imag(z);
	}

	DUMP_ARRAY(mag_      , "mag.csv");
	DUMP_ARRAY(phi_a_    , "phi_a.csv");

}

