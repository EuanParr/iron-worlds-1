#include "Win32_main.h"

#include "input.h"
#include "lisp.h"
#include "logic.h"
#include "common_main.h"
#include "platform.h"
#include "scene.h"
#include "session.h"

#include <cmath>
#include <iostream>
#include <random>
#include <windows.h>

HDC deviceContextHandle;
MSG msg;
bool quit = FALSE;
Win32_main::Win32_PlatformContext platformContext;

namespace Win32_main
{
    std::unordered_map<unsigned int, platform::InputCode> Win32_PlatformContext::inputCodeMap
    {
        {0x41, platform::InputCode::A},
        {0x44, platform::InputCode::D},
        {0x45, platform::InputCode::E},
        {0x46, platform::InputCode::F},
        {0x51, platform::InputCode::Q},
        {0x52, platform::InputCode::R},
        {0x53, platform::InputCode::S},
        {0x57, platform::InputCode::W},
        {VK_RIGHT, platform::InputCode::RightArrow},
        {VK_LEFT, platform::InputCode::LeftArrow},
        {VK_UP, platform::InputCode::UpArrow},
        {VK_DOWN, platform::InputCode::DownArrow},
        {VK_SPACE, platform::InputCode::Space},
        {VK_ESCAPE, platform::InputCode::Escape},
    };

    void Win32_PlatformContext::flushToScreen()
    {
        SwapBuffers(deviceContextHandle);
    }

    void Win32_PlatformContext::sleepForMilliseconds(int time)
    {
        Sleep(time);
    }

    void Win32_PlatformContext::checkEvents()
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

}

void EnableOpenGL(HWND windowHandle, HDC* deviceContextHandle_Ptr, HGLRC* renderContextHandle_Ptr)
{
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;

    int pixelFormat;

    // Set the HDC, note that it was passed by pointer
    *deviceContextHandle_Ptr = GetDC(windowHandle);

    // set the pixel format for the DC
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

    // create and enable the render context (RC)
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
            platformContext.updateButtonInput(wParam, true);
        }
        break;

        case WM_KEYUP:
        {
            platformContext.updateButtonInput(wParam, false);
        }
        break;

        case WM_SIZE:
        {
            RECT newWindowSize;
            GetClientRect(hwnd, &newWindowSize);
            platformContext.updateViewPort(newWindowSize.right - newWindowSize.left, newWindowSize.bottom - newWindowSize.top);
        }
        break;

        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
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

    HGLRC renderContextHandle;

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

    // create main window
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

    // enable OpenGL for the window
    EnableOpenGL(windowHandle, &deviceContextHandle, &renderContextHandle);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    common_main::main(platformContext);

    // shutdown OpenGL
    DisableOpenGL(windowHandle, deviceContextHandle, renderContextHandle);

    // destroy the window explicitly
    DestroyWindow(windowHandle);

    return 0;
}
