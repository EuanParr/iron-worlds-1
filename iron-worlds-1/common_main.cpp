#include "common_main.h"

#include "input.h"
#include "logic.h"
#include "scene.h"

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

    int main(scene::Scene& sceneRef,
                 PlatformContext& context)
    {
        double k = 0.1;

        double camSpeed = 0.5;
        double camAngularSpeed = 0.02;

        matrix::Matrix<double, 4, 1> xBasisMatrix;
        xBasisMatrix.data[0] = 1;
        matrix::Matrix<double, 4, 1> yBasisMatrix;
        yBasisMatrix.data[1] = 1;
        matrix::Matrix<double, 4, 1> zBasisMatrix;
        zBasisMatrix.data[2] = 1;

        matrix::Matrix<double, 3, 1> cameraMover;

        if (context.bindings.queryState(platform::InputCode::A))
            //LOG("A");
            cameraMover.smashMatrix(sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * xBasisMatrix);
        if (context.bindings.queryState(platform::InputCode::D))
            cameraMover.smashMatrix(sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * xBasisMatrix);
        if (context.bindings.queryState(platform::InputCode::F))
            cameraMover.smashMatrix(sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * yBasisMatrix);
        if (context.bindings.queryState(platform::InputCode::R))
            cameraMover.smashMatrix(sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * yBasisMatrix);
        if (context.bindings.queryState(platform::InputCode::S))
            cameraMover.smashMatrix(sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * zBasisMatrix);
        if (context.bindings.queryState(platform::InputCode::W))
            cameraMover.smashMatrix(sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * zBasisMatrix);

        sceneRef.currentPerspective_PtrWeak->worldDisplacement = sceneRef.currentPerspective_PtrWeak->worldDisplacement + cameraMover * camSpeed;

        if (context.bindings.queryState(platform::InputCode::UpArrow))
            sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitIQuaternion, -camAngularSpeed) * sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
        if (context.bindings.queryState(platform::InputCode::DownArrow))
            sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitIQuaternion, camAngularSpeed) * sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
        if (context.bindings.queryState(platform::InputCode::LeftArrow))
            sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitJQuaternion, -camAngularSpeed) * sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
        if (context.bindings.queryState(platform::InputCode::RightArrow))
            sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitJQuaternion, camAngularSpeed) * sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
        if (context.bindings.queryState(platform::InputCode::E))
            sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitKQuaternion, -camAngularSpeed) * sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
        if (context.bindings.queryState(platform::InputCode::Q))
            sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitKQuaternion, camAngularSpeed) * sceneRef.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
        if (context.bindings.queryState(platform::InputCode::Space))
        {
            for (body::Body& newBody : sceneRef.bodies)
            {
                newBody.angularVelocityQuaternion.data[0] = logic::unitRand() * k;
                newBody.angularVelocityQuaternion.data[1] = logic::unitRand() * k;
                newBody.angularVelocityQuaternion.data[2] = logic::unitRand() * k;
                newBody.angularVelocityQuaternion.normalise();
            }
        }

        // set the background colour, black
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // clear the buffer, revert colour and depth of each pixel
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //context.logPlatform();

        glPushMatrix();
        glMultMatrixf(context.getScreenMatrixRef().getRaw());

        sceneRef.draw();
        sceneRef.simulateStep(0.1);

        glPopMatrix();

        context.flushToScreen();

        context.sleepForMilliseconds(10);

        return 0;
    }
}
