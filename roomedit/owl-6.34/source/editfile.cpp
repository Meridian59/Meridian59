//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TEditFile, a text edit which can find/replace and
/// read/write from/to a file.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/window.h>
#include <owl/control.h>
#include <owl/edit.h>
#include <owl/editsear.h>
#include <owl/editfile.h>
#include <owl/file.h>
#include <owl/applicat.h>
#include <owl/system.h>
#include <string.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;


DEFINE_RESPONSE_TABLE1(TEditFile, TEditSearch)
// !!! BUG  EV_COMMAND(CM_FILENEW,  CmFileNew),  // BUG !!!!!!!
// !!! BUG  EV_COMMAND(CM_FILEOPEN, CmFileOpen), // BUG !!!!!!!  
  EV_COMMAND(CM_FILESAVE, CmFileSave),
  EV_COMMAND(CM_FILESAVEAS, CmFileSaveAs),
  EV_COMMAND_ENABLE(CM_FILESAVE, CmSaveEnable),
END_RESPONSE_TABLE;

//
/// Constructs a TEditFile window given the parent window, resource ID (Id),
/// text, file name, and module ID. Sets Filename to fileName.
//
TEditFile::TEditFile(TWindow*        parent,
                     int             id,
                     LPCTSTR         text,
                     int x, int y, int w, int h,
                     LPCTSTR         fileName,
                     TModule*        module)
:
  TEditSearch(parent, id, text, x, y, w, h, module)
{
  FileName = fileName ? strnewdup(fileName) : 0;
}

//
/// String-aware overload
//
TEditFile::TEditFile(
  TWindow* parent, 
  int id,
  const tstring& text,
  int x, int y, int w, int h,
  const tstring& fileName,
  TModule* module
  )
  : TEditSearch(parent, id, text, x, y, w, h, module),
  FileName (fileName.empty() ? 0 : strnewdup(fileName.c_str()))
{}

//
/// Frees memory allocated to hold the name of the TEditFile.
//
TEditFile::~TEditFile()
{
  delete[] FileName;
}

//
/// Creates the edit window's Editor edit control by calling TEditFile::SetupWindow().
/// Sets the window's caption to FileName, if available; otherwise sets the name to
/// "Untitled."
//
void
TEditFile::SetupWindow()
{
  TEditSearch::SetupWindow();
  FileData.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;

  FileData.Flags |= OFN_LONGNAMES;

  FileData.Flags |= OFN_EXPLORER;

  FileData.SetFilter(LoadString(IDS_FILEFILTER).c_str());

  SetFileName(FileName);
  if (FileName && !Read()) 
  {
    FormatMessageBox(IDS_UNABLEREAD, 0, MB_ICONEXCLAMATION | MB_OK, FileName);
    SetFileName(0);
  }
}

//
/// Sets FileName and updates the caption of the window,
/// replacing an empty name with 'Untitled' in its caption
//
void
TEditFile::SetFileName(LPCTSTR fileName)
{
  if (fileName != FileName) {
    delete[] FileName;
    FileName = fileName ? strnewdup(fileName) : 0;
  }
  tstring untitled(LoadString(IDS_UNTITLEDFILE));
  SetDocTitle(FileName ? (LPCTSTR)FileName : untitled.c_str(), 0);
}

//
/// Begins the edit of a new file after calling CanClear to determine that it is
/// safe to clear the text of the editor.
//
void
TEditFile::NewFile()
{
  if (CanClear()) {
    Clear();
    Invalidate();
    ClearModify();
    SetFileName(0);
  }
}

//
/// Calls SetFileName and Read to replace the file currently being edited with a
/// file whose name is supplied.
//
void
TEditFile::ReplaceWith(LPCTSTR fileName)
{
  if (Read(fileName)) 
  {
    Invalidate();
    SetFileName(fileName);
  }
  else 
  {
    FormatMessageBox(IDS_UNABLEREAD, 0, MB_ICONEXCLAMATION | MB_OK, fileName);
  }
}

//
/// Opens a new file after determining that it is OK to clear the text of the
/// Editor. Calls CanClear, and if true is returned, brings up a file dialog box to
/// retrieve the name of a new file from the user. Calls ReplaceWith to pass the
/// name of the new file.
//
/// Same as selecting File|Open from the menus
//
void
TEditFile::Open()
{
  if (CanClear()) {
    *FileData.FileName = 0;
    if (TFileOpenDialog(this, FileData).Execute() == IDOK)
      ReplaceWith(FileData.FileName);
  }
}

//
/// Reads the contents of a previously specified file into the Editor. Returns true
/// if read operation is successful.
/// The caller is responsible for any error UI
//
bool
TEditFile::Read(LPCTSTR fileName)
{
  if (!fileName)
    if (FileName)
      fileName = FileName;
    else
      return false;

  bool   success = false;
  TFile file(fileName, TFile::ReadOnly|TFile::OpenExisting);

  if (file.IsOpen()) {
    long  charsToRead = file.Length();
    if (charsToRead >= 0) {
      Clear();

      // Lock and resize Editor's buffer to the size of the file
      // Then if OK, read the file into editBuffer
      //
      LPTSTR editBuffer = LockBuffer(uint(charsToRead+1));
      if (editBuffer) {
        if (file.Read(editBuffer, uint(charsToRead)) == uint(charsToRead)) {

          // 0 terminate Editor's buffer
          //
          editBuffer[int(charsToRead)] = 0;
          success = true;
          ClearModify();
        }
        UnlockBuffer(editBuffer, true);
      }
    }
  }
  return success;
}

//
/// Saves changes to the contents of the Editor to a file. If Editor->IsModified
/// returns false, Save returns true, indicating there have been no changes since
/// the last open or save.
//
bool
TEditFile::Save()
{
  if (IsModified()) 
  {
    if (!FileName)
      return SaveAs();

    if (!Write()) 
	{
      FormatMessageBox(IDS_UNABLEWRITE, 0, MB_ICONEXCLAMATION | MB_OK, FileName);
      return false;
    }
  }
  return true;  // editor's contents haven't been changed
}

//
/// Saves the contents of the Editor to a file whose name is retrieved from the
/// user, through execution of a File Save dialog box. If the user selects OK,
/// SaveAs calls SetFileName and Write. Returns true if the file was saved.
//
bool
TEditFile::SaveAs()
{
  if (FileName)
    ::_tcscpy(FileData.FileName, FileName);

  else
    *FileData.FileName = 0;

  if (TFileSaveDialog(this, FileData).Execute() == IDOK) 
  {
    if (Write(FileData.FileName)) 
	{
      SetFileName(FileData.FileName);
      return true;
    }
    FormatMessageBox(IDS_UNABLEWRITE, 0, MB_ICONEXCLAMATION | MB_OK, FileName);
  }
  return false;
}

//
/// Enables save command (only if text is modified).
//
void
TEditFile::CmSaveEnable(TCommandEnabler& commandHandler)
{
  commandHandler.Enable(IsModified());
}

//
/// Saves the contents of the Editor to a file whose name is specified by FileName.
/// Returns true if the write operation is successful.
/// The caller is responsible for any error UI
//
bool
TEditFile::Write(LPCTSTR fileName)
{
  if (!fileName)
    if (FileName)
      fileName = FileName;
    else
      return false;

  TFile file(fileName, TFile::WriteOnly|TFile::PermExclusive|TFile::CreateAlways);
  if (!file.IsOpen())
    return false;

  bool success = false;
  LPTSTR editBuffer = LockBuffer();
  if (editBuffer) {
    success = file.Write(editBuffer, static_cast<int>(::_tcslen(editBuffer)));
    UnlockBuffer(editBuffer);
    if (success)
      ClearModify();
  }
  return success;
}

//
/// Returns true if the text of the associated edit control can be cleared
/// which is if the text has not been changed, or if the user Oks the
/// clearing of the text
//
bool
TEditFile::CanClear()
{
  if (IsModified()) 
  {
    tstring untitled(LoadString(IDS_UNTITLEDFILE));

    int result = FormatMessageBox(IDS_FILECHANGED, 0, MB_YESNOCANCEL|MB_ICONQUESTION,
	  FileName ? (LPCTSTR)FileName : untitled.c_str());

    return result==IDYES ? Save() : result != IDCANCEL;
  }
  return true;
}

//
/// Returns true if the edit window can be closed.
//
bool
TEditFile::CanClose()
{
  return CanClear();
}


IMPLEMENT_STREAMABLE1(TEditFile, TEditSearch);

#if !defined(BI_NO_OBJ_STREAMING)

//
// reads an instance of TEditFile from the passed ipstream
//
void*
TEditFile::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TEditFile* o = GetObject();
  ReadBaseObject((TEditSearch*)o, is);

#if defined(UNICODE)
  _USES_CONVERSION;
  char * fileName = is.freadString();

  o->FileName = _A2W(fileName);

  delete[] fileName;
#else
  o->FileName = is.freadString();
#endif  
  if (!*o->FileName) {
    delete o->FileName;
    o->FileName = 0;
  }
  return o;
}

//
// writes the TEditFile to the passed opstream
//
void
TEditFile::Streamer::Write(opstream& os) const
{
  TEditFile* o = GetObject();
  WriteBaseObject((TEditSearch*)o, os);
  _USES_CONVERSION;
  os.fwriteString(o->FileName ? _W2A(o->FileName) : "");
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)


} // OWL namespace

