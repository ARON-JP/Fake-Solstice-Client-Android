#pragma once
//
// Slim, game-free port of Solstice's Interface module.
//
// The original drove rotations/packets/rendering through many game events. None
// of that exists in FakeClient, so we keep ONLY the visual configuration the
// reused ClickGUI/HUD code reads: the colour themes, the active font, naming
// style and colour speed/saturation. The settings, enums, ColorThemes table and
// getCustomColors() are copied verbatim so the look matches exactly.
//
#include <Features/Modules/Module.hpp>
#include <Features/Modules/Setting.hpp>
#include <vector>
#include <unordered_map>

class Interface : public ModuleBase<Interface>
{
public:
    enum ColorTheme {
        Trans,
        Rainbow,
        Bubblegum,
        Watermelon,
        Sunset,
        Poison,
        Custom
    };

    enum class FontType {
        Mojangles,
        ProductSans,
    };

    EnumSettingT<NamingStyle> mNamingStyle = EnumSettingT<NamingStyle>("Naming", "The style of the module names.", NamingStyle::NormalSpaced, "lowercase", "lower spaced", "Normal", "Spaced");
    EnumSettingT<ColorTheme> mMode = EnumSettingT<ColorTheme>("Theme", "The mode of the interface.", ColorTheme::Trans, "Trans", "Rainbow", "Bubblegum", "Watermelon", "Sunset", "Poison", "Custom");
    EnumSettingT<FontType> mFont = EnumSettingT<FontType>("Font", "The font of the interface.", FontType::ProductSans, "Mojangles", "Product Sans");
    NumberSetting mColors = NumberSetting("Colors", "The amount of colors in the interface.", 3, 1, 6, 1);
    ColorSetting mColor1 = ColorSetting("Color 1", "The first color of the interface.", 0xFFFF0000);
    ColorSetting mColor2 = ColorSetting("Color 2", "The second color of the interface.", 0xFFFF7F00);
    ColorSetting mColor3 = ColorSetting("Color 3", "The third color of the interface.", 0xFFFFD600);
    ColorSetting mColor4 = ColorSetting("Color 4", "The fourth color of the interface.", 0xFF00FF00);
    ColorSetting mColor5 = ColorSetting("Color 5", "The fifth color of the interface.", 0xFF0000FF);
    ColorSetting mColor6 = ColorSetting("Color 6", "The sixth color of the interface.", 0xFF8B00FF);
    NumberSetting mColorSpeed = NumberSetting("Color Speed", "The speed of the color change.", 3.f, 0.01f, 20.f, 0.01);
    NumberSetting mSaturation = NumberSetting("Saturation", "The saturation of the interface.", 1.f, 0.f, 1.f, 0.01);

    static inline std::unordered_map<int, std::vector<ImColor>> ColorThemes = {
        {Trans,     {
            ImColor(91, 206, 250, 255),
            ImColor(245, 169, 184, 255),
            ImColor(255, 255, 255, 255),
            ImColor(245, 169, 184, 255),
        }},
        {Rainbow,   {}},
        {Bubblegum, {
            ImColor(255, 99, 202, 255),
            ImColor(255, 195, 195, 255),
            ImColor(146, 245, 255, 255),
            ImColor(249, 255, 148, 255),
            ImColor(135, 255, 176, 255),
        }},
        {Watermelon, {
            ImColor(255, 70, 70, 255),
            ImColor(139, 0, 0, 255),
            ImColor(144, 238, 144, 255),
            ImColor(34, 139, 34, 255),
            ImColor(204, 255, 204, 255),
        }},
        {Sunset, {
            ImColor(213,32,0, 255),
            ImColor(239, 118, 39, 255),
            ImColor(255, 154, 86, 255),
            ImColor(255, 255, 255, 255),
            ImColor(209, 98, 164, 255),
            ImColor(181, 86, 144, 255),
        }},
        {Poison, {
            ImColor(115,222,70, 255),
            ImColor(67, 201, 89, 255),
            ImColor(41, 230, 94, 255),
            ImColor(12, 210, 83, 255),
            ImColor(87, 211, 72, 255),
            ImColor(57, 210, 124, 255),
        }},
        {Custom,    {}}
    };

    std::vector<ImColor> getCustomColors() {
        auto result = std::vector<ImColor>();
        if (mColors.mValue >= 1) result.push_back(mColor1.getAsImColor());
        if (mColors.mValue >= 2) result.push_back(mColor2.getAsImColor());
        if (mColors.mValue >= 3) result.push_back(mColor3.getAsImColor());
        if (mColors.mValue >= 4) result.push_back(mColor4.getAsImColor());
        if (mColors.mValue >= 5) result.push_back(mColor5.getAsImColor());
        if (mColors.mValue >= 6) result.push_back(mColor6.getAsImColor());
        return result;
    }

    Interface() : ModuleBase("Interface", "Customize the visuals!", ModuleCategory::Visual, 0, true) {
        addSettings(
            &mNamingStyle,
            &mMode,
            &mFont,
            &mColors,
            &mColor1, &mColor2, &mColor3, &mColor4, &mColor5, &mColor6,
            &mColorSpeed,
            &mSaturation
        );
        mNames = {
            {Lowercase, "interface"},
            {LowercaseSpaced, "interface"},
            {Normal, "Interface"},
            {NormalSpaced, "Interface"}
        };
    }
};
