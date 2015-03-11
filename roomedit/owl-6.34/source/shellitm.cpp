//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementations of the following Win95 Shell Classes:
///    TPidl, TShellItem, and TShellItemIterator
///
/// The following clases are completely implemented in the header file:
///    TShellMalloc, TExtractIcon, TContextMenu, TDataObject, TDropTarget
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/shellitm.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlShell, 1, 0);        // Shell Diagostic Group

////////////////////////////////////////////////////////////////////
//
// class TShell
// ~~~~~ ~~~~~~
// delay loading SHELL32.DLL/SHELL.DLL
  static const tchar shellStr[]            = _T("SHELL32.DLL");
  static const char DragAcceptFilesStr[]  = "DragAcceptFiles";
  static const char DragFinishStr[]        = "DragFinish";
  static const char DragQueryPointStr[]    = "DragQueryPoint";
  static const char SHAddToRecentDocsStr[]= "SHAddToRecentDocs";
  static const char SHChangeNotifyStr[]    = "SHChangeNotify";
  static const char SHGetMallocStr[]      = "SHGetMalloc";
  static const char SHGetSpecialFolderLocationStr[] = "SHGetSpecialFolderLocation";
  static const char SHGetDesktopFolderStr[] = "SHGetDesktopFolder";
#if defined(UNICODE)
    static const char DragQueryFileStr[]    = "DragQueryFileW";
    static const char ExtractIconStr[]      = "ExtractIconW";
    static const char ShellExecuteStr[]      = "ShellExecuteW";
    static const char Shell_NotifyIconStr[] = "Shell_NotifyIconW";
    static const char SHBrowseForFolderStr[]= "SHBrowseForFolderW";
    static const char SHFileOperationStr[]  = "SHFileOperationW";
    static const char SHGetFileInfoStr[]    = "SHGetFileInfoW";
    static const char SHGetPathFromIDListStr[] = "SHGetPathFromIDListW";
#else
    static const char DragQueryFileStr[]    = "DragQueryFileA";
    static const char ExtractIconStr[]      = "ExtractIconA";
    static const char ShellExecuteStr[]      = "ShellExecuteA";
    static const char Shell_NotifyIconStr[] = "Shell_NotifyIconA";
    static const char SHBrowseForFolderStr[]= "SHBrowseForFolderA";
    static const char SHFileOperationStr[]  = "SHFileOperationA";
    static const char SHGetFileInfoStr[]    = "SHGetFileInfoA";
    static const char SHGetPathFromIDListStr[] = "SHGetPathFromIDListA";
#endif


//
/// Returns TModule object wrapping the handle of the SHELL[32].DLL module
//
TModule&
TShell::GetModule()
{
  static TModule shellModule(shellStr, true, true, false);
  return shellModule;
}
//
/// Invokes 'DragAcceptFiles' indirectly
//
void          
TShell::DragAcceptFiles(HWND p1, BOOL p2)
{
  static TModuleProcV2<HWND,BOOL>
         dragAcceptFiles(GetModule(), DragAcceptFilesStr);
  dragAcceptFiles(p1,p2);
}
//
/// Invokes 'DragFinish' indirectly
//
void          
TShell::DragFinish(HDROP p1)
{
  static TModuleProcV1<HDROP>
         dragFinish(GetModule(), DragFinishStr);
  dragFinish(p1);
}
//
/// Invokes 'DragQueryFile' indirectly
//
UINT 
TShell::DragQueryFile(HDROP p1, UINT p2, LPTSTR p3, UINT p4)
{
  static TModuleProc4<UINT,HDROP,UINT,LPTSTR,UINT>
         dragQueryFile(GetModule(), DragQueryFileStr);
  return dragQueryFile(p1, p2, p3, p4);
}
//
/// Invokes 'DragQueryPoint' indirectly
//
BOOL 
TShell::DragQueryPoint(HDROP p1, LPPOINT p2)
{
  static TModuleProc2<BOOL,HDROP,LPPOINT>
         dragQueryPoint(GetModule(), DragQueryPointStr);
  return dragQueryPoint(p1, p2);
}
//
/// Invokes 'ExtractIcon' indirectly
//
HICON
TShell::ExtractIcon(HINSTANCE p1, LPCTSTR p2, UINT p3)
{
  static TModuleProc3<HICON,HINSTANCE,LPCTSTR,UINT>
         extractIcon(GetModule(), ExtractIconStr);
  return extractIcon(p1, p2, p3);
}
//
/// Invokes 'SHAddToRecentDocs' indirectly
//
void          
TShell::SHAddToRecentDocs(UINT p1, LPCVOID p2)
{
  static TModuleProcV2<UINT,LPCVOID>
         sHAddToRecentDocs(GetModule(), SHAddToRecentDocsStr);
  sHAddToRecentDocs(p1, p2);
}
//
/// Invokes 'SHBrowseForFolder' indirectly
//
LPITEMIDLIST
TShell::SHBrowseForFolder(LPBROWSEINFO p1)
{
  static TModuleProc1<LPITEMIDLIST,LPBROWSEINFO>
         sHBrowseForFolder(GetModule(), SHBrowseForFolderStr);
  return sHBrowseForFolder(p1);
}
//
/// Invokes 'SHChangeNotify' indirectly
//
void          
TShell::SHChangeNotify(LONG p1, UINT p2, LPCVOID p3, LPCVOID p4)
{
  static TModuleProcV4<LONG,UINT,LPCVOID,LPCVOID>
         sHChangeNotify(GetModule(), SHChangeNotifyStr);
  sHChangeNotify(p1, p2, p3, p4);
}
//
/// Invokes 'SHFileOperation' indirectly
//
int            
TShell::SHFileOperation(LPSHFILEOPSTRUCT p1)
{
  static TModuleProc1<int,LPSHFILEOPSTRUCT>
         sHFileOperation(GetModule(), SHFileOperationStr);
  return sHFileOperation(p1);
}
//
/// Invokes 'SHGetDesktopFolder' indirectly
//
HRESULT
TShell::SHGetDesktopFolder(LPSHELLFOLDER* p1)
{
  static TModuleProc1<HRESULT,LPSHELLFOLDER*>
         sHGetDesktopFolder(GetModule(), SHGetDesktopFolderStr);
  return sHGetDesktopFolder(p1);
}
//
/// Invokes 'SHGetFileInfo' indirectly
//
DWORD          
TShell::SHGetFileInfo(LPCTSTR p1, DWORD p2, SHFILEINFO * p3, 
                      UINT p4, UINT p5)
{
  static TModuleProc5<UINT,LPCTSTR,DWORD,SHFILEINFO *,UINT,UINT>
         sHGetFileInfo(GetModule(), SHGetFileInfoStr);
  return sHGetFileInfo(p1, p2, p3, p4, p5);
}

//
/// Invokes 'SHGetMalloc' indirectly
//
HRESULT
TShell::SHGetMalloc(LPMALLOC* p1)
{
  static TModuleProc1<HRESULT,LPMALLOC*>
         sHGetMalloc(GetModule(), SHGetMallocStr);
  return sHGetMalloc(p1);
}
//
/// Invokes 'SHGetPathFromIDList' indirectly
//
BOOL          
TShell::SHGetPathFromIDList(LPCITEMIDLIST p1, LPTSTR p2)
{
  static TModuleProc2<BOOL,LPCITEMIDLIST,LPTSTR>
         sHGetPathFromIDList(GetModule(), SHGetPathFromIDListStr);
  return sHGetPathFromIDList(p1, p2);
}
//
/// Invokes 'SHGetSpecialFolderLocation' indirectly
//
HRESULT        
TShell::SHGetSpecialFolderLocation(HWND p1, int p2, LPITEMIDLIST* p3)
{
  static TModuleProc3<HRESULT,HWND,int,LPITEMIDLIST*>
         sHGetSpecialFolderLocation(GetModule(), SHGetSpecialFolderLocationStr);
  return sHGetSpecialFolderLocation(p1, p2, p3);
}
//
/// Invokes 'ShellExecute' indirectly
//
HINSTANCE
TShell::ShellExecute(HWND p1, LPCTSTR p2, LPCTSTR p3, LPCTSTR p4,
                     LPCTSTR p5, INT p6)
{
  static TModuleProc6<HINSTANCE,HWND,LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR,INT>
         shellExecute(GetModule(), ShellExecuteStr);
  return shellExecute(p1, p2, p3, p4, p5, p6);
}
//
/// Invokes 'Shell_NotifyIcon' indirectly
//
BOOL          
TShell::Shell_NotifyIcon(DWORD p1, PNOTIFYICONDATA p2)  
{
  static TModuleProc2<BOOL,DWORD,PNOTIFYICONDATA>
         shell_NotifyIcon(GetModule(), Shell_NotifyIconStr);
  return shell_NotifyIcon(p1, p2);
}

//////////////////////////////////////////////////////////////////////////
//
///  protected function used internally to free PIDL memory with the shell's allocator
//
void
TPidl::FreePidl()
{
  if (Pidl) {
    TShellMalloc malloc;
    malloc->Free(Pidl);
    Pidl = 0;
  }
}

//
/// GetItemCount returns the number of identifiers in the
/// identifier list (pidl)
//
long 
TPidl::GetItemCount() const
{
  long cnt = 0;

  if (Pidl) {
    LPITEMIDLIST pidl = Pidl;
    while (pidl->mkid.cb) {
      cnt++;
      pidl = Next(pidl);
    }
  }
  return cnt;
}

//
/// GetSize returns the size (in bytes) of the PIDL
//
ulong 
TPidl::GetSize() const
{
  long size = sizeof(Pidl->mkid.cb);
  LPITEMIDLIST pidl = Pidl;
  while (pidl->mkid.cb) {
    size += pidl->mkid.cb;
    pidl = Next(pidl);
  }
  return size;
}

//
/// CopyPidl copies a pidl (allocates memory with the shell's allocator)
//
LPITEMIDLIST 
TPidl::CopyPidl() const
{
  if (!Pidl)
    return 0;
  else {
    LPITEMIDLIST newPidl;
    TShellMalloc malloc;

// Expanded by Val Ovechkin 11:57 AM 6/3/98
    void *p = malloc->Alloc(GetSize());
    newPidl = reinterpret_cast<LPITEMIDLIST>(p);

    WARNX(OwlShell, !newPidl, 0, \
         _T("IShellMalloc::Alloc failed.  IShellMalloc* = ")\
          << hex << STATIC_CAST(IMalloc*, malloc));
    if (!newPidl)
      TXShell::Raise(IDS_SHELLALLOCFAIL);
    LPITEMIDLIST pidlSrc = Pidl;
    LPITEMIDLIST pidlDest = newPidl;
    while (pidlSrc->mkid.cb) {
      CopyMemory(pidlDest, pidlSrc, pidlSrc->mkid.cb);
      pidlDest = Next(pidlDest);
      pidlSrc = Next(pidlSrc);
    }
    ushort zero = 0;
    CopyMemory(pidlDest, &zero, sizeof(ushort));
    return newPidl;
  }
}

//
/// GetLastItem returns the last item in an identifier list
/// for file system items, this is the rightmost part of a path
/// e.g., GetLastItem() on a pidl representing 
/// \code
///       "c:\\dir1\\dir2\\dir3\\file1" 
/// \endcode
/// returns "file1"
TPidl 
TPidl::GetLastItem() const
{
  if (!Pidl)
    return TPidl();
  else {
    long cnt = GetItemCount();
    LPITEMIDLIST pidl = Pidl;
    for (int i = 0; i < cnt - 1; i++)
      pidl = Next(pidl);
    LPITEMIDLIST newPidl;
    TShellMalloc malloc;

// Expanded by Val Ovechkin 11:57 AM 6/3/98
    void *p = malloc->Alloc(pidl->mkid.cb + sizeof (pidl->mkid.cb));
    newPidl = reinterpret_cast<LPITEMIDLIST>(p);

    WARNX(OwlShell, !newPidl, 0, _T("IShellMalloc::Alloc failed.  IShellMalloc* = ")
          << hex << STATIC_CAST(IMalloc*, malloc));
    if (!newPidl)
      TXShell::Raise(IDS_SHELLALLOCFAIL);

    CopyMemory(newPidl, pidl, pidl->mkid.cb);
    LPITEMIDLIST pidlDest = Next(newPidl);
    USHORT zero = 0;
    CopyMemory(pidlDest, &zero, sizeof (USHORT));
    return newPidl;
  }
}

//
/// StripLastItem returns a pidl stipped of its last (rightmost) item
//
TPidl 
TPidl::StripLastItem() const
{
  int i;
  if (!Pidl)
    return TPidl();
  else {
    // calculate the size less the last item id
    long cnt = GetItemCount();
    if (cnt < 2)
      return TPidl();
    long size = sizeof(Pidl->mkid.cb);
    LPITEMIDLIST pidl = Pidl;
    for (i = 0; i < cnt - 1; i++) {
      size += pidl->mkid.cb;
                pidl = Next(pidl);
    }
    LPITEMIDLIST newPidl;
    TShellMalloc malloc;
// Expanded by Val Ovechkin 11:57 AM 6/3/98
    void *p = malloc->Alloc(size);
    newPidl = reinterpret_cast<LPITEMIDLIST>(p);

    WARNX(OwlShell, !newPidl, 0, _T("IShellMalloc::Alloc failed.  IShellMalloc* = ")
          << hex << STATIC_CAST(IMalloc*, malloc));
    if (!newPidl)
      TXShell::Raise(IDS_SHELLALLOCFAIL);
    LPITEMIDLIST pidlSrc = Pidl;
    LPITEMIDLIST pidlDest = newPidl;
    for (i = 0; i < cnt - 1; i++) {
      CopyMemory(pidlDest, pidlSrc, pidlSrc->mkid.cb);
      pidlDest = Next(pidlDest);
      pidlSrc = Next(pidlSrc);
    }
    USHORT zero = 0;
    CopyMemory(pidlDest, &zero, sizeof (USHORT));

    return newPidl;
  }
}

/// TShellItem constructor to make a TShellItem for a file or directory in the file system.
/// If the throwOnInvalidPath argument is true, an exception will be raised if the path is
/// not valid or if the file or directory does not exist.  If this argument is false, no
/// exception will be raised and the Valid() function should be called to make sure it
/// returns true.  If Valid() returns false, the TShellItem does not represent a object
/// in the shell namespace.
//
TShellItem::TShellItem(const tchar* path, bool throwOnInvalidPath, HWND windowOwner)
           :TComRef<IShellFolder>(0), Pidl(0), ParentFolder(0)
{
  TShellItem desktop(TShellItem::Desktop, windowOwner);

  TCreateStruct cs;
  ulong eaten;
  HRESULT hr = desktop.ParseDisplayName(cs, path, &eaten, windowOwner);
  WARNX(OwlShell, FAILED(hr), 0, _T("Invalid path.  Path = ") << path);
  if (FAILED(hr))
    if (throwOnInvalidPath)
      TXShell::Raise(IDS_INVALIDPATH);
    else
      return;

  Pidl = cs.Pidl.GetLastItem();

// Expanded by Val Ovechkin 11:57 AM 6/3/98
  void *p = STATIC_CAST(IShellFolder**, ParentFolder);
  hr = desktop->BindToObject(cs.Pidl.StripLastItem(), 0, IID_IShellFolder,
                             REINTERPRET_CAST(void **, p));

//  hr = desktop->BindToObject(cs.Pidl.StripLastItem(), 0, IID_IShellFolder,
//                             REINTERPRET_CAST(void **, 
//                             STATIC_CAST(IShellFolder**, ParentFolder)));

  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::BindToObject failed.  TShellItem* = ")
        << hex << (LPVOID)this << _T("  IShellFolder* = ")
        << STATIC_CAST(IShellFolder*, ParentFolder));
  TXShell::Check(hr, IDS_BINDTOOBJECTFAIL);
}

//
///  TShellItem constructor to make a TShellItem for a special folder.  Special
///  folders can be any specified in the TSpecialFolderKind enum.
///
///  (see MS doc for SHGetSpecialFolderLocation for more information,
///  Programmer's Guide to MS Windows 95, MS Press, p. 209)
//
TShellItem::TShellItem(const TSpecialFolderKind kind, HWND windowOwner)
           :TComRef<IShellFolder>(0), Pidl(0), ParentFolder(0)
{
  IShellFolder* desktop;
  HRESULT hr = TShell::SHGetDesktopFolder(&desktop);
  WARNX(OwlShell, FAILED(hr), 0, _T("::SHGetDesktopFolder failed."));
  TXShell::Check(hr, IDS_SHGETDESKTOPFAIL);

  TPidl pidl;
  hr = TShell::SHGetSpecialFolderLocation(windowOwner, kind, pidl);
  WARNX(OwlShell, FAILED(hr), 0, \
        _T("::SHGetSpecialFolderLocation failed.  kind = ") << kind);
  TXShell::Check(hr, IDS_SHGETSPECIALFAIL);
  if (kind == Desktop) {
    I = desktop;       // Gets itself for the ParentFolder below
    I->AddRef();
  }
  else {
    hr = desktop->BindToObject(pidl, 0, IID_IShellFolder,
                               REINTERPRET_CAST(void **, STATIC_CAST(IShellFolder**, *this)));
    WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::BindToObject failed.  Desktop IShellFolder* = ")
          << hex << STATIC_CAST(IShellFolder*, desktop));
    TXShell::Check(hr, IDS_BINDTOOBJECTFAIL);
  }

  ParentFolder = desktop;  // It's not clearly documented but we're assuming
  Pidl = pidl;             // that all special folders are children of the
}                          // desktop/root.

//
///  TShellItem constructor to make a TShellItem from a TCreateStruct
///
///  TCreateStruct contains a TPidl and a
///    TComRef<IShellFolder>ParentFolder which represents the parent folder
///
///  TCreateStructs are returned by TShellItem::GetParentFolder() and by the
///    following TShellItemIterator functions:
///      operator ++(pre- &post-fix), operator -- (pre- & post-fix), operator [](),
///      and Current()
///
///  TCreateStructs are also returned as out parameters of the following TShellItem
///    functions:
///      BrowseForFolder(), ParseDisplayName()
//
TShellItem::TShellItem(const TCreateStruct& cs)
           :TComRef<IShellFolder>(0), Pidl(cs.Pidl), 
            ParentFolder(cs.ParentFolder)
{
}

//
///  TShellIterm constructor to make a TShellItem from a ITEMIDLIST(pidl) and a
///    TComRef<IShellFolder> (parent folder)
//
//
TShellItem::TShellItem(LPITEMIDLIST pidl, const TComRef<IShellFolder>& parentFolder)
           :TComRef<IShellFolder>(0), Pidl(pidl), ParentFolder(parentFolder)
{
}

//
///  TShellItem constructor to make a TShellItem from a TPidl and a
///    TComRef<IShellFolder> (parent folder)
//
TShellItem::TShellItem(const TPidl& pidl, const TComRef<IShellFolder>& parentFolder)
           :TComRef<IShellFolder>(0), Pidl(pidl), ParentFolder(parentFolder)
{
}

//
///  TShellItem copy constructor
//
TShellItem::TShellItem(const TShellItem& source)
           :TComRef<IShellFolder>(source), Pidl(source.Pidl), 
            ParentFolder(source.ParentFolder)
{
}

//
/// TShellItem assignment (from another TShellItem)
//
TShellItem& TShellItem::operator = (const TShellItem &source)
{
  if (&source != this) {
    TComRef<IShellFolder>::operator=(source);
    Pidl = source.Pidl;
    ParentFolder = source.ParentFolder;
  }
  return *this;
}

//
/// TShellItem assignment (from a TCreateStruct)
//
TShellItem& TShellItem::operator = (const TCreateStruct& cs)
{
  Clear();
  Pidl = cs.Pidl;
  ParentFolder = cs.ParentFolder;
  return *this;
}

//
///  GetAttributes gets Capabilities, Display, Contents, & Misc. Attributes
///    with a single call
///
///  'validateCachedInfo' defaults to false.  If true, the shell will not 
///   rely on cached information.
///
///  See MS doc for IShellFolder::GetAttributesOf for more information, Programmer's
///    Guide to MS Windows 95, MS Press, pp. 194-195.
//
ulong 
TShellItem::GetAttributes(const ulong reqAttrib, const bool validateCachedInfo) const
{
  EnforceValidity();
  ulong req = reqAttrib | ((validateCachedInfo)? SFGAO_VALIDATE: 0);
  HRESULT hr = ParentFolder->GetAttributesOf(1, Pidl, &req);
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::GetAttributesOf failed.  TShellItem* = ")
        << hex << (LPVOID)this << _T("  Parent IShellFolder* = ")
        << STATIC_CAST(IShellFolder*, *CONST_CAST(TComRef<IShellFolder>*, &ParentFolder)));
  TXShell::Check(hr, IDS_GETATTRIBUTESOFFAIL);
  return req;
}

//
///  GetAttribute is a protected function used by all the attribute functions except
///    IsDesktop.
///
///  \note  GetAttribute gets attributes one at a time.  GetAttributes (with the 's')
///    is used to get any number of attributes with one call.
//
bool 
TShellItem::GetAttribute(const TAttribute at, const bool validateCachedInfo) const
{
  EnforceValidity();
  ulong req = at;
  req = GetAttributes(req, validateCachedInfo);
  return (req & at);
}

// Attributes - Additional (Not part of GetAttributes)

//
///  IsDesktop returns true if the TShellItem respresents the shell's desktop.
//
bool 
TShellItem::IsDesktop() const
{
  EnforceValidity();
  TShellItem desktop(Desktop);
  return (I == desktop.I);
}

//
///  GetParentFolder returns a TCreateStruct representing the folder which contains
///  the TShellItem.
//
TShellItem::TCreateStruct 
TShellItem::GetParentFolder() const
{
  EnforceValidity();
  TCreateStruct cs;

  if (IsDesktop()) {
    cs.Pidl = Pidl;                           // return self
    cs.ParentFolder = ParentFolder;           // ParentFolder is itself too
  }
  else {
    TShellItem desktop(Desktop);
    TPidl newFullPidl = GetFullyQualifiedPidl().StripLastItem();
    cs.Pidl = newFullPidl.GetLastItem();

// Expanded by Val Ovechkin 11:57 AM 6/3/98
    void *p = STATIC_CAST(IShellFolder**, cs.ParentFolder);
    HRESULT hr = desktop->BindToObject(newFullPidl.StripLastItem(), 0, IID_IShellFolder,
          REINTERPRET_CAST(void **, p));

//    HRESULT hr = desktop->BindToObject(newFullPidl.StripLastItem(), 0, IID_IShellFolder,
//          REINTERPRET_CAST(void **, STATIC_CAST(IShellFolder**, cs.ParentFolder)));


    WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::BindToObject failed.  TShellItem* = ")
          << hex << (LPVOID)this << _T("  Parent LPITEMIDLIST = ")
          << static_cast<LPITEMIDLIST>(newFullPidl.StripLastItem()));
    TXShell::Check(hr, IDS_BINDTOOBJECTFAIL);
  }

  return cs;
}

//
///  GetFullyQualifiedPidl returns a TPidl that is fully qualified (from the desktop).
//
TPidl 
TShellItem::GetFullyQualifiedPidl() const
{
  EnforceValidity();
  TString name = GetDisplayName(ForParsing);
  TShellItem desktop(Desktop);
  TCreateStruct cs;
  desktop.ParseDisplayName(cs, name);
  return cs.Pidl;
}

//
/// GetExeType returns the type of executable that is contained in the TShellItem
///    Also, the optional major and minor args return version numbers
///    e.g., return = WindowsPE, major = 4, minor = 0
///
///  See MS doc for SHGetFileInfo for more information, Programmer's Guide to MS
///    Windows 95, MS Press, pp. 205-207.
//
TShellItem::TExeKind 
TShellItem::GetExeType(uint* major, uint* minor) const
{
  EnforceValidity();

  if (!IsPartOfFileSystem() || IsFolder() || ContainsSubFolder())
    return NonExecutable;

  DWORD rc = TShell::SHGetFileInfo(GetPath(), 0, 0, 0, SHGFI_EXETYPE);

  short exeType = LOWORD(rc);

// Expanded by Val Ovechkin 11:57 AM 6/3/98
  void *p = &exeType;
  const char* charType = REINTERPRET_CAST(const char*, p);

//  const char* charType = REINTERPRET_CAST(const char*, &exeType);

  if (major)
    *major = HIBYTE(HIWORD(rc));
  if (minor)
    *minor = LOBYTE(HIWORD(rc));

  WARNX(OwlShell,
        rc && ::strncmp("PE", charType, 2) && ::strncmp("NE", charType, 2) && ::strncmp("MZ", charType, 2),
        0, "::GetFileInfo returned an invalid exeType.  exeType = " << exeType);

  if (!rc)
    return NonExecutable;
  else if (!::strncmp("PE", charType, 2))
    return (HIWORD(rc))? WindowsPE: Win32Console;
  else if (!::strncmp("NE", charType, 2))
    return WindowsNE;
  else if (!::strncmp("MZ", charType, 2))
    return MSDOS;
  else
    TXShell::Raise(IDS_GETFILEINFOFAIL);

  return NonExecutable;     // To satisfy compiler
}

//
///  GetTypeName returns the type of item that is represented by this TShellItem
///    examples:  application, notepad file, ...
///
///  See MS doc for SHGetFileInfo for more information, Programmer's Guide to MS
///   Windows 95, MS Press, pp. 205-207.
//
TString 
TShellItem::GetTypeName() const
{
  EnforceValidity();
  TString typeName;
  SHFILEINFO fi;
  LPCITEMIDLIST pidl = GetPidl();
  DWORD_PTR rc = ::SHGetFileInfo(
    reinterpret_cast<const tchar*>(pidl),
    0, &fi, sizeof (fi), SHGFI_TYPENAME | SHGFI_PIDL);

  WARNX(OwlShell, !rc, 0, _T("::GetFileInfo failed.  TShellItem = ") << (LPCSTR)GetDisplayName());
  if (!rc)
    TXShell::Raise(IDS_GETFILEINFOFAIL);
  typeName = fi.szTypeName;
  return typeName;
}

//
///  GetDisplayName returns a name for this TShellItem that is suitable to display
///    to the user.
///
///  The three options are:
///
/// - \c \b   Normal      - Suitable for displaying by itself
/// - \c \b    InFolder    - Suitable for displaying within its parent folder
/// - \c \b    ForParsing  - Suitable to pass to the ParseDiplayName member function
///
///  See MS doc for IShellFolder::GetDisplayNameOf for more information,
///    Programmer's Guide to MS Windows 95, MS Press, pp. 197-198.
//
TString 
TShellItem::GetDisplayName(const TDisplayNameKind kind) const
{
  EnforceValidity();
  TString s;
  STRRET strRet;

  strRet.uType = STRRET_WSTR;
  HRESULT hr = ParentFolder->GetDisplayNameOf(Pidl, kind, &strRet);
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::GetDisplayNameOf failed.  TShellItem* = ")
        << hex << (LPVOID)this);
  TXShell::Check(hr, IDS_GETDISPLAYNAMEFAIL);

  switch (strRet.uType) {
    case STRRET_WSTR:{   // Case requested
        s = strRet.pOleStr;
        TShellMalloc malloc;
        malloc->Free(strRet.pOleStr);
      }
      break;
    case STRRET_CSTR:   // C-string
      s = strRet.cStr;
      break;
    case STRRET_OFFSET: // Offset into the piddle
      s = reinterpret_cast<const char*>(static_cast<LPCITEMIDLIST>(Pidl)) + strRet.uOffset;
      break;
    }

  return s;
}

//
///  GetPath returns the fully qualified path representing this TShellItem
//
TString 
TShellItem::GetPath() const
{
  tchar path[_MAX_PATH];

  bool success = TShell::SHGetPathFromIDList(GetFullyQualifiedPidl(), path);
  WARNX(OwlShell, !success, 0, "::SHGetPathFromIDList failed.  TShellItem* = "
        << hex << (LPVOID)this);
  if (!success)
    TXShell::Raise(IDS_GETPATHFROMPIDLFAIL);

  TString s;
  s = path;
  return s;
}

//
///  Rename renames this TShellItem to newName.
///
///  kind indicates the kind of name being passed (Normal, InFolder, or ForParsing)
///    (see GetDisplayName for description of these)
///
///  See MS doc for IShellFolder::SetNameOf for more information, Programmer's
///    Guide to MS Windows 95, MS Press, pp. 198-199.
//
void
TShellItem::Rename(const tchar *newName, HWND windowOwner, const TDisplayNameKind kind)
{
  LPITEMIDLIST pidl;

  if (!CanBeRenamed())
    TXShell::Raise(IDS_SHELLRENAMEFAIL);

  // This stuff needed for ::SHChangeNotify
  bool isFolder = IsFolder();
  tchar oldPath[_MAX_PATH];
  memset(oldPath, 0, sizeof(oldPath));
  ::_tcscpy(oldPath, GetDisplayName(ForParsing));
  TShellItem parentFolder = GetParentFolder();
  tchar newPath[_MAX_PATH];
  memset(newPath, 0, sizeof (newPath));
  ::_tcscpy(newPath, parentFolder.GetDisplayName(ForParsing));
  _tcscat(newPath, newName);

  HRESULT hr = ParentFolder->SetNameOf(windowOwner, Pidl, TString(newName), kind, &pidl);
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::SetNameOf failed.  TShellItem* = ")
        << hex << (LPVOID)this << _T("  Parent IShellFolder* = ")
        << STATIC_CAST(IShellFolder*, ParentFolder));
  TXShell::Check(hr, IDS_SETNAMEOFFAIL);
  Pidl = pidl;

  TShell::SHChangeNotify(isFolder? SHCNE_RENAMEFOLDER: SHCNE_RENAMEITEM,
                   SHCNF_PATH /*| SHCNF_FLUSH*/, STATIC_CAST(const tchar*, oldPath),
                   newPath);
}

//
///  Copy copies this TShellItem to destination path
///
///  destIsFolder must tell whether or not dest is a folder
///
///  flags are any combination of TFileOpFlags
///    (see MS doc for SHFILEOPSTRUCT for meanings of these flags,
///    Programmer's Guide to MS Windows 95, MS Press, p. 215)
///
///  Title is the title displayed in the progress dialog box
///
///  windowOwner should be set to the owner of the progress dialog box
///
///  See MS doc for SHFileOperation for more information, Programmer's Guide to
///    MS Windows 95, MS Press, p. 204.
//
bool 
TShellItem::Copy(const tchar* dest, const bool destIsFolder,
                 const USHORT flags, const tchar* title,
                 HWND windowOwner) const
{
  EnforceValidity();
  SHFILEOPSTRUCT fo;

  if (!CanBeCopied())
    TXShell::Raise(IDS_SHELLCOPYFAIL);

  fo.hwnd = windowOwner;
  fo.wFunc = FO_COPY;
  tchar srcPath[_MAX_PATH];
  ::memset(srcPath, 0, sizeof (srcPath));
  ::_tcscpy(srcPath, GetDisplayName(ForParsing));
  fo.pFrom = srcPath;
  tchar destPath[_MAX_PATH];
  ::memset(destPath, 0, sizeof (destPath));
  ::_tcscpy(destPath, dest);
  fo.pTo = destPath;
  fo.fFlags = flags;
  fo.fAnyOperationsAborted = FALSE;
  fo.hNameMappings = 0;
  fo.lpszProgressTitle = title;

  if (!destIsFolder)
    fo.fFlags |= FOF_MULTIDESTFILES; // Destination is a filename, not a folder

  bool isFolder = IsFolder();

  int rc = TShell::SHFileOperation(&fo);

  if (rc || fo.fAnyOperationsAborted)
    return false;

  TShell::SHChangeNotify(isFolder? SHCNE_MKDIR: SHCNE_CREATE, SHCNF_PATH /*| SHCNF_FLUSH*/,
                   STATIC_CAST(const tchar*, GetDisplayName(ForParsing)), 0);
  return true;
}

//
///  Copy copies this TShellItem to destination TShellItem
///
///  flags are any combination of TFileOpFlags
///    (see MS doc for SHFILEOPSTRUCT for meanings of these flags,
///    Programmer's Guide to MS Windows 95, MS Press, p. 215)
///
///  Title is the title displayed in the progress dialog box
///
///  windowOwner should be set to the owner of the progress dialog box
///
///  See MS doc for SHFileOperation for more information, Programmer's Guide to
///    MS Windows 95, MS Press, p. 204.
//
bool 
TShellItem::Copy(const TShellItem& dest, const USHORT flags, const tchar* title,
                 HWND windowOwner) const
{
  return Copy(dest.GetDisplayName(ForParsing), dest.IsFolder(), flags, title, windowOwner);
}

//
///  Move moves this TShellItem to destination path (which must be a folder)
///
///  flags are any combination of TFileOpFlags
///    (see MS doc for SHFILEOPSTRUCT for meanings of these flags,
///    Programmer's Guide to MS Windows 95, MS Press, p. 215)
///
///  Title is the title displayed in the progress dialog box
///
///  windowOwner should be set to the owner of the progress dialog box
///
///  See MS doc for SHFileOperation for more information, Programmer's Guide to
///    MS Windows 95, MS Press, p. 204.
//
bool
TShellItem::Move(const tchar* destFolder, const USHORT flags, const tchar* title,
                 HWND windowOwner)
{
  EnforceValidity();
  SHFILEOPSTRUCT fo;

  if (!CanBeMoved())
    TXShell::Raise(IDS_SHELLMOVEFAIL);

  fo.hwnd = windowOwner;
  fo.wFunc = FO_MOVE;

  tchar oldPath[_MAX_PATH];
  ::memset(oldPath, 0, sizeof (oldPath));
  ::_tcscpy(oldPath, GetDisplayName(ForParsing));
  fo.pFrom = oldPath;
  tchar destPath[_MAX_PATH];
  ::memset(destPath, 0, sizeof (destPath));
  ::_tcscpy(destPath, destFolder);
  fo.pTo = destPath;
  fo.fFlags = flags;
  fo.fAnyOperationsAborted = FALSE;
  fo.hNameMappings = 0;
  fo.lpszProgressTitle = title;

  bool isFolder = IsFolder();

  int rc = TShell::SHFileOperation(&fo);

  if (rc || fo.fAnyOperationsAborted)
    return false;

  tchar newPath[_MAX_PATH];
  ::_tcscpy(newPath, destFolder);
#if defined(BI_DBCS_SUPPORT)
  if (*AnsiPrev(newPath, newPath + ::strlen(newPath)) != '\\')
#else
    if (newPath[::_tcslen(newPath)-1] != _T('\\'))
#endif
    _tcscat(newPath, _T("\\"));
  _tcscat(newPath, GetDisplayName());
  *this = TShellItem(newPath);              // change myself to the new location

  // Notify deletion of old path
  TShell::SHChangeNotify(isFolder? SHCNE_RMDIR: SHCNE_DELETE, SHCNF_PATH /*| SHCNF_FLUSH*/,
                   STATIC_CAST(const tchar*, oldPath), 0);
  // Notify creation of new path
  TShell::SHChangeNotify(isFolder? SHCNE_MKDIR: SHCNE_CREATE, SHCNF_PATH /*| SHCNF_FLUSH*/,
                   STATIC_CAST(const tchar*, GetDisplayName(ForParsing)), 0);

  return true;
}

//
///  Move moves this TShellItem to destination TShellItem (which must be a folder)
///
///  flags are any combination of TFileOpFlags
///    (see MS doc for SHFILEOPSTRUCT for meanings of these flags,
///    Programmer's Guide to MS Windows 95, MS Press, p. 215)
///
///  Title is the title displayed in the progress dialog box
///
///  windowOwner should be set to the owner of the progress dialog box
///
///  See MS doc for SHFileOperation for more information, Programmer's Guide to
///    MS Windows 95, MS Press, p. 204.
//
bool 
TShellItem::Move(const TShellItem& destFolder, const USHORT flags,
                 const tchar* title, HWND windowOwner)
{
  return Move(destFolder.GetDisplayName(ForParsing), flags, title, windowOwner);
}

//
///  Delete deletes this TShellItem
///
///  flags are any combination of TFileOpFlags
///    (see MS doc for SHFILEOPSTRUCT for meanings of these flags,
///    Programmer's Guide to MS Windows 95, MS Press, p. 215)
///
///  Title is the title displayed in the progress dialog box
///
///  windowOwner should be set to the owner of the progress dialog box
///
///  See MS doc for SHFileOperation for more information, Programmer's Guide to
///    MS Windows 95, MS Press, p. 204.
//
bool 
TShellItem::Delete(const USHORT flags, const tchar* title, HWND windowOwner)
{
  EnforceValidity();
  if (!Pidl)           // Nothing to delete!
    return true;

  if (!CanBeDeleted())
    TXShell::Raise(IDS_SHELLDELETEFAIL);

  SHFILEOPSTRUCT fo;
  fo.hwnd = windowOwner;
  fo.wFunc = FO_DELETE;
  tchar path[_MAX_PATH];
  memset(path, 0, sizeof (path));
  ::_tcscpy(path, GetDisplayName(ForParsing)); // Also used to reconstruct if delete fails/user aborts
  fo.pFrom = path;
  fo.pTo = 0;
  fo.fFlags = flags;
  fo.fAnyOperationsAborted = FALSE;
  fo.hNameMappings = 0;
  fo.lpszProgressTitle = title;

  bool isFolder = IsFolder();               // Needed for ::SHChangeNotify

  // About to delete: save and release everything
  //
  TComRef<IShellFolder>::operator=(0);
  Pidl = 0;
  ParentFolder = 0;

  int rc = TShell::SHFileOperation(&fo);
  if (rc || fo.fAnyOperationsAborted) {
    *this = TShellItem(path);  // restore everything
    return false;
  }

  TShell::SHChangeNotify(isFolder? SHCNE_RMDIR: SHCNE_DELETE, SHCNF_PATH /*| SHCNF_FLUSH*/,
                   path, 0);

  return true;
}

//
///  GetExtractIcon returns an interface pointer to this TShellItem's IExtractIcon
///    OLE interface
///
///    (see MS doc for information on IExtractIcon,
///    Programmer's Guide to MS Windows 95, MS Press, pp. 242-244)
//
TExtractIcon 
TShellItem::GetExtractIcon(HWND windowOwner)
{
  EnforceValidity();
  IExtractIcon* iface;

// Expanded by Val Ovechkin 12:29 PM 6/3/98
  void *p = &iface;
  HRESULT hr = ParentFolder->GetUIObjectOf(windowOwner, 1, Pidl, IID_IExtractIcon,
                                           0, REINTERPRET_CAST(void**, p));

//  HRESULT hr = ParentFolder->GetUIObjectOf(windowOwner, 1, Pidl, IID_IExtractIcon,
//                                           0, REINTERPRET_CAST(void**, &iface));
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::GetUIObjectOf(IExtractIcon) ")
        << _T("failed.  TShellItem* = ")
        << hex << (LPVOID)this << _T("  Parent IShellFolder* = ")
        << STATIC_CAST(IShellFolder*, ParentFolder));
  TXShell::Check(hr, IDS_GETUIOBJECTOFFAIL);
  return TExtractIcon(iface);
}

//
///  GetContextMenu returns an interface pointer to this TShellItem's IContextMenu
///    OLE interface
///
///    (see MS doc for information on IContextMenu,
///    Programmer's Guide to MS Windows 95, MS Press, pp. 237-240)
//
TContextMenu 
TShellItem::GetContextMenu(HWND windowOwner)
{
  EnforceValidity();
  IContextMenu* iface;

// Expanded by Val Ovechkin 12:29 PM 6/3/98
  void *p = &iface;
  HRESULT hr = ParentFolder->GetUIObjectOf(windowOwner, 1, Pidl, IID_IContextMenu,
                                           0, REINTERPRET_CAST(void**, p));

//  HRESULT hr = ParentFolder->GetUIObjectOf(windowOwner, 1, Pidl, IID_IContextMenu,
//                                           0, REINTERPRET_CAST(void**, &iface));
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::GetUIObjectOf(IContextMenu) ")
        << _T("failed.  TShellItem* = ")
        << hex << (LPVOID)this << _T("  Parent IShellFolder* = ")
        << STATIC_CAST(IShellFolder*, ParentFolder));
  TXShell::Check(hr, IDS_GETUIOBJECTOFFAIL);
  return TContextMenu(iface);
}

//
///  GetDataObject returns an interface pointer to this TShellItem's IDataObject
///    OLE interface
///
///    (see MS doc for a very brief description of IDataObject,
///    Programmer's Guide to MS Windows 95, MS Press, p. 113)
//
TDataObject 
TShellItem::GetDataObject(HWND windowOwner)
{
  EnforceValidity();
  IDataObject* iface;

// Expanded by Val Ovechkin 12:29 PM 6/3/98
  void *p = &iface;
  HRESULT hr = ParentFolder->GetUIObjectOf(windowOwner, 1, Pidl, IID_IDataObject
,
                                           0, REINTERPRET_CAST(void**, p));

//  HRESULT hr = ParentFolder->GetUIObjectOf(windowOwner, 1, Pidl, IID_IDataObject,
//                                           0, REINTERPRET_CAST(void**, &iface));
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::GetUIObjectOf(IDataObject) ")
        << _T("failed.  TShellItem* = ")
        << hex << (LPVOID)this << _T("  Parent IShellFolder* = ")
        << STATIC_CAST(IShellFolder*, ParentFolder));
  TXShell::Check(hr, IDS_GETUIOBJECTOFFAIL);
  return TDataObject(iface);
}

//
///  GetDropTarget returns an interface pointer to this TShellItem's IDropTarget
///    OLE interface
//
TDropTarget 
TShellItem::GetDropTarget(HWND windowOwner)
{
  EnforceValidity();
  IDropTarget* iface;

  void *p = &iface;
  HRESULT hr = ParentFolder->GetUIObjectOf(windowOwner, 1, Pidl, IID_IDropTarget
,
                                           0, REINTERPRET_CAST(void**, p));

//  HRESULT hr = ParentFolder->GetUIObjectOf(windowOwner, 1, Pidl, IID_IDropTarget,
//                                           0, REINTERPRET_CAST(void**, &iface));
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::GetUIObjectOf(IDropTarget) ")
        << _T("failed.  TShellItem* = ")
        << hex << (LPVOID)this << _T("  Parent IShellFolder* = ")
        << STATIC_CAST(IShellFolder*, ParentFolder));
  TXShell::Check(hr, IDS_GETUIOBJECTOFFAIL);
  return TDropTarget(iface);
}

//
///  GetIcon returns a handle to an icon representing this TShellItem
///
///  size can be Large, Small, or Shell
///  kind can be Link, Open, or Selected
///    (see MS doc on SHGetFileInfo for meanings,
///    Programmer's Guide to MS Windows 95, MS Press, pp. 206-207)
//
HICON 
TShellItem::GetIcon(TIconSize size, uint kind)
{
  EnforceValidity();

  if (!IsPartOfFileSystem()) {
    uint inFlags = (size & Shell)? GIL_FORSHELL: 0 | (kind & Open)? GIL_OPENICON: 0;
    tchar fileName[_MAX_PATH];
    int index;
    uint outFlags;
    HICON largeIcon;
    HICON smallIcon;

    TExtractIcon extractIcon = GetExtractIcon();
    HRESULT hr = extractIcon->GetIconLocation(inFlags, fileName, COUNTOF(fileName),
                                              &index, &outFlags);
    TXShell::Check(hr, IDS_GETICONLOCFAIL);
    if (hr != S_FALSE)
      if (outFlags & GIL_NOTFILENAME || outFlags & GIL_DONTCACHE) {
        hr = extractIcon->Extract(fileName, index, &largeIcon, &smallIcon, GetSystemMetrics(SM_CXICON));
        TXShell::Check(hr, IDS_EXTRICONEXTRACTFAIL);
        if (hr != S_FALSE)
          return (size == Small)? smallIcon: largeIcon;
      }
  }

  // TShellItem is part of file system or couldn't get the icon with
  // IExtractIcon::Extract
  //
  SHFILEINFO fi;
  uint flags = SHGFI_ICON | kind | size;

  TShell::SHGetFileInfo(reinterpret_cast<const tchar*>(static_cast<LPCITEMIDLIST>(*this)),
                  0, &fi, sizeof (fi), flags | SHGFI_PIDL);
  WARNX(OwlShell, !fi.hIcon, 0, _T("::SHGetFileInfo failed.  TShellItem* = ")
        << hex << (LPVOID)this);
  if (!fi.hIcon)
    TXShell::Raise(IDS_GETFILEINFOFAIL);
  return fi.hIcon;
}

//
/// ResolveShortcut resolves this TShellItem (if it's a shortcut) and returns the
/// underlying TShellItem.  If this TShellItem is not a shortcut, it just returns itself.
///
/// \note IMPORTANT:  This function calls CoCreateInstance.  Therefore an application that
///             uses this function MUST have previously called CoInitialize (or OLEInitialize).
///
/// \note IMPORTANT:  If the shortcut cannot be resolved, this function will return an
///             empty (invalid) item.  One MUST check for this by calling IsValid()
///             upon return from this function.
//
TShellItem 
TShellItem::ResolveShortcut(HWND windowOwner)
{
  EnforceValidity();

  if (!IsShortcut())
    return *this;

  HRESULT hr;
  TComRef<IShellLink> shellLink;
  hr = TOle::CoCreateInstance(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER, IID_IShellLink,
                        REINTERPRET_CAST(void **, STATIC_CAST(IShellLink**, shellLink)));
  TXShell::Check(hr, IDS_CREATESHELLLINKFAIL);

  TComRef<IPersistFile> persistFile;
  hr = shellLink->QueryInterface(IID_IPersistFile,
                                 REINTERPRET_CAST(void **, STATIC_CAST(IPersistFile**, persistFile)));
  TXShell::Check(hr, IDS_LINKQUERYINTRFCFAIL);
  TString shortcutFile = GetPath();
  hr = persistFile->Load(shortcutFile, STGM_READ);
  TXShell::Check(hr, IDS_PERSISTFILELOADFAIL);

  hr = shellLink->Resolve(windowOwner, SLR_ANY_MATCH | SLR_UPDATE);
  if (FAILED(hr)) {
    // Failure to Resolve: return empty/invalid shell item object
    //
    return TShellItem();        
  }

  TPidl resolvedPidl;
  hr = shellLink->GetIDList(resolvedPidl);
  TXShell::Check(hr, IDS_LINKGETIDLISTFAIL);

  // Construct the resolved TShellItem
  //
  TCreateStruct cs;
  cs.Pidl = resolvedPidl.GetLastItem();
  if (resolvedPidl.GetItemCount() == 1)
    cs.ParentFolder = TShellItem(Desktop);
  else {

// Expanded by Val Ovechkin 12:29 PM 6/3/98
    void *p = STATIC_CAST(IShellFolder**, cs.ParentFolder);
    hr = TShellItem(Desktop)->BindToObject(resolvedPidl.StripLastItem(), 0, IID_IShellFolder,
                    REINTERPRET_CAST(void **, p));

//    hr = TShellItem(Desktop)->BindToObject(resolvedPidl.StripLastItem(), 0, IID_IShellFolder,
//                    REINTERPRET_CAST(void **, STATIC_CAST(IShellFolder**, cs.ParentFolder)));

    TXShell::Check(hr, IDS_BINDTOOBJECTFAIL);
  }
  return TShellItem(cs);
}

//
/// Create a shortcut to a file object
///
/// \note IMPORTANT:  This function calls CoCreateInstance.  Therefore an 
///             application that uses this function MUST have previously 
///             called CoInitialize (or OLEInitialize).
//
HRESULT 
TShellItem::CreateShortCut(LPCTSTR objPath, LPTSTR pathLink, LPTSTR description)
{
  HRESULT hr;
  TComRef<IShellLink> shellLink;
  hr = TOle::CoCreateInstance(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER, IID_IShellLink,
                        REINTERPRET_CAST(void **, STATIC_CAST(IShellLink**, shellLink)));
  TXShell::Check(hr, IDS_CREATESHELLLINKFAIL);

  // Set path to shortcut target and add description
  //
  hr = shellLink->SetPath(TString(objPath));
  TXShell::Check(hr, IDS_SHLLINKSETPATHFAIL);
  hr = shellLink->SetDescription(TString(description));
  TXShell::Check(hr, IDS_SHLLINKSETDESCFAIL);

  // Query IShellLink for the IPersistFile Interface for saving the 
  // shortcut in persistent storage
  //
  TComRef<IPersistFile> persistFile;
  hr = shellLink->QueryInterface(IID_IPersistFile,
                                 REINTERPRET_CAST(void **, STATIC_CAST(IPersistFile**, persistFile)));
  TXShell::Check(hr, IDS_LINKQUERYINTRFCFAIL);

  // Save link by calling IPersistFile::Save
  //
  hr = persistFile->Save(TString(pathLink), TRUE);
  return hr;
}

//
/// Create a shortcut to a non-file object
///
/// \note IMPORTANT:  This function calls CoCreateInstance.  Therefore an 
///             application that uses this function MUST have previously 
///             called CoInitialize (or OLEInitialize).
//
HRESULT 
TShellItem::CreateShortCut(LPCITEMIDLIST pidl, LPTSTR pathLink, LPTSTR description)
{
  HRESULT hr;
  TComRef<IShellLink> shellLink;
  hr = TOle::CoCreateInstance(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER, IID_IShellLink,
                        REINTERPRET_CAST(void **, STATIC_CAST(IShellLink**, shellLink)));
  TXShell::Check(hr, IDS_CREATESHELLLINKFAIL);

  // Set path to shortcut target and add description
  //
  shellLink->SetIDList(pidl);
  shellLink->SetDescription(description);

  // Query IShellLink for the IPersistFile Interface for saving the 
  // shortcut in persistent storage
  //
  TComRef<IPersistFile> persistFile;
  hr = shellLink->QueryInterface(IID_IPersistFile,
                                 REINTERPRET_CAST(void **, STATIC_CAST(IPersistFile**, persistFile)));
  TXShell::Check(hr, IDS_LINKQUERYINTRFCFAIL);

  // Save link by calling IPersistFile::Save
  //
  hr = persistFile->Save(TString(pathLink), TRUE);
  return hr;
}

//
/// Creates a shortcut to this object
///
/// \note IMPORTANT:  This function calls CoCreateInstance.  Therefore an 
///             application that uses this function MUST have previously 
///             called CoInitialize (or OLEInitialize).
//
TShellItem     
TShellItem::CreateShortCut(LPTSTR linkPath, LPTSTR description)
{
  EnforceValidity();

  if (IsPartOfFileSystem()) {
    if (SUCCEEDED(CreateShortCut(GetPath(), linkPath, description)))
      return TShellItem(linkPath);
  } else {
    if (SUCCEEDED(CreateShortCut(GetFullyQualifiedPidl(), linkPath,
                                 description)))
      return TShellItem(linkPath);
  }

  // Failure: return empty/invalid Shell Item object
  //
  return TShellItem();
}

//
///  Adds the TShellItem to the taskbar's recent document list
///    (see MS doc for more info on SHAddToRecentDocs,
///    Programmer's Guide to MS Windows 95, MS Press, p. 202)
//
void 
TShellItem::AddToRecentDocs() const
{
  EnforceValidity();
  TShell::SHAddToRecentDocs(SHARD_PIDL, STATIC_CAST(const ITEMIDLIST *, GetFullyQualifiedPidl()));
}

//
///  CompareIDs is a protected function that compares a TShellItem's pidl with
///    another TShellItem's pidl
///
///    (see MS doc for more info CompareIDs,
///    Programmer's Guide to MS Windows 95, MS Press, p. 194)
//
short 
TShellItem::CompareIDs(const TShellItem& rhs) const
{
  EnforceValidity();
  rhs.EnforceValidity();
  HRESULT hr;

  if (!HaveSameParent(rhs)) {                                // use the desktop
    TShellItem desktop(Desktop);
    hr = desktop->CompareIDs(0, GetFullyQualifiedPidl(), rhs.GetFullyQualifiedPidl());
    WARNX(OwlShell, FAILED(hr), 0, _T("Desktop->CompareIDs failed.")
          << _T("  this TShellItem* = ") << hex << (LPVOID)this
          << _T("  other TShellItem* = ") << (LPVOID)&rhs);
    TXShell::Check(hr, IDS_COMPAREIDSFAIL);
  }
  else {
    hr = ParentFolder->CompareIDs(0, Pidl, rhs.Pidl);
    WARNX(OwlShell, FAILED(hr), 0, _T("SameParent->CompareIDs failed.")
          << _T("  this TShellItem* = ") << hex << (LPVOID)this
          << _T("  other TShellItem* = ") << (LPVOID)&rhs);
    TXShell::Check(hr, IDS_COMPAREIDSFAIL);
  }

  return STATIC_CAST(short, HRESULT_CODE(hr));
}

//
///  EnforceValidity is a protected function that checks that a TShellItem is valid
///    before proceeding with an operation that requires the TShellItem to be valid
//
void 
TShellItem::EnforceValidity() const
{
  WARNX(OwlShell, !Valid(), 0, _T("TShellItem is not valid.")
        << _T("  TShellItem* = ") << hex << (LPVOID)this);
  if (!Valid())
    TXShell::Raise(IDS_TSHELLITEMINVALID);
}

// Folder Functions

//
///  RetrieveIShellFolder is a protected function that checks to see if a TShellItem's
///    IShellFolder interface is already assigned a value.  If not, it will BindToObject
///    in order to assign the interface a value.  Note:  Only TShellItem's that are
///    folders have an IShellFolder interface;  consequently, if this function is
///    called on a TShellItem that is not a folder, an exception will be thrown.
//
void 
TShellItem::RetrieveIShellFolder() const
{
  EnforceValidity();
  if (!I) { // Need to call BindToObject
    IShellFolder* iface;
    HRESULT hr = ParentFolder->BindToObject(Pidl, 0, IID_IShellFolder,
                                            REINTERPRET_CAST(void **, &iface));
    WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::BindToObject failed.")
          << _T("  TShellItem* = ") << hex << (LPVOID)this << _T("  Parent IShellFolder* = ")
          << STATIC_CAST(IShellFolder*, *CONST_CAST(TComRef<IShellFolder>*, &ParentFolder)));
    TXShell::Check(hr, IDS_BINDTOOBJECTFAIL);
    // CONST_CAST(IShellFolder*, I) = iface;
    CONST_CAST(TShellItem*,this)->I = iface;
  }
}

//
///  EnumObjects is a protected function that returns an IEnumIDList enumerator on
///    a folder.  Note:  This function is only meaningful for folders;  consequently,
///    if this function is called on a TShellItem that is not a folder, an
///    exception will be thrown.
//
void 
TShellItem::EnumObjects(IEnumIDList** iface, HWND windowOwner,
                        const int kind) const
{
  EnforceValidity();
  RetrieveIShellFolder();
  HRESULT hr = I->EnumObjects(windowOwner, (kind == -1)? TShellItemIterator::Folders |
                              TShellItemIterator::NonFolders: kind, iface);
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::EnumObjects failed.")
        << _T("  TShellItem* = ") << hex << (LPVOID)this);
  TXShell::Check(hr, IDS_ENUMOBJECTSFAIL);
}

//
///  BrowseForFolder presents a dialog box to the user in which he can select a
///    subfolder.  The root of the dialog box is this TShellItem folder.
///
/// \note  It is only meaningful to call this function if the TShellItem is a
///    folder;  consequently, if this function is called on a TShellItem that is
///    not a folder, an exception will be thrown.
///
///  Additional Documentation:  See MS doc on SHBrowseForFolder for more info,
///    more info, Programmer's Guide to MS Windows 95, MS Press, pp. 202-203.
//
bool 
TShellItem::BrowseForFolder(TCreateStruct& cs, HWND windowOwner,
                            const tchar* title, const UINT flags, int* image,
                            const bool includeStatus, BFFCALLBACK func,
                            const LPARAM param) const
{
  EnforceValidity();
  RetrieveIShellFolder();
  tchar displayName[_MAX_PATH];

  BROWSEINFO bi;
  bi.hwndOwner = windowOwner;
  TPidl fullPidl = GetFullyQualifiedPidl();
  bi.pidlRoot = fullPidl;
  bi.pszDisplayName = displayName;
  bi.lpszTitle = title;
  bi.ulFlags = flags;
  if (includeStatus)
    bi.ulFlags |= BIF_STATUSTEXT;
  bi.lpfn = func;
  bi.lParam = param;

  TPidl pidl = TShell::SHBrowseForFolder(&bi);
  if (!pidl)
    return false;
  else {
    if (image)
      *image = bi.iImage;
    TShellItem desktop(Desktop);
    cs.Pidl = pidl.GetLastItem();

// Expanded by Val Ovechkin 12:29 PM 6/3/98
    void *p = STATIC_CAST(IShellFolder**, cs.ParentFolder);
    HRESULT hr = desktop->BindToObject(pidl.StripLastItem(), 0, IID_IShellFolder
,
         REINTERPRET_CAST(void **, p));

//    HRESULT hr = desktop->BindToObject(pidl.StripLastItem(), 0, IID_IShellFolder,
//         REINTERPRET_CAST(void **, STATIC_CAST(IShellFolder**, cs.ParentFolder)));
    WARNX(OwlShell, FAILED(hr), 0, _T("IShellFolder::BindToObject failed.")
          << _T("  TShellItem* = ") << hex << (LPVOID)this
          << _T("  Desktop IShellFolder* = ") << STATIC_CAST(IShellFolder*, desktop));
    TXShell::Check(hr, IDS_BINDTOOBJECTFAIL);

    return true;
  }
}

//
///  ParseDisplayName parses a "for parsing" display name into a TCreateStruct
///    (which can be used to create a TShellItem).  In general, it is not necessary
///    to call this function when using OWL's shell classes.
///
/// \note It is only meaningful to call this function if the TShellItem is a
///    folder;  consequently, if this function is called on a TShellItem that is
///    not a folder, an exception will be thrown.
///
///  Additional Documentation:  See MS doc on IShellFolder::ParseDisplayName for
///    more info, Programmer's Guide to MS Windows 95, MS Press, pp. 191-192.
//
HRESULT 
TShellItem::ParseDisplayName(TCreateStruct& cs, const tchar* displayName,
                             ulong* eaten, HWND windowOwner,
                             ulong* attrib) const
{
  EnforceValidity();
  TPidl pidl;
  ulong charsEaten;
  ulong attributes;
  RetrieveIShellFolder();

  HRESULT hr = I->ParseDisplayName(windowOwner, 0, TString(displayName),
                                   &charsEaten, pidl, &attributes);

  if (hr == NOERROR) {
    if (eaten)
      *eaten = charsEaten;
    if (attrib)
     *attrib = attributes;
    cs.Pidl = pidl;
    cs.ParentFolder = *this;
  }

  return hr;
}

//
///  TShellItemIterator constructor.
///
/// \note It is only meaningful to construct iterators on folders;  therefore,
///    if this constructor is passed a TShellItem that is not a folder, an exception
///    will be thrown.
///
///  Additional Documentation:  See MS doc on IEnumIDList for more info,
///    Programmer's Guide to MS Windows 95, MS Press, pp. 199-201.
//
TShellItemIterator::TShellItemIterator(const TShellItem& folder, HWND windowOwner,
                                       const int kind)
:
  Index(0),
  Pidl(0),
  Folder(folder)
{
  folder.EnumObjects(*this, windowOwner, kind);
  Folder = folder; // even though filled in above, the IShellFolder* may not have
                   // yet have been filled in.  (See RetrieveIShellFolder)
  Next();
}

//
///  TShellItemIterator copy constructor.
///
/// \note This function does not work with the intial release of Win95 because
///    Win95's IEnumIDList::Clone is broken (it returns E_FAIL (unspecified error))
//
TShellItemIterator::TShellItemIterator(const TShellItemIterator& source)
:
  Index(source.Index),
  Pidl(source.Pidl),
  Folder(source.Folder)
{
  if (!source.I)
    I = 0;
  else {
    HRESULT hr = source.I->Clone(*this);
    WARNX(OwlShell, FAILED(hr), 0, _T("IEnumIDList::Clone failed.")
          << _T("  TShellItemIterator* = ") << hex << (LPVOID)this
          << _T("  source IEnumIDList* = ")
          << STATIC_CAST(IEnumIDList*, CONST_CAST(TShellItemIterator&, source)));
    TXShell::Check(hr, IDS_CLONEFAIL);
  }
}

//
///  TShellItemIterator assignment operator
///
/// \note  This function does not work with the intial release of Win95 because
///    Win95's IEnumIDList::Clone is broken (it returns E_FAIL (unspecified error))
//
TShellItemIterator& TShellItemIterator::operator = (const TShellItemIterator& source)
{
  if (&source != this) {
    if (!source.I)
      I = 0;
    else {
      HRESULT hr = source.I->Clone(*this);
      WARNX(OwlShell, FAILED(hr), 0, _T("IEnumIDList::Clone failed.")
          << _T("  TShellItemIterator* = ") << hex << (LPVOID)this << _T("  source IEnumIDList* = ")
          << STATIC_CAST(IEnumIDList*, CONST_CAST(TShellItemIterator&, source)));
      TXShell::Check(hr, IDS_CLONEFAIL);
      Index = source.Index;
      Pidl = source.Pidl;
      Folder = source.Folder;
    }
  }
  return *this;
}

//
///  operator IEnumIDList** resets the interface pointer and returns a pointer
///    to this interface pointer.
//
TShellItemIterator::operator IEnumIDList**()
{
  return TComRef<IEnumIDList>::operator IEnumIDList**();
}

//
///  operator ++ increments the "cursor" in the iterator, then returns
///    the item pointed to by the cursor
///
///  The item is returned as a TShellItem::TCreateStruct structure.  For more
///    information about TCreateStruct, see the comments in the
///    TShellItem::TShellItem(const TCreateStruct& cs) constructor.
//
TShellItem::TCreateStruct TShellItemIterator::operator ++()
{
  EnforceValidInterface();
  return operator[] (Index + 1);
}

//
///  operator ++(int) returns the item pointed to by the "cursor," then increments
///    the cursor.
///
///  The item is returned as a TShellItem::TCreateStruct structure.  For more
///    information about TCreateStruct, see the comments in the
///    TShellItem::TShellItem(const TCreateStruct& cs) constructor.
//
TShellItem::TCreateStruct  
TShellItemIterator::operator ++(int)
{
  EnforceValidInterface();
  WARNX(OwlShell, Index == -1, 0, _T("Attempt to read past end of TShellItemIterator")
        << _T("  TShellItemIterator* = ") << hex << (LPVOID)this << _T("  IEnumIDList* = ")
        << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this)));
  if (Index == -1)
      TXShell::Raise(IDS_SHELLITERATORATEND);
  TShellItem::TCreateStruct cs(Pidl, Folder);
  Next();
  if (Index != -1)
    Index++;
  return cs;
}

//
///  operator -- decrements the "cursor" in the iterator, then returns
///    the item pointed to by the cursor
///
///  The item is returned as a TShellItem::TCreateStruct structure.  For more
///    information about TCreateStruct, see the comments in the
///    TShellItem::TShellItem(const TCreateStruct& cs) constructor.
///
/// \note  This function does not work with the intial release of Win95 because
///    it requires IEnumIDList::Reset to be called and IEnumIDList::Reset returns
///    E_NOTIMPL (not implemented) in the intitial Win95 release.
//
TShellItem::TCreateStruct  
TShellItemIterator::operator --()
{
  EnforceValidInterface();
  return operator[] (Index - 1);
}

//
///  operator --(int) returns the item pointed to by the "cursor," then decrements
///    the cursor.
///
///  The item is returned as a TShellItem::TCreateStruct structure.  For more
///    information about TCreateStruct, see the comments in the
///    TShellItem::TShellItem(const TCreateStruct& cs) constructor.
///
/// \note This function does not work with the intial release of Win95 because
///    it requires IEnumIDList::Reset to be called and IEnumIDList::Reset returns
///    E_NOTIMPL (not implemented) in the intitial Win95 release.
//
TShellItem::TCreateStruct  
TShellItemIterator::operator --(int)
{
  EnforceValidInterface();
  WARNX(OwlShell, Index == -1, 0, _T("Attempt to read past end of TShellItemIterator")
        << _T("  TShellItemIterator* = ") << hex << (LPVOID)this << _T("  IEnumIDList* = ")
        << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this)));
  if (Index == -1)
      TXShell::Raise(IDS_SHELLITERATORATEND);
  TShellItem::TCreateStruct cs(Pidl, Folder);
  Reset();
  Skip(--Index);
  Next();
  return cs;
}

//
///  operator Current returns the item pointed to by the "cursor"
///
///  The item is returned as a TShellItem::TCreateStruct structure.  For more
///    information about TCreateStruct, see the comments in the
///    TShellItem::TShellItem(const TCreateStruct& cs) constructor.
//
TShellItem::TCreateStruct 
TShellItemIterator::Current()
{
  EnforceValidInterface();
  WARNX(OwlShell, Index == -1, 0, _T("Attempt to read past end of TShellItemIterator")
        << _T("  TShellItemIterator* = ") << hex << (LPVOID)this << _T("  IEnumIDList* = ")
        << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this)));
  if (Index == -1)
      TXShell::Raise(IDS_SHELLITERATORATEND);
  return TShellItem::TCreateStruct(Pidl, Folder);
}

//
///  Next is a protected function that increments the cursor.  Equivalent to Skip(1).
//
void TShellItemIterator::Next()
{
  EnforceValidInterface();

  WARNX(OwlShell, Index == -1, 0, _T("Attempt to read past end of TShellItemIterator")
        << _T("  TShellItemIterator* = ") << hex << (LPVOID)this << _T("  IEnumIDList* = ")
        << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this)));
  if (Index == -1)
    TXShell::Raise(IDS_SHELLITERATORATEND);

  HRESULT hr = I->Next(1, Pidl, 0);
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellItemIterator::Next failed.")
        << _T("  TShellItemIterator* = ") << hex << (LPVOID)this << _T("  IEnumIDList* = ")
        << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this)));
  TXShell::Check(hr, IDS_IDLISTNEXTFAIL);
  if (hr == S_FALSE)
    Index = -1;
}

//
/// Skip advances the cursor \<count\> times.  Equivalent to calling Next \<count\>
///    times.
///
/// \note  This function does not work with the intial release of Win95 because
///    IEnumIDList::Skip returns E_NOTIMPL (not implemented) in the intitial Win95
///    release.
//
void 
TShellItemIterator::Skip(const ulong count)
{
  EnforceValidInterface();
  if (count) {
    HRESULT hr = I->Skip(count);
    WARNX(OwlShell, FAILED(hr), 0, _T("IShellItemIterator::Skip failed.")
          << _T("  TShellItemIterator* = ") << hex << (LPVOID)this << _T("  IEnumIDList* = ")
          << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this))
          << _T("  count = ") << count);
    TXShell::Check(hr, IDS_IDLISTSKIPFAIL);
    if (hr == S_FALSE)
      Index = -1;
    else
      Index += count;
  }
}

//
///  Reset resets the cursor to the beginning.
///
/// \note  This function does not work with the intial release of Win95 because
///    IEnumIDList::Reset returns E_NOTIMPL (not implemented) in the intitial Win95
///    release.
//
void 
TShellItemIterator::Reset()
{
  EnforceValidInterface();
  HRESULT hr = I->Reset();
  WARNX(OwlShell, FAILED(hr), 0, _T("IShellItemIterator::Reset failed.")
        << _T("  TShellItemIterator* = ") << hex << this << _T("  IEnumIDList* = ")
        << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this)));
  TXShell::Check(hr, IDS_IDLISTRESETFAIL);
  Index = 0;
}

//
///  operator [] returns the item at the \<index\> location.  index is zero based.
///
///  The item is returned as a TShellItem::TCreateStruct structure.  For more
///    information about TCreateStruct, see the comments in the
///    TShellItem::TShellItem(const TCreateStruct& cs) constructor.
///
/// \note  operator[] doesn't work with the intial release of Win95 because it
///    calls Skip and Reset wich are broken in the intial Win95 release.  Both of
///    these functions return E_NOTIMPL (not implemented).  The only way operator[]
///    will work is to call it sequentially, beginning at index 0 (i.e., to use it
///    like operator ++().
//
TShellItem::TCreateStruct 
TShellItemIterator::operator [](const long index)
{
  EnforceValidInterface();
  if (Index == index)
    return TShellItem::TCreateStruct(Pidl, Folder);
  if (Index < index)
    Skip(index - Index - 1);
  else if (Index > index) {
    Reset();
    Skip(index);
  }
  Next();
  WARNX(OwlShell, Index == -1, 0, _T("Attempt to read past end of TShellItemIterator")
        << _T("  TShellItemIterator* = ") << hex << this << _T("  IEnumIDList* = ")
        << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this)));
  if (Index == -1) {
    TXShell::Raise(IDS_SHELLITERATORATEND);
    return TShellItem::TCreateStruct();                   // To satisfy compiler
  }
  else
    return TShellItem::TCreateStruct(Pidl, Folder);
}

//
///  GetCount returns the number of items in the iterator's list
///
///  \note  This function does not work with the intial release of Win95 because
///    Clone and Reset are borken in the intitial Win95 release.
//
long 
TShellItemIterator::GetCount() const
{
  EnforceValidInterface();
  TShellItemIterator list(*this);  // clone so we don't affect position
  list.Reset();
  if (list.Index == -1)
    return 0;
  long cnt = 1;
  while (list.Skip(1), Index != -1)
    cnt++;
  return cnt;
}

//
///  EnforceValidInterface is a protected function that checks to see that this
///    TShellItemIterator contains a valid IEnumIDList interface pointer.  If it
///    doesn't, it throws an exception.
//
void 
TShellItemIterator::EnforceValidInterface() const
{
  WARNX(OwlShell, !I, 0, _T("TShellItemIterator is not valid (IEnumIDList* == 0).")
        << _T("  TShellItemIterator* = ") << hex
        << STATIC_CAST(IEnumIDList*, *CONST_CAST(TShellItemIterator*, this)));
  if (!I) {
    TXShell::Raise(IDS_IDLISTZEROPOINTER);
  }
}

//
/// Constructs a TXShell object with a default IDS_SHELLFAILURE message.
//
TXShell::TXShell(uint resId, HANDLE handle)
:
  TXOwl(MakeMessage(resId, uint(handle)), resId)
{
}

TXShell* TXShell::Clone() const
{
  return new TXShell(*this);
}


//
/// Throws the exception object.
//
void TXShell::Throw()
{
  throw *this;
}

//
/// Constructs a TXShell exception from scratch, and throws it.
//
void TXShell::Raise(uint resId, HANDLE handle)
{
  TXShell(resId, handle).Throw();
}

//
/// Checks an HResult and throws a TXShell if not SUCCEEDED(hr).
//
void TXShell::Check(HRESULT hr, uint resID, HANDLE handle)
{
  if FAILED(hr)
    Raise(resID, handle);
}

} // OWL namespace

/* ========================================================================== */

