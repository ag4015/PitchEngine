
#include <vector>
#include "main.h"

struct Tuple
{
	uint16_t m;
	uint16_t n;
	Tuple(uint16_t &&m, uint16_t &&n) : m(m), n(n) {};
};

class setI
{
private:
	const float* mag_;
	const float* magPrev_;
	float* phi_s_;
	float abstol_;
	std::vector<Tuple> tuple_;
	bool comp(uint16_t &a, uint16_t &b);
	float get_max(const float* in, const int &size);
public:
	setI(const float* mag, const float* magPrev, float* phi_s, const int size, float tol);
	void sort();
	void pop();
	const int size() { return tuple_.size(); }
};
