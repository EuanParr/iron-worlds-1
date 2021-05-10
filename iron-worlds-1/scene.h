#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include "body.h"
#include "renderer.h"

#include <list>

namespace scene
{
    class Perspective
    {
    public:
        matrix::Matrix<double, 3, 1> worldDisplacement, worldAngularDisplacement;

        matrix::Matrix<double, 4> getPerspectiveMatrix()
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

            matrix::smashMatrix(result, identity3);

            result *= matrix::makeTranslate(worldDisplacement.data[0], worldDisplacement.data[1], worldDisplacement.data[2]);

            return matrix::makeFrustum<double>() * result;
        }
    };

    class Scene
    {
    public:
        std::list<body::Body> bodies;
        std::list<Perspective> perspectives;

        Perspective* currentPerspective_PtrWeak = nullptr;

        void draw();
        void simulateStep(double deltaT);
    };
}

#endif // SCENE_H_INCLUDED
