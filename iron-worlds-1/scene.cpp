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

        double groundRadius = 100.0;
            double groundAlt = -10.0;
            glBegin(GL_TRIANGLE_FAN);
            glColor3d(0.0, 1.0, 1.0);
            glVertex4d(groundRadius, groundAlt, groundRadius, 1);
            glVertex4d(-groundRadius, groundAlt, groundRadius, 1);
            glVertex4d(-groundRadius, groundAlt, -groundRadius, 1);
            glVertex4d(groundRadius, groundAlt, -groundRadius, 1);
            glEnd();

        glMultMatrixd(matrix::makeScale(1000.0, 1000.0, 1000.0).getRaw());
        renderer::Sphere().draw();

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
