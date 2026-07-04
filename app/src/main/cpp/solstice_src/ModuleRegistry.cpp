//
// FakeClient module registry.
//
// Populates the ClickGUI with Solstice's authentic module list so the overlay
// looks like the real client. Every entry here is an INERT FakeModule: toggling
// it only flips a bool and (for Visual HUD entries) controls whether the fake
// HUD widget draws. None of them touch a game. The two exceptions are the real
// reused ClickGui module and the slim Interface module, which other reused code
// looks up by type.
//
#include "pch.hpp"

#include <Features/Modules/ModuleManager.hpp>
#include <Features/Modules/Module.hpp>
#include <Features/Modules/Visual/ClickGui.hpp>
#include <Features/Modules/Visual/Interface.hpp>
#include <Features/Modules/Visual/HudEditor.hpp>
#include <Features/Modules/Visual/Watermark.hpp>
#include <Features/Modules/Visual/Arraylist.hpp>
#include <Features/Modules/Visual/Keystrokes.hpp>
#include <Features/Modules/Visual/Notifications.hpp>
#include <Features/Modules/Misc/ToggleSounds.hpp>

#include <cctype>

// A generic, behaviour-free module used for every entry whose only job is to
// appear (and toggle) in the ClickGUI / arraylist.
class FakeModule : public ModuleBase<FakeModule>
{
public:
    FakeModule(const std::string& name, const std::string& description, ModuleCategory category,
               int key = 0, bool enabled = false)
        : ModuleBase(name, description, category, key, enabled, buildNames(name)) {}

private:
    // Build the four naming-style variants ("KillAura" -> "kill aura", etc.).
    static std::unordered_map<NamingStyle, std::string> buildNames(const std::string& raw)
    {
        std::string spaced;
        for (size_t i = 0; i < raw.size(); ++i)
        {
            char c = raw[i];
            if (i > 0 && std::isupper((unsigned char)c) && !std::isupper((unsigned char)raw[i - 1]))
                spaced += ' ';
            spaced += c;
        }
        auto lower = [](std::string s) { for (auto& c : s) c = (char)std::tolower((unsigned char)c); return s; };
        return {
            {Lowercase,       lower(raw)},
            {LowercaseSpaced, lower(spaced)},
            {Normal,          raw},
            {NormalSpaced,    spaced},
        };
    }
};

static void add(ModuleManager& mm, const std::string& name, ModuleCategory cat, bool enabled = false)
{
    mm.registerModule(std::make_shared<FakeModule>(name, name, cat, 0, enabled));
}

void registerFakeModules(ModuleManager& mm)
{
    using C = ModuleCategory;

    // Visual first. Interface must exist before others reference its theme/font;
    // HudEditor must exist before HUD widgets register their elements.
    mm.registerModule(std::make_shared<Interface>());
    mm.registerModule(std::make_shared<HudEditor>());
    mm.registerModule(std::make_shared<ClickGui>());

    // ---- ported (real) HUD modules ----
    mm.registerModule(std::make_shared<Watermark>());
    mm.registerModule(std::make_shared<Arraylist>());
    mm.registerModule(std::make_shared<Keystrokes>());
    mm.registerModule(std::make_shared<Notifications>());
    mm.registerModule(std::make_shared<ToggleSounds>());

    // ---- Combat ----
    add(mm, "KillAura", C::Combat);
    add(mm, "TriggerBot", C::Combat);
    add(mm, "AutoClicker", C::Combat);
    add(mm, "Reach", C::Combat);
    add(mm, "Criticals", C::Combat);
    add(mm, "InfiniteAura", C::Combat);

    // ---- Movement ----
    for (const char* n : { "AirJump","AirSpeed","AntiImmobile","AutoPath","DamageBoost","DebugFly",
                           "FastStop","Fly","HiveFly","InventoryMove","Jesus","Jetpack","LongJump",
                           "NoJumpDelay","NoSlowDown","Phase","ReverseStep","SafeWalk","ServerSneak",
                           "Speed","Spider","Sprint","Step","TargetStrafe","Velocity" })
        add(mm, n, C::Movement);

    // ---- Player ----
    for (const char* n : { "AntiVoid","AutoBoombox","AutoKick","AutoSpellBook","AutoTool","ChestAura",
                           "ChestStealer","ClickTp","Derp","Extinguisher","FastMine","Freecam","InvManager",
                           "MidclickAction","NoFall","NoRotate","Nuker","OreMiner","Regen","RegenRecode",
                           "Scaffold","Teams","Timer","FastEat" })
        add(mm, n, C::Player);

    // ---- Visual (HUD widgets default-on so the fake HUD is visible) ----
    for (const char* n : { "Animations","AutoScale","BlockESP","BoneEsp","ChinaHat","CustomChat",
                           "DestroyProgress","ESP","Freelook","FullBright","Glint","Goofy",
                           "ItemESP","ItemPhysics","JumpCircles","LevelInfo","MotionBlur","NameProtect",
                           "Nametags","NoCameraClip","NoDebuff","NoHurtcam","NoRender","RobloxCamera",
                           "Tracers","UpdateForm","ViewModel","Zoom" })
        add(mm, n, C::Visual);
    add(mm, "SessionInfo",  C::Visual, false);
    add(mm, "TargetHUD",    C::Visual, false);

    // ---- Misc ----
    for (const char* n : { "AntiBot","Anticheat","AntiCheatDetector","AutoAccept","AutoCosmetic","AutoDodge",
                           "AutoLootbox","AutoMessage","AutoQueue","AutoReport","AutoSnipe","AutoVote",
                           "CostumeSpammer","Desync","DeviceSpoof","Disabler","EditionFaker","Friends","IRC",
                           "KickSounds","Killsults","NetSkip","NoFilter","NoPacket","PacketLogger",
                           "PartySpammer","SkinBlinker","SkinStealer","Spammer","StaffAlert","TestModule",
                           "VoiceChat" })
        add(mm, n, C::Misc);
}
