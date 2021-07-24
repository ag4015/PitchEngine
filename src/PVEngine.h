#pragma once
#include "PitchEngine.h"
#include "audioData.h"

// Classical phase vocoder
class PVEngine : public PitchEngine
{
public:
    virtual ~PVEngine();
    PVEngine(buffer_data_t* bf, audio_data_t* audat);
    void process() override;
    void transform(cpx* input, cpx* output) override;
    void inverseTransform(cpx* input, cpx* output) override;
    virtual void processFrame();
    virtual void propagatePhase();
    virtual void computeDifferenceStep();
    void overlapAdd(my_float* input, my_float* frame, my_float* output, int hop, uint32_t frameNum);
    void strechFrame(my_float* input, my_float* output);
    void interpolate(my_float* input, my_float* output);

protected:
    buffer_data_t* bf_;
    audio_data_t* audat_;
    uint32_t frameNum_;
    uint32_t vTimeIdx_;
    uint32_t cleanIdx_;
    my_float pOutBuffLastSample_;
    my_float inWinScale_;
    my_float outWinScale_;
};
