#pragma once
//
// Minimal <Windows.h> shim for the Android port of FakeClient.
//
// The reused Solstice GUI/HUD code (Keyboard.hpp, ProcUtils.hpp, Keystrokes.cpp,
// pch.hpp) includes <Windows.h> purely for the VK_* virtual-key constants and a
// handful of input/window query functions. None of Win32's real functionality is
// needed. This header provides:
//   * the VK_* constants (real Win32 numeric values, so our Android keycode ->VK
//     mapping and the existing Keyboard::mKeyMap stay consistent),
//   * opaque HWND/RECT/POINT types,
//   * GetAsyncKeyState / GetKeyState backed by a global key-state table that the
//     JNI input bridge updates,
//   * GetWindowRect / GetDesktopWindow / GetForegroundWindow / GetCursorPos that
//     report the overlay surface (always "fullscreen", origin 0,0).
//
// Because fakeclient/reused is *not* edited, this file lives first on the include
// path and transparently satisfies every `#include <Windows.h>`.
//
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdarg>

// ---- MSVC secure CRT shims --------------------------------------------------
// reused/Features/GUI/ModernDropdown.cpp uses sprintf_s; map it to vsnprintf.
inline int sprintf_s(char* buf, size_t size, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int r = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return r;
}

// ---- MSVC fixed-width integer aliases ---------------------------------------
// Some reused Solstice headers (e.g. Features/Events/MouseEvent.hpp) use the
// MSVC __intN spellings; provide them for the clang/NDK toolchain.
typedef signed char __int8;
typedef short       __int16;
typedef int         __int32;
typedef long long   __int64;

// ---- basic typedefs ---------------------------------------------------------
typedef void*          HWND;
typedef void*          HINSTANCE;
typedef void*          HICON;
typedef int            BOOL;
typedef unsigned long  DWORD;
typedef unsigned short WORD;
typedef unsigned int   UINT;
typedef long           LONG;
typedef unsigned char  BYTE;
typedef short          SHORT;
typedef char*          LPSTR;
typedef const char*    LPCSTR;
typedef intptr_t       LPARAM;
typedef uintptr_t      WPARAM;

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif
#ifndef WINAPI
#define WINAPI
#endif

typedef struct tagRECT  { LONG left, top, right, bottom; } RECT, *LPRECT;
typedef struct tagPOINT { LONG x, y; } POINT, *LPPOINT;

// ---- virtual-key constants (real Win32 values) ------------------------------
#define VK_LBUTTON    0x01
#define VK_RBUTTON    0x02
#define VK_MBUTTON    0x04
#define VK_BACK       0x08
#define VK_TAB        0x09
#define VK_RETURN     0x0D
#define VK_SHIFT      0x10
#define VK_CONTROL    0x11
#define VK_MENU       0x12
#define VK_PAUSE      0x13
#define VK_CAPITAL    0x14
#define VK_ESCAPE     0x1B
#define VK_SPACE      0x20
#define VK_PRIOR      0x21
#define VK_NEXT       0x22
#define VK_END        0x23
#define VK_HOME       0x24
#define VK_LEFT       0x25
#define VK_UP         0x26
#define VK_RIGHT      0x27
#define VK_DOWN       0x28
#define VK_INSERT     0x2D
#define VK_DELETE     0x2E
#define VK_NUMPAD0    0x60
#define VK_NUMPAD1    0x61
#define VK_NUMPAD2    0x62
#define VK_NUMPAD3    0x63
#define VK_NUMPAD4    0x64
#define VK_NUMPAD5    0x65
#define VK_NUMPAD6    0x66
#define VK_NUMPAD7    0x67
#define VK_NUMPAD8    0x68
#define VK_NUMPAD9    0x69
#define VK_MULTIPLY   0x6A
#define VK_ADD        0x6B
#define VK_SUBTRACT   0x6D
#define VK_DECIMAL    0x6E
#define VK_DIVIDE     0x6F
#define VK_F1         0x70
#define VK_F2         0x71
#define VK_F3         0x72
#define VK_F4         0x73
#define VK_F5         0x74
#define VK_F6         0x75
#define VK_F7         0x76
#define VK_F8         0x77
#define VK_F9         0x78
#define VK_F10        0x79
#define VK_F11        0x7A
#define VK_F12        0x7B
#define VK_F13        0x7C
#define VK_F14        0x7D
#define VK_F15        0x7E
#define VK_F16        0x7F
#define VK_F17        0x80
#define VK_F18        0x81
#define VK_F19        0x82
#define VK_F20        0x83
#define VK_F21        0x84
#define VK_F22        0x85
#define VK_F23        0x86
#define VK_F24        0x87
#define VK_OEM_1      0xBA  // ;:
#define VK_OEM_PLUS   0xBB
#define VK_OEM_COMMA  0xBC
#define VK_OEM_MINUS  0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2      0xBF  // /?
#define VK_OEM_3      0xC0  // `~
#define VK_OEM_4      0xDB  // [{
#define VK_OEM_5      0xDC  // backslash
#define VK_OEM_6      0xDD  // ]}
#define VK_OEM_7      0xDE  // '"

// ---- global state shared with the JNI input bridge --------------------------
// The bridge sets/clears entries here on key down/up; the GUI/HUD query them
// through the GetAsyncKeyState/GetKeyState shims below.
namespace fc_shim {
    inline unsigned char gKeyState[256] = {0};
    inline int gDisplayW = 1920;
    inline int gDisplayH = 1080;
    inline int gCursorX = 0;
    inline int gCursorY = 0;
}

// ---- function shims ---------------------------------------------------------
inline SHORT GetAsyncKeyState(int vk) {
    if (vk < 0 || vk > 255) return 0;
    return fc_shim::gKeyState[vk] ? (SHORT)0x8000u : 0;
}
inline SHORT GetKeyState(int vk) { return GetAsyncKeyState(vk); }

inline HWND GetDesktopWindow()    { return (HWND)1; }
inline HWND GetForegroundWindow() { return (HWND)1; }

inline BOOL GetWindowRect(HWND, LPRECT r) {
    if (r) { r->left = 0; r->top = 0; r->right = fc_shim::gDisplayW; r->bottom = fc_shim::gDisplayH; }
    return TRUE;
}
inline BOOL GetClientRect(HWND, LPRECT r) { return GetWindowRect(nullptr, r); }
inline BOOL GetCursorPos(LPPOINT p) {
    if (p) { p->x = fc_shim::gCursorX; p->y = fc_shim::gCursorY; }
    return TRUE;
}
inline BOOL ScreenToClient(HWND, LPPOINT) { return TRUE; }
inline BOOL ClientToScreen(HWND, LPPOINT) { return TRUE; }
