
#include "CQPVEngine.h"
#include "ConstantQ.h"
#include "CQInverse.h"


CQPVEngine::~CQPVEngine()
{
}

CQPVEngine::CQPVEngine(buffer_data_t* bf, audio_data_t* audat) :
	PVDREngine(bf, audat)
  , minFreq_(40)
  , maxFreq_(1000)
  , bpo_(64)
{
	CQParameters params(static_cast<my_float>(audat->sampleRate), minFreq_, maxFreq_, bpo_);
	ConstantQ cq(params);
	CQInverse cqi(params);

	std::vector<my_float> cqin;
	cqin.reserve(bf->buflen);
}
