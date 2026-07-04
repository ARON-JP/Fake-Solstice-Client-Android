#pragma once
//
// Android replacement for fakeclient/src/Overlay.{hpp,cpp}.
//
// The desktop build hosted ImGui on a D3D11 + DirectComposition window. Here we
// host the exact same ImGui pipeline (NewFrame -> RenderEvent -> Render) on an
// EGL + OpenGL ES 3.0 surface backed by the overlay SurfaceView's ANativeWindow.
// Everything below the ImGui layer (the reused ClickGUI / HUD drawing code) is
// untouched and renders into ImGui's background draw list as before.
//
// All methods are intended to be called from a single (the UI) thread.
//
#include <EGL/egl.h>
#include <GLES3/gl3.h>

struct ANativeWindow;

class AndroidRenderer {
public:
    static inline AndroidRenderer* sInstance = nullptr;

    // Create the EGL context/surface for a freshly-created Surface and bring up
    // the ImGui OpenGL3 backend. ImGui context + fonts are created once in
    // Wiring/native init, so this can run again after the surface is recreated.
    bool onSurfaceCreated(ANativeWindow* window);
    void onSurfaceChanged(int width, int height);
    void onSurfaceDestroyed();

    // One full frame: ImGui NewFrame -> RenderEvent (HUD/ClickGUI) -> Render ->
    // swap. Background is cleared fully transparent so only the GUI is visible
    // over whatever app is underneath.
    void renderFrame();

    int width()  const { return mWidth; }
    int height() const { return mHeight; }
    bool valid() const { return mDisplay != EGL_NO_DISPLAY && mSurface != EGL_NO_SURFACE; }

private:
    bool initEgl();
    void destroyEgl();

    ANativeWindow* mWindow  = nullptr;
    EGLDisplay     mDisplay = EGL_NO_DISPLAY;
    EGLContext     mContext = EGL_NO_CONTEXT;
    EGLSurface     mSurface = EGL_NO_SURFACE;
    EGLConfig      mConfig  = nullptr;
    int            mWidth   = 0;
    int            mHeight  = 0;
    bool           mBackendInit = false;
};
