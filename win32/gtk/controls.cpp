/*
 * controls.cpp:  Native controls on GTK 4.
 */

#include "gtk/gtk_backend.h"

namespace w32::native {

bool ControlHasFocus(Top *)
{
   return false;
}

}  // namespace w32::native
