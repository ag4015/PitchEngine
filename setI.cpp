
#include "setI.h"
#include <algorithm>

setI::setI(const float* mag, const float* magPrev, float* phi_s, const int size, float tol)
			: mag_(mag)
			, magPrev_(magPrev)
			, phi_s_(phi_s)
{
	std::srand(magPrev_[(int)(size/2)]); // Initialize with a random seed
	tuple_.reserve(size);
	for (int k = 0; k < size; k++)
	{
		if (mag_[k] > abstol_) { 
			tuple_.push_back(Tuple(k,1)); // (n)
			tuple_.push_back(Tuple(k,0)); // (n - 1)
		}
		else { phi_s_[k] = (std::rand()/RAND_MAX) * 2 * PI - PI; }
	}
	float maxMag  = get_max(mag_, size);
	float maxPrev = get_max(mag_, size);
	abstol_ = tol * ((maxMag >= maxPrev) ? (maxMag) : (maxPrev));
}

void setI::sort()
{
	auto lessThanMagFunction = [this] (const Tuple &a, const Tuple &b) -> bool
	{
		// n = 1 : current magnitude, n = 0 : previous magnitude
		const float* lhs = (a.n ? this->mag_ : this->magPrev_);
		const float* rhs = (b.n ? this->mag_ : this->magPrev_);
		return lhs[a.m] < rhs[b.m];
	};
	std::make_heap(tuple_.begin(), tuple_.end(), lessThanMagFunction);
}

float setI::get_max(const float* in, const int &size)
{
	float max = 0;
	for (int k = 0; k < size; k++)
	{
		if (in[k] > max) { max = in[k]; }
	}
	return max;
}