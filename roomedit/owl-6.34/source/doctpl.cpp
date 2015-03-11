//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TDocTemplate
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/doctpl.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;

//
// Construct a Doc/View template from the specified parameters.
//
/// Uses the information in the registration table (regList) to construct a
/// TDocTemplate with the specified file description, file filter pattern, search
/// path for viewing the directory, default file extension, and flags representing
/// the view and creation options from the registration list.  Then, adds this
/// template to the document manager's template list. If the document manager is not
/// yet constructed, adds the template to a static list, which the document manager
/// will later add to its template list.
/// The argument, module, specifies the TModule of the caller. phead specifies the
/// template head for the caller's module. See the Registration macros entry in this
/// manual for information about the registration macros that generate a TRegList,
/// which contains the attributes used to create a TDocTemplate object.
//
TDocTemplate::TDocTemplate(TRegList& regList, TModule*& module,
                           TDocTemplate*& rptpl)
:
  TRegLink(regList, (TRegLink*&)rptpl),
  Directory(0),
  ModulePtr(&module)
{
  RefCnt = module ? 1 : 0x8001;  // static if constructed before Module
  Flags = _ttol((*RegList)["docflags"]);
}

//
/// Sets the document template constants, which indicate how the document is created
/// and opened.
//
void TDocTemplate::SetFlag(long flag)
{
  Flags = GetFlags() | flag;
}

//
/// Clears a document template constant.
//
void TDocTemplate::ClearFlag(long flag)
{
  Flags = GetFlags() & ~flag;
}

//
/// Gets the directory path to use when searching for matching files. This will get
/// updated if a file is selected and the dtUpdateDir flag is set.
//
LPCTSTR TDocTemplate::GetDirectory() const
{
  if (Directory)
    return Directory;
  return (*RegList)["directory"];
}

//
/// Sets the directory path to use when searching for matching files. This will get
/// updated if a file is selected and the dtUpdateDir flag is set.
//
void TDocTemplate::SetDirectory(LPCTSTR txt)
{
  delete[] Directory;
  Directory = 0;
  if (txt)
    Directory = strnewdup(txt);
}

//
/// Sets the directory path to use when searching for matching files. This will get
/// updated if a file is selected and the dtUpdateDir flag is set.
//
void TDocTemplate::SetDirectory(LPCTSTR txt, int len)
{
  delete[] Directory;
  Directory = 0;
  if (txt && len > 0) {
    Directory = strnewdup(txt, len);
    Directory[len] = 0;
  }
}

//
// Called only when RefCnt goes to 0
//
/// Destroys a TDocTemplate object and frees the data members (FileFilter,
/// Description, Directory, and DefaultExt). The Destructor is called only when no
/// views or documents are associated with the template. Instead of calling this
/// Destructor directly, use the Delete member function.
//
TDocTemplate::~TDocTemplate()
{
  if (GetFlags() & dtDynRegInfo) {
    delete RegList;
    RegList = 0;
  }
  delete[] Directory;
}

//
/// Gets any valid document matching pattern to use when searching for files.
//
LPCTSTR TDocTemplate::GetFileFilter() const
{
  return (*RegList)["docfilter"];
}

//
/// Gets the template description to put in the file-selection list box or the
/// File|New menu-selection list box.
//
LPCTSTR TDocTemplate::GetDescription() const
{
  return (*RegList)["description"];
}

//
/// Gets the default extension to use if the user has entered the name of a file
/// without any extension. If there is no default extension, GetDefaultExt contains
/// 0.
//
LPCTSTR TDocTemplate::GetDefaultExt() const
{
  return (*RegList)["extension"];
}

//
//
//
class TRegListOldDocTemplate : public TRegList {
  public:
    TRegListOldDocTemplate(LPCTSTR desc, LPCTSTR filt,
                           LPCTSTR dir,  LPCTSTR ext, long flags);
    TRegItem List[6];  // 4 strings, flags, terminator
    tchar   FlagBuf[12];  // for string representation of doc template flags
};

//
//
//
TRegListOldDocTemplate::TRegListOldDocTemplate(LPCTSTR desc,
                                               LPCTSTR filt,
                                               LPCTSTR dir,
                                               LPCTSTR ext,
                                               long    flags)
:
  TRegList(List)
{
  _stprintf(FlagBuf,_T("0x%lX"),flags);
  List[0].Key = "description";
  List[0].Value = desc;
  List[1].Key = "docfilter";
  List[1].Value = filt;
  List[2].Key = "directory";
  List[2].Value = dir;
  List[3].Key = "extension";
  List[3].Value = ext;
  List[4].Key = "docflags";
  List[4].Value = FlagBuf;
  List[5].Key = 0;
}

//
/// Constructs a Doc/View template from the description, filter, directory, file
/// extension, 'dt' flags, module and template head parameters. This constructor is
/// primarily for backward compatibility with earlier implementation of
/// ObjectWindows' Doc/View subsystem.
//
TDocTemplate::TDocTemplate(LPCTSTR desc, LPCTSTR filt,
                           LPCTSTR dir, LPCTSTR ext,
                           long flags, TModule*& module,
                           TDocTemplate*& rphead)
:
  TRegLink(),
  Directory(0),
  Flags(flags | dtDynRegInfo),
  ModulePtr(&module)
{
//JJH this is nasty...but works.
#if (__GNUC__ >= 3)
  AddLink((TRegLink**)&rphead, reinterpret_cast<TRegLink*>(this));
#else
  AddLink((TRegLink**)&rphead, (TRegLink*)this);
#endif
  RefCnt = module ? 1 : 0x8001;  // static if contructed before Module
//  NextTemplate = 0;
  RegList = new TRegListOldDocTemplate(desc, filt, dir, ext, flags);
}


IMPLEMENT_ABSTRACT_STREAMABLE(TDocTemplate);

#if !defined(BI_NO_OBJ_STREAMING)

//
//
//
void*
TDocTemplate::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TDocTemplate* o = GetObject();
  bool wasStatic = o->IsStatic();  // test in case dummy template passed
  is >> o->RefCnt;  // need to set back to 1 if doc attach increments!!?
  is >> o->Flags;
  if (o->IsStatic()) {
    delete[] o->Directory;
  }
#if defined(UNICODE)
  _USES_CONVERSION;
  char * dir = is.freadString();
  o->Directory  = strnewdup(_A2W(dir));
  delete[] dir;
#else
  o->Directory   = is.freadString();
#endif
  if (o->IsStatic() && !wasStatic) {  // dummy template passed as sink
    delete[] o->Directory;
  }
  return o;
  ///JD need to link up reg info table!!
}

//
//
//
void
TDocTemplate::Streamer::Write(opstream& os) const
{
  TDocTemplate* o = GetObject();
  os << o->RefCnt;
  os << o->GetFlags();
  _USES_CONVERSION;
  os.fwriteString(_W2A(o->Directory));
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace

