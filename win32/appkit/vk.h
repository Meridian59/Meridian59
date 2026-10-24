/*
 * vk.h:  Windows' virtual-key codes and mouse key flags (winuser.rh's values) for
 *   the Objective-C++ files, which can't include Windows' headers (bridge.h).
 */

#ifndef W32_APPKIT_VK_H
#define W32_APPKIT_VK_H

namespace w32::mac {

constexpr int VK_ADD = 0x6B;
constexpr int VK_BACK = 0x08;
constexpr int VK_CAPITAL = 0x14;
constexpr int VK_CONTROL = 0x11;
constexpr int VK_DECIMAL = 0x6E;
constexpr int VK_DELETE = 0x2E;
constexpr int VK_DIVIDE = 0x6F;
constexpr int VK_DOWN = 0x28;
constexpr int VK_END = 0x23;
constexpr int VK_ESCAPE = 0x1B;
constexpr int VK_F1 = 0x70;
constexpr int VK_F10 = 0x79;
constexpr int VK_F11 = 0x7A;
constexpr int VK_F12 = 0x7B;
constexpr int VK_F13 = 0x7C;
constexpr int VK_F14 = 0x7D;
constexpr int VK_F15 = 0x7E;
constexpr int VK_F16 = 0x7F;
constexpr int VK_F17 = 0x80;
constexpr int VK_F18 = 0x81;
constexpr int VK_F19 = 0x82;
constexpr int VK_F2 = 0x71;
constexpr int VK_F20 = 0x83;
constexpr int VK_F3 = 0x72;
constexpr int VK_F4 = 0x73;
constexpr int VK_F5 = 0x74;
constexpr int VK_F6 = 0x75;
constexpr int VK_F7 = 0x76;
constexpr int VK_F8 = 0x77;
constexpr int VK_F9 = 0x78;
constexpr int VK_HOME = 0x24;
constexpr int VK_INSERT = 0x2D;
constexpr int VK_LBUTTON = 0x01;
constexpr int VK_LCONTROL = 0xA2;
constexpr int VK_LEFT = 0x25;
constexpr int VK_LMENU = 0xA4;
constexpr int VK_LSHIFT = 0xA0;
constexpr int VK_LWIN = 0x5B;
constexpr int VK_MBUTTON = 0x04;
constexpr int VK_MENU = 0x12;
constexpr int VK_MULTIPLY = 0x6A;
constexpr int VK_NEXT = 0x22;
constexpr int VK_NUMLOCK = 0x90;
constexpr int VK_NUMPAD0 = 0x60;
constexpr int VK_NUMPAD1 = 0x61;
constexpr int VK_NUMPAD2 = 0x62;
constexpr int VK_NUMPAD3 = 0x63;
constexpr int VK_NUMPAD4 = 0x64;
constexpr int VK_NUMPAD5 = 0x65;
constexpr int VK_NUMPAD6 = 0x66;
constexpr int VK_NUMPAD7 = 0x67;
constexpr int VK_NUMPAD8 = 0x68;
constexpr int VK_NUMPAD9 = 0x69;
constexpr int VK_OEM_1 = 0xBA;
constexpr int VK_OEM_102 = 0xE2;
constexpr int VK_OEM_2 = 0xBF;
constexpr int VK_OEM_3 = 0xC0;
constexpr int VK_OEM_4 = 0xDB;
constexpr int VK_OEM_5 = 0xDC;
constexpr int VK_OEM_6 = 0xDD;
constexpr int VK_OEM_7 = 0xDE;
constexpr int VK_OEM_COMMA = 0xBC;
constexpr int VK_OEM_MINUS = 0xBD;
constexpr int VK_OEM_PERIOD = 0xBE;
constexpr int VK_OEM_PLUS = 0xBB;
constexpr int VK_PRIOR = 0x21;
constexpr int VK_RBUTTON = 0x02;
constexpr int VK_RCONTROL = 0xA3;
constexpr int VK_RETURN = 0x0D;
constexpr int VK_RIGHT = 0x27;
constexpr int VK_RMENU = 0xA5;
constexpr int VK_RSHIFT = 0xA1;
constexpr int VK_RWIN = 0x5C;
constexpr int VK_SHIFT = 0x10;
constexpr int VK_SPACE = 0x20;
constexpr int VK_SUBTRACT = 0x6D;
constexpr int VK_TAB = 0x09;
constexpr int VK_UP = 0x26;

constexpr unsigned MK_LBUTTON = 0x0001;
constexpr unsigned MK_RBUTTON = 0x0002;
constexpr unsigned MK_SHIFT = 0x0004;
constexpr unsigned MK_CONTROL = 0x0008;
constexpr unsigned MK_MBUTTON = 0x0010;

constexpr int WHEEL_DELTA = 120;

}  // namespace w32::mac

#endif
