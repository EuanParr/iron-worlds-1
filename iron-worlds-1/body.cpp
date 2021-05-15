#include "body.h"

namespace body
{
    void Body::integrateMotionStep(double deltaT)
    {
        displacement += velocity * deltaT;
        angularDisplacementQuaternion = rotation::Quaternion<double>(angularVelocityQuaternion, deltaT * 0.5) * angularDisplacementQuaternion;
        //std::cout << angularDisplacementQuaternion;
        velocity.data[1] -= 0.1 * deltaT;
        if (displacement.data[1] < -10 && velocity.data[1] < 0 && true)
        {
            velocity.data[1] = - velocity.data[1];
        }
    }

    matrix::Matrix<double, 4> Body::makeBasisMatrix()
    {
        matrix::Matrix<double, 4> result;

        result = angularDisplacementQuaternion.getMatrix();

        //std::cout << result;

        result = matrix::makeTranslate(displacement.data[0], displacement.data[1], displacement.data[2]) * matrix::makeScale(radius, radius, radius) * result;

        return result;
    }
}
