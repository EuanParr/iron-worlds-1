#include <cmath>
#include <iostream>
#include <random>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <chrono>
#include <thread>

#ifndef PLATFORM
#define PLATFORM_FEDORA
#define PLATFORM
#else
#error "Multiple definition of platform in file " + __FILE__
#endif // PLATFORM

#include "top_portability_bookend.h"
#include "input.h"
#include "lisp.h"
#include "logic.h"
#include "scene.h"

matrix::Matrix<float, 4> screenMatrix;
scene::Scene testScene;

static int singleBuffer[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, None};
static int doubleBuffer[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None};
bool isDoubleBuffer = true;

Display* x11Display_Ptr;
int x11ScreenID;
Window x11WindowID;
GC x11GraphicsContext;
XVisualInfo* x11VisualInfo_Ptr;
Colormap x11Colourmap;
XSetWindowAttributes x11SetWindowAttributes;
GLXContext x11GlxContext;

int main(int argc, char** argv)
{
    long x11EventMask = ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask;

    //unsigned long black, white;

    // open connection to X server
    x11Display_Ptr = XOpenDisplay(nullptr);
    if (!x11Display_Ptr)
    {
        FELOG("Could not open display");
    }

    // ensure GLX is supported
    int blankInt;
    if (!glXQueryExtension(x11Display_Ptr, &blankInt, &blankInt))
    {
        FELOG("X server has no OpenGl GLX extension");
    }

    //find appropriate visual
    x11VisualInfo_Ptr = glXChooseVisual(x11Display_Ptr, DefaultScreen(x11Display_Ptr), doubleBuffer);
    if (!x11VisualInfo_Ptr)
    {
        // could not get double buffer
        ELOG("Double buffer not available");
        x11VisualInfo_Ptr = glXChooseVisual(x11Display_Ptr, DefaultScreen(x11Display_Ptr), singleBuffer);
        isDoubleBuffer = false;
        if (!x11VisualInfo_Ptr)
        {
            // could not get single buffer either
            FELOG("No RGB visual with depth buffer");
        }
    }
    if (x11VisualInfo_Ptr->c_class != TrueColor)
    {
        FELOG("The program requires TrueColor visual");
    }

    // create a rendering context
    x11GlxContext = glXCreateContext(x11Display_Ptr, x11VisualInfo_Ptr, None, true);
    if (!x11GlxContext)
    {
        FELOG("Could not create rendering context");
    }

    // create x window
    x11Colourmap = XCreateColormap(x11Display_Ptr, RootWindow(x11Display_Ptr, x11VisualInfo_Ptr->screen), x11VisualInfo_Ptr->visual, AllocNone);
    x11SetWindowAttributes.colormap = x11Colourmap;
    x11SetWindowAttributes.border_pixel = 0;
    x11SetWindowAttributes.event_mask = x11EventMask;

    x11WindowID = XCreateWindow(x11Display_Ptr, RootWindow(x11Display_Ptr, x11VisualInfo_Ptr->screen), 0, 0, 300, 200, 0, x11VisualInfo_Ptr->depth, InputOutput, x11VisualInfo_Ptr->visual, CWBorderPixel | CWColormap | CWEventMask, &x11SetWindowAttributes);
    XSetStandardProperties(x11Display_Ptr, x11WindowID, "Iron Worlds 1", "Iron Worlds 1", None, argv, argc, nullptr);

    // bind rendering context to window
    glXMakeCurrent(x11Display_Ptr, x11WindowID, x11GlxContext);

    // request displaying window
    XMapWindow(x11Display_Ptr, x11WindowID);

    //x11ScreenID = DefaultScreen(x11Display_Ptr);
    //black = BlackPixel(x11Display_Ptr, x11ScreenID);
    //white = WhitePixel(x11Display_Ptr, x11ScreenID);

    //x11WindowID = XCreateSimpleWindow(
    //    x11Display_Ptr, DefaultRootWindow(x11Display_Ptr),
    //    0, 0, 200, 300, 5, white, black);

    //XSetStandardProperties(x11Display_Ptr, x11WindowID,
    //    "Iron Worlds 1", "Iron Worlds 1", None, nullptr, 0, nullptr);

    XSelectInput(x11Display_Ptr, x11WindowID, x11EventMask);

    //x11GraphicsContext = XCreateGC(x11Display_Ptr, x11WindowID, 0, 0);

    //XSetBackground(x11Display_Ptr, x11GraphicsContext, white);
    //XSetForeground(x11Display_Ptr, x11GraphicsContext, black);

    //XClearWindow(x11Display_Ptr, x11WindowID);
    //XMapRaised(x11Display_Ptr, x11WindowID);

    XEvent x11Event;

    bool bQuit = false;
    bool bRedraw = true;
    float theta = 0.0f;
    float phi = 34.6f;

    glEnable(GL_DEPTH_TEST);

    matrix::Matrix<double, 4, 1> xBasisMatrix;
    xBasisMatrix.data[0] = 1;
    matrix::Matrix<double, 4, 1> yBasisMatrix;
    yBasisMatrix.data[1] = 1;
    matrix::Matrix<double, 4, 1> zBasisMatrix;
    zBasisMatrix.data[2] = 1;

    scene::Perspective testPerspective;
    testPerspective.worldDisplacement.data[2] = 50;
    testPerspective.worldAngularDisplacementQuaternion.data[2] = 0.0;

    testScene.perspectives.push_back(testPerspective);
    testScene.currentPerspective_PtrWeak = &testScene.perspectives.front();

    double k = 0.1;

    for (int i = 0; i < 100; i++)
    {
        body::Body newBody;
        newBody.velocity.data[0] = logic::unitRand();
        newBody.velocity.data[1] = logic::unitRand();
        newBody.velocity.data[2] = logic::unitRand();
        newBody.angularVelocityQuaternion.data[0] = logic::unitRand() * k;
        newBody.angularVelocityQuaternion.data[1] = logic::unitRand() * k;
        newBody.angularVelocityQuaternion.data[2] = logic::unitRand() * k;
        /*newBody.angularVelocityQuaternion.data[0] = 1 * k;
        newBody.angularVelocityQuaternion.data[1] = 1 * k;
        newBody.angularVelocityQuaternion.data[2] = 1 * k;*/
        newBody.angularVelocityQuaternion.normalise();
        newBody.myShape = new renderer::Cube();
        testScene.bodies.push_back(newBody);
    }

    /* program main loop */
    while (!bQuit)
    {
        bRedraw = false;
        // check for messages
        // use if instead of when, so that if a message induces a quit the
        // process quits immediately
        if (XCheckWindowEvent(x11Display_Ptr, x11WindowID, x11EventMask, &x11Event))
        {
            switch (x11Event.type)
            {
                case ConfigureNotify:
                {
                    // window resizing
                    if (true) // TODO: find function to check if context exists
                    {
                        glViewport(0, 0, x11Event.xconfigure.width,
                            x11Event.xconfigure.height);
                    }
                    bRedraw = true;

                    float aspectRatio = ((float)x11Event.xconfigure.width) / ((float)x11Event.xconfigure.height);
                    screenMatrix = matrix::makeScale(1.0f / aspectRatio, 1.0f, 1.0f);
                }
                break;
                case Expose:
                    // window became more visible, redraw
                    // count indicates number of expose events to follow
                    // for now we process them together on the last one
                    if (x11Event.xexpose.count == 0)
                    {
                        bRedraw = true;
                    }
                break;
                case KeyPress:
                {
                    switch (x11Event.xkey.keycode)
                    {
                        case XK_Escape:
                            bQuit = true;
                        break;
                        case XK_D:
                        case XK_A:
                        case XK_F:
                        case XK_R:
                        case XK_S:
                        case XK_W:
                        case XK_Up:
                        case XK_Down:
                        case XK_Right:
                        case XK_Left:
                        case XK_E:
                        case XK_Q:
                        case XK_space:
                            input::keyStates[x11Event.xkey.keycode] = true;
                        break;
                    }

                }
                break;

                case KeyRelease:
                {
                    switch (x11Event.xkey.keycode)
                    {
                        case XK_Escape:
                            bQuit = true;
                        break;
                        case XK_D:
                        case XK_A:
                        case XK_F:
                        case XK_R:
                        case XK_S:
                        case XK_W:
                        case XK_Up:
                        case XK_Down:
                        case XK_Right:
                        case XK_Left:
                        case XK_E:
                        case XK_Q:
                        case XK_space:
                            input::keyStates[x11Event.xkey.keycode] = False;
                        break;
                    }

                }
                break;
            }
        }
        else
        {
            // set the background colour, black
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

            // clear the buffer, revert colour and depth of each pixel
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glPushMatrix();
            glMultMatrixf(screenMatrix.getRaw());

            testScene.draw();
            testScene.simulateStep(0.1);

            double camSpeed = 0.5;
            double camAngularSpeed = 0.02;

            matrix::Matrix<double, 3, 1> cameraMover;

            if (input::keyStates[XK_A])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * xBasisMatrix);
            if (input::keyStates[XK_D])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * xBasisMatrix);
            if (input::keyStates[XK_F])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * yBasisMatrix);
            if (input::keyStates[XK_R])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * yBasisMatrix);
            if (input::keyStates[XK_S])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * zBasisMatrix);
            if (input::keyStates[XK_W])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * zBasisMatrix);

            testScene.currentPerspective_PtrWeak->worldDisplacement = testScene.currentPerspective_PtrWeak->worldDisplacement + cameraMover * camSpeed;

            if (input::keyStates[XK_Up])
                //std::cout << "moving\n";
                //std::cout << rotation::Quaternion<double>(rotation::unitIQuaternion, camAngularSpeed);
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitIQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[XK_Down])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitIQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[XK_Left])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitJQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[XK_Right])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitJQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[XK_E])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitKQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[XK_Q])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitKQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[XK_space])
            {
                for (body::Body& newBody : testScene.bodies)
                {
                    newBody.angularVelocityQuaternion.data[0] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.data[1] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.data[2] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.normalise();
                }
            }

            glPopMatrix();

            if (isDoubleBuffer)
            {
                glXSwapBuffers(x11Display_Ptr, x11WindowID);
            }
            else
            {
                glFlush();
            }

            theta += 1.0f;
            phi += 0.6f;

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    LOG("BYE");

    /* shutdown OpenGL */
    //DisableOpenGL(windowHandle, deviceContextHandle, renderContextHandle);

    // destroy the window explicitly
    //XFreeGC(x11Display_Ptr, x11GraphicsContext);
    //XDestroyWindow(x11Display_Ptr, x11WindowID);
    //XCloseDisplay(x11Display_Ptr);

    return 0;
}
