//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TFileName class implementation.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/filename.h>
#include <owl/private/memory.h>
#include <owl/commdial.h>


namespace owl {


#define WIN95_LONGFILENAME_MAX 260

      static tchar serverNamePrefix[] = _T("\\\\");
      const int serverNamePrefixLen = sizeof serverNamePrefix / sizeof(tchar) - 1;
//      static tchar devTerminator[] = "\\/:";
      static tchar ldevTerminatorCh = _T(':');
      static tchar ldevTerminatorStr[] = _T(":");
#if defined(UNIX)
      static tchar dirSeparator[] = _T("/");
//      static tchar dirSeparatorCh = _T('/');
      static tchar dirSeparatorStr[] = _T("/");
#else
      static tchar dirSeparator[] = _T("\\/");
//      static tchar dirSeparatorCh = _T('\\');
      static tchar dirSeparatorStr[] = _T("\\");
#endif //if UNIX
      static tchar extSeparatorCh = _T('.');
      static tchar extSeparatorStr[] = _T(".");


//
// Some Win32 APIs that are available under Win16 via DOS calls
//

//
// overloaded functions to perform string scanning
//
static const tchar* fn_strend(const tchar* str)
{
  while (*str)
    str++;
  return str;
}

static const tchar* fn_find(const tchar* start, const tchar* end, int match)
{
  while (start != end && *start != match)
    start++;
  return start;
}

static const tchar* fn_find(const tchar* start, const tchar* end, const tchar* set)
{
  const tchar* setend = set + ::_tcslen(set);
  while (start != end && fn_find(set, setend, *start) == setend)
    start++;
  return start;
}

static const tchar* fn_findl(const tchar* start, const tchar* end, int match)
{
  while (start != end && *start != match)
    start--;
  return start;
}

static const tchar* fn_findl(const tchar* start, const tchar* end, const tchar* set)
{
  const tchar* setend = set + ::_tcslen(set);
  while (start != end && fn_find(set, setend, *start) == setend)
    start--;
  return start;
}

static const tchar* fn_skipstr(const tchar* start, const tchar* end,
                               const tchar* skip)
{
  while (start != end && *skip && *start == *skip)
    start++, skip++;

  return start;

/*
// TODO: Verify this does not break anything -jhh
  if ((start[0] == '\\') && (start[1] == '\\'))
    return &start[2];
  else
    return start;
//jhh  while (*skip && *start == *skip && start != end)
//jhh    start++, skip++;

//jhh
//jhh  return start;
*/
}

//-----
// Low level helper functions for parsing filename parts
//

//
//
//
static 
tstring __formServer(const tchar* serverName)
{
  if (!serverName)
    return serverName;

  const tchar* serverNameEnd = fn_strend(serverName);

  if (_tcsncmp(serverName, serverNamePrefix, serverNamePrefixLen) == 0)
    serverName += serverNamePrefixLen;               // skip "\\"

  const tchar* ds = fn_find(serverName, serverNameEnd, dirSeparator);
  return tstring(serverName, 0, (int)(ds - serverName));
}

//
//
//
static 
tstring __formDevice(const tchar* deviceName, bool isUNC)
{
  if (!deviceName)
    return deviceName;

  const tchar* deviceNameEnd = fn_strend(deviceName);


  if (isUNC)
    deviceNameEnd = fn_find(deviceName, deviceNameEnd, dirSeparator);
  else
    deviceNameEnd = fn_find(deviceName, deviceNameEnd, ldevTerminatorCh);

  return tstring(deviceName, 0, (int)(deviceNameEnd - deviceName));
}

//
// Return a well formed path from any kind of filename part. A lone name
// is assumed to be just the path
//
static 
tstring __formPath(const tchar* path)
{
  if (!path)
    return path;

  const tchar* pathEnd = fn_strend(path);

  // Find the start of the path, skipping logical device or server+share
  //
  const tchar* dvs = fn_find(path, pathEnd, ldevTerminatorCh);
  if (dvs < pathEnd) {
    path = dvs + 1;  // skip logical device
  }

  return tstring(path, 0, (int)(pathEnd - path));
}


//
// Return a well formed file from any kind of filename part. A lone name
// is assumed to be just the file
//
//   "dir.x\file.ext"  -> "file"
//   "dir.x\"          -> ""
//   "file"            -> "file"
//
static 
tstring __formFile(const tchar* file)
{
  if (!file)
    return file;

  const tchar* fileEnd = fn_strend(file);

  const tchar* ds = fn_findl(fileEnd-1, file-1, dirSeparator);
  if (ds >= file)
    file = ds + 1;

  const tchar* es = fn_findl(fileEnd-1, file-1, extSeparatorCh);
  if (es >= file)
    fileEnd = es;

  return tstring(file, 0, (int)(fileEnd - file));
}

//
// Return a well formed extension from any kind of filename part. A lone name
// is assumed to be just the extension and a '.' is prepended
//
//   "dir.x\file.ext"  -> ".ext"
//   "dir.x\"          -> "."
//   "ext"             -> ".ext"
//
static 
tstring __formExt(const tchar* ext)
{
  if (!ext)
    return ext;  // or extSeparatorCh?

  const tchar* extEnd = fn_strend(ext);

  const tchar* ds = fn_findl(extEnd-1, ext-1, dirSeparator);
  if (ds >= ext)
    ext = ds + 1;

  const tchar* es = fn_findl(extEnd-1, ext-1, extSeparatorCh);
  if (es >= ext)
    return es;

  return tstring(extSeparatorStr) + ext;
}

//
/// Constructs an empty filename.
//
TFileName::TFileName()
{
}

//
/// Constructs a filename from another filename.
//
TFileName::TFileName(const TFileName& src)
:
  Unc(src.Unc),
  ServerStr(src.ServerStr),
  DeviceStr(src.DeviceStr),
  PathStr(src.PathStr),
  FileStr(src.FileStr),
  ExtStr(src.ExtStr)
{
  FormCanonicalName();
}

//
/// Constructs a filename from its parts. Use 0 to skip any of the parameters.
/// The form is:
/// \code
///   filename := [[servername]devicename][path][file[ext]]
/// \endcode
///  where:
/// \code
///   servername := \\multiname\         //
///   devicename := name(: | \ | :\)
///   sharename := name
///   logicaldevice := name
///
///   path := multiname\[...]
///   file := multiname
///   ext := [.[name]]
///
///   name := (filecharset)[...]
///   multiname := (filecharset | .)[...]
/// \endcode
TFileName::TFileName(LPCTSTR serverName, LPCTSTR deviceName, LPCTSTR path,
              LPCTSTR file, LPCTSTR ext)
{
  ServerStr = __formServer(serverName);

  Unc = serverName != 0;

  DeviceStr = __formDevice(deviceName, Unc);
  PathStr += __formPath(path);
  FileStr += __formFile(file);
  ExtStr += __formExt(ext);
  FormCanonicalName();
}


//
/// Parses the freeform string into the filename parts.
//
void
TFileName::Parse(LPCTSTR freeform, bool pathOnly)
{
  const tchar* nextp;
  const tchar* freeformEnd = fn_strend(freeform);

  if (*freeform == '\0')
  {
    Unc = false;
    ServerStr = _T("");
    DeviceStr = _T("");
    ExtStr = _T("");
    PathStr = _T("");
    FileStr = _T("");
    FormCanonicalName();
    return;
  }
  // Parse from the left, the server name and device if UNC "\\", or the
  // logical device name if one.
  //
  nextp = fn_skipstr(freeform, freeformEnd, serverNamePrefix);
  if (nextp >= freeform+serverNamePrefixLen) {
    Unc = true;
    freeform = nextp;               // skip "\\"
    nextp = fn_find(freeform, freeformEnd, dirSeparator);
    ServerStr = tstring(freeform, 0, (int)(nextp-freeform));
    freeform = nextp + 1;               // skip past "\"

    nextp = fn_find(freeform, freeformEnd, dirSeparator);
    DeviceStr = tstring(freeform, 0, (int)(nextp-freeform));
    freeform = nextp + 1;               // skip past "\"
  }
  else {
    Unc = false;
    ServerStr = _T("");

    nextp = fn_find(freeform, freeformEnd, ldevTerminatorCh);
    if (nextp != freeformEnd) {
      DeviceStr = tstring(freeform, 0, (int)(nextp-freeform));
      freeform = nextp + 1;               // skip past "\/:"
    }
    else
      DeviceStr = _T("");
  }
  
  if (!pathOnly){
    // Parse the extension and filename from the right.
    //
    nextp = fn_findl(freeformEnd-1, freeform-1, extSeparatorCh);
  //DLN freeform is just before first char in string, so boundschecker will
  //    report an erroneous error here
    const tchar* dirp = fn_findl(freeformEnd-1, freeform-1, dirSeparator);

    if (nextp > dirp) {
      ExtStr = tstring(nextp, 0, (int)(freeformEnd-nextp));
      freeformEnd = nextp;
    }
    else
      ExtStr = _T("");

    FileStr = tstring(dirp+1, 0, (int)(freeformEnd - (dirp+1)));
    freeformEnd = dirp;
  }
  else{
    FileStr = _T("");
    ExtStr = _T("");
  }

  // What is left is the path
  //
  if (freeformEnd > freeform)
    PathStr = tstring(freeform, 0, (int)(freeformEnd-freeform));
  else
    PathStr = _T("");
  FormCanonicalName();
}

//
/// Construct a filename given the full path in any form
//
TFileName::TFileName(LPCTSTR freeform, bool pathOnly)
{
  Parse(freeform,pathOnly);
}


//
/// Construct a filename given the full path in any form
//
TFileName::TFileName(const tstring& freeform, bool pathOnly)
{
  Parse(freeform.c_str(),pathOnly);
}


//
/// Construct a filename representing a special file or directory. The parameter
/// type can be specified from TSpecialType.
/// \todo Looks like not all the types are implemented or work correctly. Need to research
///
//
TFileName::TFileName(TFileName::TSpecialType type)
{
  switch (type) {
    case TempFile:
    case TempDir: {
      int tmpLen = ::GetTempPath(0,0);
      TTmpBuffer<tchar> __clnObj(tmpLen);
      tchar* tmpPath = __clnObj;
      ::GetTempPath(tmpLen, tmpPath);
      if (type == TempDir) {
        //_tcscat(tmpPath,dirSeparatorStr); // !BUG: Writes past EOB. Also, path already has trailing backslash. [VH]
        Parse(tmpPath,true);
      }
      else {
        TTmpBuffer<tchar> tmpFile(WIN95_LONGFILENAME_MAX);
        ::GetTempFileName((tchar*)tmpPath, _T("TFN"), 0, tmpFile);
        Parse((tchar*)tmpFile);
      }
    }
    break;
    case CurrentDir: {
      TTmpBuffer<tchar> buffer(WIN95_LONGFILENAME_MAX);
      ::GetCurrentDirectory(WIN95_LONGFILENAME_MAX, (tchar*)buffer);
      _tcscat((tchar*)buffer, dirSeparatorStr);
      Parse((tchar*)buffer,true);
    }
    break;
    case HomeDir: {
      TTmpBuffer<tchar> buffer(WIN95_LONGFILENAME_MAX);
      ::GetWindowsDirectory((tchar*)buffer, WIN95_LONGFILENAME_MAX);
      _tcscat((tchar*)buffer,dirSeparatorStr);
      Parse((tchar*)buffer,true);
    }
    break;
    case BootDir:{
        LPCTSTR path = _tgetenv(_T("wonbootdir"));
        if(path)
          Parse(path,true);
        break;
      }
    case MachineDir:
      // !Y.B ??????????????
    case SharedDir:
      // !Y.B ??????????????
    break;
    case SysDir: {
      tchar tmp[WIN95_LONGFILENAME_MAX];
      ::GetSystemDirectory(tmp, WIN95_LONGFILENAME_MAX);
      _tcscat(tmp,dirSeparatorStr);
      Parse(tmp,true);
    }
    break;
    case ComputerName: {
      tchar tmp[WIN95_LONGFILENAME_MAX];
      DWORD size = WIN95_LONGFILENAME_MAX;
      ::GetComputerName(tmp, &size);

      Parse(tmp);
    }
    break;
  }
}


//
/// Assigns new filename or freeform to this name.
//
TFileName&
TFileName::operator =(const TFileName& src)
{
  Unc = src.Unc;
  ServerStr = src.ServerStr;
  DeviceStr = src.DeviceStr;
  PathStr = src.PathStr;
  FileStr = src.FileStr;
  ExtStr = src.ExtStr;
  FormCanonicalName();
  return *this;
}


//
//
//


// JJH this function is not used
//static void
//ForceFullPath(string& canonical)
//{
//  if (canonical != "")
//    {
//    tchar buffer[WIN95_LONGFILENAME_MAX];
//    ::GetFullPathName(canonical.c_str(), WIN95_LONGFILENAME_MAX, buffer, 0);
//    canonical = buffer;
//    }
//}


tchar* TFileName::GetFullFileName()
{
  return (tchar*)CanonicalStr();
}

//
// Reassemble the filename parts into a canonical form & expand any relative
// dirs
//

//string TFileName::Canonical(bool forceUNC) const
void TFileName::FormCanonicalName()
{
//  if (forceUNC)
//    {
//    strcpy(NameBuffer,serverNamePrefix);
//    strcat(NameBuffer,DeviceStr);
//    strcat(NameBuffer, dirSeparatorStr);
//    strcat(NameBuffer,PathStr);
//    strcat(NameBuffer,dirSeparatorStr);
//    strcat(NameBuffer,FileStr);
//    strcat(NameBuffer,ExtStr);
//  else
  GetParts(Server|Device|Path|File|Ext); //fills NameBuffer

  if (_tcscmp(NameBuffer,_T(""))) //if file name is not blank
    {                        //  use it to get official file name
    tchar buffer[WIN95_LONGFILENAME_MAX];
    ::GetFullPathName(NameBuffer, WIN95_LONGFILENAME_MAX, buffer, 0);
    FullCanonicalName = buffer;
    }
  else
    FullCanonicalName = NameBuffer;
//  return NameBuffer;
}

/// Return normal fully qualified path string. Convert a possibly logical drive based
/// name to a UNC name if indicated. 
const tstring& TFileName::Canonical(bool) const
{
  return FullCanonicalName;
}

tchar* TFileName::GetNameNoExt()
{
  return (tchar*)GetParts(File);
}

tchar* TFileName::GetNameAndExt() const
{
  return (tchar*)GetParts(File|Ext);  
}

tchar* TFileName::GetFullFileNameNoExt()
{
  return (tchar*)GetParts(Server|Device|Path|File); //fills NameBuffer
}



//
/// Returns the short file path. Convert a possibly logical drive based name to a
/// UNC name if indicated.
//
const tchar*
TFileName::ShortName(bool forceUNC) const
{
  tchar tmp[WIN95_LONGFILENAME_MAX];
  ::GetShortPathName(CanonicalStr(forceUNC), tmp, WIN95_LONGFILENAME_MAX);

  _tcscpy(NameBuffer,tmp);
  return (tchar*)NameBuffer;
}


//
/// The Title member function uses system conventions to get a human-readable
/// rendering of the filename appropriate for use in titles.
/// \note Returns null string for directory names or malformed names.  For
/// directories, try Canonical() instead.
//
const tchar* TFileName::Title() const
{
  tstring canonical = Canonical();
  tchar tmp[WIN95_LONGFILENAME_MAX];
  TCommDlg::GetFileTitle(canonical.c_str(), tmp, sizeof tmp);
  _tcscpy(NameBuffer,tmp);
  return (tchar*)NameBuffer;
}


//
// input  filemame.ext
// output file...
//
static tstring& trimmStr(tstring& s, uint len)
{
  if(s.length() < len)
    return s;
  s.erase(len-3);
  s += _T("...");
  return s;
}

//
// input  ProgrammFiles\Common File\New Files\Short
// output ProgrammFiles\...\Short
//
static tstring& trimPath(tstring& s, uint len)
{
  if(s.length() < len)
    return s;

  tchar* addPath = _T("\\...");
  // find ?
  // 1. get last path  path1\path2\path3 -> \path3
  tstring subpath = s;
  size_t pos = subpath.find_last_of(dirSeparatorStr);
  if(pos == NPOS || len <= 5){
    s = addPath;
    return s;
  }
    
  tstring lastpath = subpath.substr(pos);
  subpath.erase(pos);

  // calculate minimum path length include "\\..."
  int length = len-4-lastpath.length();
  while(length < 0 && lastpath.length() > 0){
      lastpath = lastpath.substr(1); //Remove the first character
      length = len-4-lastpath.length();
  }
  uint iteration = 0;

  // while remainig string greater then our length
  while(static_cast<int>(subpath.length()) > length && subpath.length() > 0){
    size_t pos = subpath.find_last_of(dirSeparatorStr);
    if(pos != NPOS)
      subpath.erase(pos);
    else
      subpath = _T("");

    iteration++;
  }

  // if was only one path1 -> lastpath => path1, and subpath => 0
  if(iteration){
    s = subpath;
    s += addPath;
    s += lastpath;
  }
  else if(s.length() > len){
    s = subpath;
    s += addPath;
  }

  return s;
}
//
static tstring getAttempt(const TFileName& This, const tstring& name, 
                              int index)
{
  uint parts[] = {
    TFileName::Server | TFileName::Device | TFileName::Path,  // Whole thing
    TFileName::Server | TFileName::Device,
    TFileName::File | TFileName::Ext,
  };
  tstring attempt = This.GetParts(parts[index]);
  if(!attempt.empty()){
    tchar lastCh = attempt.c_str()[attempt.length()-1];
    if(lastCh != _T('\\') && lastCh != _T('/'))
      attempt += dirSeparatorStr;
  }
  attempt += name;
  return attempt;
}
//
/// Obtains a human-readable form of the filename. The name length is at most maxLen
/// characters.
/// \code
/// lpszCanon = C:\MYAPP\DEBUGS\C\TESWIN.C
///
/// maxLen   b   Result
/// ------   -   ---------
///  1-3     F   <empty>
///  4       F   T...
///  5       F   TE...
///  6       F   TES...
///  7       F   TESW...
///  4- 7    T   TESWIN.C
///  8-14    x   TESWIN.C
/// 15-16    x   C:\...\TESWIN.C
/// 17-23    x   C:\MYAPP\...\TESWIN.C
/// 24-25    x   C:\MYAPP\...\C\TESWIN.C
/// 26+      x   C:\MYAPP\DEBUGS\C\TESWIN.C
/// \endcode
//
const tchar* TFileName::Squeezed(int maxLen, bool keepName) const
{
  if (maxLen <= 3)
    return _T("");

  const uint n = static_cast<uint>(maxLen);
  tstring fname = Title();
  tstring attempt = getAttempt(*this, fname, 0); // full path

  // If maxLen is not enough to hold the full path name, then trim.
  // If maxLen is enough to hold at least the basename, and enough 
  // to hold at least server/device and "...\", trim the path,
  // otherwise trim (or keep) only the basename.

  if (n < attempt.length())
  {
    const tstring dname = getAttempt(*this, fname, 1); // server-device/name
    if (n > dname.length() + 3)
    {
      tstring p = PathStr;
      trimPath(p, n - dname.length()); 
      if (p.length() > 0 && p[p.length() - 1] != dirSeparatorStr[0])
        p += dirSeparatorStr;
      attempt = GetParts(Server | Device) + p + fname;
    }
    else 
    {
      attempt = keepName ? fname : trimmStr(fname, n);
  }
  }

  CHECK(attempt.length() < _MAX_PATH - 1);
    _tcscpy(NameBuffer,attempt.c_str());
  return NameBuffer;
}


//
/// Determines whether a filename is valid.
//
bool
TFileName::IsValid() const
{
  OFSTRUCT ofs;
  ofs.cBytes = sizeof ofs;
  _USES_CONVERSION; 
  return ::OpenFile(_W2A(CanonicalStr()), &ofs, OF_PARSE) != -1;
}

//
/// Determines whether the device, directory, or file exists.
//
bool
TFileName::Exists() const
{
#if 0
  OFSTRUCT ofs;
  ofs.cBytes = sizeof ofs;
  _USES_CONVERSION; 
  return ::OpenFile(_W2A(Canonical().c_str()), &ofs, OF_EXIST) != -1;
#else
  HANDLE h;
  {
    WIN32_FIND_DATA data;
    h = ::FindFirstFile(CanonicalStr(), &data);
    ::FindClose(h);
  }
  return (h != INVALID_HANDLE_VALUE);
#endif
}


//
/// Reassembles any logical subset of filename parts. More than one part can be
/// checked for by bitwise oring in the parameter p. See TFileName::TPart.
//
const tchar*
TFileName::GetParts(uint p) const
{
  *NameBuffer = 0; //JRB This crashes GWANGI


  if (Unc) 
  {
    if (p & Server) 
    {
      _tcscat(NameBuffer,serverNamePrefix);
      _tcscat(NameBuffer,ServerStr.c_str());
      if (p & Device)
        _tcscat(NameBuffer,dirSeparatorStr);
    }
    if ((p & Device) && DeviceStr.length() > 0) {
      _tcscat(NameBuffer, DeviceStr.c_str());
      if ((p & Path) || (p & File))
        _tcscat(NameBuffer, dirSeparatorStr);
    }
  }
  else 
  {
    if ((p & Device) && DeviceStr.length() > 0) {
      _tcscat(NameBuffer, DeviceStr.c_str());
      _tcscat(NameBuffer, ldevTerminatorStr);
      if ((p & Path) && PathStr.length() == 0) {
        _tcscat(NameBuffer, dirSeparatorStr);
      }
    }
  }

  if ((p & Path) && PathStr.length() > 0)
  {
    _tcscat(NameBuffer,PathStr.c_str());
    _tcscat(NameBuffer, dirSeparatorStr);
  }

  if ((p & File) && FileStr.length() > 0)
    _tcscat(NameBuffer, FileStr.c_str());

  if ((p & Ext) && ExtStr.length() > 0)
    _tcscat(NameBuffer, ExtStr.c_str());
  return (tchar*)NameBuffer;

}

//
/// Returns true if any of the parts specified by p are used. More than one part can
/// be checked for by bitwise oring in the parameter p. See TFileName::TPart.
//
bool
TFileName::HasParts(uint p) const
{
  bool has = false;
  if (p & Server)
    has |= (ServerStr.length() > 0);
  if (p & Device)
    has |= (DeviceStr.length() > 0);
  if (p & Path)
    has |= (PathStr.length() > 0);
  if (p & File)
    has |= (FileStr.length() > 0);
  if (p & Ext)
    has |= (ExtStr.length() > 0);
  return has;
}

//
/// Replaces the specified filename parts with parts extracted from partStr. More
/// than one part can be checked for by bitwise oring in the parameter p. See TFileName::TPart.
//
TFileName&
TFileName::SetPart(uint p, const tstring& partStr)
{
  if (p & Server)
    ServerStr = partStr;
  if (p & Device)
    DeviceStr = partStr;
  if (p & Path)
    PathStr = partStr;
  if (p & File)
    FileStr = partStr;
  if (p & Ext)
    ExtStr = partStr;
  FormCanonicalName();
  return *this;
}


//
/// Replaces the specified parts in this with the parts from source. More than one
/// part can be merged by bitwise oring in the parameter p. See TFileName::TPart.
//
TFileName&
TFileName::MergeParts(uint p, const TFileName& source)
{
  // Walk thru bits & SetPart(p) from source.GetParts(p)
  //
  if (p & Server)
    ServerStr = source.ServerStr;//source.GetParts(Server);
  if (p & Device)
    DeviceStr = source.DeviceStr;//source.GetParts(Device);
  if (p & Path)
    PathStr = source.PathStr;//source.GetParts(Path);
  if (p & File)
    FileStr = source.FileStr;//source.GetParts(File);
  if (p & Ext)
    ExtStr = source.ExtStr;//source.GetParts(Ext);
    
  FormCanonicalName(); // BUGFIX: FullCanonicalName wasn't updated! (VH, 2005-07-26)
  return *this;
}

//
/// Removes the specified filename parts from this set of parts. More than one part
/// can be checked for by bitwise oring in the parameter p. See TFileName::TPart.
//
TFileName&
TFileName::StripParts(uint p)
{
  if (p & Server)
    ServerStr = _T("");
  if (p & Device)
    DeviceStr = _T("");
  if (p & Path)
    PathStr = _T("");
  if (p & File)
    FileStr = _T("");
  if (p & Ext)
    ExtStr = _T("");

  FormCanonicalName(); // BUGFIX: FullCanonicalName wasn't updated! (VH, 2005-07-26)
  return *this;
}

//
/// Deletes this file associated with this name if possible; otherwise attempts to
/// delete the directory associated with this name (which must be empty).
//
bool
TFileName::Remove() const
{
  if (!::DeleteFile(CanonicalStr()))
    if(!::RemoveDirectory(CanonicalStr()))
      return false;
  return true;
}

//
/// Move (rename) the file associated with this filename, and change this name to
/// the new name.
/// 
/// Under WinNT how is used to specifie how the file is to be copied:
/// -	ReplaceExisting	= 1
/// -	CopyAllowed			= 2
/// -	DelayUntilReboot	= 4
//
bool 
TFileName::Move(const TFileName& newName, uint32 how)
{
  bool retval = ::MoveFileEx(CanonicalStr(), newName.CanonicalStr(), how);

  if(retval){
    *this = newName;
    return true;
  }

  return false;
}

//
/// Copy the file associated with this filename to a new file. If failIfExists is
/// true, the copy will not take place if a file newName already exists.
//
bool
TFileName::Copy(const TFileName& newName, bool failIfExists) const
{
  return ::CopyFile(CanonicalStr(), newName.CanonicalStr(), 
                    failIfExists) == TRUE;
}

//
/// Creates the directory associated with this filename.
/// 
/// Returns a non-zero value on success. Call LastError for extended information.
//
int
TFileName::CreateDir() const
{
  return ::CreateDirectory(CanonicalStr(), 0) == TRUE;  //LPSECURITY_ATTRIBUTES
}

//
/// Changes the current directory to the directory associated with this filename.
/// 
/// Returns a non-zero value on success. Call LastError for extended information.
//
int
TFileName::ChangeDir() const
{
  return ::SetCurrentDirectory(CanonicalStr()) == TRUE;
}

TFileName&
TFileName::AddSubDir(const tstring& subdir)
{
  if (_tcschr(dirSeparator, subdir[0]) == 0) // The subdir does not begin with the separator
    PathStr += dirSeparatorStr;

  PathStr += subdir;

  if (_tcschr(dirSeparator, PathStr[PathStr.length() - 1]) != 0)  // The resultring string ends with the separator
    PathStr = PathStr.substr(0, PathStr.length() - 1);

  FormCanonicalName();
  return *this;
}


///////////////////////
// struct __TFindFileStr
// ~~~~~~ ~~~~~~~~~~~~~~
//
/// \cond
struct __TFindFileStr{
  HANDLE          Handle;
  WIN32_FIND_DATA Data;
  tstring Direct;
};
/// \endcond

TTime __MakeTTime(uint16 fdt, uint16 ftm);
void __MakeDosTime(const TTime& ctm, uint16& fdm, uint16& ftm);
uint __sys2Atr(uint32 attrib);
uint __attr2Sys( uint attrib);
static
void __FillFileStr( __TFindFileStr& data, TFileStatus& status);


static
void __FillFileStr( __TFindFileStr& data, TFileStatus& status)
{
  ::_tcscpy(status.fullName, data.Data.cFileName);
  status.attribute  = __sys2Atr(data.Data.dwFileAttributes);
  status.createTime = TTime(TFileTime(data.Data.ftCreationTime));
  status.modifyTime = TTime(TFileTime(data.Data.ftLastWriteTime));
  status.accessTime = TTime(TFileTime(data.Data.ftLastAccessTime));
  status.size       = data.Data.nFileSizeLow;
}


//
/// Get the file status struct for the item associated with this filename.
//
int TFileName::GetStatus(TFileStatus& status) const
{
  tstring fileName = Canonical();
  __TFindFileStr findData;
  findData.Handle = ::FindFirstFile(fileName.c_str(), &findData.Data);
  if(findData.Handle == INVALID_HANDLE_VALUE)
    return false;

  __FillFileStr(findData, status);
  ::_tcscpy(status.fullName, fileName.c_str());


  ::FindClose(findData.Handle);

  return true;
}


//
/// Fills the status structure for the file associated with this filename with
/// information from status.
//
int TFileName::SetStatus(const TFileStatus& status)
{
  tstring fileName = Canonical();
  {
    TFile File(fileName.c_str(), TFile::WriteOnly|TFile::PermExclusive|TFile::OpenExisting);
    if(!File.IsOpen())
      return false;
    if(!File.Length(status.size))
      return false;
    TFileTime ftCreation(status.createTime);
    TFileTime ftLastAccess(status.accessTime);
    TFileTime ftLastWrite(status.modifyTime);

    // will work only for TDiskFileHandle
    TDiskFileHandle* handle = TYPESAFE_DOWNCAST(File.GetHandle(),TDiskFileHandle);
    if(!::SetFileTime(handle->Handle, &ftCreation, &ftLastAccess, &ftLastWrite))
      return false;
  }
  uint attr = __attr2Sys(status.attribute);
  if(!::SetFileAttributes(fileName.c_str(), attr))
  {
# pragma warn -ccc
    WARN(1, "TFile::SetStatus() error ::SetFileAttributes(), line: " << __LINE__);
# pragma warn .ccc
    return false;
  }

  return true;
}

//
//
/// Under Win32 a WIN32_FIND_DATA structure (the Data member) is allocated and
/// intialized. If a file is not found the Done member is set true; otherwise the
/// data structure is filled in.
/// 
/// wildName may contain wild card characters and a path.
//
TFileNameIterator::TFileNameIterator(const tstring& wildName)
:
  Done(false)
{
  __TFindFileStr* data = new __TFindFileStr;
  Data = data;
  tchar* p = (tchar*)_tcsrchr(wildName.c_str(),_T('\\'));
  if(p){
    tchar c = *p;
    *p = _T('\0');
    data->Direct = tstring(wildName.c_str()) + _T("\\");
    *p = c;
  }

  data->Handle = ::FindFirstFile(wildName.c_str(), &data->Data);
  if(data->Handle != INVALID_HANDLE_VALUE){
    __FillFileStr( *data, Status);
  }
  else
    Done = true;
}
//------------------------------------------------------------------------------
/// Deletes the find file Data structure.
TFileNameIterator::~TFileNameIterator()
{
  if (Data && ((__TFindFileStr*)Data)->Handle != INVALID_HANDLE_VALUE)
    ::FindClose(((__TFindFileStr*)Data)->Handle);

  delete (__TFindFileStr*)Data;
}
//------------------------------------------------------------------------------
/// Attempts to find the next file. Returns a true if found, false otherwise.
//
bool TFileNameIterator::FindNext(TFileStatus& status, void* findData)
{
  __TFindFileStr& data = *(__TFindFileStr*)findData;
  if (!::FindNextFile(data.Handle, &data.Data))
    return false;

  __FillFileStr(data, status);
  return true;
} 

//==============================================================================

} // OWL namespace

