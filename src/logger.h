#pragma once
#include "audioData.h"
#include <string>

#ifdef PDEBUG
       #define DUMP_ARRAY(a,b,c,d,e,f,g) dump_array(a,b,c,d,e,f,g)
       #define DUMP_ARRAY_COMPLEX(a,b,c,d,e,f,g) COPY(mag[k], a[k].r, b); dump_array(mag,b,c,d,e,f,g)
#else
       #define DUMP_ARRAY(a,b,c,d,e,f,g)
       #define DUMP_ARRAY_COMPLEX(a,b,c,d,e,f,g)
#endif

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

template<typename T>
void dump_array(T* buf, size_t size, std::string&& fileName, int count, int max, int auP, int auPMax);


#ifdef __cplusplus
extern "C"
{
#endif

void template_float_dump_array(float* buf, size_t size, const char* fileName, int count, int max, int auP, int auPMax);

#ifdef __cplusplus
} // extern C
#endif

