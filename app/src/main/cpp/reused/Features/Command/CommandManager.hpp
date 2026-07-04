#pragma once
//
// Slim stub of Solstice's CommandManager.
//
// FakeClient has no chat/command pipeline (that required the game). FeatureManager
// still references the type, so we keep an empty shell to satisfy the include.
//
#include <memory>
#include <vector>

class CommandManager {
public:
    void init() {}
    void shutdown() {}
};
