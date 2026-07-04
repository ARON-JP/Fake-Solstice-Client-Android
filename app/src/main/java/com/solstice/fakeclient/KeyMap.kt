package com.solstice.fakeclient

import android.view.KeyEvent

/**
 * Maps Android [KeyEvent] key codes to the Win32 VK_* codes the reused Solstice
 * code expects (see cpp/shim/Windows.h). This lets a hardware keyboard drive
 * module key-binds exactly like the Windows build.
 */
object KeyMap {
    fun toVk(keyCode: Int): Int = when (keyCode) {
        in KeyEvent.KEYCODE_A..KeyEvent.KEYCODE_Z -> 'A'.code + (keyCode - KeyEvent.KEYCODE_A)
        in KeyEvent.KEYCODE_0..KeyEvent.KEYCODE_9 -> '0'.code + (keyCode - KeyEvent.KEYCODE_0)
        in KeyEvent.KEYCODE_F1..KeyEvent.KEYCODE_F12 -> 0x70 + (keyCode - KeyEvent.KEYCODE_F1)
        KeyEvent.KEYCODE_ESCAPE -> 0x1B
        KeyEvent.KEYCODE_TAB -> 0x09
        KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_NUMPAD_ENTER -> 0x0D
        KeyEvent.KEYCODE_SPACE -> 0x20
        KeyEvent.KEYCODE_DEL -> 0x08          // backspace
        KeyEvent.KEYCODE_FORWARD_DEL -> 0x2E
        KeyEvent.KEYCODE_SHIFT_LEFT, KeyEvent.KEYCODE_SHIFT_RIGHT -> 0x10
        KeyEvent.KEYCODE_CTRL_LEFT, KeyEvent.KEYCODE_CTRL_RIGHT -> 0x11
        KeyEvent.KEYCODE_ALT_LEFT, KeyEvent.KEYCODE_ALT_RIGHT -> 0x12
        KeyEvent.KEYCODE_DPAD_UP -> 0x26
        KeyEvent.KEYCODE_DPAD_DOWN -> 0x28
        KeyEvent.KEYCODE_DPAD_LEFT -> 0x25
        KeyEvent.KEYCODE_DPAD_RIGHT -> 0x27
        KeyEvent.KEYCODE_INSERT -> 0x2D
        KeyEvent.KEYCODE_MOVE_HOME -> 0x24
        KeyEvent.KEYCODE_MOVE_END -> 0x23
        KeyEvent.KEYCODE_PAGE_UP -> 0x21
        KeyEvent.KEYCODE_PAGE_DOWN -> 0x22
        KeyEvent.KEYCODE_GRAVE -> 0xC0
        KeyEvent.KEYCODE_LEFT_BRACKET -> 0xDB
        KeyEvent.KEYCODE_RIGHT_BRACKET -> 0xDD
        KeyEvent.KEYCODE_SLASH -> 0xBF
        KeyEvent.KEYCODE_BACKSLASH -> 0xDC
        KeyEvent.KEYCODE_SEMICOLON -> 0xBA
        KeyEvent.KEYCODE_APOSTROPHE -> 0xDE
        KeyEvent.KEYCODE_COMMA -> 0xBC
        KeyEvent.KEYCODE_PERIOD -> 0xBE
        KeyEvent.KEYCODE_MINUS -> 0xBD
        KeyEvent.KEYCODE_EQUALS -> 0xBB
        else -> 0
    }
}
