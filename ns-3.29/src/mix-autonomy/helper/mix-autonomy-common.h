#ifndef MIX_AUTONOMY_COMMON_H
#define MIX_AUTONOMY_COMMON_H

#include <iostream>

namespace ns3 {

#define LOG_UNCOND(x) std::cout <<x<< std::endl
#define LOG_UNCOND_FUNCTION(x) LOG_UNCOND(__FUNCTION__)
#define LOG_WHEN(verbose, x) if (verbose) std::cout << x << std::endl
#define LOG_WHEN_LEVEL(verbose, threshold, x) if (verbose > threshold) std::cout << x << std::endl

#define MY_ASSERT(condition, message) if (!(condition)) do { \
    LOG_UNCOND (message); \
    exit (1); \
} while (false)

}

#endif