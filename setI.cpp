
#include "setI.h"
#include <algorithm>

setI::setI(const float* mag, const float* magPrev, float* phi_s, const int size)
					 : mag(mag)
					 , magPrev(magPrev)
					 , phi_s(phi_s)
{
	tuple.reserve(size);
}

void setI::sort()
{
	// auto cmpFunc = [&](auto &a, auto &b){ return this->phi_s[a] < this->phi_s[b]; };
	// std::make_heap(tuple.begin(), tuple.end(), &setI::comp);
}

bool setI::comp(uint16_t &a, uint16_t &b)
{
	return phi_s[a] < phi_s[b];
}
