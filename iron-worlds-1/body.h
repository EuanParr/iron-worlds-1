#ifndef BODY_H_INCLUDED
#define BODY_H_INCLUDED

#include "matrix.h"
#include "renderer.h"
#include "rotation.h"

#include <cmath>

namespace body
{
    class Body
    {
    public:
        matrix::Matrix<double, 3, 1> displacement, velocity;
        rotation::Quaternion<double> angularDisplacementQuaternion, angularVelocityQuaternion;
        double radius = 1;
        renderer::Shape* myShape = nullptr;

        void integrateMotionStep(double deltaT);

        matrix::Matrix<double, 4> makeBasisMatrix();
    };
}

#endif // BODY_H_INCLUDED
