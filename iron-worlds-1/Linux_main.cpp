#include "Linux_main.h"

#include "input.h"
#include "lisp.h"
#include "logic.h"
#include "common_main.h"
#include "platform.h"
#include "scene.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <thread>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

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
XEvent x11Event;
bool quit = false;
long x11EventMask = ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;
Linux_main::Linux_PlatformContext platformContext;

namespace Linux_main
{
    std::unordered_map<unsigned int, platform::InputCode> Linux_PlatformContext::inputCodeMap
    {
        {XK_a, platform::InputCode::A},
        {XK_d, platform::InputCode::D},
        {XK_e, platform::InputCode::E},
        {XK_f, platform::InputCode::F},
        {XK_q, platform::InputCode::Q},
        {XK_r, platform::InputCode::R},
        {XK_s, platform::InputCode::S},
        {XK_w, platform::InputCode::W},
        {XK_Right, platform::InputCode::RightArrow},
        {XK_Left, platform::InputCode::LeftArrow},
        {XK_Up, platform::InputCode::UpArrow},
        {XK_Down, platform::InputCode::DownArrow},
        {XK_space, platform::InputCode::Space},
        {XK_Escape, platform::InputCode::Escape},
    };

    void Linux_PlatformContext::flushToScreen()
    {
        if (isDoubleBuffer)
        {
            glXSwapBuffers(x11Display_Ptr, x11WindowID);
        }
        else
        {
            glFlush();
        }
    }

    void Linux_PlatformContext::sleepForMilliseconds(int time)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }

    void Linux_PlatformContext::checkEvents()
    {
        if (XCheckWindowEvent(x11Display_Ptr, x11WindowID, x11EventMask, &x11Event))
        {
            switch (x11Event.type)
            {
                case KeyPress:
                {
                    platformContext.updateButtonInput(XLookupKeysym(&x11Event.xkey, 0), true);
                }
                break;

                case KeyRelease:
                {
                    platformContext.updateButtonInput(XLookupKeysym(&x11Event.xkey, 0), false);
                }
                break;

                case ConfigureNotify:
                {
                    // window resizing
                    platformContext.updateViewPort(x11Event.xconfigure.width, x11Event.xconfigure.height);
                }
                break;
            }
        }
    }

}

int main(int argc, char** argv)
{
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
    XSelectInput(x11Display_Ptr, x11WindowID, x11EventMask);

    glEnable(GL_DEPTH_TEST);

    common_main::main(latformContext);

    return 0;
}
