#include "Dumper.h"

Dumper::Dumper(std::string fileName, uint32_t& audio_ptr, uint32_t dumpSize,
	uint32_t bufferSize, uint32_t maxCount, uint32_t auPMax)
  :	fileName_(fileName)
  , count_(0)
  , audioPtr_(audio_ptr)
  , bufferSize_(bufferSize)
  , dumpSize_(dumpSize)
  , maxCount_(maxCount)
  , auPMax_(auPMax)
  , outFile_(fileName_, std::ios::out)
{
}
Dumper::Dumper(const Dumper& other)
  :	fileName_(other.fileName_)
  , count_(other.count_)
  , audioPtr_(other.audioPtr_)
  , bufferSize_(other.bufferSize_)
  , dumpSize_(other.dumpSize_)
  , maxCount_(other.maxCount_)
  , auPMax_(other.auPMax_)
  , outFile_(other.fileName_, std::ios::out)
{
	
}

Dumper::~Dumper()
{
	outFile_.close();
}

template<typename T>
void Dumper::dump(T* buf)
{
	static_assert(std::is_floating_point<T>::value || isComplex<T>::value);

	if ((count_ > maxCount_ && maxCount_ != -1) ||
		(audioPtr_ != auPMax_ && auPMax_ != -1)) { return; }
	
	// Store in a format understandable by numpy in python
	for (size_t i = 0; i < bufferSize_; i++) {
		if constexpr (std::is_floating_point<T>::value) {
			outFile_ << buf[i] << ";";
		}
		else {
			outFile_ << buf[i].r << ";";
		}
	}
	outFile_ << std::endl;
	count_++;
}

