/*
 * keys.mm:  macOS virtual key codes (key positions, ANSI layout) as Windows
 *   virtual-key codes.
 */

#include "appkit/appkit_backend.h"

#import <Carbon/Carbon.h>

namespace w32::mac {

int KeyCodeToVk(unsigned short code)
{
   switch (code)
   {
   case kVK_ANSI_A: return 'A';
   case kVK_ANSI_B: return 'B';
   case kVK_ANSI_C: return 'C';
   case kVK_ANSI_D: return 'D';
   case kVK_ANSI_E: return 'E';
   case kVK_ANSI_F: return 'F';
   case kVK_ANSI_G: return 'G';
   case kVK_ANSI_H: return 'H';
   case kVK_ANSI_I: return 'I';
   case kVK_ANSI_J: return 'J';
   case kVK_ANSI_K: return 'K';
   case kVK_ANSI_L: return 'L';
   case kVK_ANSI_M: return 'M';
   case kVK_ANSI_N: return 'N';
   case kVK_ANSI_O: return 'O';
   case kVK_ANSI_P: return 'P';
   case kVK_ANSI_Q: return 'Q';
   case kVK_ANSI_R: return 'R';
   case kVK_ANSI_S: return 'S';
   case kVK_ANSI_T: return 'T';
   case kVK_ANSI_U: return 'U';
   case kVK_ANSI_V: return 'V';
   case kVK_ANSI_W: return 'W';
   case kVK_ANSI_X: return 'X';
   case kVK_ANSI_Y: return 'Y';
   case kVK_ANSI_Z: return 'Z';
   case kVK_ANSI_0: return '0';
   case kVK_ANSI_1: return '1';
   case kVK_ANSI_2: return '2';
   case kVK_ANSI_3: return '3';
   case kVK_ANSI_4: return '4';
   case kVK_ANSI_5: return '5';
   case kVK_ANSI_6: return '6';
   case kVK_ANSI_7: return '7';
   case kVK_ANSI_8: return '8';
   case kVK_ANSI_9: return '9';
   case kVK_ANSI_Keypad0: return VK_NUMPAD0;
   case kVK_ANSI_Keypad1: return VK_NUMPAD1;
   case kVK_ANSI_Keypad2: return VK_NUMPAD2;
   case kVK_ANSI_Keypad3: return VK_NUMPAD3;
   case kVK_ANSI_Keypad4: return VK_NUMPAD4;
   case kVK_ANSI_Keypad5: return VK_NUMPAD5;
   case kVK_ANSI_Keypad6: return VK_NUMPAD6;
   case kVK_ANSI_Keypad7: return VK_NUMPAD7;
   case kVK_ANSI_Keypad8: return VK_NUMPAD8;
   case kVK_ANSI_Keypad9: return VK_NUMPAD9;
   case kVK_ANSI_KeypadMultiply: return VK_MULTIPLY;
   case kVK_ANSI_KeypadPlus: return VK_ADD;
   case kVK_ANSI_KeypadMinus: return VK_SUBTRACT;
   case kVK_ANSI_KeypadDecimal: return VK_DECIMAL;
   case kVK_ANSI_KeypadDivide: return VK_DIVIDE;
   case kVK_ANSI_KeypadEnter: return VK_RETURN;
   case kVK_ANSI_KeypadClear: return VK_NUMLOCK;
   case kVK_F1: return VK_F1;
   case kVK_F2: return VK_F2;
   case kVK_F3: return VK_F3;
   case kVK_F4: return VK_F4;
   case kVK_F5: return VK_F5;
   case kVK_F6: return VK_F6;
   case kVK_F7: return VK_F7;
   case kVK_F8: return VK_F8;
   case kVK_F9: return VK_F9;
   case kVK_F10: return VK_F10;
   case kVK_F11: return VK_F11;
   case kVK_F12: return VK_F12;
   case kVK_F13: return VK_F13;
   case kVK_F14: return VK_F14;
   case kVK_F15: return VK_F15;
   case kVK_F16: return VK_F16;
   case kVK_F17: return VK_F17;
   case kVK_F18: return VK_F18;
   case kVK_F19: return VK_F19;
   case kVK_F20: return VK_F20;
   case kVK_Delete: return VK_BACK;   // the backspace key
   case kVK_ForwardDelete: return VK_DELETE;
   case kVK_Tab: return VK_TAB;
   case kVK_Return: return VK_RETURN;
   case kVK_Escape: return VK_ESCAPE;
   case kVK_Space: return VK_SPACE;
   case kVK_Shift: return VK_LSHIFT;
   case kVK_RightShift: return VK_RSHIFT;
   case kVK_Control: return VK_LCONTROL;
   case kVK_RightControl: return VK_RCONTROL;
   case kVK_Option: return VK_LMENU;
   case kVK_RightOption: return VK_RMENU;
   case kVK_Command: return VK_LWIN;
   case kVK_RightCommand: return VK_RWIN;
   case kVK_CapsLock: return VK_CAPITAL;
   case kVK_Help: return VK_INSERT;
   case kVK_Home: return VK_HOME;
   case kVK_End: return VK_END;
   case kVK_PageUp: return VK_PRIOR;
   case kVK_PageDown: return VK_NEXT;
   case kVK_LeftArrow: return VK_LEFT;
   case kVK_RightArrow: return VK_RIGHT;
   case kVK_UpArrow: return VK_UP;
   case kVK_DownArrow: return VK_DOWN;
   // US layout punctuation (the OEM keys).
   case kVK_ANSI_Semicolon: return VK_OEM_1;
   case kVK_ANSI_Equal: return VK_OEM_PLUS;
   case kVK_ANSI_Comma: return VK_OEM_COMMA;
   case kVK_ANSI_Minus: return VK_OEM_MINUS;
   case kVK_ANSI_Period: return VK_OEM_PERIOD;
   case kVK_ANSI_Slash: return VK_OEM_2;
   case kVK_ANSI_Grave: return VK_OEM_3;
   case kVK_ANSI_LeftBracket: return VK_OEM_4;
   case kVK_ANSI_Backslash: return VK_OEM_5;
   case kVK_ANSI_RightBracket: return VK_OEM_6;
   case kVK_ANSI_Quote: return VK_OEM_7;
   case kVK_ISO_Section: return VK_OEM_102;
   }
   return 0;
}

}  // namespace w32::mac
