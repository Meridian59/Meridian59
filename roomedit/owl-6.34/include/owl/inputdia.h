//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TInputDialog class
//----------------------------------------------------------------------------

#if !defined(OWL_INPUTDIA_H)
#define OWL_INPUTDIA_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/dialog.h>
#include <owl/inputdia.rh>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TValidator;

/// \addtogroup dlg
/// @{
/// \class TInputDialog
// ~~~~~ ~~~~~~~~~~~~
/// Provides a generic dialog box to retrieve text input by a user. When the input
/// dialog box is constructed, its title, prompt, and default input text are
/// specified. TInputDialog is a streamable class.
//
class _OWLCLASS TInputDialog : public TDialog {
  public:
    TInputDialog(TWindow*     parent,
                 LPCTSTR      title,
                 LPCTSTR      prompt,
                 LPTSTR        buffer,
                 int          bufferSize,
                 TModule*     module = 0,
                 TValidator*  valid = 0);  // Optional validator

    TInputDialog(
      TWindow* parent,
      const tstring& title,
      const tstring& prompt,
      LPTSTR buffer,
      int bufferSize,
      TModule* module = 0,
      TValidator* valid = 0);  // Optional validator

    TInputDialog(
      TWindow* parent, 
      const tstring& title, 
      const tstring& prompt, 
      const tstring& initValue = tstring(),
      TModule* module = 0,
      TValidator* valid = 0);

   ~TInputDialog();

    // Override TWindow virtual member functions
    //
    void TransferData(TTransferDirection);

    LPCTSTR GetPrompt() const;
    LPCTSTR GetBuffer() const;
    int     GetBufferSize() const;

  protected:
    // Override TWindow virtual member functions
    //
    void SetupWindow();

  public_data:
/// Points to the prompt for the input dialog box.
    tchar * Prompt;
    
/// Pointer to the buffer that returns the text retrieved from the user. When passed
/// to the constructor of the input dialog box, contains the default text to be
/// initially displayed in the edit control.
    tchar * Buffer;
    
/// Contains the size of the buffer that returns user input.
    int          BufferSize;

  private:

    bool ShouldDelete;

    // Hidden to prevent accidental copying or assignment
    //
    TInputDialog(const TInputDialog&);
    TInputDialog& operator=(const TInputDialog&);

  //DECLARE_STREAMABLE(_OWLCLASS, owl::TInputDialog, 1);
  DECLARE_STREAMABLE_OWL(TInputDialog, 1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TInputDialog );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the prompt for the dialog.
//
inline LPCTSTR TInputDialog::GetPrompt() const
{
  return Prompt;
}

//
/// Returns the buffer.
//
inline LPCTSTR TInputDialog::GetBuffer() const
{
  return Buffer;
}

//
/// Returns the size of the buffer.
//
inline int TInputDialog::GetBufferSize() const
{
  return BufferSize;
}



} // OWL namespace

#endif  // OWL_INPUTDIA_H
