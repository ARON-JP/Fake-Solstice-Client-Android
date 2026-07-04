//
// Glue between the host overlay and the reused Solstice feature/module system.
//
#pragma once

namespace Wiring
{
    void initialize();   // build FeatureManager, load fonts, register modules
    void renderFrame();   // apply toggles + dispatch RenderEvent (called each frame)
    void shutdown();
}
