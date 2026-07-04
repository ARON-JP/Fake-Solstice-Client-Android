//
// Utility stubs: the subset of MathUtils / StringUtils / Logger that the reused
// ClickGUI/HUD code actually references. MathUtils impls are copied verbatim
// from Solstice (they are pure math); the game-coupled MathUtils functions are
// intentionally omitted because nothing here calls them.
//
#include "pch.hpp"

#include <Utils/MiscUtils/MathUtils.hpp>
#include <Utils/StringUtils.hpp>
#include <Utils/Logger.hpp>

#include <algorithm>
#include <cctype>

// ---- MathUtils --------------------------------------------------------------
float MathUtils::animate(float endPoint, float current, float speed) {
    if (speed < 0.0) speed = 0.0;
    else if (speed > 1.0) speed = 1.0;
    float dif = std::fmax(endPoint, current) - std::fmin(endPoint, current);
    float factor = dif * speed;
    return current + (endPoint > current ? factor : -factor);
}

float MathUtils::lerp(float a, float b, float t) { return a + t * (b - a); }
glm::vec3 MathUtils::lerp(glm::vec3& a, glm::vec3& b, float t) { return a + t * (b - a); }
ImVec4 MathUtils::lerp(ImVec4& a, ImVec4& b, float t) {
    return ImVec4(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t));
}
ImVec2 MathUtils::lerp(ImVec2& a, ImVec2& b, float t) {
    return ImVec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}
ImColor MathUtils::lerpImColor(ImColor& a, ImColor& b, float t) {
    return ImColor(lerp(a.Value.x, b.Value.x, t), lerp(a.Value.y, b.Value.y, t),
                   lerp(a.Value.z, b.Value.z, t), lerp(a.Value.w, b.Value.w, t));
}

// ---- StringUtils ------------------------------------------------------------
std::string StringUtils::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return str;
}
std::string StringUtils::toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return (char)std::toupper(c); });
    return str;
}
bool StringUtils::equalsIgnoreCase(const std::string& a, const std::string& b) {
    return toLower(a) == toLower(b);
}

// ---- Logger -----------------------------------------------------------------
// (Logger::initialized is defined inline in Logger.hpp)
void Logger::initialize() { initialized = true; }
void Logger::deinitialize() { initialized = false; }
std::string Logger::getAnsiColor(float, float, float) { return ""; }
std::string Logger::getAnsiColor(int, int, int) { return ""; }
