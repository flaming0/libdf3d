#include "df3d_pch.h"
#include "Application.h"

#if defined(DF3D_WINDOWS) || defined(DF3D_LINUX)
#include <platform/glfwApplication.h>
#elif defined(DF3D_WINDOWS_PHONE)
#include <platform/windows_phone/WindowsPhoneApplication.h>
#error "Unsupported platform"
#endif

namespace df3d { namespace platform {

Application *Application::instance = nullptr;

Application::Application(const AppInitParams &params, AppDelegate *appDelegate)
    : m_appDelegate(appDelegate),
    m_appInitParams(params)
{
}

Application::~Application()
{

}

Application *Application::create(const AppInitParams &params, AppDelegate *appDelegate)
{
    assert(!Application::instance);

#if defined(DF3D_WINDOWS) || defined(DF3D_LINUX)
    Application::instance = new glfwApplication(params, appDelegate);
#elif defined(DF3D_WINDOWS_PHONE)
    Application::instance = new WindowsPhoneApplication(params, appDelegate);
#else
    return nullptr;
#endif

    return Application::instance;
}

} }
