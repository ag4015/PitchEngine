#pragma once
#include "PVEngine.h"

#define MAGNITUDE_TOLERANCE 1e-4

// Phase Vocoder Done Right
class PVDREngine : public PVEngine
{
public:
    virtual ~PVDREngine();
    PVDREngine(buffer_data_t* bf, audio_data_t* audat);
    void propagatePhase() override;
    void computeDifferenceStep() override;
private:

	my_float b_a = 1; // b_a/b_s = shift; The value of b_a has no effect on the result

};

