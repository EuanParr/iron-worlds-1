#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

namespace matrix
{


    template <typename CELLTYPE, unsigned int N, unsigned int M = N>
    class Matrix
    {
    public:
        CELLTYPE data[N * M];

        Matrix()
        {
            for (unsigned int i = 0; i < N * M; i++)
            {
                data[i] = 0;
            }
        }

        CELLTYPE* getRaw() {CELLTYPE* result = data; return result;}
    };

    template <typename T>
    class Mat4
    {
    public:
        T data[16];

        /*
        0   4   8  12
        1   5   9  13
        2   6  10  14
        3   7  11  15
        */

        Mat4()
        {
            for (int i = 0; i < 16; i++)
            {
                data[i] = 0;
            }
            data[0] = 1;
            data[5] = 1;
            data[10] = 1;
            data[15] = 1;
        }

        T* getRaw() {return data;}
    };

    template <typename T>
    Mat4<T> makeFrustum()
    {
        Mat4<T> result;
        T* data = result.getRaw();
        data[10] = 0.5;
        data[11] = 0.5;
        return result;
    }

    template <typename T>
    Mat4<T> makeScale(T x, T y, T z)
    {
        Mat4<T> result;
        T* data = result.getRaw();
        data[0] = x;
        data[5] = y;
        data[10] = z;
        return result;
    }

    template <typename T>
    Mat4<T> makeTranslate(T x, T y, T z)
    {
        Mat4<T> result;
        T* data = result.getRaw();
        data[12] = x;
        data[13] = y;
        data[14] = z;
        return result;
    }
}

#endif // MATRIX_H_INCLUDED
