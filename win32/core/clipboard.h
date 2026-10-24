/*
 * clipboard.h:  The clipboard's link to the platform's (the store is the
 *   layer's, in process; a backend can publish what is copied and offer what
 *   other programs copied).
 */

#ifndef W32_CORE_CLIPBOARD_H
#define W32_CORE_CLIPBOARD_H

#include <string>

namespace w32 {

struct ClipboardBackend
{
   // Text the program put on the clipboard (CloseClipboard after a change), UTF-8.
   void (*publish)(const std::string &utf8) = nullptr;
   // Text another program copied since our last publish, if any.
   bool (*fetch)(std::string *utf8) = nullptr;
};

// None by default: the clipboard is the process's own.
void SetClipboardBackend(const ClipboardBackend &backend);

}  // namespace w32

#endif
