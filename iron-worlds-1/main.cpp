#include <cmath>
#include <iostream>
#include <random>
#include <windows.h>

#include "logic.h"
#include "scene.h"

matrix::Matrix<float, 4> screenMatrix;
scene::Scene testScene;

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
                case 0x57:
                    testScene.currentPerspective_PtrWeak->worldDisplacement.data[2] -= 0.3;
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
            screenMatrix = matrix::makeScale(1.0f / aspectRatio, 1.0f, 1.0f);
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
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
    HDC deviceContextHandle;
    HGLRC renderContextHandle;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;
    float phi = 34.6f;

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
    windowClassEx.lpszClassName = "GLSample";
    windowClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&windowClassEx))
    {
        return 0;
    }

    /* create main window */
    windowHandle = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
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

    /* enable OpenGL for the window */
    EnableOpenGL(windowHandle, &deviceContextHandle, &renderContextHandle);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    body::Body testBody;
    testBody.angularVelocity.data[0] = 0.06;
    testBody.angularVelocity.data[1] = 0.1;
    testBody.angularVelocity.data[2] = 0.13;
    testBody.velocity.data[2] = 0.01;
    testBody.radius = 1;
    testBody.myShape = new renderer::Cube();



    scene::Perspective testPerspective;
    testPerspective.worldDisplacement.data[2] = 50;
    testPerspective.worldAngularDisplacement.data[2] = 0.0;

    testScene.bodies.push_back(testBody);
    testScene.perspectives.push_back(testPerspective);
    testScene.currentPerspective_PtrWeak = &testScene.perspectives.front();

    for (int i = 0; i < 100; i++)
    {
        body::Body newBody;
        newBody.velocity.data[0] = logic::unitRand();
        newBody.velocity.data[1] = logic::unitRand();
        newBody.velocity.data[2] = logic::unitRand();
        //newBody.velocity *= 1/matrix::AbsoluteOfVector(newBody.velocity);
        newBody.angularVelocity.data[0] = logic::unitRand();
        newBody.angularVelocity.data[1] = logic::unitRand();
        newBody.angularVelocity.data[2] = logic::unitRand();
        newBody.myShape = new renderer::Shard();
        testScene.bodies.push_back(newBody);
    }

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        // use if instead of when, so that if a message induces a quit the
        // process quits immediately
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
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

            glPopMatrix();

            SwapBuffers(deviceContextHandle);

            theta += 1.0f;
            phi += 0.6f;

            // Sleep is provided by WinAPI
            Sleep(10);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(windowHandle, deviceContextHandle, renderContextHandle);

    /* destroy the window explicitly */
    DestroyWindow(windowHandle);

    return msg.wParam;
}





