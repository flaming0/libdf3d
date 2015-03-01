#pragma once

#include "AppDelegate.h"

namespace df3d { namespace platform {

class DF3D_DLL AppEvent
{
public:
    enum class Type
    {
        QUIT,
        KEYBOARD,
        MOUSE_BUTTON,
        MOUSE_MOTION,
        MOUSE_WHEEL,
        FINGER
    };

    Type type;

    base::MouseMotionEvent mouseMotion;
    base::MouseButtonEvent mouseButton;
    base::MouseWheelEvent mouseWheel;
    base::KeyboardEvent keyboard;
};

struct DF3D_DLL AppInitParams
{
    int windowWidth = DEFAULT_WINDOW_WIDTH;
    int windowHeight = DEFAULT_WINDOW_HEIGHT;
    bool fullscreen = false;
};

class DF3D_DLL Application
{
protected:
    AppDelegate *m_appDelegate;
    AppInitParams m_appInitParams;

    Application(const AppInitParams &params, AppDelegate *appDelegate);

public:
    static Application *instance;

    virtual ~Application();

    virtual void run() = 0;
    virtual void setTitle(const char *title) = 0;

    // Creates application instance depending on platform.
    static Application *create(const AppInitParams &params, AppDelegate *appDelegate);
};

} }
