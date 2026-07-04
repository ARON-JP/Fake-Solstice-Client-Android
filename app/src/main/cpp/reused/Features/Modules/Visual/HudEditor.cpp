//
// Slim HudEditor implementation for FakeClient.
//
// HudElement (the positioning data used by Watermark/Keystrokes/etc.) is fully
// defined inline in HudEditor.hpp. Only the editor's own behaviour lives here,
// and the drag-to-move / save-load editor UI is omitted — HUD widgets simply
// render at their default anchored positions. This keeps the HUD "as-is" while
// dropping the file IO the original needed.
//
#include "HudEditor.hpp"

void HudEditor::onInit() {}
void HudEditor::onEnable() { showAllElements(); }
void HudEditor::onDisable() { hideAllElements(); }

void HudEditor::showAllElements()
{
    for (auto* e : mElements) if (e) e->mSampleMode = true;
}

void HudEditor::hideAllElements()
{
    for (auto* e : mElements) if (e) e->mSampleMode = false;
}

void HudEditor::saveToFile() {}
void HudEditor::loadFromFile() {}

// The editor overlay (draggable handles) is intentionally not drawn here.
void HudEditor::onRenderEvent(RenderEvent&) {}
void HudEditor::onCustomRenderEvent(RenderEvent&) {}
void HudEditor::onKeyEvent(KeyEvent&) {}
void HudEditor::onMouseEvent(MouseEvent&) {}
