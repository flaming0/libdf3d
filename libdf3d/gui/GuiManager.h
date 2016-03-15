#pragma once

#include "RocketRefWrapper.h"
#include <Rocket/Core.h>

namespace tb {
class TBRenderer;
class TBSkin;
class TBWidgetsReader;
class TBLanguage;
class TBFontManager;
}

namespace df3d {

namespace gui_impl {

class FileInterface;
class SystemInterface;
class RenderInterface;

}

class DF3D_DLL GuiManager : utils::NonCopyable
{
    unique_ptr<gui_impl::FileInterface> m_fileInterface;
    unique_ptr<gui_impl::SystemInterface> m_systemInterface;
    unique_ptr<gui_impl::RenderInterface> m_renderInterface;

    Rocket::Core::Context *m_rocketContext = nullptr;

public:
    GuiManager();
    ~GuiManager();

    void initialize(int contextWidth, int contextHeight);
    void shutdown();

    RocketDocument loadDocument(const std::string &name);

    void showDebugger(bool show);
    bool isDebuggerVisible() const;

    Rocket::Core::Context* getContext() { return m_rocketContext; }

    template<typename T>
    RocketRefWrapper<T> createDocument(const std::string &id)
    {
        auto el = dynamic_cast<T*>(m_rocketContext->CreateDocument(id.c_str()));
        RocketRefWrapper<T> result(el);
        el->RemoveReference();
        return el;
    }

    tb::TBRenderer* getRenderer();
    tb::TBSkin* getSkin();
    tb::TBWidgetsReader* getWidgetsReader();
    tb::TBLanguage* getLang();
    tb::TBFontManager* getFontManager();
};

}
