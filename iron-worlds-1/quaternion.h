#ifndef QUATERNION_H_INCLUDED
#define QUATERNION_H_INCLUDED

#include "matrix.h"

namespace quaternion
{
    template <typename T>
    class Quaternion : public matrix::Matrix<T, 4, 1>
    {
    public:
        Quaternion()
        {
            this->data[0] = 0;
            this->data[1] = 0;
            this->data[2] = 0;
            this->data[3] = 1;
        }

        void normalise()
        {
            T absoluteVal = this->data[0] * this->data[0] +
                            this->data[1] * this->data[1] +
                            this->data[2] * this->data[2] +
                            this->data[3] * this->data[3];

            if (absoluteVal != 1)
            {
                T k = pow(absoluteVal, -0.5);
                this->data[0] *= k;
                this->data[1] *= k;
                this->data[2] *= k;
                this->data[3] *= k;
            }
        }

        /*Quaternion(matrix::Matrix<T, 4, 1> vec)
        {
            for (int i = 0; i < 4; i++)
            {
                this->data[i] = vec.data[i];
            }
            normalise();
        }*/

        matrix::Matrix<T, 4> getMatrix()
        {
            normalise();
            matrix::Matrix<T, 4> result;
            result.data[0] = 1 - 2 * this->data[1] * this->data[1] - 2 * this->data[2] * this->data[2];
            result.data[1] = 2 * (this->data[0] * this->data[1] + this->data[3] * this->data[2]);
            result.data[2] = 2 * (this->data[0] * this->data[2] - this->data[3] * this->data[1]);
            result.data[3] = 0;

            result.data[4] = 2 * (this->data[0] * this->data[1] - this->data[3] * this->data[2]);
            result.data[5] = 1 - 2 * this->data[0] * this->data[0] - 2 * this->data[2] * this->data[2];
            result.data[6] = 2 * (this->data[1] * this->data[2] + this->data[3] * this->data[0]);
            result.data[7] = 0;

            result.data[8] = 2 * (this->data[0] * this->data[2] + this->data[3] * this->data[1]);
            result.data[9] = 2 * (this->data[1] * this->data[2] - this->data[3] * this->data[0]);
            result.data[10] = 1 - 2 * this->data[0] * this->data[0] - 2 * this->data[1] * this->data[1];
            result.data[11] = 0;

            result.data[12] = 0;
            result.data[13] = 0;
            result.data[14] = 0;
            result.data[15] = 1;
            return result;
        }
    };

    template <typename T>
    Quaternion<T> operator*(Quaternion<T> left, Quaternion<T> right)
    {
        //std::cout << "Multiplying quaternions\n";
        Quaternion<T> result;
        result.data[0] =  left.data[3] * right.data[0]
                        + left.data[0] * right.data[3]
                        + left.data[1] * right.data[2]
                        - left.data[2] * right.data[1];
        result.data[1] =  left.data[3] * right.data[1]
                        - left.data[0] * right.data[2]
                        + left.data[1] * right.data[3]
                        + left.data[2] * right.data[0];
        result.data[2] =  left.data[3] * right.data[2]
                        + left.data[0] * right.data[1]
                        - left.data[1] * right.data[0]
                        + left.data[2] * right.data[3];
        result.data[3] =  left.data[3] * right.data[3]
                        - left.data[0] * right.data[0]
                        - left.data[1] * right.data[1]
                        - left.data[2] * right.data[2];
        //std::cout << result;
        return result;
    }
}

#endif // QUATERNION_H_INCLUDED
