/*
 * wine/exception.h:  Wine's structured exception macros for its vendored sources.
 *   There is no SEH here: a __TRY block runs, its handler never does (an access
 *   violation in it crashes, as it would without the handler).
 */

#ifndef __WINE_WINE_EXCEPTION_H
#define __WINE_WINE_EXCEPTION_H

#include <windef.h>
#include <excpt.h>

#define __TRY if (1) {
#define __EXCEPT(func) } else {
#define __EXCEPT_CTX(func, ctx) } else {
#define __EXCEPT_PAGE_FAULT } else {
#define __EXCEPT_ALL } else {
#define __FINALLY(func) } { (func)(TRUE); }
#define __FINALLY_CTX(func, ctx) } { (func)(TRUE, ctx); }
#define __ENDTRY }

#endif
