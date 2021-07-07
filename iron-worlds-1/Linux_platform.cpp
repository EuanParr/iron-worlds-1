#include "platform.h"
#include "Linux_platform.h"

#include <chrono>
#include <thread>
#include <unordered_map>

namespace platform
{
    void sleepForMilliseconds(int time)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }

    std::unordered_map<unsigned int, InputCode> inputCodeMap =
    {
        {XK_A, InputCode::A},
        {XK_D, InputCode::D},
        {XK_E, InputCode::E},
        {XK_F, InputCode::F},
        {XK_Q, InputCode::Q},
        {XK_R, InputCode::R},
        {XK_S, InputCode::S},
        {XK_W, InputCode::W},
        {XK_Right, InputCode::RightArrow},
        {XK_Left, InputCode::LeftArrow},
        {XK_Up, InputCode::UpArrow},
        {XK_space, InputCode::Space},
    };
}
