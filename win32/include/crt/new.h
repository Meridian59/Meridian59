/* new.h:  MSVC's new-handler hook. */
#ifndef M59_NEW_H
#define M59_NEW_H
#include <new>
typedef int (*_PNH)(size_t);
// MSVC calls the handler with the failed size; std::set_new_handler takes none.
_PNH _set_new_handler(_PNH handler);
#endif
