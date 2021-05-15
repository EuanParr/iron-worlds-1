#ifndef ROTATION_H_INCLUDED
#define ROTATION_H_INCLUDED

#include "matrix.h"

namespace rotation
{
    template <typename T>
    class Quaternion : public matrix::Matrix<T, 4, 1>
    {
        // following the convention set with homogeneous vectors and matrices,
        // the special (real) coefficient comes last. This does not follow
        // usual quaternion notation

        // hence xi + yj + zk + w

    public:
        Quaternion()
        {
            this->data[0] = 0;
            this->data[1] = 0;
            this->data[2] = 0;
            this->data[3] = 1;
        }

        T getAbsolute()
        {
            T absoluteSqr = this->data[0] * this->data[0] +
                            this->data[1] * this->data[1] +
                            this->data[2] * this->data[2] +
                            this->data[3] * this->data[3];
            return pow(absoluteSqr, 0.5);
        }

        void normalise()
        {
            T k = 1/getAbsolute();

            if (k > 1.01 || k < 0.99)
            {
                //std::cout << "had to normalise\n";
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

        matrix::Matrix<T, 3, 1> getImaginary() const
        {
            matrix::Matrix<T, 3, 1> result;
            result.data[0] = this->data[0];
            result.data[1] = this->data[1];
            result.data[2] = this->data[2];
            return result;
        }

        T getReal()
        {
            return this->data[3];
        }

        Quaternion naturalLog () const
        {
            Quaternion<T> result;
            T imaginaryAbsolute = matrix::vectorAbsolute(getImaginary());
            T realPart = log(imaginaryAbsolute);
            result.data[3] = realPart;
            matrix::Matrix<T, 3, 1> imaginaryPart = getImaginary() * (imaginaryAbsolute > 0.00001 ? atan(realPart / imaginaryAbsolute) / imaginaryAbsolute : 0);
            result.data[0] = imaginaryPart.data[0];
            result.data[1] = imaginaryPart.data[1];
            result.data[2] = imaginaryPart.data[2];
            return result;
        }

        Quaternion exponential()
        {
            Quaternion<T> result;
            T imaginaryAbsolute = matrix::vectorAbsolute(getImaginary());
            result.data[3] = cos(imaginaryAbsolute);
            result.smashMatrix(getImaginary() * (imaginaryAbsolute > 0.00001 ? sin(imaginaryAbsolute) / imaginaryAbsolute : 0));
            return result * exp(getReal());
        }

        Quaternion(const Quaternion<T>& oldQ, T power)
        {
            Quaternion<T> result = oldQ.naturalLog();
            result = result * power;
            result = result.exponential();
            for (int i = 0; i < 4; i++)
            {
                this->data[i] = result.data[i];
            }
        }

        Quaternion(T x, T y, T z, T w)
        {
            this->data[0] = x;
            this->data[1] = y;
            this->data[2] = z;
            this->data[3] = w;
        }

        Quaternion<T> conjugate()
        {
            return Quaternion<T>(-this->data[0], -this->data[1], -this->data[2], this->data[3]);
        }
    };

    const double basicAngle = 1.0;

    const Quaternion<double> unitIQuaternion(sin(basicAngle/2), 0, 0, cos(basicAngle/2));
    const Quaternion<double> unitJQuaternion(0, sin(basicAngle/2), 0, cos(basicAngle/2));
    const Quaternion<double> unitKQuaternion(0, 0, sin(basicAngle/2), cos(basicAngle/2));
    const Quaternion<double> unitRQuaternion(0, 0, 0, 1);

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

    template <typename T>
    Quaternion<T> operator*(Quaternion<T> q, T scalar)
    {
        Quaternion<T> result;
        for (int i = 0; i < 4; i++)
        {
            result.data[i] = q.data[i] * scalar;
        }
        return result;
    }

    template <typename T>
    Quaternion<T> operator*(T scalar, Quaternion<T> q)
    {
        return q * scalar;
    }
}

#endif // ROTATION_H_INCLUDED
