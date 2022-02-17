
#include "StrechEngine.h"
#include "DumperContainer.h"


StrechEngine::StrechEngine(int steps, int buflen, int hopA)
	: PVEngine(steps, buflen, hopA)
{
	algorithmName_ = "Strecher";
}

StrechEngine::~StrechEngine()
{
}

void StrechEngine::process()
{
	CREATE_TIMER("process", timeUnit::MILISECONDS);
	// TODO what happens to numFrames_ with different steps?
	for (int f = 0; f < numFrames_; f++)
	{
		createFrame(inbuffer_, inframe_, outframe_, hopA_, frameNum_);
		DUMP_ARRAY(inframe_,  "inframe.csv");
		DUMP_ARRAY(outframe_, "outframe.csv");

		strechFrame(outframe_, vTime_);
		DUMP_ARRAY(vTime_, "vTime.csv");

		if ((++frameNum_) >= numFrames_) frameNum_ = 0;
	}

	interpolate(vTime_, outbuffer_);
    DUMP_ARRAY(outbuffer_, "outbuffer.csv");

	vTimeIdx_ += numFrames_ * hopS_;
	if ((vTimeIdx_) >= numFrames_ * hopS_ * 2) vTimeIdx_ = 0;

}

