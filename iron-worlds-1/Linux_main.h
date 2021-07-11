#ifndef LINUX_MAIN_H_INCLUDED
#define LINUX_MAIN_H_INCLUDED

#include "common_main.h"

namespace Linux_main
{
    class Linux_PlatformContext : public common_main::PlatformContext
    {
        static std::unordered_map<unsigned int, platform::InputCode> inputCodeMap;

    public:
        void checkEvents();
        void flushToScreen();
        std::unordered_map<unsigned int, platform::InputCode>& getInputCodeMapRef() {return inputCodeMap;}
        void logPlatform() {LOG("Linux");}
        void sleepForMilliseconds(int time);
    };
}

#endif // LINUX_MAIN_H_INCLUDED
