# FakeClient — Android system-overlay port

Android port of the Windows "fake hack" FakeClient. It reuses Solstice's **real**
ClickGUI/HUD drawing code (`app/src/main/cpp/reused/`, copied verbatim) and hosts
it on **EGL + OpenGL ES 3.0** instead of D3D11, drawn as a **system overlay**
(draw-over-other-apps) so it floats above any app — the mobile equivalent of the
desktop overlay window.

Nothing reads or injects into any game. Every value is fake/static, exactly like
the Windows version.

## Features
- On launch you grant the "display over other apps" permission and press **開始**.
- A round floating **icon** appears over everything.
  - **Tap** the icon → open/close the ClickGUI.
  - **Drag** the icon → move it.
  - **Long-press** the icon → show a **✕ 終了** (exit) button to quit the overlay.
- Inside the ClickGUI (touch maps to the desktop mouse):
  - **Tap** = left click (toggle a module / press a button)
  - **Drag** = left-drag (sliders, move category panels)
  - **Long-press** = right click (expand a module's settings)
  - **Two-finger tap** = middle click (start key-bind capture)
  - A connected **USB/Bluetooth keyboard & mouse** work just like Windows
    (real left/right/middle mouse buttons, scroll, and keys drive module
    key-binds via the same code path).
  - The hardware **Back** button closes the GUI.

## Build location ⚠️
Build from an **ASCII path** (this folder, `C:\AndroidProjects\FakeClientAndroid`).
The Android NDK/Ninja toolchain does **not** handle the Japanese characters/spaces
in the original OneDrive project path, which is why this self-contained copy lives
here. A mirror of the sources is also kept in the repo under `fakeclient-android/`
for reference, but do not build from there.

## Build
Toolchain already installed on this machine:
- Android SDK: `C:\Android\Sdk` (platform-34, build-tools 34, **NDK 26.3.11579264**, cmake 3.22.1)
- JDK 17: `C:\Android\jdk17\jdk-17.0.19+10` (Gradle runs on it; see `gradle.properties`)

```sh
cd C:/AndroidProjects/FakeClientAndroid
JAVA_HOME="C:/Android/jdk17/jdk-17.0.19+10" ANDROID_HOME="C:/Android/Sdk" \
  ./gradlew :app:assembleDebug
```
First configure fetches glm / nlohmann-json / spdlog via CPM (needs internet).
Output: `app/build/outputs/apk/debug/app-debug.apk` (arm64-v8a).

Or open this folder in **Android Studio** (it will use the SDK/NDK above) and press Run.

## Install / run on a device
```sh
C:/Android/Sdk/platform-tools/adb install -r app/build/outputs/apk/debug/app-debug.apk
```
Then open **FakeClient**, tap **オーバーレイ権限を許可** and enable it in Settings,
return and tap **オーバーレイを開始**. The floating icon appears.

> Built for **arm64-v8a** only (covers essentially all real phones). To run on an
> x86_64 emulator, add `'x86_64'` to `abiFilters` in `app/build.gradle` and rebuild.

## How the port is structured
```
app/src/main/
  cpp/
    native-lib.cpp          JNI bridge + Wiring + FakeInput + touch→mouse gestures
    AndroidRenderer.*        EGL/GLES3 host (replaces the desktop D3D11 Overlay)
    Stubs_Game_android.cpp   fake ClientInstance + fonts loaded from APK assets
    shim/Windows.h           VK_* constants, __int16/sprintf_s, GetAsyncKeyState…
    shim/d3d11.h             opaque D3D handles so D3DHook.hpp/Watermark compile
    backends/imgui_impl_opengl3.*   Dear ImGui GLES3 backend (v1.89.8)
    reused/                  Solstice ClickGUI/HUD — copied VERBATIM from fakeclient/reused
    solstice_src/            FakeClient glue copied from fakeclient/src
  java/com/solstice/fakeclient/
    MainActivity.kt          permission + start/stop UI
    OverlayService.kt        floating icon + ClickGUI SurfaceView + render loop
    NativeBridge.kt          JNI declarations
    KeyMap.kt                Android keycode → Win32 VK
  assets/fonts/              the shipped TTFs
../thirdparty/               vendored ImGui / entt / nes (copied from repo include/)
```

The reused C++ is unmodified; the only desktop-source change is a 2-line
portability guard in `fakeclient/src/pch.hpp` (skips the MSVC-only
`corecrt_math_defines.h` on non-MSVC). `__forceinline` is mapped to `inline` by
CMake; the remaining MSVC-isms are absorbed by `shim/Windows.h`.

## Known thing to verify on-device
SurfaceView transparency in a system overlay uses `setZOrderOnTop(true)` +
`PixelFormat.TRANSLUCENT`. On most devices the GUI composites over the app below
with a transparent background. If a black background appears instead, switch
`setZOrderOnTop(true)` to `setZOrderMediaOverlay(true)` in `OverlayService.kt`.
