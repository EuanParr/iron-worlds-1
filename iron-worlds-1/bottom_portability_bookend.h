#ifndef BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED
#define BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED

#include <iostream>
#include <fstream>

#define LOG(x) std::clog << x << std::endl
#define ELOG(x) std::cerr << "Error in " << __FILE__ << " at line " << __LINE__ << ": " << x << std::endl

namespace bottom_portability_bookend
{
    extern std::ifstream standardLisp;
}

#endif // BOTTOM_PORTABILITY_BOOKEND_H_INCLUDED
