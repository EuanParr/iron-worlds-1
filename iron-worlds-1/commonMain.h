#ifndef TOP_PORTABILITY_BOOKEND_H_INCLUDED
#define TOP_PORTABILITY_BOOKEND_H_INCLUDED

#include "scene.h"

namespace commonMain
{
    class PlatformContext
    {
    public:
        virtual ~PlatformContext() {};

        virtual void logPlatform() = 0;
    };

    void doFrame(void (* flushToScreen)(),
                 scene::Scene& sceneRef,
                 PlatformContext& context);
    int main();

}

#endif // TOP_PORTABILITY_BOOKEND_H_INCLUDED
