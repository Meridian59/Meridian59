/*
 * rle.h:  Run-length encoded DIBs (BI_RLE8, BI_RLE4): what GDI's DIB functions
 *   and the RLE video codec (cc_msvfw.cpp) decode them to.
 */

#ifndef W32_CORE_RLE_H
#define W32_CORE_RLE_H

#include "core/win32.h"

#include <vector>

namespace w32 {

// Decodes RLE data (size bytes) onto out, an uncompressed bottom-up DIB of in's
// width, height and depth with rows stride bytes apart; pixels the data skips
// (deltas, ends of lines) keep their values.
void DecodeRle(const BITMAPINFOHEADER *in, const BYTE *data, DWORD size, BYTE *out, DWORD stride);

// bi with RLE bits as an uncompressed DIB: its header and color table (info,
// a BITMAPINFO) and pixels (skipped pixels index 0).  False if bi isn't RLE.
bool DecodeRleDib(const BITMAPINFO *bi, const void *bits, std::vector<BYTE> *info, std::vector<BYTE> *pixels);

}  // namespace w32

#endif
