#ifndef BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED
#define BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED

#define LOG(x) std::clog << x << std::endl
#define ELOG(x) std::cerr << "Error in " << __FILE__ << " at line " << __LINE__ << ": " << x << std::endl
#define FELOG(x) ELOG(x); exit(1)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// compiling on windows
#ifdef _WIN64
// 64bit windows
#else
// 32bit windows
#define PLATFORM_WIN32
#define PLATFORM
#endif // _WIN64

#elif __linux__
#define PLATFORM_LINUX
#define PLATFORM

#else
#error "unknown platform"
#endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)


#ifdef PLATFORM_LINUX
#include "Linux_platform.h"
#endif // PLATFORM_LINUX

#ifdef PLATFORM_WIN32
#include "Win32_platform.h"
#endif // PLATFORM_WIN32

#ifdef DEBUG
#else
#define NDEBUG
#endif // DEBUG

#include <assert.h>
#include <fstream>
#include <iostream>
#include <unordered_map>

namespace platform
{
    extern std::ifstream standardLisp;

    enum class InputCode : unsigned int
    {
        A, D, E, F, Q, R, S, W, RightArrow, LeftArrow, UpArrow, DownArrow, Space, Escape
    };

    struct InputCodeHash
    {
        std::hash<unsigned int> hashFun;

        size_t operator()(const InputCode& code) const
        {
            return hashFun(static_cast<unsigned int>(code));
        }
    };

    extern std::unordered_map<unsigned int, InputCode> inputCodeMap;

    void sleepForMilliseconds(int time);
}

#endif // BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED
