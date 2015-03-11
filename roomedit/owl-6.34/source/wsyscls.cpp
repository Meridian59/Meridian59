//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file wsyscls.cpp
/// Implementation of window system structure and type encapsulation
//----------------------------------------------------------------------------

#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/wsyscls.h>
#include <owl/private/memory.h>
#include <owl/shellitm.h>
#include <owl/time.h>
#include <Oleauto.h> // for VarDateFromStr

#include <owl/except.rh>

namespace owl {

//
/// Supports drag and drop.
//
TFileDroplet::TFileDroplet(const tstring& fileName, const TPoint& p, bool inClient)
:
  FileName(strnewdup(fileName.c_str())),
  Point(p),
  InClientArea(inClient)
{
}

//
/// Constructs a TFileDroplet given a DropInfo and a file index.
///
/// The location is relative to the client coordinates, and will have negative
/// values if dropped in the non-client parts of the window.
///
/// DragQueryPoint copies that point where the file was dropped and returns whether
/// or not the point is in the client area.  Regardless of whether or not the file
/// is dropped in the client or non-client area of the window, you will still
/// receive the file name.
//
TFileDroplet::TFileDroplet(TDropInfo& drop, int i)
{
  // Tell DragQueryFile the file wanted (i) and the length of the buffer.
  //
  int  namelen = drop.DragQueryFileNameLen(i) + 1;
  FileName = new tchar[namelen];

  drop.DragQueryFile(i, FileName, namelen);

  InClientArea = drop.DragQueryPoint(Point);
}

//
/// The destructor for this class.
///
/// Clean up the new'd filename
//
TFileDroplet::~TFileDroplet()
{
  delete[] FileName;
}

//
/// Releases any memory allocated for the transferring of this TDropInfo object's
/// files during drag operations.
//
void 
TDropInfo::DragFinish() const
{
  TShell::DragFinish(Handle);
}

//
// Functor for CopyText; see DragQueryFile below.
//
struct TDragQueryFile
{
  HDROP Handle;
  uint Index;

  int operator()(LPTSTR buf, int bufSize)
  {return TShell::DragQueryFile(Handle, Index, buf, bufSize);}
};

//
/// String-aware overload
//
tstring
TDropInfo::DragQueryFile(uint index) const
{
  PRECONDITION(index <= 0x7FFFFFFFu);
  TDragQueryFile f = {Handle, index};
  return CopyText(DragQueryFileNameLen(index), f);
}

//
/// Retrieves the name of the file and related information for this i object. If
/// index is set to -1 (0xFFFF), DragQueryFile returns the number of dropped files.
/// This is equivalent to calling DragQueryFileCount.
/// If index lies between 0 and the total number of dropped files for this object,
/// DragQueryFile copies to the name buffer (of length nameLen bytes) the name of
/// the dropped file that corresponds to index, and returns the number of bytes
/// actually copied.
/// If name is 0, DragQueryFile returns the required buffer size (in bytes) for the
/// given index. This is equivalent to calling DragQueryFileNameLen.
//
uint 
TDropInfo::DragQueryFile(uint index, LPTSTR name, uint nameLen) const
{
  PRECONDITION(name && nameLen);
  return TShell::DragQueryFile(Handle, index, name, nameLen);
}
//
/// Returns the number of dropped files in this TDropInfo object. This call is
/// equivalent to calling DragQueryFile(-1, 0, 0).
//
uint 
TDropInfo::DragQueryFileCount() const
{
  return TShell::DragQueryFile(Handle, uint(-1), 0, 0);
}
//
/// Returns the length of the name of the file in this TDropInfo object
/// corresponding to the given index. This call is equivalent to calling
/// DragQueryFile(index, 0, 0).
//
uint 
TDropInfo::DragQueryFileNameLen(uint index) const
{
  return TShell::DragQueryFile(Handle, index, 0, 0);
}

//
/// Functional-style overload
//
std::pair<TPoint, bool>
TDropInfo::DragQueryPoint() const
{
  TPoint p;
  bool r = DragQueryPoint(p);
  return std::make_pair(p, r);
}

//
/// Retrieves the mouse pointer position when this object's files are dropped and
/// copies the coordinates to the given point object. point refers to the window
/// that received the WM_DROPFILES message. DragQueryPoint returns true if the drop
/// occurs inside the window's client area, otherwise false.
//
bool 
TDropInfo::DragQueryPoint(TPoint& point) const
{
  return TShell::DragQueryPoint(Handle, &point);
}

TFileTime::TFileTime(const TTime& tm)
{
  *this = tm.GetFileTime();
  //uint16 fdate, ftime;
  //__MakeDosTime(tm, fdate, ftime);
  //DosDateTimeToFileTime(fdate,ftime,this)
}

TFileTime& 
TFileTime::operator=(const TTime& tm)
{
  *this = tm.GetFileTime();
  //uint16 fdate, ftime;
  //__MakeDosTime(tm, fdate, ftime);
  //DosDateTimeToFileTime(fdate,ftime,this)
  return *this;
}

bool TFileTime::ToLocalTime()
{ 
  TFileTime tm; 
  bool retval = ::FileTimeToLocalFileTime(this, &tm); 
  *this = tm;
  return retval; 
}

bool TFileTime::ToUniversalTime()
{ 
  TFileTime tm; 
  bool retval = ::LocalFileTimeToFileTime(this, &tm); 
  *this = tm;
  return retval; 
}

//#if WINVER < 0x0501 // TODO: This is a guess (Windows XP). In which version did the signature actually change?
#if (defined(__BORLANDC__) && __BORLANDC__ < 0x590) || (defined(_MSC_VER) && _MSC_VER < 1500)
#define VARDATEFROMSTR_PARAM1(a) const_cast<OLECHAR*>(a)
#else
#define VARDATEFROMSTR_PARAM1(a) a
#endif

TSystemTime::TSystemTime(const tstring& s, LCID locale)
{
  _USES_CONVERSION_A;
  LPCOLESTR olestr = _A2W_A(s.c_str());
  const LCID lcid = locale ? locale : GetUserDefaultLCID();
  const ULONG flags = 0;
  DATE date;
  HRESULT hr = VarDateFromStr(VARDATEFROMSTR_PARAM1(olestr), lcid, flags, &date);
  if (hr != S_OK) TXOwl::Raise(IDS_SYSTEMTIMEPARSEFAIL);
  int r = VariantTimeToSystemTime(date, this);
  if (r == FALSE) TXOwl::Raise(IDS_SYSTEMTIMEPARSEFAIL);
}

} // OWL namespace
