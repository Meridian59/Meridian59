//----------------------------------------------------------------------------
//  ObjectWindows, OWL NExt
//  Copyright 1998-1999 by Yura Bidus. All Rights reserved.
//
/// \file
///  Class definition for TMemComboBox (TComboBox).
//----------------------------------------------------------------------------

#if !defined(OWL_MEMCBOX_H) // Sentry, use file only if it's not already included.
#define OWL_MEMCBOX_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/combobox.h>

namespace owl {class _OWLCLASS TConfigFile;};

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{
//
/// \class TMemComboBox
// ~~~~~ ~~~~~~~~~~~~
/// class remember old user selections.
/// Saves strings in registry under: 
/// \code
/// CURRENT_USER\Software\OWL NExt\History Lists\[Name]
/// \endcode
/// Where [Name] - parameter passed in constructor.
/// Override CreateConfigFile(); to customize this
/// Support Transfer mechanizm, to pass value to Edit part of ComboBox. 
/// No ComboBox transfer Data, 
/// Use as transfer structure: tchar buffer[textLimit]; 
/// textLimit -  parameter passed in constructor.
/// Example:
/// \code
/// struct TXFer{
///   uint16 CheckBox1;
///   uint16 CheckBox2;
///   tchar Edit1[255];
///   tchar MemComboBox[255];
/// };
/// \endcode  
//

// If you use this class as is, all programs will share common data,
// Example Find dialog, all programms can share Search string data 
// Common values, use them, and send me your suggestions 
#define HL_SearchText  "hlSearchText"
#define HL_ReplaceText "hlReplaceText"
#define HL_GotoLine    "hlGotoLine"
#define HL_BinGoto     "hlBinGoto"
#define HL_BlockIndent "hlBlockIndent"
#define HL_RightMargin "hlRightMargin"
#define HL_GutterWidth "hlGutterWidth"
#define HL_UndoLimit   "hlUndoLimit"
#define HL_TabStops    "hlTabStops"
#define HL_OtherText   "hlOtherText"

class _OWLCLASS TMemComboBox: public TComboBox {
  typedef TComboBox Inherited;
  public:
    TMemComboBox(TWindow* parent, int id, int x, int y, int w, int h, const tstring& name, 
      uint textLimit = 255, uint itemLimit = 25, TModule* = 0);
    TMemComboBox(TWindow* parent, int resId, const tstring& name, 
      uint textLimit = 255, uint itemLimit = 25, TModule* = 0);
  
    //
    /// TWindow override; transfers data of the configured type to or from the given buffer.
    /// See SetTransferBufferFieldType.
    //
    virtual uint Transfer(void* buffer, TTransferDirection direction);

    /// \name Accessors and mutators for setting the transfer buffer field type
    /// These are used by the Safe Buffer Transfer machinery, and should not be
    /// used in general. TODO: Consider making these private with friend access.
    /// @{

    enum TTransferBufferFieldType
    {
      tbftCharArray, tbftString, tbftComboBoxData
    };
    TTransferBufferFieldType GetTransferBufferFieldType() const;
    void SetTransferBufferFieldType(TTransferBufferFieldType);

    /// @}

  protected:
    tstring Name;
    uint ItemLimit;
    TTransferBufferFieldType TransferBufferFieldType;

    void LoadContents();
    void SaveContents();

    virtual void CleanupWindow();
    virtual void SetupWindow();

    void EvKillFocus(HWND hWndGetFocus);
    virtual TConfigFile* CreateConfigFile();
    virtual void AddToList(LPTSTR text);
    virtual tstring GetSectionName();

  private:

    // Transfer implementations for different field types
    //
    uint TransferCharArray(void* buffer, TTransferDirection direction);
    uint TransferString(void* buffer, TTransferDirection direction);

  DECLARE_RESPONSE_TABLE(TMemComboBox);
  DECLARE_STREAMABLE_OWL(TMemComboBox, 1);
}; 

/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TMemComboBox );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

#endif  // OWL_MEMCBOX_H sentry.

