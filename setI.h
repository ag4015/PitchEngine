
#include <vector>
#include "main.h"

struct Tuple
{
	uint16_t m;
	uint16_t n;
};

class setIIterator
{
};

class setI
{
public:
	setI(const float* mag, const float* magPrev, float* phi_s, const int size);
	void sort();
	void pop();
	// typedef setI::iterator iterator;
	// typedef std::vector<Point>::const_iterator const_iterator;
  //
	// iterator begin() { return m_shape.container.begin(); }
	// const_iterator begin() const { return m_shape.container.begin(); }
	// iterator end() { return m_shape.container.end(); }
	// const_iterator end() const { return m_shape.const_container.end(); }

private:
	std::vector<Tuple> tuple;
	const float* mag;
	const float* magPrev;
	float* phi_s;
	bool comp(uint16_t &a, uint16_t &b);
};
