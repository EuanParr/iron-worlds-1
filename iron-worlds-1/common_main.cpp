#include "common_main.h"

#include "input.h"
#include "lisp.h"
#include "logic.h"
#include "scene.h"

#include <thread>

namespace common_main
{
    void PlatformContext::updateButtonInput(unsigned int keyCode, bool newState)
    {
        platform::InputCode inputCode = getInputCodeMapRef()[keyCode];
        bindings.updateButton(inputCode, newState);
    }

    void PlatformContext::updateViewPort(int newWidth, int newHeight)
    {
        glViewport(0, 0, newWidth, newHeight);
        screenMatrix = matrix::makeScale((float)newHeight / (float)newWidth, 1.0f, 1.0f);
    }

    void lispThreadFunc()
    {
        lisp::VirtualMachine lispVM;
        //lispVM.print(lispVM.read(std::cin), std::cout);
        lisp::SymbolString s;
        while (s != "quit")
        {
            s = lispVM.readToken(std::cin);
            LOG(s);
        }
    }

    int main(PlatformContext& context)
    {
        scene::Scene testScene;

        scene::Perspective testPerspective;
        testPerspective.worldDisplacement.data[2] = 50;
        testPerspective.worldAngularDisplacementQuaternion.data[2] = 0.0;

        testScene.perspectives.push_back(testPerspective);
        testScene.currentPerspective_PtrWeak = &testScene.perspectives.front();

        context.bindings.bindings[platform::InputCode::Escape].setAction(new input::LambdaButtonAction([&] (bool down) {if (down) context.quit = true;}));

        // set the background colour, black
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        double k = 0.1;

        double camSpeed = 0.5;
        double camAngularSpeed = 0.02;

        matrix::Matrix<double, 4, 1> xBasisMatrix;
        xBasisMatrix.data[0] = 1;
        matrix::Matrix<double, 4, 1> yBasisMatrix;
        yBasisMatrix.data[1] = 1;
        matrix::Matrix<double, 4, 1> zBasisMatrix;
        zBasisMatrix.data[2] = 1;

        for (int i = 0; i < 100; i++)
        {
            body::Body newBody;
            newBody.velocity.data[0] = logic::unitRand();
            newBody.velocity.data[1] = logic::unitRand();
            newBody.velocity.data[2] = logic::unitRand();
            newBody.angularVelocityQuaternion.data[0] = logic::unitRand() * k;
            newBody.angularVelocityQuaternion.data[1] = logic::unitRand() * k;
            newBody.angularVelocityQuaternion.data[2] = logic::unitRand() * k;
            newBody.angularVelocityQuaternion.normalise();
            newBody.myShape = new renderer::Cube();
            testScene.bodies.push_back(newBody);
        }

        std::thread lispThread(lispThreadFunc);

        while (!context.quit)
        {
            context.checkEvents();

            matrix::Matrix<double, 3, 1> cameraMover;

            if (context.bindings.queryState(platform::InputCode::A))
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * xBasisMatrix);
            if (context.bindings.queryState(platform::InputCode::D))
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * xBasisMatrix);
            if (context.bindings.queryState(platform::InputCode::F))
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * yBasisMatrix);
            if (context.bindings.queryState(platform::InputCode::R))
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * yBasisMatrix);
            if (context.bindings.queryState(platform::InputCode::S))
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * zBasisMatrix);
            if (context.bindings.queryState(platform::InputCode::W))
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * zBasisMatrix);

            testScene.currentPerspective_PtrWeak->worldDisplacement = testScene.currentPerspective_PtrWeak->worldDisplacement + cameraMover * camSpeed;

            if (context.bindings.queryState(platform::InputCode::UpArrow))
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitIQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (context.bindings.queryState(platform::InputCode::DownArrow))
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitIQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (context.bindings.queryState(platform::InputCode::LeftArrow))
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitJQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (context.bindings.queryState(platform::InputCode::RightArrow))
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitJQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (context.bindings.queryState(platform::InputCode::E))
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitKQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (context.bindings.queryState(platform::InputCode::Q))
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitKQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (context.bindings.queryState(platform::InputCode::Space))
            {
                for (body::Body& newBody : testScene.bodies)
                {
                    newBody.angularVelocityQuaternion.data[0] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.data[1] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.data[2] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.normalise();
                }
            }

            // clear the buffer, revert colour and depth of each pixel
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //context.logPlatform();

            glPushMatrix();
            glMultMatrixf(context.getScreenMatrixRef().getRaw());

            testScene.draw();
            testScene.simulateStep(0.1);

            glPopMatrix();

            context.flushToScreen();

            context.sleepForMilliseconds(10);
        }

        lispThread.join();

        return 0;
    }
}
