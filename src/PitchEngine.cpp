
#include "PitchEngine.h"

PitchEngine::PitchEngine(int buflen, uint32_t steps)
	: buflen_(buflen)
	, steps_(steps)
{
	alpha_ = POW(2, (steps_/ SEMITONES_PER_OCTAVE ));
	allocateMemoryPE();
}

PitchEngine::~PitchEngine()
{
	freeMemoryPE();
}

void PitchEngine::allocateMemoryPE()
{
	inbuffer_  = new my_float[buflen_]();
	outbuffer_ = new my_float[buflen_]();
}

void PitchEngine::freeMemoryPE()
{
	delete[] inbuffer_;
	delete[] outbuffer_;
}

