#ifndef BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED
#define BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED

#include <iostream>

#define LOG(x) std::clog << x << std::endl
#define ELOG(x) std::cerr << "Error in " << __FILE__ << " at line " << __LINE__ << ": " << x << std::endl
#define FELOG(x) ELOG(x); exit(1)

#ifndef PLATFORM
#warning "No platform"
#endif // PLATFORM


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// compiling on windows
#ifdef _WIN64
// 64bit windows
#else
// 32bit windows
#define PLATFORM_WIN32
#endif // _WIN64

#elif __linux__
#define PLATFORM_FEDORA

#else
#error "unknown platform"
#endif // defined


#ifdef PLATFORM_FEDORA
#include "Fedora_foundation.h"
#endif // PLATFORM_FEDORA

#ifdef PLATFORM_WIN32
#include "Win32_foundation.h"
#endif // PLATFORM_WIN32

namespace bottom_portability_bookend
{

}

#endif // BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED
