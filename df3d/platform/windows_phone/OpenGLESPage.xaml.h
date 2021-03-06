﻿#pragma once

#include <platform/windows_phone/OpenGLESPage.g.h>
#include <concurrent_queue.h>
#include <angle_gl.h>
#include <angle_windowsstore.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <base/MacroDefs.h>

FWD_MODULE_CLASS(platform, AppDelegate)

class OpenGLES;

// FIXME:
// All C++\CX classes should be in one namespace.
// https://social.msdn.microsoft.com/Forums/windowsapps/en-US/d0b940d2-1c4e-4f28-9130-b45067088aa1/vs2013-update-2-wp-81-arm-deploying-to-device-problem-dep6810-in-release-mode?forum=wpdevelop

namespace df3d_winrt
{
    enum class TouchType
    {
        TOUCH_DOWN,
        TOUCH_MOVED,
        TOUCH_UP
    };

    class TouchEvent
    {
    public:
        TouchEvent(TouchType type, Windows::UI::Core::PointerEventArgs^ args)
        {
            this->type = type;
            this->x = args->CurrentPoint->Position.X;
            this->y = args->CurrentPoint->Position.Y;
        }

        TouchType type;
        float x, y;
    };

    public ref class OpenGLESPage sealed
    {
    public:
        OpenGLESPage();
        virtual ~OpenGLESPage();

    internal:
        OpenGLESPage(df3d::platform::AppDelegate *appDelegate);

    private:
        void OnPageLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
        void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
        void GetSwapChainPanelSize(int *width, int *height);
        void CreateRenderSurface();
        void DestroyRenderSurface();
        void RecoverFromLostDevice();
        void StartRenderLoop();
        void StopRenderLoop();

        std::unique_ptr<OpenGLES> mOpenGLES;
        df3d::platform::AppDelegate* m_appDelegate;
        bool m_appInitialized;

        Windows::Foundation::Size mSwapChainPanelSize;
        Concurrency::critical_section mSwapChainPanelSizeCriticalSection;

        Windows::Foundation::Size mCustomRenderSurfaceSize;
        bool mUseCustomRenderSurfaceSize;

        EGLSurface mRenderSurface;     // This surface is associated with a swapChainPanel on the page
        Concurrency::critical_section mRenderSurfaceCriticalSection;
        Windows::Foundation::IAsyncAction^ mRenderLoopWorker;

        // Track user input on a background worker thread.
        Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
        Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;
        Concurrency::concurrent_queue<std::shared_ptr<TouchEvent>> pointers;
        void ProcessPointers();
        void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
    };
}
