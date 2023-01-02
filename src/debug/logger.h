#pragma once
#include <iostream>
#include <string>
#include <mutex>

#ifdef DEBUG_LOG
       #define PRINT_LOG(...)       printLog(__VA_ARGS__)
#else
       #define PRINT_LOG(...) 
#endif

std::mutex printLogMutex;

inline void printLog2() {
    std::cout << std::endl;
}

template<typename First, typename ... Strings>
inline void printLog2(First arg, const Strings&... rest) {
    std::cout << arg << " ";
    printLog2(rest...);
}

template<typename First, typename ... Strings>
inline void printLog(First arg, const Strings&... rest) {
	std::lock_guard<std::mutex> printLogLock(printLogMutex);
    std::cout << "Thread id: " << std::this_thread::get_id() << " ";
    printLog2(arg, rest...);
}

