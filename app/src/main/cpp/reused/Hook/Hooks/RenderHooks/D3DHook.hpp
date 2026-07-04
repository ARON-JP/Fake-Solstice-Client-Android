#pragma once
//
// Slim stub of Solstice's D3DHook. The original hooked the game's swapchain and
// loaded embedded textures. FakeClient doesn't load the optional watermark image
// (the default "Solstice" text style needs no texture), so this just satisfies
// the call site and reports failure.
//
#include <d3d11.h>
#include <cstdint>

class D3DHook {
public:
    static bool loadTextureFromEmbeddedResource(const char* /*name*/, ID3D11ShaderResourceView** /*out*/, int* /*w*/, int* /*h*/) { return false; }
    static bool createTextureFromData(const uint8_t* /*data*/, int /*w*/, int /*h*/, ID3D11ShaderResourceView** /*out*/) { return false; }
};
