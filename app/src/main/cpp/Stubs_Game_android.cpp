//
// Android replacement for fakeclient/src/stubs/Stubs_Game.cpp.
//
// Same role: provide the fake ClientInstance/GuiData and load the shipped TTF
// fonts. The only differences from the desktop version are platform plumbing:
//   * fonts are read from the APK's assets (AAssetManager) into memory and
//     registered with AddFontFromMemoryTTF, instead of from disk, and
//   * sound playback is a no-op (no winmm; audio is inessential to the prop).
//
#include "pch.hpp"

#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Rendering/GuiData.hpp>
#include <SDK/Minecraft/MinecraftGame.hpp>
#include <Utils/Resources.hpp>
#include <Utils/FontHelper.hpp>
#include <Utils/SoundUtils.hpp>

#include <android/asset_manager.h>
#include <android/log.h>
#include <vector>
#include <string>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "FakeClient", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "FakeClient", __VA_ARGS__)

// Set by native-lib.cpp on nativeInit (AAssetManager_fromJava).
extern AAssetManager* g_AssetManager;

// ---- shared fake game state -------------------------------------------------
namespace FakeState {
    GuiData gGuiData;
    bool    gMouseGrabbed = false;
}

ClientInstance* ClientInstance::get() {
    static ClientInstance instance;
    return &instance;
}
MinecraftGame* ClientInstance::getMinecraftGame() {
    static MinecraftGame game;
    return &game;
}
GuiData* ClientInstance::getGuiData()       { return &FakeState::gGuiData; }
bool     ClientInstance::getMouseGrabbed()  { return FakeState::gMouseGrabbed; }
void     ClientInstance::grabMouse()        { FakeState::gMouseGrabbed = true; }
void     ClientInstance::releaseMouse()     { FakeState::gMouseGrabbed = false; }
std::string ClientInstance::getScreenName() { return "hud_screen"; }
void     ClientInstance::playUi(const std::string&, float, float) {}

// ---- sound (no-op on Android) ----------------------------------------------
void SoundUtils::playSoundFromEmbeddedResource(const std::string&, float) {}

// ---- font loading from assets ----------------------------------------------
namespace {
    // Keeps font blobs alive for the lifetime of the atlas (we register them with
    // FontDataOwnedByAtlas = false so ImGui doesn't free them).
    std::vector<std::vector<unsigned char>> gFontBlobs;

    bool readAsset(const std::string& path, std::vector<unsigned char>& out) {
        if (!g_AssetManager) return false;
        AAsset* a = AAssetManager_open(g_AssetManager, path.c_str(), AASSET_MODE_BUFFER);
        if (!a) return false;
        off_t len = AAsset_getLength(a);
        out.resize((size_t)len);
        int read = AAsset_read(a, out.data(), (size_t)len);
        AAsset_close(a);
        return read == (int)len && len > 0;
    }

    void addFont(const std::string& file, const std::string& key) {
        ImGuiIO& io = ImGui::GetIO();
        gFontBlobs.emplace_back();
        auto& blob = gFontBlobs.back();
        if (!readAsset("fonts/" + file, blob)) {
            LOGE("font asset missing: %s", file.c_str());
            gFontBlobs.pop_back();
            return;
        }
        ImFontConfig cfg;
        cfg.FontDataOwnedByAtlas = false;   // we own the blob (kept in gFontBlobs)
        FontHelper::Fonts[key] =
            io.Fonts->AddFontFromMemoryTTF(blob.data(), (int)blob.size(), 20.f, &cfg);
        FontHelper::Fonts[key + "_large"] =
            io.Fonts->AddFontFromMemoryTTF(blob.data(), (int)blob.size(), 42.f, &cfg);
    }
}

void ResourceLoader::loadResources() {
    ImGuiIO& io = ImGui::GetIO();
    if (!FontHelper::Fonts.empty()) return;

    io.Fonts->AddFontDefault();

    addFont("Product-Sans.ttf",      "product_sans");
    addFont("Product-Sans-Bold.ttf", "product_sans_bold");
    addFont("Mojangles.ttf",         "mojangles");
    addFont("Mojangles-Bold.ttf",    "mojangles_bold");

    io.Fonts->Build();
    LOGI("fonts loaded (%zu entries)", FontHelper::Fonts.size());
}
