//
// FakeClient implementation of ModuleManager.
//
// Method bodies mirror Solstice's originals; only init() differs - instead of
// constructing the real (game-coupled) cheat modules, it builds the authentic
// module *list* as inert FakeModules (see ModuleRegistry.cpp). ClickGui and
// Interface are the real reused/slim types so getModule<>() lookups resolve.
//
#include "pch.hpp"
#include <Features/Modules/ModuleManager.hpp>
#include <Utils/StringUtils.hpp>

// Defined in ModuleRegistry.cpp
void registerFakeModules(ModuleManager& mm);

void ModuleManager::init()
{
    registerFakeModules(*this);
}

void ModuleManager::shutdown()
{
    mModules.clear();
}

void ModuleManager::registerModule(const std::shared_ptr<Module>& module)
{
    mModules.push_back(module);
}

std::vector<std::shared_ptr<Module>>& ModuleManager::getModules()
{
    return mModules;
}

Module* ModuleManager::getModule(const std::string& name) const
{
    for (const auto& module : mModules)
        if (StringUtils::equalsIgnoreCase(module->mName, name))
            return module.get();
    return nullptr;
}

void ModuleManager::removeModule(const std::string& name)
{
    for (auto it = mModules.begin(); it != mModules.end(); ++it)
        if (StringUtils::equalsIgnoreCase((*it)->mName, name)) { mModules.erase(it); return; }
}

std::vector<std::shared_ptr<Module>>& ModuleManager::getModulesInCategory(int catId)
{
    static std::unordered_map<int, std::vector<std::shared_ptr<Module>>> categoryMap = {};
    if (categoryMap.contains(catId))
        return categoryMap[catId];

    std::vector<std::shared_ptr<Module>> modules;
    for (const auto& module : mModules)
        if (static_cast<int>(module->mCategory) == catId)
            modules.push_back(module);
    categoryMap[catId] = modules;
    return categoryMap[catId];
}

std::unordered_map<std::string, std::shared_ptr<Module>> ModuleManager::getModuleCategoryMap()
{
    static std::unordered_map<std::string, std::shared_ptr<Module>> map;
    if (!map.empty()) return map;
    for (const auto& module : mModules)
        map[module->getCategory()] = module;
    return map;
}

void ModuleManager::onClientTick()
{
    for (auto& module : mModules)
    {
        if (module->mWantedState != module->mEnabled)
        {
            module->mEnabled = module->mWantedState;
            if (module->mEnabled) module->onEnable();
            else                  module->onDisable();
        }
    }
}
