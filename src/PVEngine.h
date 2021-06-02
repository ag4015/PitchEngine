
#include "PitchEngine.h"
#include "audioData.h"

// Classical phase vocoder
class PVEngine : public PitchEngine
{
public:
    virtual ~PVEngine();
    PVEngine(buffer_data_t* bf, audio_data_t* audat);
    void process() override;

private:
    buffer_data_t* bf_;
    audio_data_t* audat_;
    uint8_t frameNum_;
    uint32_t audio_ptr_;
    uint32_t* vTimeIdx_;
    uint32_t cleanIdx_;
    my_float pOutBuffLastSample_;
    my_float inWinScale_;
    my_float outWinScale_;
};
