#include "platform.h"
#include "Win32_platform.h"

#include <windows.h>
#include <unordered_map>

namespace platform
{
    void sleepForMilliseconds(int time)
    {
        if (static_cast<int>(InputCode::A) == 0)
        Sleep(time);
    }

    std::unordered_map<unsigned int, InputCode> inputCodeMap =
    {
        {0x41, InputCode::A},
        {0x44, InputCode::D},
        {0x45, InputCode::E},
        {0x46, InputCode::F},
        {0x51, InputCode::Q},
        {0x52, InputCode::R},
        {0x53, InputCode::S},
        {0x57, InputCode::W},
        {VK_RIGHT, InputCode::RightArrow},
        {VK_LEFT, InputCode::LeftArrow},
        {VK_UP, InputCode::UpArrow},
        {VK_DOWN, InputCode::DownArrow},
        {VK_SPACE, InputCode::Space},
    };
}
