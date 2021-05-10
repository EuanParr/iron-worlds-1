#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <cmath>
#include <gl/gl.h>

namespace renderer
{
    const double pi = 3.14159265358979323846;

    class Shape
    {
    public:
        virtual void draw() = 0;
    };

    class InvisibleShape
    {
    public:
        void draw() {};
    };

    class Cube : public Shape
    {
    public:
        void draw()
        {
            int coords[3] = {0, 1, 2};
            int coordIndices[3] = {0, 1, 2};

            // for each axis
            for (int i = 0; i < 3; i++)
            {
                // exchange axes
                int temp = coordIndices[0];
                coordIndices[0] = coordIndices[1];
                coordIndices[1] = coordIndices[2];
                coordIndices[2] = temp;

                // for each face on the axis
                for (int j = 0; j < 2; j++)
                {
                    coords[coordIndices[0]] = j;
                    glBegin(GL_TRIANGLE_STRIP);
                    for (int k = 0; k < 2; k++)
                    {
                        coords[coordIndices[1]] = k;
                        for (int l = 0; l < 2; l++)
                        {
                            coords[coordIndices[2]] = l;
                            float x = coords[0] - 0.5f;
                            float y = coords[1] - 0.5f;
                            float z = coords[2] - 0.5f;
                            glColor3f(2*x, 2*y, 2*z);
                            glVertex3f(1.4*x, 1.4*y, 1.4*z);
                        }
                    }
                    glEnd();
                }
            }
        }
    };

    class Shard : public Shape
    {
        void draw()
        {
            int coords[4][3][3] =
            {
                {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                {{0, 1, 0}, {1, 0, 0}, {1, 1, 1}},
                {{0, 0, 1}, {0, 1, 0}, {1, 1, 1}},
                {{0, 0, 1}, {1, 0, 0}, {1, 1, 1}}
            };

            glBegin(GL_TRIANGLES);

            for (int face = 0; face < 4; face++)
            {
                for (int point = 0; point < 3; point++)
                {
                    float x, y ,z;
                    x = coords[face][point][0] - 0.5f;
                    y = coords[face][point][1] - 0.5f;
                    z = coords[face][point][2] - 0.5f;
                    glColor3f(1, 2*x, 0);
                    glVertex3f(1.4*x, 1.4*y, 1.4*z);
                }
            }
            glEnd();
        }
    };

    class Sphere : public Shape
    {
        void draw()
        {
            double step = pi / 5;
            for (float i = 0; i < 2 * pi; i+= step)
            {
                glBegin(GL_TRIANGLE_STRIP);
                float x, y, z;

                for (float j = -pi; j < pi; j+= step)
                {

                    float sinJ = sin(j);
                    x = cos(i) * sinJ;
                    y = sin(i) * sinJ;
                    z = cos(j);

                    glColor3f(x/2 + 0.5f, y/2 + 0.5f, z/2 + 0.5f);
                    glVertex3f(x, y, z);

                    i += step;

                    x = cos(i) * sinJ;
                    y = sin(i) * sinJ;
                    //z = cos(j);

                    glColor3f(x/2 + 0.5f, y/2 + 0.5f, z/2 + 0.5f);
                    glVertex3f(x, y, z);

                    i -= step;
                }
                glEnd();
            }
        }
    };
}

#endif // RENDERER_H_INCLUDED
