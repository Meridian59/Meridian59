//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TXOwl, the base exception class for OWL exceptions
/// that can forward handling to the app module by default.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/window.h>
#include <owl/except.h>
#include <owl/module.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;

//
// Global exception handler used when an application object is not available.
// May be overriden by user code by redefining this function. Note that the
// program must be linked statically to OWL in order for an overridden version
// of this function to be called by the framework. If a valid
// application object is found by GetApplicationObject, then the virtual
// TModule::Error(TXOwl& x, char* caption, bool canResume) is usually used
// instead.
//
_OWLFUNC(int)
HandleGlobalException(TXBase& x, LPCTSTR caption, LPCTSTR canResume)
{
#if BI_MSG_LANGUAGE == 0x0411
  static const char defCaption[] = "–¢ˆ—‚Ì—áŠO";
  static const char defError[] = "–?’m‚I—aSO";
#else
  static const tchar defCaption[] = _T("Unhandled Exception");
  static const tchar defError[] = _T("Unknown Exception");
#endif
  
  tstring msg = x.why();
  if (msg.length() == 0) 
    msg = defError;
  
  if (canResume)
  {
    msg += _T("\n\n");
    msg += canResume;
  }

  const int flags = MB_ICONSTOP | MB_TOPMOST | MB_TASKMODAL | (canResume ? MB_YESNO : MB_OK);
  const int r = ::MessageBox(0, msg.c_str(), caption ? caption : defCaption, flags);
  return (r == IDYES) ? 0 : -1;
}

//----------------------------------------------------------------------------

/// \class TXOwl
/// TXOwl is a parent class for several classes designed to describe exceptions. In
/// most cases, you derive a new class from TXOwl instead of using this one
/// directly. The ObjectWindows classes derived from TXOwl include
/// - \c \b  TXCompatibility	Describes an exception that occurs if  TModule::Status is not
/// zero
/// - \c \b  TXValidator	Describes an exception that occurs if there is an invalid validator
/// expression
/// - \c \b  TXWindow	Describes an exception that results from trying to create an invalid
/// window
/// - \c \b  TXGdi	Describes an exception that results from creating an invalid GDI object
/// - \c \b  TXInvalidMainWindow	Describes an exception that results from creating an invalid
/// main window
/// - \c \b  TXMenu	Describes an exception that occurs when a menu object is invalid
/// - \c \b  TXInvalidModule	Describes an exception that occurs if a TModule object is
/// invalid
/// - \c \b  TXPrinter	Describes an exception that occurs if a printer device context is
/// invalid
/// - \c \b  TXOutOfMemory	Describes an exception that occurs if an out of memory error
/// occurs
/// 
/// Each of the exception classes describes a particular type of exception. When
/// your program encounters a given situation that's likely to produce this
/// exception, it passes control to the specified exception-handling object. If you
/// use exceptions in your code, you can avoid having to scatter error-handling
/// procedures throughout your program.
/// 
/// To create an exception handler, place the keyword try before the block of code
/// that might produce the abnormal condition (the code that might generate an
/// exception object) and the keyword catch before the block of code that follows
/// the try block. If an exception is thrown within the try block, the classes
/// within each of the subsequent catch clauses are checked in sequence. The first
/// one that matches the class of the exception object is executed.
/// 
/// The following example from MDIFILE.CPP, a sample program on BC5.0x distribution
/// disk, shows how to set up a try/catch block around the code that might throw an
/// exception.
/// \code
/// void TMDIFileApp::CmRestoreState()
/// { 
///    char* errorMsg = 0;
///    ifpstream is(DskFile);
///    if (is.bad())
///       errorMsg = "Unable to open desktop file.";
/// // try block of code //
///    else { 
///    if (Client->CloseChildrenParen) { 
///       try { 
///          is >>* this;
///          if (is.bad())
///             errorMsg = "Error reading desktop file.";
///          else
///              Client->CreateChildren();
///      }
/// // catch block of code //
///    catch (xalloc) { 
///       Client->CloseChildren();
///       errorMsg = "Not enough memory to open file.";
///     }
///    }
///  }
///  if (errorMsg)
///    MainWindow->MessageBox(errorMsg, "Error",
///       MB_OK | MB_ICONEXCLAMATION);
/// }
/// \endcode


//
/// An OWL exception with a given message for displaying and an unsigned Id
/// that can be used for identification or loading a string
//
TXOwl::TXOwl(const tstring& msg, uint resId)
:
  TXBase(msg),
  ResId(resId)
{
}

//
/// An OWL exception with a given unsigned Id that can is used for loading a
/// message string & identification
//
/// Loads the string resource identified by the resId parameter and uses this resId
/// to initialize the TXBase object.
//
TXOwl::TXOwl(uint resId, TModule* module)
:
  TXBase(ResourceIdToString(0, resId, module)),
  ResId(resId)
{
}

//
/// Destroys a TXOwl object.
//
TXOwl::~TXOwl() throw()
{
}

//
/// Called when an unhandled exception is caught at the main message loop level.
//
int
TXOwl::Unhandled(TModule* app, uint promptResId)
{
  return app->Error(*this, IDS_OWLEXCEPTION, promptResId);
}

TXOwl*
TXOwl::Clone() const
{
	return new TXOwl(*this);
}

//
/// Throws the exception object. Throw must be implemented in any class derived from
/// TXOwl.
//
void
TXOwl::Throw()
{
  throw *this;
}

//
// Construct a TXOwl exception from scratch, and throw it. Two versions
// corresponding to the two constructor signatures
//
void
TXOwl::Raise(const tstring& msg, uint resId)
{
  TXOwl(msg, resId).Throw();
}

//
//
//
void
TXOwl::Raise(uint resId, TModule* module)
{
  TXOwl(resId, module).Throw();
}

//
/// Static member function used to convert a resource id to a 'string'. This
/// is necessary since we must pass a string to the xmsg base class
/// constructor.  Sets found to true if the resource was located, otherwise
/// false.  In either case, the string is initialized to something
/// printable.
/// If the string message cannot be loaded, returns a "not found" message.
//
tstring
TXOwl::ResourceIdToString(bool* found, uint resId, TModule* module)
{
  tchar buf[128];

  bool status = module && module->LoadString(resId, buf, COUNTOF(buf));
  if (found)
    *found = status;

  if (!status)
#if BI_MSG_LANGUAGE == 0x0411
    sprintf(buf, "—áŠO #%u (Ò¯¾°¼Þ‚ª—pˆÓ‚³‚ê‚Ä‚¢‚Ü‚¹‚ñ. <owl/except.rc> ‚ÌÊÞ²ÝÄÞ‚ðŠm”F‚µ‚Ä‚­‚¾‚³‚¢).", resId);
#else
    _stprintf(buf, _T("Exception #%u (Could not load description string; <owl/except.rc> not bound?)."), resId);
#endif

  tstring rscStr(buf);
  return rscStr;
}

//
/// Extension to string loader adds the feature of sprintf'ing an
/// additional information string into the resource message string.
//
tstring
TXOwl::MakeMessage(uint resId, LPCTSTR infoStr, TModule* module)
{
  tstring rscMsg = ResourceIdToString(0, resId, module);
  tchar buf[255];
  _stprintf(buf, rscMsg.c_str(), infoStr);
  return tstring(buf);
}

tstring
TXOwl::MakeMessage(uint resId, const tstring& infoStr, TModule* module)
{
  return MakeMessage(resId, infoStr.c_str(), module);
}

//
/// This extension to the string loader adds the feature of sprintf'ing an
/// additional information string into the resource message string.
//
tstring
TXOwl::MakeMessage(uint resId, uint infoNum, TModule* module)
{
  tstring rscMsg = ResourceIdToString(0, resId, module);
  tchar buf[255];
  _stprintf(buf, rscMsg.c_str(), infoNum);
  return tstring(buf);
}
#if defined(UNICODE)
tstring 
TXOwl::MakeMessage(uint resId, LPCSTR infoStr, TModule* module)
{
  _USES_CONVERSION;
  tstring rscMsg = ResourceIdToString(0, resId, module);
  tchar buf[255];
  _stprintf(buf, rscMsg.c_str(), _A2W(infoStr));
  return tstring(buf);
}
#endif
//----------------------------------------------------------------------------

//
/// Constructs a TXOutOfMemory object.
//
TXOutOfMemory::TXOutOfMemory()
:
  TXOwl(IDS_OUTOFMEMORY)
{
}

TXOutOfMemory*
TXOutOfMemory::Clone() const
{
	return new TXOutOfMemory(*this);
}

//
/// Throws the exception object. Throw must be implemented in any class derived from
/// TXOwl.
//
void
TXOutOfMemory::Throw()
{
  throw *this;
}

//
/// Construct a TXOutOfMemory exception from scratch, and throw it
//
void
TXOutOfMemory::Raise()
{
  TXOutOfMemory().Throw();
}
//
//
//
TXNotSupportedCall::TXNotSupportedCall()
:
  TXOwl(IDS_NOTSUPPORTEDCALL)
{
}

TXNotSupportedCall* 
TXNotSupportedCall::Clone() const
{
	return new TXNotSupportedCall(*this);
}

//
void TXNotSupportedCall::Throw()
{
  throw *this;
}
//
void TXNotSupportedCall::Raise()
{
  TXOutOfMemory().Throw();
}


///
} // OWL namespace
/* ========================================================================== */

