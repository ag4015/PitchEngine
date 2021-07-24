#pragma once
#include <stdint.h>
#include <string>
#include <fstream>

class Dumper
{
public:
    const std::string fileName_;
    uint32_t count_;
    const uint32_t& audioPtr_;
    const uint32_t bufferSize_;
    const uint32_t dumpSize_;
    const uint32_t maxCount_;
    const uint32_t auPMax_;
    std::ofstream outFile_;

public:
    Dumper(std::string fileName, uint32_t& audio_ptr, uint32_t dumpSize,
        uint32_t bufferSize, uint32_t maxCount, uint32_t auPMax);
    Dumper(const Dumper& other);
    //Dumper& operator=(const Dumper&);
    //Dumper(Dumper&&) noexcept = default;
    //Dumper &operator=(const Dumper&) = default;
    ~Dumper();
	template<typename T>
    void dump(T* buf);
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
