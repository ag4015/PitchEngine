#pragma once
#include "Timer.h"
#include <stdint.h>
#include <string>
#include <fstream>
#include <unordered_map>

class Dumper
{
public:
    const std::string fileName_;
    uint32_t count_;
    const uint32_t* audioPtr_;
    const uint32_t bufferSize_;
    const uint32_t dumpSize_;
    const uint32_t maxCount_;
    const uint32_t auPMax_;
    std::ofstream outFile_;

public:
    Dumper(std::string fileName, uint32_t* audio_ptr, uint32_t dumpSize,
        uint32_t bufferSize, uint32_t maxCount, uint32_t auPMax);
    Dumper(std::string& fileName);
    Dumper(const Dumper& other);
    ~Dumper();
	// Default dumper
	template<typename T>
	void dump(T buf)
	{
		if (!audioPtr_) { return; }
		using remove_pointer_t = typename std::remove_pointer<T>::type;

		static_assert(std::is_pointer<T>::value);
		static_assert(std::is_floating_point<remove_pointer_t>::value || isComplex<remove_pointer_t>::value);

		if ((count_ > maxCount_ && maxCount_ != -1) ||
			(*audioPtr_ != auPMax_ && auPMax_ != -1)) { return; }
		
		// Store in a format understandable by numpy in python
		for (uint32_t i = 0; i < bufferSize_; i++) {
			if constexpr (std::is_floating_point<remove_pointer_t>::value) {
				outFile_ << buf[i] << ";";
			}
			else {
				outFile_ << buf[i].r << ";";
			}
		}
		outFile_ << std::endl;
		count_++;
	}
	// Timer dumper
	void dump(std::unordered_map<std::string, std::tuple<int, uint64_t, timeUnit> >& timerMap)
	{
		for (auto& [timerName, timingData] : timerMap)
		{
			outFile_ << timerName << " ";
			switch(std::get<2>(timingData))
			{
				case timeUnit::MICROSECONDS:
				{
					outFile_ << "(us);";
					break;
				}
				case timeUnit::MILISECONDS:
				{
					outFile_ << "(ms);";
					break;
				}
				case timeUnit::SECONDS:
				{
					outFile_ << "(s);";
					break;
				}
			}

		}
		outFile_ << std::endl;
		for (auto& [timerName, timingData] : timerMap)
		{
			outFile_ << std::to_string(std::get<1>(timingData)/std::get<0>(timingData)) << "; ";
		}
		outFile_ << std::endl;
	}
};

template <typename T>
class isComplex
{
private:
	typedef char YesType[1];
	typedef char NoType[2];

	template <typename C> static YesType& test(decltype(&C::r));
	template <typename C> static NoType& test(...);

public:
	enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};
