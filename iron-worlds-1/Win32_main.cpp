#include "Win32_main.h"

#include "input.h"
#include "lisp.h"
#include "logic.h"
#include "common_main.h"
#include "platform.h"
#include "scene.h"

#include <cmath>
#include <iostream>
#include <random>
#include <thread>
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

    HWND windowHandle;

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
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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

    void soundThreadFunc(char* waveRiffBuffer)
    {
        PlaySound(waveRiffBuffer, NULL, SND_MEMORY);
        delete[] waveRiffBuffer;
    }

    // adapted from https://stackoverflow.com/a/1451799
    struct WaveRiffHeader
    {
        DWORD riffChunkTag; // 0x46464952 is binary "RIFF", specifies RIFF format
        DWORD riffChunkSize; // 4<waveChunkTag> + (8 + subChunk0Size)<subChunk0> + (8 + subChunk1Size)<subChunk1>
        DWORD waveChunkTag; // 0x45564157 is binary "WAVE", specifies WAVE subformat

        DWORD subChunk0Tag; // 0x20746d66 is binary "fmt ", specifies wave metadata format
        DWORD subChunk0Size; // 16 for PCM metadata
        WORD audioFormat; // 1 for integer PCM. 3 for floating point, 7 for mu-law
        WORD numChannels; // mono, stereo, etc.
        DWORD sampleRate; // samples per second
        DWORD byteRate; // sampleRate * numChannels * bitDepth/8
        WORD blockAlign; // numChannels * bitDepth / 8
        WORD bitDepth; // number of bits in a sample

        DWORD subChunk1Tag; // 0x61746164 is binary "data"
        DWORD subChunk1Size; // numSamples * numChannels * bitDepth/8
    };

    void Win32_PlatformContext::playSoundFromMemory(audio::PCMBuffer& buffer)
    {
        char* waveRiffBuffer = new char[sizeof(WaveRiffHeader) + buffer.getBufSize()];

        WaveRiffHeader* riffHeader = reinterpret_cast<WaveRiffHeader*>(waveRiffBuffer);
        riffHeader->riffChunkTag = 0x46464952;
        riffHeader->riffChunkSize = 4 + 8 + 16 + 8 + buffer.getBufSize();
        riffHeader->waveChunkTag = 0x45564157;

        riffHeader->subChunk0Tag = 0x20746d66;
        riffHeader->subChunk0Size = 16;
        riffHeader->audioFormat = 3;
        riffHeader->numChannels = 1;
        riffHeader->sampleRate = long(buffer.sampleRate);
        riffHeader->byteRate = riffHeader->sampleRate * 1 * sizeof(float);
        riffHeader->blockAlign = 1 * sizeof(float);
        riffHeader->bitDepth = 8 * sizeof(float);

        riffHeader->subChunk1Tag = 0x61746164;
        riffHeader->subChunk1Size = buffer.getBufSize();

        float* riffDataBuffer = reinterpret_cast<float*>(waveRiffBuffer + sizeof(WaveRiffHeader));
        float* riffBufPtr = riffDataBuffer;
        float* internalBufPtr = buffer.buf;

        for (unsigned int i = 0; i < buffer.numSamples; ++i)
        {
            *riffDataBuffer++ = *internalBufPtr++;
        }

        std::thread audioThread(soundThreadFunc, waveRiffBuffer);
        audioThread.detach();

        /*IAudioClient::Initialize();
        char deviceAlias[] = "Dev1";
        MCIDEVICEID deviceId = 0;
        MCI_WAVE_OPEN_PARMS openParams
        {
            DWORD_PTR(windowHandle), // window to report messages to
            deviceId, // id of device to open to
            LPCSTR(MCI_DEVTYPE_WAVEFORM_AUDIO), // device type
            "waveaudio", // device name
            deviceAlias, // optional device alias
            DWORD(buffer.numSamples / buffer.sampleRate)
        };
        tryMciCommand(deviceId, MCI_OPEN, MCI_WAIT, DWORD(&openParams));
        //mciSendCommand(deviceId, )*/
    }

    /*void Win32_PlatformContext::tryMciCommand(MCIDEVICEID deviceId, UINT message, DWORD flags, DWORD paramStruct)
    {
        MCIERROR errorCode = mciSendCommand(deviceId, message, flags, paramStruct);
        // report error if any
        if (errorCode)
        {
            // error code is up to 128 chars long and written to a C-style string
            char errorString[129];
            mciGetErrorString(errorCode, errorString, 129);
            ELOG(errorString);
        }
    }*/
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
    wglMakeCurrent(nullptr, nullptr);
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

        // various messages that we do not handle currently but are aware can occur:
        case WM_GETMINMAXINFO:
        case WM_NCCREATE:
        case WM_NCCALCSIZE:
        case WM_CREATE:
        case WM_WINDOWPOSCHANGING:
        case WM_WINDOWPOSCHANGED:
        case WM_MOVE:
        case WM_GETICON:
        case WM_SHOWWINDOW:
        case WM_ACTIVATEAPP:
        case WM_NCACTIVATE:
        case WM_ACTIVATE:
        case WM_IME_SETCONTEXT:
        case WM_IME_NOTIFY:
        case WM_SETFOCUS:
        case WM_NCPAINT:
        case WM_ERASEBKGND:
        case 799: // not a documented message type
        case 49511:
        case WM_NCHITTEST:
        case WM_SETCURSOR:
        case WM_MOUSEMOVE:
        case WM_PAINT:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSCOMMAND:
        case WM_ENTERMENULOOP:
        case WM_INITMENU:
        case WM_MENUSELECT:
        case WM_ENTERIDLE:
        case WM_CAPTURECHANGED:
        case WM_EXITMENULOOP:
        case WM_KILLFOCUS:

        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        break;
        default:
        {
            TLOG("Unrecognised WindowProc message: " << uMsg);
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
    HINSTANCE, //previousProgramInstanceHandle,
    // pointer to command line arguments as a string
    PSTR, //commandLineString_Ptr,
    // legacy
    int) //nCmdShow)
{
    auto& windowHandle = Win32_main::windowHandle;
    WNDCLASSEX windowClassEx;

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
    windowClassEx.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    // similarly use default cursor
    windowClassEx.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClassEx.lpszMenuName = nullptr;
    windowClassEx.lpszClassName = "MainWindow";
    windowClassEx.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);;

    if (!RegisterClassEx(&windowClassEx))
    {
        return 0;
    }

    // create main window
    windowHandle = CreateWindowEx(0,
                          "MainWindow",
                          platformContext.programName.c_str(),
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          nullptr,
                          nullptr,
                          programInstanceHandle,
                          nullptr);

    // enable OpenGL for the window
    EnableOpenGL(windowHandle, &deviceContextHandle, &renderContextHandle);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    common_main::main(platformContext);

    return 0;
}
