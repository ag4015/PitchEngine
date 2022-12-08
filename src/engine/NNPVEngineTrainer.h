#pragma once

#include "PVEngine.h"

// Analysis and FFT stages of the Phase Vocoder
// Used to generate training data
class NNPVEngineTrainer : public PVEngine
{
public:
    virtual ~NNPVEngineTrainer();
    NNPVEngineTrainer(int steps, int buflen, int hopA);

protected:

    void process() override;
    void processFrame() override;

};

