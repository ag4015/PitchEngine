#pragma once
#include "PVDREngine.h"

// Constant Q Phase Vocoder Done Right
class CQPVEngine : public PVDREngine
{
public:
    virtual ~CQPVEngine();
    CQPVEngine(buffer_data_t* bf, audio_data_t* audat);
    //process() override;

protected:

	uint32_t minFreq_;
	uint32_t maxFreq_;
	uint8_t  bpo_;

};


