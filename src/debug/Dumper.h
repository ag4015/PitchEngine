#pragma once
#include "Timer.h"
#include <fstream>
#include <unordered_map>

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

class Dumper
{
public:
    const std::string fileName_;
    const int* audioPtr_;
    const int dumpSize_;
    const int bufferSize_;
    const int maxCount_;
    const int auPMax_;
    int count_;
	std::string data_;
    std::ofstream outFile_;

public:
    Dumper(std::string fileName, int* audio_ptr, int dumpSize,
        int bufferSize, int maxCount, int auPMax);
    Dumper(std::string fileName);
	Dumper(Dumper& other) = delete;
    ~Dumper();
	// Default dumper
	template<typename T>
	void dump(T buf)
	{
		CREATE_TIMER("dumpVar", timeUnit::MILISECONDS);
		if (!audioPtr_) { return; }
		using remove_pointer_t = typename std::remove_pointer<T>::type;

		static_assert(std::is_pointer<T>::value);
		static_assert(std::is_floating_point<remove_pointer_t>::value || isComplex<remove_pointer_t>::value);

		if ((count_ > maxCount_ && maxCount_ != -1) ||
			(*audioPtr_ != auPMax_ && auPMax_ != -1)) { return; }
		
		// Store in a format understandable by numpy in python
		for (int i = 0; i < dumpSize_; i++)
		{
			std::stringstream ss;
			if constexpr (std::is_floating_point<remove_pointer_t>::value)
			{
				ss << buf[i] << ";";
			}
			else
			{
				ss << buf[i].r << ";";
			}
			data_ += std::move(ss.str());
		}
		data_ += "\n";
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
			(void)timerName;
			outFile_ << std::to_string(std::get<1>(timingData)/std::get<0>(timingData)) << "; ";
		}
		outFile_ << std::endl;
	}
};

