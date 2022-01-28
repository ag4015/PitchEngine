#pragma once
#include  "pe_common_defs.h"

class PitchEngine
{
public:
    virtual ~PitchEngine();
    PitchEngine(int buflen, uint32_t steps);
    virtual void process()                                 = 0;

    int buflen_;
    int steps_;
    my_float alpha_;
    my_float* inbuffer_;
    my_float* outbuffer_;

protected:

    virtual void transform(cpx* input, cpx* output)        = 0;
    virtual void inverseTransform(cpx* input, cpx* output) = 0;

    virtual void allocateMemoryPE();
    virtual void freeMemoryPE();

};

