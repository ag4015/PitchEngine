#pragma once
#include "PVEngine.h"

// Phase Vocoder without phase adjustments
class StrechEngine : public PVEngine
{
public:
    virtual ~StrechEngine();
    StrechEngine(int steps, int buflen, int hopA);

protected:

    void processFrame() override;

};
