
#include "PVEngine.h"
#include "audioUtils.h"

PVEngine::~PVEngine()
{
}

extern "C" void swap_ping_pong_buffer_data(buffer_data_t* bf, audio_data_t* audat);
PVEngine::PVEngine(buffer_data_t* bf, audio_data_t* audat) :
    bf_(bf),
    audat_(audat)
{
	inWinScale_ = sqrt(((bf->buflen / bf->hopA) / 2));
	outWinScale_ = sqrt(((bf->buflen / bf->hopS) / 2));
}

void PVEngine::process()
{
	for (uint8_t f = 0; f < audat_->numFrames; f++)
	{
		swap_ping_pong_buffer_data(bf_, audat_);

        /************ ANALYSIS STAGE ***********************/

		// Using mag as output buffer, nothing to do with magnitude
		overlapAdd(audat_->inbuffer, audat_->inframe, audat_->outframe, bf_->hopA, frameNum_, audat_->numFrames);  

		// TODO: Need to fix this for floats
#ifdef RESET_BUFFER
		if (++reset_counter == 256)
		{
			reset_buffer_data_arrays(bf_);
			reset_counter = 0;
		}
#endif

		for (uint32_t k = 0; k < bf_->buflen; k++)
		{
			bf_->cpxIn[k].r = (audat_->outframe[k] * audat_->inwin[k]) / inWinScale_;
			bf_->cpxIn[k].i = 0;
		}

        /************ PROCESSING STAGE *********************/

		// DUMP_ARRAY_COMPLEX(bf_->cpxIn  , bf_->buflen, DEBUG_DIR "cpxIn.csv"  , counter_1,  5, sample_counter, -1);

		kiss_fft( bf_->cfg , bf_->cpxIn , bf_->cpxOut );

		process_frame(bf_, audat_);

		// DUMP_ARRAY_COMPLEX(bf_->cpxOut, bf_->buflen, DEBUG_DIR "cpxOut.csv"  , counter_1, 40, sample_counter , -1);
		// DUMP_ARRAY(audat_->inbuffer , bf_->buflen, DEBUG_DIR "inbuffer.csv", counter_1, -1, sample_counter , bf_->buflen);
		// DUMP_ARRAY(audat_->inwin    , bf_->buflen, DEBUG_DIR "inwin.csv"   , counter_1, -1, sample_counter , bf_->buflen);
		// DUMP_ARRAY(audat_->outwin   , bf_->buflen, DEBUG_DIR "outwin.csv"  , counter_1, -1, sample_counter , bf_->buflen);
		// DUMP_ARRAY(bf_->phi_a       , bf_->buflen, DEBUG_DIR "phi_a.csv"   , counter_1, -1, sample_counter , bf_->buflen);
		// DUMP_ARRAY(bf_->phi_s       , bf_->buflen, DEBUG_DIR "phi_s.csv"   , counter_1, -1, sample_counter , bf_->buflen);

		std::swap(bf_->cpxIn, bf_->cpxOut);

		kiss_fft( bf_->cfgInv , bf_->cpxIn, bf_->cpxOut);

		// DUMP_ARRAY(bf_->cpxIn       , bf_->buflen, DEBUG_DIR "cpxOut.csv"  , counter_1, 5, sample_counter, -1);

		for (uint32_t k = 0; k < bf_->buflen; k++)
		{
			audat_->outframe[k] = bf_->cpxOut[k].r * (audat_->outwin[k] / bf_->buflen) / outWinScale_;
		}

        /************ SYNTHESIS STAGE ***********************/

		strechFrame(audat_->vTime, audat_->outframe, &audat_->cleanIdx, bf_->hopS, frameNum_, *vTimeIdx_, audat_->numFrames * bf_->hopS * 2, bf_->buflen);

		// DUMP_ARRAY(audat_->vTime, audat_->numFrames*bf_->hopS*2, DEBUG_DIR "vTimeXXX.csv", counter_1, 40, sample_counter, -1);

		if ((++frameNum_) >= audat_->numFrames) frameNum_ = 0;

	}

    /************* LINEAR INTERPOLATION *****************/

	interpolate(bf_, audat_, *vTimeIdx_, &pOutBuffLastSample_);

	// DUMP_ARRAY(audat_->outbuffer, bf_->buflen, DEBUG_DIR "outXXX.csv", counter_2, 10, audio_ptr, -1);

	*vTimeIdx_ += audat_->numFrames * bf_->hopS;
	if ((*vTimeIdx_) >= audat_->numFrames * bf_->hopS * 2) *vTimeIdx_ = 0;

#ifdef DEBUG_DUMP
	counter_2++;
#endif

}
