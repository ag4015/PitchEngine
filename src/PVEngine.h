#pragma once
#include "PitchEngine.h"
#include "EasyTimer.h"

#ifdef RESET_BUFFER
#define RESET_PV() if (++reset_counter == 256) { resetDataPV(); reset_counter = 0; }
#else
#define RESET_PV()
#endif

// Classical phase vocoder
class PVEngine : public PitchEngine
{
public:
    virtual ~PVEngine();
    PVEngine(int steps, int buflen, int hopA);

    void process() override;

    virtual void resetData();

protected:

    void transform(cpx* input, cpx* output) override;
    void inverseTransform(cpx* input, cpx* output) override;

    virtual void allocateMemoryPV();
    virtual void freeMemoryPV();

    void swapPingPongPV();

    virtual void initializeTransformer();
    virtual void destroyTransformer();

    virtual void processFrame();
    virtual void propagatePhase();
    virtual void computeDifferenceStep();

    void createFrame(my_float* input, my_float* frame, my_float* output, int hop, int frameNum);
    void strechFrame(my_float* input, my_float* output);
    void interpolate(my_float* input, my_float* output);

    int hopA_;
    int hopS_;
    int numFrames_;            // Number of frames that overlap in a buffer. 75% overlap for 4 frames.
    int frameNum_;
    int vTimeIdx_;
    int cleanIdx_;             // Circular index where vTime is reset.
    int vTimeSize_;

    my_float pOutBuffLastSample_;

    my_float inWinScale_;
    my_float outWinScale_;

	kiss_fft_cfg cfg_;
	kiss_fft_cfg cfgInv_;

    my_float* phi_a_;
	my_float* phi_s_;
	my_float* phi_sPrev_;

    my_float* mag_;                                // Transform magnitude
    my_float* magPrev_;                            // Transform magnitude for previous buffer
    my_float* delta_t_;                            // Phase difference in time
    my_float* delta_tPrev_;                        // Phase difference in time for previous buffer
	my_float* vTime_;                              // Overlap-add signal
	my_float* inframe_     , *outframe_;           // Pointer to the current frame
	my_float* inwin_       , *outwin_;             // Input and output windows
    my_float* mag_ping_    , *mag_pong_;           // Frame magnitude ping-pong buffers
    my_float* phi_s_ping_  , *phi_s_pong_;         // Synthesis phase ping-pong buffers
    my_float* delta_t_ping_, *delta_t_pong_;       // Frame phase time derivative ping-pong buffers

    kiss_fft_cpx* cpxIn_;                          // Input of transform
    kiss_fft_cpx* cpxOut_;                         // Output of transform
};

