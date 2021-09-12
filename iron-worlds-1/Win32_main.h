#ifndef WIN32_MAIN_H_INCLUDED
#define WIN32_MAIN_H_INCLUDED

#include "common_main.h"

namespace Win32_main
{
    class Win32_PlatformContext : public common_main::PlatformContext
    {
        static std::unordered_map<unsigned int, platform::InputCode> inputCodeMap;

    public:
        void checkEvents();
        void flushToScreen();
        std::unordered_map<unsigned int, platform::InputCode>& getInputCodeMapRef() {return inputCodeMap;}
        void logPlatform() {LOG("Win32");}
        void sleepForMilliseconds(int time);
        void playSoundFromMemory(audio::PCMBuffer&);
    };
}

#endif // WIN32_MAIN_H_INCLUDED
