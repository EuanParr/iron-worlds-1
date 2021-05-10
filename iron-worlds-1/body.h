#ifndef BODY_H_INCLUDED
#define BODY_H_INCLUDED

#include "matrix.h"
#include "renderer.h"

#include <cmath>

namespace body
{
    class Body
    {
    public:
        matrix::Matrix<double, 3, 1> displacement, velocity, angularDisplacement, angularVelocity;
        double radius = 1;

        void integrateMotionStep(double deltaT)
        {
            displacement += velocity * deltaT;
            angularDisplacement += angularVelocity * deltaT;
        }

        matrix::Matrix<double, 4> makeBasisMatrix()
        {
            matrix::Matrix<double, 4> result;


            // to convert from angular displacement vector(3) to rotation
            // matrix(3), use formula
            // R = I + K * sin theta + K^2 * (1 - cos theta)
            // where K is cross product matrix of normalised angular
            // displacement vector, and theta is magnitude of vector

            matrix::Matrix<double, 3> identity3;
            double a, b, c;
            a = angularDisplacement.data[0];
            b = angularDisplacement.data[1];
            c = angularDisplacement.data[2];
            double theta = pow(a*a + b*b + c*c, 0.5);

            if (theta != 0)
            {
                matrix::Matrix<double, 3, 1> omega = angularDisplacement * (1.0f/theta);

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

            result = matrix::makeTranslate(displacement.data[0], displacement.data[1], displacement.data[2]) * matrix::makeScale(radius, radius, radius) * result;

            return result;
        }
    };
}

#endif // BODY_H_INCLUDED
