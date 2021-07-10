
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