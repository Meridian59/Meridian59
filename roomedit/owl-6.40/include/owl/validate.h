//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OWL_VALIDATE_H)
#define OWL_VALIDATE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>
#include <owl/bitset.h>
#include <owl/validate.rh>
#include <owl/contain.h>
#include <owl/template.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup validator
/// @{

class _OWLCLASS TXValidator;

//
/// Validator option flags
//
/// Constants that represent bits in the bitmapped Options word in validator
/// objects.
enum TValidatorOptions {
  voFill     =  0x0001,   ///< option to fill in chars on IsValidInput checks
													///< Used by picture validators to indicate whether to fill in literal
													///< characters as the user types.
  voTransfer =  0x0002,   ///< option to perform conversion & transfer
													///< The validator handles data transfer for the input line. Currently
													///< only used by range validators.
  voOnAppend =  0x0004,   ///< option to only validate input on appending
													///< Used by picture validators to determine how to interact with edit
													///< controls.
  voReserved =  0x00F8    ///< reserved for future use
};

//
/// \class TValidator
// ~~~~~ ~~~~~~~~~~
/// A streamable class, TValidator defines an abstract data validation object.
/// Although you will never actually create an instance of TValidator, it provides
/// the abstract functions for the other data validation objects.
/// 
/// The VALIDATE.CPP sample program on BC5.0x distribution disk derives TValidateApp
/// from TApplication in the following manner:
/// \code
/// class TValidateApp : public TApplication { 
///    public:
///       TValidateApp() : TApplication("ValidateApp") { }
///       void InitMainWindow() { 
///          MainWindow = new TTestWindow(0, "Validate Dialog Input");
/// }
/// \endcode
/// The program displays the following message box if the user enters an invalid
/// employee ID:
/// \image html bm275.BMP
//
class _OWLCLASS TValidator : public TStreamableBase {
  public:
    TValidator();
    virtual ~TValidator();

    virtual void Error(TWindow* owner); // !CQ incompatible change, needs doc!
    virtual bool IsValidInput(LPTSTR input, bool suppressFill);
    virtual bool IsValid(LPCTSTR input);
    virtual uint Transfer(LPTSTR text, void* buffer, TTransferDirection direction);
    virtual int  Adjust(tstring& text, int& begPos, int& endPos, int amount); // !CQ new!

    // Checks input against validator for completeness. Never modifies input.
    // Calls error if not valid.
    //
    bool         Valid(LPCTSTR str, TWindow* owner = 0);
    bool         Valid(const tstring& str, TWindow* owner = 0) {return Valid(str.c_str(), owner);}
    bool         HasOption(int option);
    void         SetOption(int option);
    void         UnsetOption(int option);

  protected:
/// A bitmap member used to control options for various descendants of TValidator.
/// By default, the TValidator constructor clears all the bits in Options.
    uint16   Options;

//  DECLARE_STREAMABLE(_OWLCLASS, owl::TValidator, 1);
  DECLARE_STREAMABLE_OWL(TValidator, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TValidator );

//
/// \class TXValidator
// ~~~~~ ~~~~~~~~~~~
/// A nested class, TXValidator describes an exception that results from an invalid
/// validator object. That is, if a validator expression is not valid, this
/// exception is thrown.
class _OWLCLASS TXValidator : public TXOwl {
  public:
    TXValidator(uint resId = IDS_VALIDATORSYNTAX);

    virtual TXValidator* Clone() const; // override
    void Throw();

    static void Raise();
};

//
/// \enum TPicResult
// ~~~~ ~~~~~~~~~~~~~~~~~~~
/// Validation result type
//
/// TPicResult is the result type returned by the Picture member function of
/// TPXPictureValidator. The result type indicates whether the data entered into the
/// edit control matches a specified format. For example, prIncomplete indicates
/// that the data entered is missing some information that was specified in the
/// format picture of the data.
//
enum TPicResult {
  prComplete,
  prIncomplete,
  prEmpty,
  prError,
  prSyntax,
  prAmbiguous,
  prIncompNoFill
};

//
/// \class TPXPictureValidator
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
/// TPXPictureValidator objects compare user input with a picture of a data format
/// to determine the validity of entered data. The pictures are compatible with the
/// pictures Borland's Paradox relational database uses to control data entry. For a
/// complete description of picture specifiers, see the Picture member function.
class _OWLCLASS TPXPictureValidator : public TValidator {
  public:
    TPXPictureValidator(LPCTSTR pic, bool autoFill=false);
    TPXPictureValidator(const tstring& pic, bool autoFill=false);

    // Override TValidator's virtuals
    //
    void         Error(TWindow* owner);
    bool         IsValidInput(LPTSTR str, bool suppressFill);
    bool         IsValid(LPCTSTR str);
    int          Adjust(tstring& text, int& begPos, int& endPos, int amount);  // !CQ new!

    virtual TPicResult Picture(LPTSTR input, bool autoFill=false);

  protected:
    const tstring& GetPic() const;
    void              SetPic(const tstring& pic);

  protected_data:
/// Points to a string containing the picture that specifies the format for data in
/// the associated edit control. The constructor sets Pic to a string that is passed
    tstring Pic;

  private:
    void Init(bool autoFill);
    bool         IsComplete(TPicResult rslt);
    bool         IsIncomplete(TPicResult rslt);
    void         ToGroupEnd(uint termCh, uint& i);
    bool         SkipToComma(uint termCh, uint& i);
    uint         CalcTerm(uint termCh, uint i);
    TPicResult   Iteration(LPTSTR input, uint termCh, uint& i, uint& j);
    TPicResult   Group(LPTSTR input, uint termCh, uint& i, uint& j);
    TPicResult   CheckComplete(uint termCh, uint& i, TPicResult rslt);

    TPicResult   Scan(LPTSTR input, uint termCh, uint& i, uint& j);
    TPicResult   Process(LPTSTR input, uint termCh, uint& i, uint& j);
    bool         SyntaxCheck();

  //DECLARE_STREAMABLE(_OWLCLASS, TPXPictureValidator, 1);
  DECLARE_STREAMABLE_OWL(TPXPictureValidator, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TPXPictureValidator );

//
/// \class TFilterValidator
// ~~~~~ ~~~~~~~~~~~~~~~~
/// A streamable class, TFilterValidator checks an input field as the user types
/// into it. The validator holds a set of allowed characters. When the user enters a
/// character, the filter validator indicates whether the character is valid or
/// invalid. See TValidator for an example of input validation.
class _OWLCLASS TFilterValidator : public TValidator {
  public:
    TFilterValidator(const TCharSet& validChars);

    // Override TValidator's virtuals
    //
    void         Error(TWindow* owner);
    bool         IsValid(LPCTSTR str);
    bool         IsValidInput(LPTSTR str, bool suppressFill);

  protected:
    const TCharSet&  GetValidChars();
    void             SetValidChars(const TCharSet& vc);

  protected_data:
/// Contains the set of all characters the user can type. For example, to allow only
/// numeric digits, set ValidChars to "0-9". ValidChars is set by the validChars
/// parameter passed to the constructor.
    TCharSet     ValidChars;

  //DECLARE_STREAMABLE(_OWLCLASS, TFilterValidator, 1);
  DECLARE_STREAMABLE_OWL(TFilterValidator, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TFilterValidator );

//
/// \class TRangeValidator
// ~~~~~ ~~~~~~~~~~~~~~~
/// Determines whether the data typed by a user falls within a designated range of
/// integers. TRangeValidator is a streamable class.
class _OWLCLASS TRangeValidator : public TFilterValidator 
{
  public:
    TRangeValidator(long minValue, long maxValue);

    // Override TValidator's virtuals
    //
    void         Error(TWindow* owner);
    bool         IsValid(LPCTSTR str);
    uint         Transfer(LPTSTR str, void* buffer, TTransferDirection direction);
    int          Adjust(tstring& text, int& begPos, int& endPos, int amount);  // !CQ new!

  protected:
    long   GetMin();
    void   SetMin(long minValue);
    long   GetMax();
    void   SetMax(long maxValue);

  protected_data:
/// Min is the lowest valid long integer value for the edit control.
    long   Min;

/// Max is the highest valid long integer value for the edit control.
    long   Max;

  //DECLARE_STREAMABLE(_OWLCLASS, TRangeValidator, 1);
  DECLARE_STREAMABLE_OWL(TRangeValidator, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TRangeValidator );

//
/// \class TLookupValidator
// ~~~~~ ~~~~~~~~~~~~~~~~
/// A streamable class, TLookupValidator compares the string typed by a user with a
/// list of acceptable values. TLookupValidator is an abstract validator type from
/// which you can derive useful lookup validators. You will never create an instance
/// of TLookupValidator. When you create a lookup validator type, you need to
/// specify a list of valid items and override the Lookup method to return true only
/// if the user input matches an item in that list. One example of a working
/// descendant of TLookupValidator is TStringLookupValidator.
class _OWLCLASS TLookupValidator : public TValidator {
  public:
    TLookupValidator();

    // Override TValidator's virtuals
    //
    bool         IsValid(LPCTSTR str);

    // Virtual lookup of a string
    //
    virtual bool Lookup(LPCTSTR str);

  //DECLARE_STREAMABLE(_OWLCLASS, TLookupValidator, 1);
  DECLARE_STREAMABLE_OWL(TLookupValidator, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TLookupValidator );


//
/// \class TStringLookupValidator
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~~
/// Derived from TLookupValidator, TStringLookupValidator is a streamable class. A
/// TStringLookupValidator object verifies the data in its associated edit control
/// by searching through a collection of valid strings. You can use string-lookup
/// validators when your edit control needs to accept only members of a certain set
/// of strings.
class _OWLCLASS TStringLookupValidator : public TLookupValidator {
  public:
    TStringLookupValidator(TSortedStringArray* strings);
   ~TStringLookupValidator();

    // Override TValidator's virtuals
    //
    void         Error(TWindow* owner);
    int          Adjust(tstring& text, int& begPos, int& endPos, int amount); // !CQ new!

    // Override TLookupValidator's virtuals
    //
    bool         Lookup(LPCTSTR str);

    void         NewStringList(TSortedStringArray* strings);

  protected:
    const TSortedStringArray* GetStrings() const;
    void                      SetStrings(TSortedStringArray* strings);

  protected_data:
/// Points to a string collection containing all the valid strings the user can
/// type. If Strings is NULL, all input is validated.
    TSortedStringArray* Strings;

  //DECLARE_STREAMABLE(_OWLCLASS, TStringLookupValidator, 1);
  DECLARE_STREAMABLE_OWL(TStringLookupValidator, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TStringLookupValidator );

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline Implementation
//

//
/// Returns true if IsValid returns true. Otherwise, calls Error and returns false.
/// A validator's Valid method is called by the IsValid method of its associated
/// edit control.
/// Edit controls with associated validator objects call the validator's Valid
/// method under two conditions. The first condition is when the edit control's
/// IsValid method is called with the reportErr parameter set to true. The second
/// condition is when the dialog box that contains the edit control calls Valid for
/// all its controls, usually because the user requested to close the dialog box or
/// to accept an entry screen.
//
inline bool TValidator::Valid(LPCTSTR str, TWindow* owner)
{
  if (!IsValid(str)) {
    Error(owner);
    return false;
  }
  return true;
}

//
/// Gets the Options bits. Returns true if a specified option is set.
//
inline bool TValidator::HasOption(int option)
{
  return Options & option;
}

//
/// Sets the bits for the Options data member.
//
inline void TValidator::SetOption(int option)
{
  Options |= uint16(option);
}

//
/// Unsets the bits specified in the Options data member.
//
inline void TValidator::UnsetOption(int option)
{
  Options &= uint16(~option);
}

//
/// Returns the picture mask used by the validator.
//
inline const tstring& TPXPictureValidator::GetPic() const
{
  return Pic;
}

//
/// Sets the picture mask for the validator.
//
inline void TPXPictureValidator::SetPic(const tstring& pic)
{
  Pic = pic;
}

//
/// Returns the valid character set for the validator.
//
inline const TCharSet& TFilterValidator::GetValidChars()
{
  return ValidChars;
}

//
/// Sets the valid character set for the validator.
//
inline void TFilterValidator::SetValidChars(const TCharSet& vc)
{
  ValidChars = vc;
}

//
/// Returns the minimum number the validator can accept.
//
inline long TRangeValidator::GetMin()
{
  return Min;
}

//
/// Sets the minimum number the validator can accept.
//
inline void TRangeValidator::SetMin(long minValue)
{
  Min = minValue;
}

//
/// Returns the maximum number the validator can accept.
//
inline long TRangeValidator::GetMax()
{
  return Max;
}

//
/// Sets the maximum number the validator can accept.
//
inline void TRangeValidator::SetMax(long maxValue)
{
  Max = maxValue;
}

//
/// Returns the set of valid strings used by the validator.
//
inline const TSortedStringArray* TStringLookupValidator::GetStrings() const
{
  return Strings;
}

//
/// Sets the valid strings used by the validator.
//
inline void TStringLookupValidator::SetStrings(TSortedStringArray* strings)
{
  delete Strings;
  Strings = strings;
}

} // OWL namespace


#endif  // OWL_VALIDATE_H
