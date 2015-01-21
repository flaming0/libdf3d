#pragma once

#include "../Application.h"

namespace df3d { namespace platform {

class WindowsApplication : public Application
{
    struct Impl;
    unique_ptr<Impl> m_pImpl;

public:
    WindowsApplication(const AppInitParams &params);
    virtual ~WindowsApplication();

    bool pollEvents() override;
    void swapBuffers() override;

    void setTitle(const char *title) override;
};

} }