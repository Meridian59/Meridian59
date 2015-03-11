//----------------------------------------------------------------------------
// OWLNext
//
/// \file
/// Microsoft UxTheme Library Encapsulation
//----------------------------------------------------------------------------

#include <owl/pch.h>
#include <owl/theme.h>

#if defined(__BORLANDC__)
# pragma option -w-inl // Disable "Function containing 'statment' is not expanded inline"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlTheme, 1, 0);

//
/// Private constructor
/// Loads the DLL and dynamically links the module functions.
//
TThemeModule::TThemeModule()
  : TModule(_T("uxtheme.dll"), true, true, false), // shouldLoad, mustLoad and !addToList

    // General
    //
    CloseThemeData(*this, "CloseThemeData"),
    DrawThemeParentBackground(*this, "DrawThemeParentBackground"),
    EnableThemeDialogTexture(*this, "EnableThemeDialogTexture"),
    EnableTheming(*this, "EnableTheming"),
    GetCurrentThemeName(*this, "GetCurrentThemeName"),
    GetThemeAppProperties(*this, "GetThemeAppProperties"),
    GetThemeDocumentationProperty(*this, "GetThemeDocumentationProperty"),
    GetWindowTheme(*this, "GetWindowTheme"),
    IsAppThemed(*this, "IsAppThemed"),
    IsThemeActive(*this, "IsThemeActive"),
    IsThemeDialogTextureEnabled(*this, "IsThemeDialogTextureEnabled"),
    OpenThemeData(*this, "OpenThemeData"),
    SetThemeAppProperties(*this, "SetThemeAppProperties"),
    SetWindowTheme(*this, "SetWindowTheme"),

    // Theme sys properties
    //
    GetThemeSysBool(*this, "GetThemeSysBool"),
    GetThemeSysColor(*this, "GetThemeSysColor"),
    GetThemeSysColorBrush(*this, "GetThemeSysColorBrush"),
    GetThemeSysFont(*this, "GetThemeSysFont"),
    GetThemeSysInt(*this, "GetThemeSysInt"),
    GetThemeSysSize(*this, "GetThemeSysSize"),
    GetThemeSysString(*this, "GetThemeSysString"),

    // Theme parts
    //
    DrawThemeBackground(*this, "DrawThemeBackground"),
    DrawThemeEdge(*this, "DrawThemeEdge"),
    DrawThemeIcon(*this, "DrawThemeIcon"),
    DrawThemeText(*this, "DrawThemeText"),
    GetThemeBackgroundContentRect(*this, "GetThemeBackgroundContentRect"),
    GetThemeBackgroundExtent(*this, "GetThemeBackgroundExtent"),
    GetThemeBackgroundRegion(*this, "GetThemeBackgroundRegion"),
    GetThemeBool(*this, "GetThemeBool"),
    GetThemeColor(*this, "GetThemeColor"),
    GetThemeEnumValue(*this, "GetThemeEnumValue"),
    GetThemeFilename(*this, "GetThemeFilename"),
    GetThemeFont(*this, "GetThemeFont"),
    GetThemeInt(*this, "GetThemeInt"),
    GetThemeIntList(*this, "GetThemeIntList"),
    GetThemeMargins(*this, "GetThemeMargins"),
    GetThemeMetric(*this, "GetThemeMetric"),
    GetThemePartSize(*this, "GetThemePartSize"),
    GetThemePosition(*this, "GetThemePosition"),
    GetThemePropertyOrigin(*this, "GetThemePropertyOrigin"),
    GetThemeRect(*this, "GetThemeRect"),
    GetThemeString(*this, "GetThemeString"),
    GetThemeTextExtent(*this, "GetThemeTextExtent"),
    GetThemeTextMetrics(*this, "GetThemeTextMetrics"),
    HitTestThemeBackground(*this, "HitTestThemeBackground"),
    IsThemeBackgroundPartiallyTransparent(*this, "IsThemeBackgroundPartiallyTransparent"),
    IsThemePartDefined(*this, "IsThemePartDefined")
{
  TRACEX(OwlTheme, 1, "Initializing " << TraceId(this));
}

//
/// Singleton accessor
//
TThemeModule& TThemeModule::GetInstance() 
{
  //
  // Note that while this lazy initialization avoids problems with global initialization order,
  // the initial call of this function is not thread-safe. As a work-around, we ensure this 
  // function is called during program start-up (single-thread, safe). 
  // See InitThemeModuleInstance below.
  //
  // The added complexity with the local flag here is needed because the construction of the 
  // singleton may fail and throw exceptions. If that happens we cannot allow construction to be 
  // retried in subsequent (possibly multi-threaded) calls, since construction of function-local
  // static variables is not thread-safe (pre C++11). So we remember that the initial construction 
  // failed and rethrow without trying construction again on subsequent calls.
  // 
  // All of this complexity, including InitThemeModuleInstance, can be removed and replaced by a
  // simple Meyers Singleton (the current try-block) when C++11 compliant compilers are mandated.
  //
  static bool initOk = true; // safe trivial compile-time initialization
  if (!initOk) throw TXTheme(_T("TThemeModule failed initialization"));
  try
  {
    static TThemeModule instance; // initial call (construction) not thread-safe pre-C++11
    return instance;
  }
  catch (...)
  {
    initOk = false;
    throw;
  }
}

namespace
{
  //
  // Ensure singleton initialization at start-up (single-threaded, safe).
  //
  static struct TInitThemeModuleInstance
  {
    TInitThemeModuleInstance()
    {
      try {TThemeModule::GetInstance();} 
      catch (...) {}
    }
  } 
  InitThemeModuleInstance;
}

// 
/// Constructs a theme handler.
//
TTheme::TTheme(HWND w, LPCWSTR cls)
: m_handle(TThemeModule::GetInstance().OpenThemeData(w, cls))
{
  if (m_handle == NULL)
    TXTheme::Raise(_T("Unable to open theme")); // TODO: Load resource string.
}

// 
/// Releases the theme handle.
//
TTheme::~TTheme()
{
  TThemeModule::GetInstance().CloseThemeData(m_handle);
}

//
/// Constructs a theme part handler.
//
TThemePart::TThemePart(HWND w, LPCWSTR cls, int part, int state)
: TTheme(w, cls), m_part (part), m_state(state)
{}

//
/// Paints the background of the themed part.
//
void TThemePart::DrawBackground(HDC dc, const TRect& dest)
{
  TThemeModule& m = TThemeModule::GetInstance();
  HRESULT r = m.DrawThemeBackground(GetHandle(), dc, m_part, m_state, &dest, NULL);
  if (r) TXTheme::Raise(_T("DrawThemeBackground failed"), r);
}

//
/// Paints the background of the themed part using a clip rectangle.
//
void TThemePart::DrawBackground(HDC dc, const TRect& dest, const TRect& clip)
{
  TThemeModule& m = TThemeModule::GetInstance();
  HRESULT r = m.DrawThemeBackground(GetHandle(), dc, m_part, m_state, &dest, &clip);
  if (r) TXTheme::Raise(_T("DrawThemeBackground failed"), r);
}

//
/// Paints the background of the themed part, properly composited with the parent background.
//
void TThemePart::DrawTransparentBackground(HWND parent, HDC dc, const TRect& dest)
{
  TThemeModule& m = TThemeModule::GetInstance();
  if (IsBackgroundPartiallyTransparent())
  {
    // The parent may or may not be themed so we just warn (level 1) if painting fails.
    //
    TRect rect = dest;
    HRESULT r = m.DrawThemeParentBackground(parent, dc, &rect); InUse(r);
    WARNX(OwlTheme, r, 1, _T("DrawThemeParentBackground failed"));
  }
  HRESULT r = m.DrawThemeBackground(GetHandle(), dc, m_part, m_state, &dest, NULL);
  if (r) TXTheme::Raise(_T("DrawThemeBackground failed"), r);
}

//
/// Returns true if the theme-specified background for a part has 
/// transparent pieces or alpha-blended pieces.
//
bool TThemePart::IsBackgroundPartiallyTransparent()
{
  TThemeModule& m = TThemeModule::GetInstance();
  return m.IsThemeBackgroundPartiallyTransparent(GetHandle(), m_part, m_state);
}

//
/// Creates a theme exception.
//
TXTheme::TXTheme(const tstring& name, HRESULT r)
:
  TXOwl(name), result (r)
{
}

//
/// Creates a copy of the exception
//
TXTheme*
TXTheme::Clone() const
{
  return new TXTheme(*this);
}

//
/// Throws the exception.
//
void
TXTheme::Throw()
{
  throw *this;
}

//
/// Throws the exception.
//
void
TXTheme::Raise(const tstring& name, HRESULT r)
{
  TXTheme(name, r).Throw();
}


} // OWL namespace

