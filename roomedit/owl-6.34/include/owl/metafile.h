//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TMetafilePict, a MetaFile wrapper class
//----------------------------------------------------------------------------

#if !defined(OWL_METAFILE_H)
#define OWL_METAFILE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gdibase.h>
#include <owl/wsyscls.h>

namespace owl {


class _OWLCLASS TClipboard;
class _OWLCLASS TDC;

/// \addtogroup graphics
/// @{

//
// METAFILEHEADER is used generally to create placeable metafiles
// (Originally created by Aldus; described by Microsoft in the
// Win 3.1 SDK; not declared in any Microsoft headers.)
//
// The placeable metafile header was originally a 16-bit structure, but it
// contains two fields of types that are normally polymorphic between
// 16 and 32 bits: RECT and HANDLE.  RECT16 is a non-polymorphic 16-bit
// RECT, needed to preserve alignment when reading mf headers into
// 32-bit programs.
//

// Users may have defined METAFILEHEADER for themselves.
//
#if !defined(METAFILEHEADER)

#include <pshpack1.h>                // always byte packed...

typedef struct {
  int16 left;
  int16 top;
  int16 right;
  int16 bottom;
} RECT16;

typedef struct {
    DWORD   key;            ///< identifies file type

    /// hmf is defined as HANDLE, but in Win16 that's an unsigned int
    WORD    hmf;            ///< unused (0)

    RECT16  bbox;           ///< bounding rectangle
    WORD    inch;           ///< units per inch
    DWORD   reserved;       ///< unused (0)
    WORD    checksum;       ///< XOR of previous fields
} METAFILEHEADER;

#include <poppack.h>                // restore previous packing

#endif

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TMetaFilePict
// ~~~~~ ~~~~~~~~~~~~~
/// TMetaFilePict is a support class used with TMetaFileDC to simplify metafile
/// operations, such as playing into a device context (DC) or storing data on the
/// Clipboard. TMetaFilePict automatically handles the conversion between a metafile
/// and a metafilepict.
//
class _OWLCLASS TMetaFilePict : private TGdiBase {
  public:
    TMetaFilePict(HMETAFILE handle, TAutoDelete autoDelete);
    TMetaFilePict(const TClipboard& clipboard);
    TMetaFilePict(const tstring& filename);
    TMetaFilePict(uint size, void * data);
    TMetaFilePict(const TMetaFilePict&, LPCTSTR fileName = 0);
    TMetaFilePict(const TMetaFilePict&, const tstring& fileName);
   ~TMetaFilePict();

    operator    HMETAFILE() const;

    uint32      GetMetaFileBitsEx(uint size, void* data);

    // Play this metafile onto a dc
    //
    TSize     CalcPlaySize(TDC& dc, const TSize& defSize) const;
    bool      PlayOnto(TDC& dc, const TSize& defSize) const;

    // Put this MetaFilePict onto the clipboard
    //
    void      ToClipboard(TClipboard& clipboard,
                          uint mapMode = MM_ANISOTROPIC,
                          const TSize& extent=TSize(0,0));

    // Retrieve attributes of this metafile
    //
    uint      MappingMode() const;
    int       Width() const;
    int       Height() const;
    TSize     Size() const;

    // Set attributes of this metafile
    //
    void      SetMappingMode(uint mm);
    void      SetSize(const TSize& size);

    // Methods for placeable metafiles
    //
    bool IsPlaceable();
    bool GetPlaceableHeader(METAFILEHEADER& header);
    void SetPlaceableHeader(TRect& bounds, uint16 unitsPerInch);

    ///BGM consider adding these, as in TDib:
    ///BGM Write(ostream& os, bool writeFileHeader = false);
    ///BGM Write(TFile& file, bool writeFileHeader = false);

  protected:
    uint16 CalcHeaderChecksum(const METAFILEHEADER& mfHeader);

  protected_data:
    int    Mm;        // Mapping mode
    TSize  Extent;
    bool Placeable;
    METAFILEHEADER MFHeader;
    static const uint32 MFHeaderKey;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TMetaFilePict& operator=(const TMetaFilePict&);
};

//
/// \class TEnhMetaFilePict
// ~~~~~ ~~~~~~~~~~~~~~~~
/// TEnhMetaFilePict is a class that encapsulates the enhanced metafile.
//
class _OWLCLASS TEnhMetaFilePict : private TGdiBase {
  public:
    TEnhMetaFilePict(HENHMETAFILE handle, TAutoDelete autoDelete);
    TEnhMetaFilePict(const tstring& filename);
    TEnhMetaFilePict(const TEnhMetaFilePict& metafilepict, LPCTSTR filename = 0);
    TEnhMetaFilePict(const TEnhMetaFilePict& metafilepict, const tstring& filename);
    TEnhMetaFilePict(uint bytes, const void* buffer);
   ~TEnhMetaFilePict();

    operator HENHMETAFILE() const;

    // Play this metafile onto a dc
    //
    bool PlayOnto(TDC& dc, const TRect* rect) const;

    // Retrieve attributes of this metafile
    //
    uint GetBits(uint bytes, void* buffer);
    uint GetDescription(uint bytes, void* buffer);
    uint GetHeader(uint bytes, ENHMETAHEADER* record);
    uint GetPaletteEntries(uint count, PALETTEENTRY* entries);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TEnhMetaFilePict& operator=(const TEnhMetaFilePict&);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Inserts the metafile picture onto the clipboard.
//
inline TClipboard& operator <<(TClipboard& clipboard, TMetaFilePict& mfp) {
  mfp.ToClipboard(clipboard);
  return clipboard;
}

//
/// Returns the associated handle for the metafile object.
//
inline TMetaFilePict::operator HMETAFILE() const {
  return HMETAFILE(Handle);
}

//
//
//
inline bool
TMetaFilePict::IsPlaceable() {
  return Placeable;
}

//
//
//
inline bool
TMetaFilePict::GetPlaceableHeader(METAFILEHEADER& header) {
  if (IsPlaceable()) {
    header = MFHeader;
    return true;
  }
  else
  return false;
}

//
///  Warning: values in the bounds rectangle will be cast to unsigned
///  int values.
//
inline void
TMetaFilePict::SetPlaceableHeader(TRect& bounds, uint16 unitsPerInch) {
  MFHeader.key         = MFHeaderKey;
  //MFHeader.hmf         = (WORD)(uint)Handle; /*?????????????????*/ 
  MFHeader.bbox.left   = (int16)bounds.left;
  MFHeader.bbox.top    = (int16)bounds.top;
  MFHeader.bbox.right  = (int16)bounds.right;
  MFHeader.bbox.bottom = (int16)bounds.bottom;
  MFHeader.inch        = unitsPerInch;
  MFHeader.reserved    = 0;
  MFHeader.checksum    = CalcHeaderChecksum(MFHeader);
  Placeable = true;
}

//
/// Returns the mapping mode of the metafile.
//
inline uint TMetaFilePict::MappingMode() const {
  return Mm;
}

//
/// Returns the width of the metafile.
//
inline int TMetaFilePict::Width() const {
  return Extent.cx;
}

//
/// Returns the height of the metafile.
//
inline int TMetaFilePict::Height() const {
  return Extent.cy;
}

//
/// Returns the size of the metafile.
//
inline TSize TMetaFilePict::Size() const {
  return Extent;
}

//
/// Sets the mapping mode for the metafile.
//
inline void TMetaFilePict::SetMappingMode(uint mm) {
  Mm = mm;
}

//
/// Sets the size of the metafile.
//
inline void TMetaFilePict::SetSize(const TSize& size) {
  Extent = size;
}

//
/// Returns the associated handle of the enhanced metafile.
//
inline TEnhMetaFilePict::operator HENHMETAFILE() const {
  return HENHMETAFILE(Handle);
}

//
/// Returns the bits of the metafile.
//
inline uint TEnhMetaFilePict::GetBits(uint bytes, void* buffer) {
  return ::GetEnhMetaFileBits((HENHMETAFILE)*this, bytes, (LPBYTE)buffer);
}

//
/// Retrieves the description of this enhanced metafile.
//
inline uint TEnhMetaFilePict::GetDescription(uint bytes, void* buffer) {
  return ::GetEnhMetaFileDescription(*this, bytes, (LPTSTR)buffer);
}

//
/// Retrieves the header information for the enhanced metafile.
//
inline uint TEnhMetaFilePict::GetHeader(uint bytes, ENHMETAHEADER* record) {
  return ::GetEnhMetaFileHeader(*this, bytes, record);
}

//
/// Retrieves the palette entries of the enhanced metafile.
//
inline uint TEnhMetaFilePict::GetPaletteEntries(uint count, PALETTEENTRY* entries) {
  return ::GetEnhMetaFilePaletteEntries(*this, count, entries);
}



} // OWL namespace

#endif  // OWL_METAFILE_H
