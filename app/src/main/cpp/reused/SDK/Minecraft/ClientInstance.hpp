#pragma once
//
// Slim stub of Solstice's ClientInstance.
//
// The real client resolved this from Minecraft's memory and used it to read the
// local player, level, packet sender, etc. FakeClient touches the game in NO
// way, so this only exposes the handful of members the reused GUI code calls,
// all backed by fake state (see ClientInstanceStub.cpp). mResolution is kept in
// sync with the overlay window by the host.
//
#include <SDK/Minecraft/Rendering/GuiData.hpp>
#include <string>

class MinecraftGame;

class ClientInstance {
public:
    static ClientInstance* get();

    GuiData* getGuiData();
    MinecraftGame* getMinecraftGame();
    bool getMouseGrabbed();
    void grabMouse();
    void releaseMouse();
    std::string getScreenName();
    void playUi(const std::string& soundName, float volume, float pitch);
};
