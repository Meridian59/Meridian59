//----------------------------------------------------------------------------
// OWLNext
//
/// \file
/// Microsoft UxTheme Library Encapsulation
//----------------------------------------------------------------------------

#if !defined(OWL_THEME_H)
#define OWL_THEME_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/module.h>

#if defined(BI_NEED_UXTHEME_H)
#include <api_upd/uxtheme.h>
#include <api_upd/tmschema.h>
#else
#include <uxtheme.h>
#if _MSC_VER >= 1700
#include <vssym32.h>
#else
#include <tmschema.h>
#endif
#endif

namespace owl {


//
/// This singleton encapsulates the "uxtheme.dll" module.
//
class _OWLCLASS TThemeModule : public TModule
{
public:

  /// Returns the module instance.
  /// Exceptions are thrown if the module cannot be loaded.
  //
  static TThemeModule& GetInstance();

  // API functors - dynamically linked functions
  // General
  //
  TModuleProc1 <HRESULT, HTHEME> CloseThemeData;
  TModuleProc3 <HRESULT, HWND, HDC, RECT*> DrawThemeParentBackground;
  TModuleProc2 <HRESULT, HWND, DWORD> EnableThemeDialogTexture;
  TModuleProc1 <HRESULT, BOOL> EnableTheming;
  TModuleProc6 <HRESULT, LPWSTR, int, LPWSTR, int, LPWSTR, int> GetCurrentThemeName;
  TModuleProc0 <DWORD> GetThemeAppProperties;
  TModuleProc4 <HRESULT, LPCWSTR, LPCWSTR, LPWSTR, int> GetThemeDocumentationProperty;
  TModuleProc1 <HTHEME, HWND> GetWindowTheme;
  TModuleProc0 <BOOL> IsAppThemed;
  TModuleProc0 <BOOL> IsThemeActive;
  TModuleProc1 <BOOL, HWND> IsThemeDialogTextureEnabled;
  TModuleProc2 <HTHEME, HWND, LPCWSTR> OpenThemeData;
  TModuleProcV1 <DWORD> SetThemeAppProperties;
  TModuleProc3 <HRESULT, HWND, LPCWSTR, LPCWSTR> SetWindowTheme;

  // API functors (continued)
  // Theme sys properties
  //
  TModuleProc2 <BOOL, HTHEME, int> GetThemeSysBool;
  TModuleProc2 <COLORREF, HTHEME, int> GetThemeSysColor;
  TModuleProc2 <HBRUSH, HTHEME, int> GetThemeSysColorBrush;
  TModuleProc3 <HRESULT, HTHEME, int, LOGFONT*> GetThemeSysFont;
  TModuleProc3 <HRESULT, HTHEME, int, int*> GetThemeSysInt;
  TModuleProc2 <int, HTHEME, int> GetThemeSysSize;
  TModuleProc4 <HRESULT, HTHEME, int, LPWSTR, int> GetThemeSysString;
  
  // API functors (continued)
  // Theme parts
  //
  TModuleProc6 <HRESULT, HTHEME, HDC, int, int, const RECT*, const RECT*> DrawThemeBackground;
  TModuleProc8 <HRESULT, HTHEME, HDC, int, int, const RECT*, UINT, UINT, RECT*> DrawThemeEdge;
  TModuleProc7 <HRESULT, HTHEME, HDC, int, int, const RECT*, HIMAGELIST, int> DrawThemeIcon;
  TModuleProc9 <HRESULT, HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*> DrawThemeText;
  TModuleProc6 <HRESULT, HTHEME, HDC, int, int, const RECT*, RECT*> GetThemeBackgroundContentRect;
  TModuleProc6 <HRESULT, HTHEME, HDC, int, int, const RECT*, RECT*> GetThemeBackgroundExtent;
  TModuleProc6 <HRESULT, HTHEME, HDC, int, int, const RECT*, HRGN*> GetThemeBackgroundRegion;
  TModuleProc5 <HRESULT, HTHEME, int, int, int, BOOL*> GetThemeBool;
  TModuleProc5 <HRESULT, HTHEME, int, int, int, COLORREF*> GetThemeColor;
  TModuleProc5 <HRESULT, HTHEME, int, int, int, int*> GetThemeEnumValue;
  TModuleProc6 <HRESULT, HTHEME, int, int, int, LPWSTR, int> GetThemeFilename;
  TModuleProc6 <HRESULT, HTHEME, HDC, int, int, int, LOGFONT*> GetThemeFont;
  TModuleProc5 <HRESULT, HTHEME, int, int, int, int*> GetThemeInt;
  TModuleProc5 <HRESULT, HTHEME, int, int, int, INTLIST*> GetThemeIntList;
  TModuleProc7 <HRESULT, HTHEME, HDC, int, int, int, RECT*, MARGINS*> GetThemeMargins;
  TModuleProc6 <HRESULT, HTHEME, HDC, int, int, int, int*> GetThemeMetric;
  TModuleProc7 <HRESULT, HTHEME, HDC, int, int, RECT*, THEMESIZE, SIZE*> GetThemePartSize;
  TModuleProc5 <HRESULT, HTHEME, int, int, int, POINT*> GetThemePosition;
  TModuleProc5 <HRESULT, HTHEME, int, int, int, PROPERTYORIGIN*> GetThemePropertyOrigin;
  TModuleProc5 <HRESULT, HTHEME, int, int, int, RECT*> GetThemeRect;
  TModuleProc6 <HRESULT, HTHEME, int, int, int, LPWSTR, int> GetThemeString;
  TModuleProc9 <HRESULT, HTHEME, HDC, int, int, LPCWSTR, int, DWORD, const RECT*, RECT*> GetThemeTextExtent;
  TModuleProc5 <HRESULT, HTHEME, HDC, int, int, TEXTMETRIC*> GetThemeTextMetrics;
  TModuleProc9 <HRESULT, HTHEME, HDC, int, int, DWORD, const RECT*, HRGN, POINT, WORD*> HitTestThemeBackground;
  TModuleProc3 <BOOL, HTHEME, int, int> IsThemeBackgroundPartiallyTransparent;
  TModuleProc3 <BOOL, HTHEME, int, int> IsThemePartDefined;

private:
  TThemeModule();
};

// 
/// Encapsulates a theme handle.
/// TODO: Add appropriate member functions from the module API.
// 
class _OWLCLASS TTheme
{
public:
  TTheme(HWND, LPCWSTR cls);
  virtual ~TTheme();

  // Accessors
  //
  HTHEME GetHandle() {return m_handle;}
  operator HTHEME() {return m_handle;}

private:
  HTHEME m_handle;
};

// 
/// Encapsulates a themed part.
// 
class _OWLCLASS TThemePart : public TTheme
{
public:
  TThemePart(HWND, LPCWSTR cls, int part, int state);

  // Accessors and mutators
  //
  int GetPart() const {return m_part;}
  int GetState() const {return m_state;}
  void SetState(int state) {m_state = state;}

  // UxTheme interface
  // TODO: Complete the interface.
  //
  void DrawBackground(HDC, const TRect& dest);
  void DrawBackground(HDC, const TRect& dest, const TRect& clip);
  bool IsBackgroundPartiallyTransparent();

  /// Combines a call to DrawThemeParentBackground and DrawBackground if
  /// the part is partially transparent. Otherwise just calls DrawBackground.
  //
  void DrawTransparentBackground(HWND parent, HDC, const TRect& dest);

private:
  int m_part;
  int m_state;
};

//
// Exception class
//
class _OWLCLASS TXTheme : public TXOwl 
{
public:
  HRESULT result;

  TXTheme(const tstring& = tstring(), HRESULT = 0);

  virtual TXTheme* Clone() const; // override
  void Throw();

  static void Raise(const tstring& name = tstring(), HRESULT = 0);
};


} // OWL namespace


#endif // OWL_THEME_H


