#include "Dumper.h"

Dumper::Dumper(std::string fileName, int* audio_ptr, int dumpSize,
	int bufferSize, int maxCount, int auPMax)
  :	fileName_(fileName)
  , audioPtr_(audio_ptr)
  , dumpSize_(dumpSize)
  , bufferSize_(bufferSize)
  , maxCount_(maxCount)
  , auPMax_(auPMax)
  , count_(0)
  , outFile_(fileName_, std::ios::out)
{
}
Dumper::Dumper(const Dumper& other)
  :	fileName_(other.fileName_)
  , audioPtr_(other.audioPtr_)
  , dumpSize_(other.dumpSize_)
  , bufferSize_(other.bufferSize_)
  , maxCount_(other.maxCount_)
  , auPMax_(other.auPMax_)
  , count_(other.count_)
  , outFile_(other.fileName_, std::ios::out)
{
}
Dumper::Dumper(std::string fileName)
  :	fileName_(fileName)
  , audioPtr_(nullptr)
  , dumpSize_(0)
  , bufferSize_(0)
  , maxCount_(0)
  , auPMax_(0)
  , count_(0)
  , outFile_(fileName, std::ios::out)
{
}

Dumper::~Dumper()
{
	outFile_.close();
}


