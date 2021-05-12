#include "scene.h"

namespace scene
{
    matrix::Matrix<double, 4> Perspective::getPerspectiveMatrix()
    {
        matrix::Matrix<double, 4> result;

        result = worldAngularDisplacementQuaternion.getMatrix();

        result *= matrix::makeTranslate(worldDisplacement.data[0], worldDisplacement.data[1], worldDisplacement.data[2]);

        return matrix::makeFrustum<double>() * result;
    }


    void Scene::draw()
    {
        if (!currentPerspective_PtrWeak)
        {
            std::cerr << "No current Perspective\n";
            return;
        }
        glPushMatrix();
        glMultMatrixd(currentPerspective_PtrWeak->getPerspectiveMatrix().getRaw());
        for (body::Body& currentBody : bodies)
        {
            glPushMatrix();
            glMultMatrixd(currentBody.makeBasisMatrix().getRaw());
            currentBody.myShape->draw();
            glPopMatrix();
        }
        glPopMatrix();
    }

    void Scene::simulateStep(double deltaT)
    {
        for (body::Body& currentBody : bodies)
        {
            currentBody.integrateMotionStep(deltaT);
        }
    }
}
