#ifndef LINUX_MAIN_H_INCLUDED
#define LINUX_MAIN_H_INCLUDED

#include "commonMain.h"
#include "platform.h"

class Linux_PlatformContext : public commonMain::PlatformContext
{
public:
    void logPlatform() {LOG("Linux");};
};

#endif // LINUX_MAIN_H_INCLUDED
