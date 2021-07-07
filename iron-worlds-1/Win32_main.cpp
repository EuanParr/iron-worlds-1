#include "Win32_main.h"

#include "input.h"
#include "lisp.h"
#include "logic.h"
#include "commonMain.h"
#include "platform.h"
#include "scene.h"
#include "session.h"

#include <cmath>
#include <iostream>
#include <random>
#include <windows.h>

void EnableOpenGL(HWND windowHandle, HDC* deviceContextHandle_Ptr, HGLRC* renderContextHandle_Ptr)
{
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;

    int pixelFormat;

    // Set the HDC, note that it was passed by pointer
    *deviceContextHandle_Ptr = GetDC(windowHandle);

    /* set the pixel format for the DC */
    // ZeroMemory is provided by WinAPI
    ZeroMemory(&pixelFormatDescriptor, sizeof(pixelFormatDescriptor));

    pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
    pixelFormatDescriptor.nVersion = 1;
    pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDescriptor.cColorBits = 24;
    pixelFormatDescriptor.cDepthBits = 16;
    pixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;

    pixelFormat = ChoosePixelFormat(*deviceContextHandle_Ptr, &pixelFormatDescriptor);

    SetPixelFormat(*deviceContextHandle_Ptr, pixelFormat, &pixelFormatDescriptor);

    /* create and enable the render context (RC) */
    *renderContextHandle_Ptr = wglCreateContext(*deviceContextHandle_Ptr);

    wglMakeCurrent(*deviceContextHandle_Ptr, *renderContextHandle_Ptr);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            // something requested closing the window
            // we use PostQuitMessage to ensure the process dies with the window
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            // window is being destroyed, no action needed
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
                case 0x44:
                case 0x41:
                case 0x46:
                case 0x52:
                case 0x53:
                case 0x57:
                case VK_UP:
                case VK_DOWN:
                case VK_RIGHT:
                case VK_LEFT:
                case 0x45:
                case 0x51:
                case VK_SPACE:
                    input::keyStates[wParam] = true;
                break;
            }

        }
        break;

        case WM_KEYUP:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
                case 0x44:
                case 0x41:
                case 0x46:
                case 0x52:
                case 0x53:
                case 0x57:
                case VK_UP:
                case VK_DOWN:
                case VK_RIGHT:
                case VK_LEFT:
                case 0x45:
                case 0x51:
                case VK_SPACE:
                    input::keyStates[wParam] = false;
                break;
            }

        }
        break;

        case WM_SIZE:
        {
            RECT newWindowSize;
            GetClientRect(hwnd, &newWindowSize);
            if (wglGetCurrentContext())
            {
                glViewport(0, 0, newWindowSize.right - newWindowSize.left, newWindowSize.bottom - newWindowSize.top);
            }

            float aspectRatio = ((float)newWindowSize.right - newWindowSize.left) / ((float)newWindowSize.bottom - newWindowSize.top);
            //screenMatrix = matrix::makeScale(1.0f / aspectRatio, 1.0f, 1.0f);
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

HDC deviceContextHandle;
MSG msg;
bool quit = FALSE;
Win32_PlatformContext testPlat;

void flushToScreen()
{
    SwapBuffers(deviceContextHandle);
}

void checkEvents()
{

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        // handle or dispatch messages
        if (msg.message == WM_QUIT)
        {
            quit = true;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

int WINAPI WinMain(
    // WinAPI handle to the instance of this executable
    // currently running
    HINSTANCE programInstanceHandle,
    // deprecated
    HINSTANCE previousProgramInstanceHandle,
    // pointer to command line arguments as a string
    PSTR commandLineString_Ptr,
    // legacy
    int nCmdShow)
{
    (void)previousProgramInstanceHandle;
    (void)commandLineString_Ptr;
    (void)nCmdShow;

    WNDCLASSEX windowClassEx;
    HWND windowHandle;

    HGLRC renderContextHandle;

    /*lisp::LispVirtualMachine lispVM;
    lispVM.read(std::cin);

    session::Session mySession;
    auto result = mySession.apply0(mySession.read(platform::standardLisp));

    //std::cout << sizeof(lisp::ListNode) << " is size of list,\n" << sizeof(lisp::BasicSymbol) << " is size of symbol\n";

    while (result != mySession.errorAtomPtr)
    {
        std::cout << ">>> ";
        result = mySession.apply0(mySession.read(std::cin));
        //result = mySession.read(std::cin);
        std::cout << "--> ";
        mySession.printS(result, std::cout);
        std::cout << "\n";
    }*/

    // register window class

    // must set size because we are using Ex version
    windowClassEx.cbSize = sizeof(WNDCLASSEX);
    // the Render Context is attached to the Device Context,
    // so we use CS_OWNDC to ensure the context is valid for the duration of the program
    windowClassEx.style = CS_OWNDC;
    // attach the window's message handling callback
    windowClassEx.lpfnWndProc = WindowProc;
    // no extra bytes needed
    windowClassEx.cbClsExtra = 0;
    windowClassEx.cbWndExtra = 0;
    // attach the handle to this program instance
    windowClassEx.hInstance = programInstanceHandle;
    // set the icon, NULL specifies a standard icon,
    // using default application icon
    windowClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    // similarly use default cursor
    windowClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClassEx.lpszMenuName = NULL;
    windowClassEx.lpszClassName = "MainWindow";
    windowClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;

    if (!RegisterClassEx(&windowClassEx))
    {
        return 0;
    }

    /* create main window */
    windowHandle = CreateWindowEx(0,
                          "MainWindow",
                          "Iron Worlds 1",
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          programInstanceHandle,
                          NULL);

    //ShowWindow(windowHandle, nCmdShow);

    for (int i = 0; i < NUM_KEYS; i++)
    {
        input::keyStates[i] = false;
    }

    /* enable OpenGL for the window */
    EnableOpenGL(windowHandle, &deviceContextHandle, &renderContextHandle);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    matrix::Matrix<double, 4, 1> xBasisMatrix;
    xBasisMatrix.data[0] = 1;
    matrix::Matrix<double, 4, 1> yBasisMatrix;
    yBasisMatrix.data[1] = 1;
    matrix::Matrix<double, 4, 1> zBasisMatrix;
    zBasisMatrix.data[2] = 1;

    scene::Scene testScene;

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
        newBody.angularVelocityQuaternion.normalise();
        newBody.myShape = new renderer::Cube();
        testScene.bodies.push_back(newBody);
    }

    /* program main loop */
    while (!quit)
    {
        /* check for messages */
        // use if instead of when, so that if a message induces a quit the
        // process quits immediately
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                quit = true;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            double camSpeed = 0.5;
            double camAngularSpeed = 0.02;

            matrix::Matrix<double, 3, 1> cameraMover;

            if (input::keyStates[0x41])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * xBasisMatrix);
            if (input::keyStates[0x44])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * xBasisMatrix);
            if (input::keyStates[0x46])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * yBasisMatrix);
            if (input::keyStates[0x52])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * yBasisMatrix);
            if (input::keyStates[0x53])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * zBasisMatrix);
            if (input::keyStates[0x57])
                cameraMover.smashMatrix(testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion.conjugate().getMatrix() * -1.0 * zBasisMatrix);

            testScene.currentPerspective_PtrWeak->worldDisplacement = testScene.currentPerspective_PtrWeak->worldDisplacement + cameraMover * camSpeed;

            if (input::keyStates[VK_UP])
                //std::cout << "moving\n";
                //std::cout << rotation::Quaternion<double>(rotation::unitIQuaternion, camAngularSpeed);
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitIQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[VK_DOWN])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitIQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[VK_LEFT])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitJQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[VK_RIGHT])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitJQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[0x45])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitKQuaternion, -camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[0x51])
                testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion = rotation::Quaternion<double>(rotation::unitKQuaternion, camAngularSpeed) * testScene.currentPerspective_PtrWeak->worldAngularDisplacementQuaternion;
            if (input::keyStates[VK_SPACE])
            {
                for (body::Body& newBody : testScene.bodies)
                {
                    newBody.angularVelocityQuaternion.data[0] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.data[1] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.data[2] = logic::unitRand() * k;
                    newBody.angularVelocityQuaternion.normalise();
                }
            }

            commonMain::doFrame(flushToScreen, testScene, testPlat);

            platform::sleepForMilliseconds(10);
        }
    }

    // shutdown OpenGL
    DisableOpenGL(windowHandle, deviceContextHandle, renderContextHandle);

    // destroy the window explicitly
    DestroyWindow(windowHandle);

    return msg.wParam;
}
