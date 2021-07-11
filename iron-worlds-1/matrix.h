#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include <cmath>
#include <iostream>

namespace matrix
{
    /*
    stored in column-major arrays, meaning we read first down then right
    dimensions: first dimension M is vertical, second dimension N is horizontal
    indices: first index i is vertical, second index j is horizontal

    2x3 matrix -> M = 2, N = 3:
    024
    135

    matrix[1, 0] -> i = 1, j = 0:
    024
    X35

    */
    template <typename ELEMENTTYPE, unsigned int M, unsigned int N = M>
    class Matrix
    {
    public:
        ELEMENTTYPE data[M * N];

        Matrix()
        {
            // initialise as a null matrix
            for (unsigned int i = 0; i < M * N; i++)
            {
                data[i] = (ELEMENTTYPE)0.0;
            }
            // but if square, initialise as identity matrix
            if (M == N)
            {
                for (unsigned int i = 0; i < M * N; i += M + 1)
                {
                    data[i] = (ELEMENTTYPE)1.0;
                }
            }
        }

        ELEMENTTYPE* getRaw() {ELEMENTTYPE* result = data; return result;};
        ELEMENTTYPE* getPtrAt(int i, int j) {return (ELEMENTTYPE*)data + i + M*j;}
        ELEMENTTYPE& getRefAt(int i, int j) {return data[i + M*j];}
        ELEMENTTYPE getAtConst (int i, int j) const {return data[i + M * j];}
        void loadArray(ELEMENTTYPE* arr)
        {
            for (unsigned int i = 0; i < M * N; i++)
            {
                data[i] = arr[i];
            }
        }

        // converts matrices forcefully
        // aligns the 2 at top left corner and overwrites, casting cell type,
        // at the area of intersection
        // useful for changing precision or converting between
        // homogeneous and traditional coordinates
        template <typename OTHERTYPE, unsigned int OTHERM, unsigned int OTHERN>
        void smashMatrix(const Matrix<OTHERTYPE, OTHERM, OTHERN>& fromMatrix)
        {
            unsigned int m, n;
            m = std::min(M, OTHERM);
            n = std::min(N, OTHERN);
            for (unsigned int i = 0; i < m; i++)
            {
                for (unsigned int j = 0; j < n; j++)
                {
                    *getPtrAt(i, j) = (ELEMENTTYPE)fromMatrix.getAtConst(i, j);
                }
            }
        }
    };

    template <typename T>
    Matrix<T, 4> makeFrustum()
    {
        Matrix<T, 4> result;
        T* data = result.getRaw();
        data[10] = 0.5;
        data[11] = 0.5;
        return result;
    }

    template <typename T>
    Matrix<T, 4> makeScale(T x, T y, T z)
    {
        Matrix<T, 4> result;
        T* data = result.getRaw();
        data[0] = x;
        data[5] = y;
        data[10] = z;
        return result;
    }

    template <typename T>
    Matrix<T, 4> makeTranslate(T x, T y, T z)
    {
        Matrix<T, 4> result;
        T* data = result.getRaw();
        data[12] = x;
        data[13] = y;
        data[14] = z;
        return result;
    }

    template <typename T>
    Matrix<T, 4> makeTranslate(Matrix<T, 4, 1> vec)
    {
        Matrix<T, 4> result;
        T* data = result.getRaw();
        T w = vec.data[3];
        data[12] = vec.data[0] / w;
        data[13] = vec.data[1] / w;
        data[14] = vec.data[2] / w;
        return result;
    }

    template <typename T, unsigned int M>
    T vectorAbsolute(Matrix<T, M, 1> vec)
    {
        T sum = 0;
        for (unsigned int i = 0; i < M; i++)
        {
            sum += vec.data[i] * vec.data[i];
        }
        return pow(sum, 0.5);
    }

    template <typename T, unsigned int M>
    Matrix<T, M> minor(Matrix<T, M+1> oldMat, unsigned int i, unsigned int j)
    {
        Matrix<T, M> result;
        unsigned int newI = 0;
        for (unsigned int oldI = 0; oldI <= M; oldI++)
        {
            if (oldI != i)
            {
                unsigned int newJ = 0;
                for (unsigned int oldJ = 0; oldJ <= M; oldJ++)
                {
                    if (oldJ != j)
                    {
                        &result.getPtrAt(newI, newJ) = oldMat.getAt(oldI, oldJ);
                    }
                    newJ++;
                }
                newI++;
            }
        }
        return result;
    }

    template <typename T, unsigned int M>
    T determinant(Matrix<T, M> oldMat)
    {

    }

    template <typename T, unsigned int MLEFT, unsigned int MNCOMMON, unsigned int NRIGHT>
    Matrix<T, MLEFT, NRIGHT> operator*(const Matrix<T, MLEFT, MNCOMMON>& leftMatrix, const Matrix<T, MNCOMMON, NRIGHT>& rightMatrix)
    {
        Matrix<T, MLEFT, NRIGHT> result;
        for (unsigned int i = 0; i < MLEFT; i++)
        {
            for (unsigned int j = 0; j < NRIGHT; j++)
            {
                T sum = 0;
                for (unsigned int k = 0; k < MNCOMMON; k++)
                {
                    sum += leftMatrix.getAtConst(i, k) * rightMatrix.getAtConst(k, j);
                }
                *(result.getPtrAt(i, j)) = sum;
            }
        }
        return result;
    }

    template <typename T, unsigned int M, unsigned int N>
    Matrix<T, M, N> operator*(const Matrix<T, M, N>& mat, const T scalar)
    {
        Matrix<T, M, N> result;
        for (unsigned int i = 0; i < M * N; i++)
        {
            *(result.getRaw() + i) = mat.data[i] * scalar;
        }
        return result;
    }

    template <typename T, unsigned int M, unsigned int N>
    Matrix<T, M, N> operator*(const T scalar, const Matrix<T, M, N>& mat)
    {
        return mat * scalar;
    }

    template <typename T, unsigned int M, unsigned int N>
    void operator*=(Matrix<T, M, N>& mat, const T scalar)
    {
        mat = mat * scalar;
    }

    template <typename T, unsigned int M, unsigned int N>
    void operator*=(Matrix<T, M, N>& mat, const Matrix<T, N> squareMat)
    {
        mat = mat * squareMat;
    }

    template <typename T, unsigned int M, unsigned int N>
    Matrix<T, M, N> operator+(const Matrix<T, M, N>& leftMatrix, const Matrix<T, M, N>& rightMatrix)
    {
        Matrix<T, M, N> result;
        for (unsigned int i = 0; i < M * N; i++)
        {
            *(result.getRaw() + i) = leftMatrix.data[i] + rightMatrix.data[i];
        }
        return result;
    }

    template <typename T, unsigned int M, unsigned int N>
    void operator+=(Matrix<T, M, N>& leftMatrix, const Matrix<T, M, N>& rightMatrix)
    {
        for (unsigned int i = 0; i < M * N; i++)
        {
            leftMatrix.data[i] += rightMatrix.data[i];
        }
    }

    template <typename T, unsigned int M, unsigned int N>
    std::ostream& operator<<(std::ostream& outStream, const Matrix<T, M, N>& mat)
    {
        outStream << "Matrix:\n    ";
        for (unsigned int i = 0; i < M; i++)
        {
            for (unsigned int j = 0; j < N; j++)
            {
                outStream << mat.getAt(i, j) << ", ";
            }
            outStream << "\n    ";
        }
        return outStream;
    }
}

#endif // MATRIX_H_INCLUDED
