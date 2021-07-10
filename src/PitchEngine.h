#pragma once
#include "DSPConfig.h"
#include "audioData.h"

class PitchEngine
{
public:
    virtual ~PitchEngine();
    virtual void process()                                 = 0;
    virtual void transform(cpx* input, cpx* output)        = 0;
    virtual void inverseTransform(cpx* input, cpx* output) = 0;
};

