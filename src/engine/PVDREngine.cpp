
#include "PVDREngine.h"
#include "logger.h"
#include "DumperContainer.h"
#include <set>
#include <vector>
#include <random>
#include <queue>
#include <complex>
#include <algorithm>

struct Tuple
{
	uint16_t m;
	uint16_t n;
	Tuple(uint16_t m, uint16_t n) : m(m), n(n) {};
};

// Function object
template<typename T>
struct TupleCompareObject
{
	const T *mag_, *magPrev_;
	TupleCompareObject(const T* mag, const T* magPrev) : mag_(mag), magPrev_(magPrev) {};
	inline bool operator()(const Tuple& a, const Tuple& b)
	{
		const T* lhs = (a.n ? mag_ : magPrev_);
		const T* rhs = (b.n ? mag_ : magPrev_);
		if(lhs[a.m] == rhs[b.m])
			return a.n < b.n;
		return lhs[a.m] < rhs[b.m];
	}
};

PVDREngine::PVDREngine(int steps, int buflen, int hopA)
	: PVEngine(steps, buflen, hopA)
	, maxMagPrev_(0)
{
	allocateMemoryPVDR();
}

PVDREngine::~PVDREngine()
{
	freeMemoryPVDR();
}

void PVDREngine::propagatePhase()
{
	std::set<uint16_t> setI;
	std::vector<Tuple> container;
	container.reserve(buflen_);
	TupleCompareObject cmp(mag_, magPrev_);
	std::priority_queue<Tuple, std::vector<Tuple>, TupleCompareObject<my_float> > h{cmp, std::move(container)}; // STEP 4

	my_float b_s = b_a_ * shift_;

	// STEP 1
	my_float tol = MAGNITUDE_TOLERANCE;
	my_float maxMag     = *std::max_element(mag_, mag_ + buflen_);
	my_float abstol  = tol * ((maxMag >= maxMagPrev_) ? (maxMag) : (maxMagPrev_));
	maxMagPrev_ = maxMag;

	for (uint16_t m = 0; m < buflen_; m++)
	{
		if (mag_[m] > abstol)
		{ 
			setI.insert(m); // STEP 2
			h.push(Tuple(m, 0)); // STEP 5
		}
		else // STEP 3
		{
			phi_s_[m] = (std::rand()/(my_float)RAND_MAX) * 2 * PI - PI;
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
				phi_s_[current.m] = phi_sPrev_[current.m] + (hopS_/2)*(delta_tPrev_[current.m] + delta_t_[current.m]); // STEP 10
				setI.erase(current.m); // STEP 11
				h.push(Tuple(current.m, 1)); // STEP 12
			}
		}
		if (current.n == 1) // STEP 15
		{
			if (setI.count(current.m + 1)) // STEP 16
			{
				phi_s_[current.m + 1] = phi_s_[current.m] + (b_s/2) * (delta_f_[current.m] + delta_f_[current.m + 1]); // STEP 17
				setI.erase(current.m + 1); // STEP 18
				h.push(Tuple(current.m + 1, 1)); // STEP 19
			}
			if (setI.count(current.m - 1)) // STEP 21
			{
				phi_s_[current.m - 1] = phi_s_[current.m] - (b_s/2) * (delta_f_[current.m] + delta_f_[current.m - 1]); // STEP 22
				setI.erase(current.m - 1); // STEP 23
				h.push(Tuple(current.m - 1, 1)); // STEP 24
			}
		}
	}

	DUMP_ARRAY(delta_f_, "delta_f.csv");
	DUMP_ARRAY(delta_t_, "delta_t.csv");

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

	for(int k = 0; k < buflen_; k++)
	{
		// Computation of magnitude and phase
		mag_[k] = std::abs(std::complex<my_float>{cpxOut_[k].r, cpxOut_[k].i});
		my_float phi_aPrev_ = phi_a_[k];
		phi_a_[k] = std::arg(std::complex<my_float>{cpxOut_[k].r, cpxOut_[k].i});

		// Time differentiation
		phi_diff = phi_a_[k] - phi_aPrev_;
		deltaPhiPrime_t_back = phi_diff - ((my_float)hopA_ * 2 * PI * k)/buflen_;
		deltaPhiPrimeMod_t_back = std::remainder(deltaPhiPrime_t_back, 2 * PI);
		delta_t_[k] = deltaPhiPrimeMod_t_back/hopA_ + (2 * PI * k)/buflen_;

		// Backward frequency differentiation
		if (k > 0)
		{
			phi_diff = phi_a_[k] - phi_a_[k - 1];
			delta_f_back = (1 / b_a_) * std::remainder(phi_diff, 2 * PI);
		}
		else { delta_f_back = 0; }

		// Forward frequency differentiation
		if(k < (buflen_ - 1))
		{
			phi_diff = phi_a_[k + 1] - phi_a_[k]; 
			delta_f_fwd = (1 / b_a_) * std::remainder(phi_diff, 2 * PI);
		}
		else { delta_f_fwd = 0; }

		// Take the average of fwd and back or whichever is non-zero
		delta_f_[k] = (delta_f_back && delta_f_fwd)
                   ? (0.5f * (delta_f_back + delta_f_fwd))
			   	   : (!delta_f_back ? delta_f_fwd : delta_f_back);
	}
}

void PVDREngine::resetData()
{
	for (int k = 0; k < buflen_; k++)
	{
		cpxIn_[k].r     = 0;
		cpxIn_[k].i     = 0;
		cpxOut_[k].r    = 0;
		cpxOut_[k].i    = 0;
		mag_[k]         = 0;
		magPrev_[k]     = 0;
		phi_a_[k]       = 0;
		phi_s_[k]       = 0;
		phi_sPrev_[k]   = 0;
		delta_t_[k]     = 0;
		delta_tPrev_[k] = 0;
		delta_f_[k]     = 0;
	}
}

void PVDREngine::allocateMemoryPVDR()
{
	delta_f_ = new my_float[buflen_]();
}

void PVDREngine::freeMemoryPVDR()
{
	delete[] delta_f_;
}

