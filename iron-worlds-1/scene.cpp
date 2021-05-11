#include "scene.h"

namespace scene
{
    matrix::Matrix<double, 4> Perspective::getPerspectiveMatrix()
    {
        matrix::Matrix<double, 4> result;


        // to convert from angular displacement vector(3) to rotation
        // matrix(3), use formula
        // R = I + K * sin theta + K^2 * (1 - cos theta)
        // where K is cross product matrix of normalised angular
        // displacement vector, and theta is magnitude of vector

        matrix::Matrix<double, 3> identity3;
        double a, b, c;
        a = worldAngularDisplacement.data[0];
        b = worldAngularDisplacement.data[1];
        c = worldAngularDisplacement.data[2];
        double theta = pow(a*a + b*b + c*c, 0.5);

        if (theta != 0)
        {
            matrix::Matrix<double, 3, 1> omega = worldAngularDisplacement * (1.0f/theta);

            matrix::Matrix<double, 3> K;
            double (&kCells)[9] = K.data;
            kCells[0] = 0.0;
            kCells[1] = omega.data[2];
            kCells[2] = -omega.data[1];
            kCells[3] = -omega.data[2];
            kCells[4] = 0.0;
            kCells[5] = omega.data[0];
            kCells[6] = omega.data[1];
            kCells[7] = -omega.data[0];
            kCells[8] = 0.0;

            identity3 += K * sin(theta) + K * K * (1 - cos(theta));
        }

        result.smashMatrix(identity3);

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
