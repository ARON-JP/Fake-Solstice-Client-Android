#pragma once
//
// Minimal <d3d11.h> shim for the Android port.
//
// Only reused/Hook/Hooks/RenderHooks/D3DHook.hpp and Watermark.cpp reference
// Direct3D types, and only the optional "SevenDays" watermark image path uses a
// texture (which never loads here - loadTextureFromEmbeddedResource returns
// false). Treating the texture handles as opaque void pointers lets that code
// compile unchanged; ImGui's AddImage(ImTextureID=void*) accepts them directly.
//
typedef void ID3D11ShaderResourceView;
typedef void ID3D11Device;
typedef void ID3D11DeviceContext;
typedef void ID3D11Texture2D;
typedef void IDXGISwapChain;
