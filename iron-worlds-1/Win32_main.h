#ifndef WIN32_MAIN_H_INCLUDED
#define WIN32_MAIN_H_INCLUDED

#include "commonMain.h"
#include "platform.h"

class Win32_PlatformContext : public commonMain::PlatformContext
{
public:
    void logPlatform() {LOG("Win32");};
};

#endif // WIN32_MAIN_H_INCLUDED
