
#include "CQPVEngine.h"
#include "ConstantQ.h"
#include "CQInverse.h"


CQPVEngine::~CQPVEngine()
{
}

CQPVEngine::CQPVEngine(buffer_data_t* bf, audio_data_t* audat) :
	PVDREngine(bf, audat)
{
	// Constant Q variable initialization
	uint32_t minFreq = 40;
	uint32_t maxFreq = 1000;
	uint8_t  bpo = 64;

	CQParameters params(audat->sampleRate, minFreq, maxFreq, bpo);
	ConstantQ cq(params);
	CQInverse cqi(params);

	std::vector<my_float> cqin;
	cqin.reserve(bf->buflen);
}
