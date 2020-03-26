#ifndef UTILS_TIME_HPP
#define UTILS_TIME_HPP

#include <chrono>
#include <cstdint>

namespace utils {

inline uint64_t NowSec() 
{
    return std::chrono::duration_cast<std::chrono::seconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline uint64_t NowMilliSec() 
{
    return std::chrono::duration_cast<std::chrono::milliseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline uint64_t NowMicroSec() 
{
    return std::chrono::duration_cast<std::chrono::microseconds>
              (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

inline uint64_t NowNanoSec()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>
                  (std::chrono::high_resolution_clock::now().time_since_epoch()).count();

}

}


#endif