/* convinient functions */
#ifndef _DSA_TOOLS_H_
#define _DSA_TOOLS_H_ 1

// ignore warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wconversion"
#include <boost/format.hpp>
#pragma clang diagnostic pop

#include <iostream>

/* debug only */
#ifndef NDEBUG
#define LOG(fmt, ...) std::cerr << boost::format(fmt "\n") __VA_ARGS__
#define LOGN(fmt, ...) std::cerr << boost::format(fmt) __VA_ARGS__

#else
#define LOG(...)
#define LOGN(...)
#endif
/* end debug only */


#define FOR(type, i, start, end) for (type i = (start); i < (end); i++)

#endif
