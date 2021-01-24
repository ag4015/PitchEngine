
#include "maxHeap.h"

// Update the TupleCompareObject with the updated mag pointers
void maxHeap::updateMagPointers(const float* mag, const float* magPrev)
{
    cmp_.mag_ = mag;
    cmp_.magPrev_ = magPrev;
}

// Erase (m,n-1) tuples and make (m,n) ones become (m, n-1)
void maxHeap::updateTuples()
{

}