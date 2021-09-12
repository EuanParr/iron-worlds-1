#ifndef TOP_PORTABILITY_BOOKEND_H_INCLUDED
#define TOP_PORTABILITY_BOOKEND_H_INCLUDED

#include "audio.h"
#include "input.h"
#include "platform.h"
#include "scene.h"

namespace common_main
{
    class PlatformContext
    {
        matrix::Matrix<float, 4> screenMatrix;

    public:
        input::InputHandler inputHandler;
        bool quit = false;
        std::string programName = "Iron Worlds";

        virtual ~PlatformContext() {};

        matrix::Matrix<float, 4>& getScreenMatrixRef() {return screenMatrix;}
        void updateButtonInput(unsigned int keyCode, bool newState);
        void updateViewPort(int newWidth, int newHeight);

        virtual void checkEvents() = 0;
        virtual void flushToScreen() = 0;
        virtual std::unordered_map<unsigned int, platform::InputCode>& getInputCodeMapRef() = 0;
        virtual void logPlatform() = 0;
        virtual void sleepForMilliseconds(int time) = 0;
        virtual void playSoundFromMemory(audio::PCMBuffer& buffer) = 0;
    };

    int main(PlatformContext& context);
}

#endif // TOP_PORTABILITY_BOOKEND_H_INCLUDED
