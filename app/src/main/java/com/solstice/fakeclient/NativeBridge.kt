package com.solstice.fakeclient

import android.content.res.AssetManager
import android.view.Surface

/**
 * Thin JNI surface onto libfakeclient.so (the reused Solstice ClickGUI/HUD code
 * hosted on EGL + OpenGL ES). Every method is expected to be called on the same
 * (UI) thread, which also drives rendering, so no native-side locking is needed.
 */
object NativeBridge {
    init { System.loadLibrary("fakeclient") }

    // Lifecycle.
    external fun nativeInit(assets: AssetManager)
    external fun nativeShutdown()

    // Surface lifecycle (from the ClickGUI SurfaceView).
    external fun nativeSurfaceCreated(surface: Surface)
    external fun nativeSurfaceChanged(width: Int, height: Int)
    external fun nativeSurfaceDestroyed()
    external fun nativeRender()

    // Input. Touch is forwarded raw; the gesture->mouse mapping lives in native.
    external fun nativeTouch(action: Int, x: Float, y: Float, pointerCount: Int)
    external fun nativeMouseMove(x: Float, y: Float)
    external fun nativeMouseButton(button: Int, down: Boolean, x: Float, y: Float)
    external fun nativeScroll(dx: Float, dy: Float)
    external fun nativeKey(vk: Int, down: Boolean)

    // ClickGUI control.
    external fun nativeToggleGui()
    external fun nativeIsGuiOpen(): Boolean
}
