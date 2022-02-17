
#include "CQPVEngine.h"
#include "ConstantQ.h"
#include "CQInverse.h"


CQPVEngine::~CQPVEngine()
{
}

CQPVEngine::CQPVEngine(uint32_t steps, uint32_t buflen, uint32_t hopA, uint32_t sampleRate, my_float magTol) :
	PVDREngine(steps, buflen, hopA, magTol)
  , minFreq_(40)
  , maxFreq_(1000)
  , bpo_(64)
  , sampleRate_(sampleRate)
{
	algorithmName_ = "Constant Q Phase Vocoder";
	CQParameters params(sampleRate_, minFreq_, maxFreq_, bpo_);
	ConstantQ cq(params);
	CQInverse cqi(params);

	std::vector<my_float> cqin;
	cqin.reserve(buflen_);
}

