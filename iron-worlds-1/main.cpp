#include <iostream>
#include <cmath>
#include <windows.h>

#include "scene.h"

const double pi = 3.14159265358979323846;

matrix::Matrix<float, 4> screenMatrix;

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

void describeShard()
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

void describeCube()
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

void describeSphere()
{
    double step = pi / 2;
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
                          WS_OVERLAPPEDWINDOW /*| WS_MAXIMIZE*/,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          1024,
                          1024,
                          NULL,
                          NULL,
                          programInstanceHandle,
                          NULL);

    ShowWindow(windowHandle, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(windowHandle, &deviceContextHandle, &renderContextHandle);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    matrix::Matrix<float, 4> leftMatrix = matrix::makeTranslate(-0.5f, 0.0f, 0.0f);
    matrix::Matrix<float, 4> rightMatrix = matrix::makeTranslate(0.5f, 0.0f, 0.0f);
    matrix::Matrix<float, 4> scaleMatrix = matrix::makeScale(0.5f, 0.5f, 0.5f);
    //matrix::Matrix<float, 4> frustumMatrix = matrix::makeFrustum<float>();

    body::Body testBody;
    testBody.angularVelocity.data[0] = 0.1;
    testBody.velocity.data[2] = 1;
    testBody.radius = 0.1;

    scene::Scene testScene;
    testScene.bodies.push_back(testBody);
    testScene.perspectives.push_back(scene::Perspective());
    testScene.currentPerspective_PtrWeak = &testScene.perspectives.front();

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
            /* OpenGL animation code goes here */

            // set the background colour, black
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

            // clear the buffer, revert colour and depth of each pixel
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glPushMatrix();

            glMultMatrixf(screenMatrix.getRaw());
            //glMultMatrixf(frustumMatrix.getRaw());

            /*glPushMatrix();
            testBody.integrateMotionStep(1.0);
            matrix::Matrix<double, 4> testMatrix = testBody.makeBasisMatrix();
            glMultMatrixd(testMatrix.getRaw());
            describeCube();
            glPopMatrix();*/

            testScene.draw();
            testScene.simulateStep(1.0);

            glPushMatrix();

            glMultMatrixf(leftMatrix.getRaw());

            // glRotatef uses degrees
            glRotatef(theta, 1.0f, 1.0f, 1.0f);
            glRotatef(phi, 0.7f, 1.5f, -1.0f);
            glMultMatrixf(scaleMatrix.getRaw());
            describeSphere();

            glPopMatrix();

            glPushMatrix();

            glMultMatrixf(rightMatrix.getRaw());
            // glRotatef uses degrees
            glRotatef(theta, 1.0f, 1.0f, 1.0f);
            glRotatef(phi, 0.7f, 1.5f, -1.0f);
            glMultMatrixf(scaleMatrix.getRaw());
            describeShard();

            glPopMatrix();

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







