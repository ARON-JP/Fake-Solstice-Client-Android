#pragma once
//
// Slim stub of Solstice's GuiData. The real one read these from Minecraft's
// memory via offsets; here they are plain fields set to the overlay resolution.
//
#include <glm/glm.hpp>
#include <string>

class GuiData {
public:
    glm::vec2 mResolution = { 1920.f, 1080.f };
    glm::vec2 mResolutionRounded = { 1920.f, 1080.f };
    glm::vec2 mResolutionScaled = { 1920.f, 1080.f };
    float mGuiScale = 4.f;
    float mScalingMultiplier = 1.f;

    void displayClientMessageQueued(const std::string&) {}
    void displayClientMessage(const std::string&) {}
};
