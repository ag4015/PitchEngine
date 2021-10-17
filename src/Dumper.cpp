#include "Dumper.h"

Dumper::Dumper(std::string fileName, uint32_t* audio_ptr, uint32_t dumpSize,
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
Dumper::Dumper(std::string& fileName)
  :	fileName_(fileName)
  , count_(0)
  , audioPtr_(nullptr)
  , bufferSize_(0)
  , dumpSize_(0)
  , maxCount_(0)
  , auPMax_(0)
  , outFile_(fileName_, std::ios::out)
{
}

Dumper::~Dumper()
{
	outFile_.close();
}


