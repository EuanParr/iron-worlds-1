#include "scene.h"

namespace scene
{
    void cube()
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

    void Scene::draw()
    {
        if (!currentPerspective_PtrWeak)
        {
            std::cerr << "No current Perspective\n";
        }
        glPushMatrix();
        glMultMatrixd(currentPerspective_PtrWeak->getPerspectiveMatrix().getRaw());
        for (body::Body& currentBody : bodies)
        {
            glPushMatrix();
            glMultMatrixd(currentBody.makeBasisMatrix().getRaw());
            cube();
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
