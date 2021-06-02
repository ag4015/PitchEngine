
#include "DSPConfig.h"

class PitchEngine
{
public:
    virtual ~PitchEngine();
    virtual void process() = 0;
};
