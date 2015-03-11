//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TEditWindow, an edit control that responds to Find,
/// Replace and FindNext commands.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/editsear.h>
#include <owl/applicat.h>
#include <owl/edit.h>
#include <owl/findrepl.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;


DEFINE_RESPONSE_TABLE1(TEditSearch, TEdit)
  EV_COMMAND(CM_EDITFIND, CmEditFind),
  EV_COMMAND_ENABLE(CM_EDITFIND, CeEditFindReplace),
  EV_COMMAND(CM_EDITREPLACE, CmEditReplace),
  EV_COMMAND_ENABLE(CM_EDITREPLACE, CeEditFindReplace),
  EV_COMMAND(CM_EDITFINDNEXT, CmEditFindNext),
  EV_COMMAND_ENABLE(CM_EDITFINDNEXT, CeEditFindNext),
  EV_REGISTERED(FINDMSGSTRING, EvFindMsg),
END_RESPONSE_TABLE;

//
/// Constructs a TEditSearch object given the parent window, resource ID,
/// and character string (text).
//
TEditSearch::TEditSearch(TWindow*        parent,
                         int             id,
                         LPCTSTR         text,
                         int x, int y, int w, int h,
                         TModule*        module)
:
  TEdit(parent, id, text, x, y, w, h, 0, true, module),
  SearchData(FR_DOWN)
{
  Attr.Style |= ES_NOHIDESEL;
  SearchDialog = 0;
  SearchCmd = 0;
}

//
/// String-aware overload
//
TEditSearch::TEditSearch(
  TWindow* parent,
  int id,
  const tstring& text,
  int x, int y, int w, int h,
  TModule* module
  )
  : TEdit(parent, id, text, x, y, w, h, 0, true, module),
  SearchData(FR_DOWN)
{
  Attr.Style |= ES_NOHIDESEL;
  SearchDialog = 0;
  SearchCmd = 0;
}

//
//
//
TEditSearch::~TEditSearch()
{
  delete SearchDialog;
}

//
// Post a CM_EDITFIND or a CM_EDITREPLACE to re-open a previously open
// find or replace modeless dialog
//
void
TEditSearch::SetupWindow()
{
  TEdit::SetupWindow();
  if (SearchCmd)
    PostMessage(WM_COMMAND, SearchCmd);
}

//
/// Performs a search or replace operation base on information in SearchData.
//
void
TEditSearch::DoSearch()
{
  do {
    if (Search(-1, SearchData.FindWhat, bool(SearchData.Flags&FR_MATCHCASE),
               bool(SearchData.Flags&FR_WHOLEWORD),
               !(SearchData.Flags&FR_DOWN)) >= 0) 
	{
      if (SearchData.Flags & (FR_REPLACE|FR_REPLACEALL))
        Insert(SearchData.ReplaceWith);
    }
    else 
	{
      if (SearchData.Flags & (FR_FINDNEXT|FR_REPLACE)) 
	  {
        FormatMessageBox(IDS_CANNOTFIND, 0, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL, (LPCTSTR)SearchData.FindWhat);
      }
      else if (SearchData.Flags & FR_REPLACEALL)
        break;
    }
  } while (SearchData.Flags & FR_REPLACEALL);
}

//
/// Opens a modeless TFindDialog in response to an incoming Find command with a CM_EDITFIND
/// command.
//
void
TEditSearch::CmEditFind()
{
  if (!SearchCmd) {
    SearchCmd = CM_EDITFIND;
    delete SearchDialog;
    SearchDialog = new TFindDialog(this, SearchData);
    SearchDialog->Create();
  }
}

//
/// Opens a TReplaceDialog in response to an incoming Replace command with a
/// CM_EDITREPLACE command.
//
void
TEditSearch::CmEditReplace()
{
  if (!SearchCmd) {
    SearchCmd = CM_EDITREPLACE;
    delete SearchDialog;
    SearchDialog = new TReplaceDialog(this, SearchData);
    SearchDialog->Create();
  }
}

//
/// Enables the find or replace option (only if no dialog is up).
//
void
TEditSearch::CeEditFindReplace(TCommandEnabler& ce)
{
  ce.Enable(!SearchCmd);
}

//
/// Responds to an incoming FindNext command with a CM_EDITFINDNEXT command
/// identifier by calling DoSearch to repeat the search operation.
//
void
TEditSearch::CmEditFindNext()
{
  if (SearchDialog)
    SearchDialog->UpdateData();
  SearchData.Flags |= FR_FINDNEXT;
  DoSearch();
}

//
/// Enables FindNext (only if there's data to search for).
//
void
TEditSearch::CeEditFindNext(TCommandEnabler& ce)
{
  ce.Enable(SearchData.FindWhat && *SearchData.FindWhat);
}

//
/// Responds to a message sent by the modeless find or replace dialog box. Calls
/// DoSearch to continue searching if text is not found or the end of the document
/// has not been reached.
//
TResult
TEditSearch::EvFindMsg(TParam1, TParam2 param2)
{
  PRECONDITION(SearchDialog);

  SearchDialog->UpdateData(param2);
  if (SearchData.Flags & FR_DIALOGTERM)
    SearchCmd = 0;

  else
    DoSearch();
  return 0;
}



IMPLEMENT_STREAMABLE1(TEditSearch, TEdit);

#if !defined(BI_NO_OBJ_STREAMING)

//
// reads an instance of TEditSearch from the passed ipstream.
// Re-opens the modeless find or replace dialog if one was up.
//
void*
TEditSearch::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TEdit*)GetObject(), is);

  GetObject()->SearchData.Read(is);
  is >> GetObject()->SearchCmd;
  GetObject()->SearchDialog = 0;
  return GetObject();
}

//
// writes the TEditSearch to the passed opstream
//
void
TEditSearch::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TEdit*)GetObject(), os);

  GetObject()->SearchData.Write(os);
  os << GetObject()->SearchCmd;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace

