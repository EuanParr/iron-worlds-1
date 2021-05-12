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
        matrix::Matrix<double, 3, 1> worldDisplacement;
        rotation::Quaternion<double> worldAngularDisplacementQuaternion;

        matrix::Matrix<double, 4> getPerspectiveMatrix();
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
