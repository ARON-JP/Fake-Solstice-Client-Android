//
// FakeClient Android - JNI bridge + glue (the Android counterpart of
// fakeclient/src/main.cpp + Wiring.cpp).
//
// Responsibilities:
//   * own the ImGui context, fonts and the reused FeatureManager/module system,
//   * forward the overlay SurfaceView lifecycle to AndroidRenderer (EGL/GLES3),
//   * translate Android touch gestures + hardware mouse/keyboard into the same
//     ImGui mouse state and FakeInput::onKey path the desktop client used, so
//     ClickGUI binds/toggles behave exactly like Windows.
//
// All entry points are invoked on the Android UI thread (see OverlayService),
// which is also where rendering happens, so no locking is required.
//
#include "pch.hpp"

#include "AndroidRenderer.hpp"
#include "FakeInput.hpp"
#include "Wiring.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Events/RenderEvent.hpp>
#include <Features/Events/KeyEvent.hpp>
#include <Features/Modules/Module.hpp>
#include <Features/Modules/Visual/ClickGui.hpp>
#include <Utils/FontHelper.hpp>
#include <Utils/Keyboard.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Rendering/GuiData.hpp>

#include <imgui.h>

#include <jni.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include <chrono>
#include <functional>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "FakeClient", __VA_ARGS__)

// ---- globals shared with the other native TUs -------------------------------
AAssetManager*        g_AssetManager = nullptr;   // used by Stubs_Game_android.cpp
std::function<void()> g_OnRender;                 // used by AndroidRenderer.cpp

static AndroidRenderer g_renderer;
static bool            g_inited = false;

static long nowMs() {
    using namespace std::chrono;
    return (long)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

// ===================== Wiring (reused-system driver) =========================
void Wiring::initialize() {
    FontHelper::load();                       // fonts -> atlas (from assets)
    gFeatureManager = std::make_shared<FeatureManager>();
    gFeatureManager->init();                  // registers the authentic module list
}

void Wiring::renderFrame() {
    if (!gFeatureManager) return;

    // Keep fake GuiData resolution in sync with the surface so ClickGUI layout
    // and getScreenSize() match the overlay size.
    ClientInstance::get()->getGuiData()->mResolution =
        glm::vec2((float)g_renderer.width(), (float)g_renderer.height());

    gFeatureManager->mModuleManager->onClientTick();   // apply queued toggles

    auto holder = nes::make_holder<RenderEvent>();
    gFeatureManager->mDispatcher->trigger(holder);
}

void Wiring::shutdown() {
    if (gFeatureManager) gFeatureManager->shutdown();
    gFeatureManager.reset();
}

// ===================== FakeInput (host -> event system) ======================
void FakeInput::toggleClickGui() {
    if (!gFeatureManager) return;
    if (auto* cg = gFeatureManager->mModuleManager->getModule<ClickGui>())
        cg->toggle();
}

void FakeInput::onKey(int vk, bool down) {
    if (!gFeatureManager || !gFeatureManager->mDispatcher) return;

    if (vk >= 0 && vk < 256) fc_shim::gKeyState[vk] = down ? 1 : 0;
    Keyboard::mPressedKeys[vk] = down;

    auto holder = nes::make_holder<KeyEvent>(vk, down);
    gFeatureManager->mDispatcher->trigger<KeyEvent>(holder);
    if (holder->mCancelled) return;

    for (auto& module : gFeatureManager->mModuleManager->getModules()) {
        if (vk != 0 && module->mKey == vk) {
            if (module->mEnableWhileHeld) module->mWantedState = down;
            else if (down)                module->toggle();
        }
    }
}

static bool isGuiOpen() {
    if (!gFeatureManager) return false;
    auto* cg = gFeatureManager->mModuleManager->getModule<ClickGui>();
    return cg && cg->mEnabled;
}

// ===================== touch gesture -> ImGui mouse ==========================
// One primary pointer drives the mouse. Mapping (mirrors the desktop mouse):
//   * quick tap            -> left click   (toggle module / press button)
//   * drag                 -> left button held + move (sliders, window drag)
//   * long press (>450ms)  -> right click  (expand module settings)
//   * two-finger tap       -> middle click (start keybind capture)
namespace {
    enum class Phase { None, Pending, Drag, Consumed };
    Phase  gPhase = Phase::None;
    float  gStartX = 0, gStartY = 0, gCurX = 0, gCurY = 0;
    long   gDownTime = 0;
    const float kMoveThresh = 14.f;
    const long  kLongPressMs = 450;

    void addPos(float x, float y) {
        fc_shim::gCursorX = (int)x; fc_shim::gCursorY = (int)y;
        ImGui::GetIO().AddMousePosEvent(x, y);
    }
    void clickButton(int btn) {  // queued down+up -> recognised as a click (input trickling)
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(btn, true);
        io.AddMouseButtonEvent(btn, false);
    }
}

// Called once per frame from render: realise a long-press while the finger is
// still held (there is no touch event at that moment).
static void updateGesture() {
    if (gPhase == Phase::Pending && (nowMs() - gDownTime) >= kLongPressMs) {
        addPos(gCurX, gCurY);
        clickButton(1);            // right click -> expand settings
        gPhase = Phase::Consumed;
    }
}

// Android MotionEvent.actionMasked values.
enum { A_DOWN = 0, A_UP = 1, A_MOVE = 2, A_CANCEL = 3, A_PTR_DOWN = 5, A_PTR_UP = 6 };

static void onTouch(int action, float x, float y, int pointerCount) {
    ImGuiIO& io = ImGui::GetIO();
    switch (action) {
        case A_DOWN:
            gPhase = Phase::Pending;
            gStartX = gCurX = x; gStartY = gCurY = y;
            gDownTime = nowMs();
            addPos(x, y);
            break;
        case A_MOVE:
            gCurX = x; gCurY = y;
            addPos(x, y);
            if (gPhase == Phase::Pending) {
                float dx = x - gStartX, dy = y - gStartY;
                if (dx*dx + dy*dy > kMoveThresh*kMoveThresh) {
                    gPhase = Phase::Drag;
                    io.AddMouseButtonEvent(0, true);   // begin left drag
                }
            }
            break;
        case A_PTR_DOWN:
            if (pointerCount >= 2 && gPhase != Phase::Drag) {
                addPos(gCurX, gCurY);
                clickButton(2);            // middle click -> keybind capture
                gPhase = Phase::Consumed;
            }
            break;
        case A_UP:
            if (gPhase == Phase::Pending)      clickButton(0);          // tap
            else if (gPhase == Phase::Drag)    io.AddMouseButtonEvent(0, false);
            gPhase = Phase::None;
            break;
        case A_CANCEL:
            if (gPhase == Phase::Drag) io.AddMouseButtonEvent(0, false);
            gPhase = Phase::None;
            break;
        default: break;
    }
}

// ===================== JNI exports ==========================================
extern "C" {

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeInit(JNIEnv* env, jobject, jobject assetMgr) {
    if (g_inited) return;
    g_AssetManager = AAssetManager_fromJava(env, assetMgr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;                 // no imgui.ini on Android
    io.DisplaySize = ImVec2(1920.f, 1080.f);
    io.MouseDrawCursor = false;
    ImGui::StyleColorsDark();

    Wiring::initialize();

    g_OnRender = []() { Wiring::renderFrame(); };
    g_inited = true;
    LOGI("native init done");
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeShutdown(JNIEnv*, jobject) {
    if (!g_inited) return;
    Wiring::shutdown();
    if (ImGui::GetCurrentContext()) ImGui::DestroyContext();
    g_inited = false;
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeSurfaceCreated(JNIEnv* env, jobject, jobject surface) {
    ANativeWindow* win = ANativeWindow_fromSurface(env, surface);
    g_renderer.onSurfaceCreated(win);
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeSurfaceChanged(JNIEnv*, jobject, jint w, jint h) {
    g_renderer.onSurfaceChanged(w, h);
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeSurfaceDestroyed(JNIEnv*, jobject) {
    g_renderer.onSurfaceDestroyed();
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeRender(JNIEnv*, jobject) {
    updateGesture();
    g_renderer.renderFrame();
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeTouch(JNIEnv*, jobject, jint action, jfloat x, jfloat y, jint pointerCount) {
    onTouch(action, x, y, pointerCount);
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeMouseMove(JNIEnv*, jobject, jfloat x, jfloat y) {
    addPos(x, y);
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeMouseButton(JNIEnv*, jobject, jint button, jboolean down, jfloat x, jfloat y) {
    addPos(x, y);
    ImGui::GetIO().AddMouseButtonEvent(button, down == JNI_TRUE);   // 0=L 1=R 2=M
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeScroll(JNIEnv*, jobject, jfloat dx, jfloat dy) {
    ImGui::GetIO().AddMouseWheelEvent(dx, dy);
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeKey(JNIEnv*, jobject, jint vk, jboolean down) {
    FakeInput::onKey(vk, down == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeToggleGui(JNIEnv*, jobject) {
    FakeInput::toggleClickGui();
}

JNIEXPORT jboolean JNICALL
Java_com_solstice_fakeclient_NativeBridge_nativeIsGuiOpen(JNIEnv*, jobject) {
    return isGuiOpen() ? JNI_TRUE : JNI_FALSE;
}

} // extern "C"
