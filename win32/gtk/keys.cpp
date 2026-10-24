/*
 * keys.cpp:  GDK key values as Windows virtual-key codes.
 */

#include "gtk/gtk_backend.h"

namespace w32::native {

int KeyvalToVk(guint keyval)
{
   // Letters and digits are their upper-case ASCII on Windows.
   guint upper = gdk_keyval_to_upper(keyval);
   if (upper >= GDK_KEY_A && upper <= GDK_KEY_Z)
      return 'A' + (int) (upper - GDK_KEY_A);
   if (keyval >= GDK_KEY_0 && keyval <= GDK_KEY_9)
      return '0' + (int) (keyval - GDK_KEY_0);
   if (keyval >= GDK_KEY_F1 && keyval <= GDK_KEY_F24)
      return VK_F1 + (int) (keyval - GDK_KEY_F1);
   if (keyval >= GDK_KEY_KP_0 && keyval <= GDK_KEY_KP_9)
      return VK_NUMPAD0 + (int) (keyval - GDK_KEY_KP_0);
   switch (keyval)
   {
   case GDK_KEY_BackSpace: return VK_BACK;
   case GDK_KEY_Tab: case GDK_KEY_ISO_Left_Tab: return VK_TAB;
   case GDK_KEY_Return: return VK_RETURN;
   case GDK_KEY_KP_Enter: return VK_RETURN;
   case GDK_KEY_Shift_L: return VK_LSHIFT;
   case GDK_KEY_Shift_R: return VK_RSHIFT;
   case GDK_KEY_Control_L: return VK_LCONTROL;
   case GDK_KEY_Control_R: return VK_RCONTROL;
   case GDK_KEY_Alt_L: case GDK_KEY_Meta_L: return VK_LMENU;
   case GDK_KEY_Alt_R: case GDK_KEY_Meta_R: case GDK_KEY_ISO_Level3_Shift: return VK_RMENU;
   case GDK_KEY_Pause: return VK_PAUSE;
   case GDK_KEY_Caps_Lock: return VK_CAPITAL;
   case GDK_KEY_Escape: return VK_ESCAPE;
   case GDK_KEY_space: return VK_SPACE;
   case GDK_KEY_Page_Up: case GDK_KEY_KP_Page_Up: return VK_PRIOR;
   case GDK_KEY_Page_Down: case GDK_KEY_KP_Page_Down: return VK_NEXT;
   case GDK_KEY_End: case GDK_KEY_KP_End: return VK_END;
   case GDK_KEY_Home: case GDK_KEY_KP_Home: return VK_HOME;
   case GDK_KEY_Left: case GDK_KEY_KP_Left: return VK_LEFT;
   case GDK_KEY_Up: case GDK_KEY_KP_Up: return VK_UP;
   case GDK_KEY_Right: case GDK_KEY_KP_Right: return VK_RIGHT;
   case GDK_KEY_Down: case GDK_KEY_KP_Down: return VK_DOWN;
   case GDK_KEY_Print: return VK_SNAPSHOT;
   case GDK_KEY_Insert: case GDK_KEY_KP_Insert: return VK_INSERT;
   case GDK_KEY_Delete: case GDK_KEY_KP_Delete: return VK_DELETE;
   case GDK_KEY_KP_Begin: return VK_CLEAR;
   case GDK_KEY_Super_L: return VK_LWIN;
   case GDK_KEY_Super_R: return VK_RWIN;
   case GDK_KEY_Menu: return VK_APPS;
   case GDK_KEY_KP_Multiply: return VK_MULTIPLY;
   case GDK_KEY_KP_Add: return VK_ADD;
   case GDK_KEY_KP_Subtract: return VK_SUBTRACT;
   case GDK_KEY_KP_Decimal: return VK_DECIMAL;
   case GDK_KEY_KP_Divide: return VK_DIVIDE;
   case GDK_KEY_Num_Lock: return VK_NUMLOCK;
   case GDK_KEY_Scroll_Lock: return VK_SCROLL;
   // US layout punctuation (the OEM keys), whatever the character typed.
   case GDK_KEY_semicolon: case GDK_KEY_colon: return VK_OEM_1;
   case GDK_KEY_equal: case GDK_KEY_plus: return VK_OEM_PLUS;
   case GDK_KEY_comma: case GDK_KEY_less: return VK_OEM_COMMA;
   case GDK_KEY_minus: case GDK_KEY_underscore: return VK_OEM_MINUS;
   case GDK_KEY_period: case GDK_KEY_greater: return VK_OEM_PERIOD;
   case GDK_KEY_slash: case GDK_KEY_question: return VK_OEM_2;
   case GDK_KEY_grave: case GDK_KEY_asciitilde: return VK_OEM_3;
   case GDK_KEY_bracketleft: case GDK_KEY_braceleft: return VK_OEM_4;
   case GDK_KEY_backslash: case GDK_KEY_bar: return VK_OEM_5;
   case GDK_KEY_bracketright: case GDK_KEY_braceright: return VK_OEM_6;
   case GDK_KEY_apostrophe: case GDK_KEY_quotedbl: return VK_OEM_7;
   // Shifted digits on a US layout.
   case GDK_KEY_exclam: return '1';
   case GDK_KEY_at: return '2';
   case GDK_KEY_numbersign: return '3';
   case GDK_KEY_dollar: return '4';
   case GDK_KEY_percent: return '5';
   case GDK_KEY_asciicircum: return '6';
   case GDK_KEY_ampersand: return '7';
   case GDK_KEY_asterisk: return '8';
   case GDK_KEY_parenleft: return '9';
   case GDK_KEY_parenright: return '0';
   }
   return 0;
}

}  // namespace w32::native
