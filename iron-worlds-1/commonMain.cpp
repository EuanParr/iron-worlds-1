#include "commonMain.h"

#include "input.h"
#include "scene.h"

namespace commonMain
{
    matrix::Matrix<float, 4> screenMatrix;

    void doFrame(void (* flushToScreen)(),
                 scene::Scene& sceneRef,
                 PlatformContext& context)
    {
        // set the background colour, black
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // clear the buffer, revert colour and depth of each pixel
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        context.logPlatform();

        glPushMatrix();
        glMultMatrixf(screenMatrix.getRaw());

        sceneRef.draw();
        sceneRef.simulateStep(0.1);

        glPopMatrix();

        flushToScreen();

        platform::sleepForMilliseconds(10);
    }

    int main()
    {
        bool quit = false;
        return 0;
        input::BindingSet inputBindings;

        while (!quit)
        {

        }
    }
}
