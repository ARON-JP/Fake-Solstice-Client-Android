//
// FakeClient implementation of FeatureManager::init/shutdown.
//
// The original also spun up commands, configs, auth, IRC, etc. Here we only need
// the event dispatcher and the module manager so the reused ClickGUI/HUD work.
//
#include "pch.hpp"
#include <Features/FeatureManager.hpp>

void FeatureManager::init()
{
    mDispatcher = std::make_unique<nes::event_dispatcher>();
    mModuleManager = std::make_shared<ModuleManager>();
    mModuleManager->init(); // constructs + registers all (fake) modules
}

void FeatureManager::shutdown()
{
    mModuleManager.reset();
    mDispatcher.reset();
}
