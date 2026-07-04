//
// See AndroidRenderer.hpp. EGL/GLES3 host for the reused ImGui GUI/HUD.
//
#include "pch.hpp"   // force-included too; the guard makes this a no-op
#include "AndroidRenderer.hpp"

#include <android/native_window.h>
#include <android/log.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <functional>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "FakeClient", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "FakeClient", __VA_ARGS__)

// Provided by native-lib.cpp: drives the RenderEvent through the reused module
// system (equivalent to the desktop Overlay's mOnRender callback).
extern std::function<void()> g_OnRender;

bool AndroidRenderer::onSurfaceCreated(ANativeWindow* window) {
    sInstance = this;
    mWindow = window;
    if (!initEgl()) {
        LOGE("EGL init failed");
        return false;
    }

    // (Re)create the GL-side ImGui backend for this context. The ImGui context
    // itself + fonts are owned by native init / Wiring and survive across
    // surface recreation.
    ImGui_ImplOpenGL3_Init("#version 300 es");
    mBackendInit = true;
    LOGI("surface created %dx%d", mWidth, mHeight);
    return true;
}

void AndroidRenderer::onSurfaceChanged(int width, int height) {
    mWidth = width;
    mHeight = height;
    fc_shim::gDisplayW = width;
    fc_shim::gDisplayH = height;
    if (ImGui::GetCurrentContext())
        ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
}

void AndroidRenderer::onSurfaceDestroyed() {
    if (mBackendInit) {
        ImGui_ImplOpenGL3_Shutdown();
        mBackendInit = false;
    }
    destroyEgl();
    if (mWindow) {
        ANativeWindow_release(mWindow);
        mWindow = nullptr;
    }
    if (sInstance == this) sInstance = nullptr;
}

bool AndroidRenderer::initEgl() {
    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mDisplay == EGL_NO_DISPLAY) return false;
    if (!eglInitialize(mDisplay, nullptr, nullptr)) return false;

    const EGLint cfgAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 8,   // per-pixel alpha so the overlay can be transparent
        EGL_DEPTH_SIZE, 0,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };
    EGLint numConfigs = 0;
    if (!eglChooseConfig(mDisplay, cfgAttribs, &mConfig, 1, &numConfigs) || numConfigs < 1)
        return false;

    EGLint format = 0;
    eglGetConfigAttrib(mDisplay, mConfig, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(mWindow, 0, 0, format);

    const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, ctxAttribs);
    if (mContext == EGL_NO_CONTEXT) return false;

    mSurface = eglCreateWindowSurface(mDisplay, mConfig, mWindow, nullptr);
    if (mSurface == EGL_NO_SURFACE) return false;

    if (!eglMakeCurrent(mDisplay, mSurface, mSurface, mContext)) return false;

    eglQuerySurface(mDisplay, mSurface, EGL_WIDTH,  &mWidth);
    eglQuerySurface(mDisplay, mSurface, EGL_HEIGHT, &mHeight);
    fc_shim::gDisplayW = mWidth;
    fc_shim::gDisplayH = mHeight;
    return true;
}

void AndroidRenderer::destroyEgl() {
    if (mDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (mSurface != EGL_NO_SURFACE) eglDestroySurface(mDisplay, mSurface);
        if (mContext != EGL_NO_CONTEXT) eglDestroyContext(mDisplay, mContext);
        eglTerminate(mDisplay);
    }
    mDisplay = EGL_NO_DISPLAY;
    mContext = EGL_NO_CONTEXT;
    mSurface = EGL_NO_SURFACE;
}

void AndroidRenderer::renderFrame() {
    if (!valid() || !ImGui::GetCurrentContext()) return;

    eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)mWidth, (float)mHeight);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (g_OnRender) g_OnRender();

    ImGui::Render();

    glViewport(0, 0, mWidth, mHeight);
    glClearColor(0.f, 0.f, 0.f, 0.f);   // fully transparent overlay
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    eglSwapBuffers(mDisplay, mSurface);
}
