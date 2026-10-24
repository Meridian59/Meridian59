/*
 * cursor.h:  Cursors and icons (HCURSOR, HICON): a standard shape, or an image
 *   with a hot spot from the binary's resources.
 */

#ifndef W32_CORE_CURSOR_H
#define W32_CORE_CURSOR_H

#include "core/win32.h"

#include <cstdint>
#include <string>
#include <vector>

namespace w32 {

struct CursorImage
{
   std::string standard;           // a CSS cursor name ("default", "wait" ...), or empty
   int width = 0, height = 0;
   int hot_x = 0, hot_y = 0;
   std::vector<uint32_t> pixels;   // straight-alpha ARGB (0xAARRGGBB), rows top first
   // An icon from a resource: every image its group has (this one among them),
   // for window and application icons that want several sizes.
   std::vector<CursorImage> sizes;
};

// The cursor or icon behind h, or nullptr.
const CursorImage *GetCursorImage(HANDLE h);
// A new cursor or icon handle for image (made with new; the table keeps it).
HANDLE AddCursorImage(CursorImage *image);

// A new cursor or icon handle for image (taking it over): CreateIconIndirect.
HANDLE AddCursorImage(CursorImage *image);

}  // namespace w32

#endif
