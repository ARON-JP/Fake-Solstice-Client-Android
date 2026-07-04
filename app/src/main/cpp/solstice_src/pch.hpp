//
// Standalone precompiled / force-included header for FakeClient.
//
// Mirrors the essential parts of Solstice's src/pch.hpp that the reused GUI/HUD
// code relies on (the nes priority enum, glm, ImGui with math operators, json),
// but drops everything tied to injection (MinHook, libhat, d3d11on12, kiero).
//
#ifndef FAKECLIENT_PCH_HPP
#define FAKECLIENT_PCH_HPP

// nes (Nuvola Event System) lets the project override its priority enum. The
// reused modules reference nes::event_priority::{FIRST,NORMAL,LAST,...}, so we
// must define the same enum BEFORE including the dispatcher.
enum struct EventPriorities {
    ABSOLUTE_FIRST,
    VERY_FIRST,
    FIRST,
    KINDA_FIRST,
    NORMAL,
    KINDA_LAST,
    LAST,
    VERY_LAST,
    ABSOLUTE_LAST,
};
#define NES_PRIORITY_TYPE EventPriorities

// corecrt_math_defines.h is MSVC-only; on other toolchains (the Android NDK /
// clang port) M_PI & friends come from <cmath>/<math.h> once _USE_MATH_DEFINES
// is set. Guarded so the desktop MSVC build is unaffected.
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#if defined(_MSC_VER)
#include <corecrt_math_defines.h>
#endif
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <set>
#include <map>
#include <unordered_map>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <array>
#include <functional>
#include <cmath>
#include <cfloat>
#include <cstdint>
#include <limits>

#include <Windows.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// IMGUI_DEFINE_MATH_OPERATORS is defined globally by CMake so ImGui's own TUs
// and the reused code agree on the ImVec2/ImVec4 operators.
#include <imgui.h>
#include <imgui_internal.h>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <nes/event_dispatcher.hpp>

// Light, frequently-assumed-preincluded headers from the reused code. These all
// resolve inside fakeclient/reused and pull in no game/SDK dependencies.
#include <Utils/MiscUtils/EasingUtil.hpp>
#include <Utils/MiscUtils/MathUtils.hpp>
#include <Utils/MiscUtils/ColorUtils.hpp>
#include <Utils/MiscUtils/ImRenderUtils.hpp>
#include <Utils/FontHelper.hpp>
#include <Features/Events/RenderEvent.hpp>
#include <Features/Events/WindowResizeEvent.hpp>
#include <Features/Events/MouseEvent.hpp>
#include <Features/Events/KeyEvent.hpp>
#include <Features/Events/ModuleStateChangeEvent.hpp>
#include <Features/FeatureManager.hpp>

#endif // FAKECLIENT_PCH_HPP
