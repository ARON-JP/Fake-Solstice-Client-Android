#pragma once
//
// Slim stub of Solstice's SoundUtils. The original played WAVs embedded in the
// DLL; here we play the same files from disk (resources/<name>) using winmm.
//
#include <string>

class SoundUtils {
public:
    static void playSoundFromEmbeddedResource(const std::string& name, float volume);
};
