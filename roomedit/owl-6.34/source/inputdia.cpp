//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TInputDialog.  User string input dialog box
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/inputdia.h>
#include <owl/edit.h>
#include <owl/validate.h>
#include <algorithm>

namespace owl {

OWL_DIAGINFO;

/** Let the compiler know that the following template instances will be defined elsewhere. **/
//#pragma option -Jgx


//
/// Invokes TDialog's constructor, passing it parent, the resource identifier, and
/// module. Sets the caption of the dialog box to title and the prompt static
/// control to prompt. Sets the Buffer and BufferSize data members to buffer and
/// bufferSize.
//
TInputDialog::TInputDialog(TWindow*        parent,
                           LPCTSTR         title,
                           LPCTSTR         prompt,
                           LPTSTR          buffer,
                           int             bufferSize,
                           TModule*        module,
                           TValidator*     validator)
: TDialog(parent, IDD_INPUTDIALOG, module),
  ShouldDelete(false)
{
  PRECONDITION(buffer);
  SetCaption(title); // Override title in resource.
  Prompt = strnewdup(prompt);
  Buffer = buffer;
  BufferSize = bufferSize;
  if (validator)
    (new TEdit(this,ID_INPUT))->SetValidator(validator);
}

//
/// String-aware overload
//
TInputDialog::TInputDialog(
  TWindow* parent,
  const tstring& title,
  const tstring& prompt,
  LPTSTR buffer,
  int bufferSize,
  TModule* module,
  TValidator* validator
  )
  : TDialog(parent, IDD_INPUTDIALOG, module),
  ShouldDelete(false)
{
  PRECONDITION(buffer);
  SetCaption(title); // Override title in resource.
  Prompt = strnewdup(prompt.c_str());
  Buffer = buffer;
  BufferSize = bufferSize;
  if (validator)
    (new TEdit(this,ID_INPUT))->SetValidator(validator);
}

namespace 
{

  const size_t DefaultBufferSize_ = 1024;

  int CalculateBufferSize_(const tstring& s)
  {
    PRECONDITION(s.size() <= INT_MAX);
    return static_cast<int>(std::max(DefaultBufferSize_, s.size() + 1));
  }

  tchar* AllocateAndInitializeBuffer_(const tstring& s)
  {
    tchar* b = new tchar[CalculateBufferSize_(s)];
    std::copy(s.begin(), s.end(), b);
    b[s.size()] = _T('\0');
    return b;
  }

} // namespace

//
/// String-aware overload
/// This overload requires no buffer to be passed. A buffer is allocated internally.
/// Optionally you can pass the initial string.
//
TInputDialog::TInputDialog(
  TWindow* parent,
  const tstring& title,
  const tstring& prompt,
  const tstring& initValue,
  TModule* module,
  TValidator* validator
  )
  : TDialog(parent, IDD_INPUTDIALOG, module),
  Prompt(strnewdup(prompt.c_str())),
  Buffer(AllocateAndInitializeBuffer_(initValue)),
  BufferSize(CalculateBufferSize_(initValue)),
  ShouldDelete(true)
{
  try
  {
    SetCaption(title); // Override title in resource.
    if (validator)
      (new TEdit(this, ID_INPUT))->SetValidator(validator);
  }
  catch (...)
  {
    delete[] Prompt;
    delete[] Buffer;
    throw;
  }
}

//
/// Destructor for this class.
//
TInputDialog::~TInputDialog()
{
  delete[] Prompt;
  if (ShouldDelete)
    delete[] Buffer;
}

//
/// Transfers the data of the input dialog box. If direction is tdSetData, sets the
/// text of the static and edit controls of the dialog box to the text in prompt and
/// buffer.
//
void
TInputDialog::TransferData(TTransferDirection direction)
{
  if (direction == tdSetData) {
    SetDlgItemText(ID_PROMPT, Prompt);
    SetDlgItemText(ID_INPUT, Buffer);
  }
  else if (direction == tdGetData) {
    GetDlgItemText(ID_INPUT, Buffer, BufferSize);
  }
}

//
/// Calls TDialog::SetupWindow to set up the window, then limits the number of
/// characters the user can enter to bufferSize - 1.
//
void
TInputDialog::SetupWindow()
{
  TDialog::SetupWindow();
  SendDlgItemMessage(ID_INPUT, EM_LIMITTEXT, BufferSize - 1, 0);
}


IMPLEMENT_STREAMABLE2(TInputDialog, TDialog, TWindow);

#if !defined(BI_NO_OBJ_STREAMING)

//
/// Reads an instance of TInputDialog from the passed ipstream
//
void*
TInputDialog::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TDialog*)GetObject(), is);
#if defined(UNICODE)
  _USES_CONVERSION;
  char * prompt = is.freadString();

  GetObject()->Prompt = strnewdup(_A2W(prompt));
  
  delete[] prompt;
#else
  GetObject()->Prompt = is.freadString();
#endif
  return GetObject();
}

//
/// Writes the TInputDialog to the passed opstream
//
void
TInputDialog::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TDialog*)GetObject(), os);
  _USES_CONVERSION;
  os.fwriteString(_W2A(GetObject()->Prompt));
}


#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */

