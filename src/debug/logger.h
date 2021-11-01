#pragma once
#include <iostream>
#include <string>

#ifdef DEBUG_LOG
       #define PRINT_LOG(...)       printLog(__VA_ARGS__)
#else
       #define PRINT_LOG(...) 
#endif

inline void printLog() {
    std::cout << std::endl;
}

template<typename First, typename ... Strings>
inline void printLog(First arg, const Strings&... rest) {
    std::cout << arg << " ";
    printLog(rest...);
}

