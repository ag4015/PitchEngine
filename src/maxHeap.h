
#ifndef MAX_HEAP_H
#define MAX_HEAP_H

#include "main.h"
#include <queue>
#include "Tuple.h"

class maxHeap
{
private:
    typedef std::priority_queue<Tuple, std::vector<Tuple>, TupleCompareObject> maxHeap_t;
	maxHeap_t heap_;
    TupleCompareObject cmp_;
public:
    maxHeap_t* getHeap() { return &heap_; };
    void pop() { heap_.pop(); }
    void size() { heap_.size(); }
    void updateTuples(); // Erase (m,n-1) tuples and make (m,n) ones become (m, n-1)
    void updateMagPointers(const float* mag, const float* magPrev); // Update the TupleCompareObject with the updated mag pointers

}

#endif //MAX_HEAP_H
