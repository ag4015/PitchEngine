#pragma once
#include "PVDREngine.h"

// Constant Q Phase Vocoder Done Right
class CQPVEngine : public PVDREngine
{
public:
    virtual ~CQPVEngine();
	CQPVEngine(uint32_t steps, uint32_t buflen, uint32_t hopA, uint32_t sampleRate);
    //process() override;

protected:

	my_float minFreq_;
	my_float maxFreq_;
	uint8_t  bpo_;
	uint32_t sampleRate_;

};


