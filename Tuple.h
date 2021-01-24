
#include "main.h"

struct Tuple
{
	uint16_t m;
	uint16_t n;
	Tuple(uint16_t m, uint16_t n) : m(m), n(n) {};
};

// Function object
struct TupleCompareObject
{
	const float *mag_, *magPrev_;
	TupleCompareObject(const float* mag, const float* magPrev) : mag_(mag), magPrev_(magPrev) {};
	inline bool operator()(const Tuple& a, const Tuple& b)
	{
		const float* lhs = (a.n ? mag_ : magPrev_);
		const float* rhs = (b.n ? mag_ : magPrev_);
		if(lhs[a.m] == rhs[b.m])
			return a.n < b.n;
		return lhs[a.m] < rhs[b.m];
	}
};