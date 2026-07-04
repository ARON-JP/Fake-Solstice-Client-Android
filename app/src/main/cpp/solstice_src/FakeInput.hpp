//
// Bridge between the host window and the (reused) module/event system.
// Implemented in Wiring.cpp so the host stays free of game-system includes.
//
#pragma once

namespace FakeInput
{
    // Called by the overlay WndProc on real key down/up. Dispatches a KeyEvent
    // through the nes dispatcher and toggles modules bound to that key, exactly
    // like the original KeyHook did (minus the game-state guards).
    void onKey(int vk, bool down);

    // Toggle the ClickGUI (used by the global Shift+F3 hotkey).
    void toggleClickGui();
}
