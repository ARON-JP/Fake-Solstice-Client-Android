#pragma once
//
// Slim stub of Solstice's D2D blur helper.
//
// The original used Direct2D + D3D11On12 interop on the GAME's swapchain to blur
// behind the ClickGUI. FakeClient renders to its own swapchain, and a backdrop
// blur is purely cosmetic, so these are no-ops (the GUI simply isn't blurred).
// Signatures match ImRenderUtils' call sites.
//
#include <optional>
#include <imgui.h>

class D2D {
public:
    static bool addBlur(ImDrawList* /*drawList*/, float /*strength*/, std::optional<ImVec4> /*clipRect*/, float /*rounding*/) { return false; }
    static bool addBlurOptimized(ImDrawList* /*drawList*/, float /*strength*/, std::optional<ImVec4> /*clipRect*/, float /*rounding*/) { return false; }
};
