#pragma once
//
// Slim stub of Solstice's ProcUtils. The reused code only calls
// getMinecraftWindow() (in ImRenderUtils::isFullScreen). We return the overlay
// window handle, which the host registers at startup.
//
#include <Windows.h>
#include <string>

class ProcUtils {
public:
    static inline HWND sOverlayWindow = nullptr;

    static HWND getMinecraftWindow() { return sOverlayWindow ? sOverlayWindow : GetDesktopWindow(); }
    static std::string getVersion() { return "0.0.0.0"; }
    static int getModuleCount() { return 0; }
};
