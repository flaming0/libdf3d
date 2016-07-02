#include "../AppDelegate.h"
#include "JNIHelpers.h"
#include <libdf3d/base/EngineController.h>
#include <libdf3d/platform/android/FileDataSourceAndroid.h>
#include <libdf3d/input/InputManager.h>
#include <libdf3d/input/InputEvents.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

struct AndroidAppState
{
    JavaVM *javaVm = nullptr;

    unique_ptr<df3d::AppDelegate> appDelegate;

    df3d::TouchID primaryTouchId = df3d::Touch::INVALID_ID;
};

static unique_ptr<AndroidAppState> g_appState;

namespace df3d {

void Application::setupDelegate(unique_ptr<AppDelegate> appDelegate)
{
    g_appState->appDelegate = std::move(appDelegate);
}

void Application::setTitle(const std::string &title)
{

}

}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env;
    if (vm->GetEnv((void **)(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        DFLOG_CRITICAL("Failed to get environment");
        return -1;
    }

    g_appState.reset(new AndroidAppState());
    g_appState->javaVm = vm;

    df3d::AndroidServices::init(vm);

    DFLOG_MESS("JNI_OnLoad success");



    if (!g_appState->appDelegate1)
    {
        return -1;
    }

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_setAssetManager(JNIEnv* env, jobject cls, jobject assetManager)
{
    df3d::platform_impl::FileDataSourceAndroid::setAssetManager(AAssetManager_fromJava(env, assetManager));
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_servicesInitialized(JNIEnv* env, jobject cls, jobject services)
{
    df3d::AndroidServices::setServicesObj(services);
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_init(JNIEnv* env, jclass cls, jint jscreenWidth, jint jscreenHeight)
{
    DFLOG_MESS("Doing native init...");

    if (!g_appState->engine)
    {
        g_appState->engine.reset(new df3d::EngineController());

        auto engineInitParams = g_appState->appDelegate->getInitParams();
        engineInitParams.windowWidth = jscreenWidth;
        engineInitParams.windowHeight = jscreenHeight;

        g_appState->engine->initialize(engineInitParams);
        if (!g_appState->appDelegate->onAppStarted())
            throw std::runtime_error("Game code initialization failed.");
    }
    else
    {
        g_appState->appDelegate->onRenderRecreated();
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_onResume(JNIEnv* env, jclass cls)
{
    g_appState->appDelegate->onAppWillEnterForeground();
    g_appState->appDelegate->onAppDidBecomeActive();
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_onPause(JNIEnv* env, jclass cls)
{
    g_appState->appDelegate->onAppWillResignActive();
    g_appState->appDelegate->onAppDidEnterBackground();
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_onDestroy(JNIEnv* env, jclass cls)
{
    DFLOG_MESS("Activity is being destroyed");

    g_appState->appDelegate->onAppEnded();

    g_appState->engine->shutdown();
    g_appState.reset();
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_onSurfaceDestroyed(JNIEnv* env, jclass cls)
{
    if (g_appState)
        g_appState->appDelegate->onRenderDestroyed();
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_draw(JNIEnv* env, jclass cls)
{
    g_appState->engine->step();
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_onTouchDown(JNIEnv* env, jclass cls, jint pointerId, jfloat x, jfloat y)
{
    df3d::svc().inputManager().onTouch(pointerId, x, y, df3d::Touch::State::DOWN);

    if (g_appState->primaryTouchId == df3d::Touch::INVALID_ID)
    {
        // DFLOG_DEBUG("Primary touch down %d, %d", (int)x, (int)y);

        df3d::svc().inputManager().onMouseButtonPressed(df3d::MouseButton::LEFT, x, y);

        g_appState->primaryTouchId = pointerId;
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_onTouchUp(JNIEnv* env, jclass cls, jint pointerId, jfloat x, jfloat y)
{
    df3d::svc().inputManager().onTouch(pointerId, x, y, df3d::Touch::State::UP);

    if (pointerId == g_appState->primaryTouchId)
    {
        // DFLOG_DEBUG("Primary touch up %d, %d", (int)x, (int)y);

        df3d::svc().inputManager().onMouseButtonReleased(df3d::MouseButton::LEFT, x, y);

        g_appState->primaryTouchId = df3d::Touch::INVALID_ID;
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_onTouchMove(JNIEnv* env, jclass cls, jint pointerId, jfloat x, jfloat y)
{
    df3d::svc().inputManager().onTouch(pointerId, x, y, df3d::Touch::State::MOVING);

    if (pointerId == g_appState->primaryTouchId)
    {
        //df3d::glog << "Touch MOVE" << x << y << pointerId << df3d::logcritical;

        df3d::svc().inputManager().setMousePosition(x, y);
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_flaming0_df3d_NativeBindings_onTouchCancel(JNIEnv* env, jclass cls, jint pointerId, jfloat x, jfloat y)
{
    df3d::svc().inputManager().onTouch(pointerId, x, y, df3d::Touch::State::CANCEL);

    if (pointerId == g_appState->primaryTouchId)
    {
        //df3d::glog << "Touch CANCEL" << x << y << pointerId << df3d::logcritical;

        df3d::svc().inputManager().onMouseButtonReleased(df3d::MouseButton::LEFT, x, y);

        g_appState->primaryTouchId = df3d::Touch::INVALID_ID;
    }
}
