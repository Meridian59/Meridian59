//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TMailer which provides Mail enabling.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/window.h>
#include <owl/mailer.h>
#include <owl/profile.h>
#include <stdio.h>
#include <owl/commdial.h>

namespace owl {

OWL_DIAGINFO;

  static tchar MapiLibName[] = _T("mapi32.dll");

//
// Loads the MAPI DLL.
//
TMailer::TMailer()
:
  TModule(MapiLibName, true, true, false),  // shouldLoad, mustLoad and !addToList
  MAPISendDocuments(*this, "MAPISendDocuments")
{
}

//
// owner
//   The owning window that is using the mailer. Is used to parent mail UI
//   windows
//
// paths
//   Pointer to string containg the files to send. The files must have a
//   fully qualified path and filename. The string could have multiple
//   files separated by a semicolon ';'.  The filenames may be long filenames.
//
// names
//
// asynchWork
//   If true, the message submission will be made from a separate thread
//   other than the thread where the SendMail was called.  With this flag,
//   the function returns immediately after the working thread has been
//   created. If false, the call is synchronous and it wll return when the
//   message is submitted to the messaging subsystem.
//
// Purpose:
//  This function is called by apps to send files to a mail recipient. The
//  attachments can be of any size. The underlying messaging system will ask
//  for credentials of the recipients.  When the send note is displayed, other
//  options could be added or modified.  Also at that time other type of
//  attachments can be added (i.e. OLE objects).
//
void
TMailer::SendDocuments(TWindow* owner, LPCTSTR docPaths,
                       LPCTSTR docNames, bool asynchWork)
{
  // Get the document names--either directly if passed, or by parsing the paths
  //
  LPTSTR tmpDocNames = 0;
  if (!docNames) {
    // The file name must be separated by semi-colon, so parse the string for
    // the sub-strings
    //
    LPTSTR tmpPaths = strnewdup(docPaths);
    tmpDocNames = new tchar[1024];
    *tmpDocNames = 0;
    LPTSTR nameToken = _tcstok(tmpPaths, _T(";\n"));
    while (nameToken) {
      // Strip leading blanks from name
      //
      while (*nameToken == _T(' '))
        nameToken++;

      // Get the file name (long file names are supported) from the path
      //
      tchar tmpName[_MAX_PATH];
      TCommDlg::GetFileTitle(nameToken, tmpName, _MAX_PATH);

      // Append to string of file names (no paths)
      //
      _tcscat(tmpDocNames, tmpName);
      _tcscat(tmpDocNames, _T(";"));

      // Get the next sub-string in the files string
      //
      nameToken = _tcstok(0, _T(";\n"));
    }
    delete[] tmpPaths;
    docNames = tmpDocNames;
  }

  // Call MAPI to send the document. This function always prompts with a
  // dialog box so that the user can provide recipients and other sending
  // options.  The function tries to etablish a session from the messaging
  // system's shared session.  If no shared session exists, it prompts for
  // logon information to establish a session.  Before the function return,
  // it closes the session.
#if defined(UNICODE)
  USES_CONVERSION;
  uint32 err = MAPISendDocuments(uint32(owner->GetHandle()), ";",
                                 W2A(docPaths),
                                 W2A(docNames),
                                 asynchWork);
#else
  uint32 err = MAPISendDocuments(uint32(owner->GetHandle()), ";",
                                 (LPSTR)docPaths,
                                 (LPSTR)docNames,
                                 asynchWork);
#endif
  if (err) {
    // A problem was encountered.  Report it to the user?
    //sprintf(docNames, "Failed to send the documents. Error: %d", err); //?????????
    //owner->MessageBox(docNames, 0, MB_OK);
  }

  // Delete the temp names string if we new'd one here
  //
  delete[] tmpDocNames;
}


//
/// Returns true if the system is configured with a MAPI compliant client
/// application installed.
/// \note Verifies the [Mail] section of WIN.INI.
//
bool
TMailer::IsMAPIAvailable() const
{
  TProfile mailSection(_T("Mail"));
  return mailSection.GetInt(_T("MAPI")) != 0;
}

} // OWL namespace
/* ========================================================================== */

