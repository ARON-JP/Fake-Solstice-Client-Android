#pragma once
//
// Slim stub of Solstice's generated ResourceLoader.
//
// The original embedded fonts/images into the binary at build time. FakeClient
// loads the same TTFs from disk at runtime (see ResourcesStub.cpp) and registers
// them under the keys FontHelper::getFont expects.
//
class ResourceLoader {
public:
    static void loadResources();
};
