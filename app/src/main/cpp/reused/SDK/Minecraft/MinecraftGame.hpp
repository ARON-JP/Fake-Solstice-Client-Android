#pragma once
//
// Slim stub of Solstice's MinecraftGame. Only playUi is referenced (ToggleSounds
// "Lever" sound). There is no game audio engine here, so it's a no-op; the other
// ToggleSounds options play real .wav files via SoundUtils.
//
#include <string>

class MinecraftGame {
public:
    void playUi(const std::string& /*sound*/, float /*volume*/, float /*pitch*/) {}
};
