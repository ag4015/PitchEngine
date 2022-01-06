#pragma once
#include "PVEngine.h"

#define MAGNITUDE_TOLERANCE static_cast<my_float>(1e-4)

// Phase Vocoder Done Right
class PVDREngine : public PVEngine
{
public:
    virtual ~PVDREngine();
    PVDREngine(int steps, int buflen, int hopA, my_float magTol);
    void propagatePhase() override;
    void computeDifferenceStep() override;
    void resetData() override;

protected:
    
    virtual void allocateMemoryPVDR();
    virtual void freeMemoryPVDR();

	my_float b_a_ = 1; // b_a/b_s = shift; The value of b_a has no effect on the result
    my_float maxMagPrev_;
    my_float magTol_;

    my_float* delta_f_;

};

