#include "PVDREngine.h"
#include "Tuple.h"
#include "logger.h"
#include "DumperContainer.h"
#include <set>
#include <vector>
#include <random>
#include <queue>
#include <complex>
#include <algorithm>

PVDREngine::~PVDREngine()
{
}

PVDREngine::PVDREngine(buffer_data_t* bf, audio_data_t* audat) :
	PVEngine(bf, audat)
{
}

void PVDREngine::propagatePhase()
{
	std::set<uint16_t> setI;
	std::vector<Tuple> container;
	container.reserve(bf_->buflen);
	TupleCompareObject cmp(bf_->mag, bf_->magPrev);
	std::priority_queue<Tuple, std::vector<Tuple>, TupleCompareObject<my_float> > h{cmp, std::move(container)}; // STEP 4

	my_float b_s = b_a * bf_->shift;

	// STEP 1
	my_float tol = MAGNITUDE_TOLERANCE;
	my_float maxMag     = *std::max_element(bf_->mag, bf_->mag + bf_->buflen);
	my_float abstol  = tol * ((maxMag >= bf_->maxMagPrev) ? (maxMag) : (bf_->maxMagPrev));
	bf_->maxMagPrev = maxMag;

	for (uint16_t m = 0; m < bf_->buflen; m++)
	{
		if (bf_->mag[m] > abstol)
		{ 
			setI.insert(m); // STEP 2
			h.push(Tuple(m, 0)); // STEP 5
		}
		else // STEP 3
		{
			bf_->phi_s[m] = (std::rand()/(my_float)RAND_MAX) * 2 * PI - PI;
		}
	}

	while(!setI.empty()) // STEP 6
	{
		// STEP 7
		Tuple current = h.top();
		h.pop();

		if (current.n == 0) // STEP 8
		{
			if(setI.count(current.m)) // STEP 9
			{
				bf_->phi_s[current.m] = bf_->phi_sPrev[current.m] + (bf_->hopS/2)*(bf_->delta_tPrev[current.m] + bf_->delta_t[current.m]); // STEP 10
				setI.erase(current.m); // STEP 11
				h.push(Tuple(current.m, 1)); // STEP 12
			}
		}
		if (current.n == 1) // STEP 15
		{
			if (setI.count(current.m + 1)) // STEP 16
			{
				bf_->phi_s[current.m + 1] = bf_->phi_s[current.m] + (b_s/2) * (bf_->delta_f[current.m] + bf_->delta_f[current.m + 1]); // STEP 17
				setI.erase(current.m + 1); // STEP 18
				h.push(Tuple(current.m + 1, 1)); // STEP 19
			}
			if (setI.count(current.m - 1)) // STEP 21
			{
				bf_->phi_s[current.m - 1] = bf_->phi_s[current.m] - (b_s/2) * (bf_->delta_f[current.m] + bf_->delta_f[current.m - 1]); // STEP 22
				setI.erase(current.m - 1); // STEP 23
				h.push(Tuple(current.m - 1, 1)); // STEP 24
			}
		}
	}

	//DUMP_ARRAY(bf_->delta_f, "delta_f.csv");
	//DUMP_ARRAY(bf_->delta_t, "delta_t.csv");

	return;
}

void PVDREngine::computeDifferenceStep()
{
	my_float phi_diff;
	const std::complex<my_float> i(0, 1);

	// Time differentiation variables.
	// Can't do forward differentiation because the algorithm is real time
	my_float deltaPhiPrime_t_back;
	my_float deltaPhiPrimeMod_t_back;

	// Frequency differentiation variables
	my_float delta_f_back;
	my_float delta_f_fwd;

	for(uint32_t k = 0; k < bf_->buflen; k++)
	{
		// Time differentiation
		phi_diff = bf_->phi_a[k] - bf_->phi_aPrev[k];
		deltaPhiPrime_t_back = phi_diff - ((my_float)bf_->hopA * 2 * PI * k)/bf_->buflen;
		deltaPhiPrimeMod_t_back = std::remainder(deltaPhiPrime_t_back, 2 * PI);
		bf_->delta_t[k] = deltaPhiPrimeMod_t_back/bf_->hopA + (2 * PI * k)/bf_->buflen;

		// Backward frequency differentiation
		if (k > 0)
		{
			phi_diff = bf_->phi_a[k] - bf_->phi_a[k - 1];
			delta_f_back = (1 / b_a) * std::remainder(phi_diff, 2 * PI);
		}
		else { delta_f_back = 0; }

		// Forward frequency differentiation
		if(k < (bf_->buflen - 1))
		{
			phi_diff = bf_->phi_a[k + 1] - bf_->phi_a[k]; 
			delta_f_fwd = (1 / b_a) * std::remainder(phi_diff, 2 * PI);
		}
		else { delta_f_fwd = 0; }

		// Take the average of fwd and back or whichever is non-zero
		bf_->delta_f[k] = (delta_f_back && delta_f_fwd)
                   ? (0.5f * (delta_f_back + delta_f_fwd))
			   	   : (!delta_f_back ? delta_f_fwd : delta_f_back);
	}
}

