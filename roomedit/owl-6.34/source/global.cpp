//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Global data required for every module using OWL
//----------------------------------------------------------------------------

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
# pragma option -w-inl // Disable warning "Functions containing 'statement' is not expanded inline".
#endif

#include <owl/pch.h>

#include <owl/applicat.h>

#if defined(_MSC_VER)
#include <owl/private/defs.h>
#include <owl/module.h>
#endif

namespace owl {
  
OWL_DIAGINFO;
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlMain, true, 0);

class _OWLCLASS TDocTemplate;

//
// Global module pointer representing this executable instance
// provides the default instance for resources contructed within this module
//
extern TModule* Module = 0;   // Global Module ptr in each DLL/EXE

void InitGlobalModule(HINSTANCE hInstance)
{
	if (Module == NULL)
	{
    tchar buffer[_MAX_PATH];
    GetModuleFileName(hInstance, buffer, _MAX_PATH);
    static TModule module(buffer, hInstance);
    Module = &module;
  	TRACEX(OwlMain, 0, "Global Module " << hInstance << ", " << buffer << " created");
	}
}

TModule& GetGlobalModule()
{
	if (Module == NULL)
	{
  	HINSTANCE hInstance = ::GetModuleHandle(NULL);
		InitGlobalModule(hInstance);
	}
	
	return *Module;
}

} // OWL namespace

using namespace owl;

//
// Document templates contructed before doc manager get linked in here
//
owl::TDocTemplate* DocTemplateStaticHead = 0;  // initialize to no templates

//
// This mask deems when certain features are available.
// See GetDocTemplateHead and GetModulePtr below.
//
const uint32 CompatibleVersionMask = 0xFFF00000;

//
// Exported access to global variables for DLLs that use OWL
//
//#if defined(BI_APP_DLL)
#if defined(BI_COMP_BORLANDC)
extern "C" { 
TDocTemplate** PASCAL __declspec(dllexport) GetDocTemplateHead(uint32 version)
{
  return (version&CompatibleVersionMask) == (OWLGetVersion()&CompatibleVersionMask) ? &DocTemplateStaticHead : 0;
}
TModule** PASCAL __declspec(dllexport)
GetModulePtr(uint32 version)
{
  return (version&CompatibleVersionMask) == (OWLGetVersion()&CompatibleVersionMask) ? &owl::Module : 0;
}
}
#elif defined(BI_COMP_GNUC)
extern "C" { 
__declspec(dllexport) TDocTemplate** PASCAL GetDocTemplateHead(uint32 version)
{
  return (version&CompatibleVersionMask) == (OWLGetVersion()&CompatibleVersionMask) ? &DocTemplateStaticHead : 0;
}
__declspec(dllexport) TModule** PASCAL 
GetModulePtr(uint32 version)
{
  return (version&CompatibleVersionMask) == (OWLGetVersion()&CompatibleVersionMask) ? &owl::Module : 0;
}
}
#else

// EMF __export removed because it isn't needed in win32
/*__export*/ STDAPI_(TDocTemplate**) GetDocTemplateHead(uint32 version)
{
  return (version&CompatibleVersionMask) == (OWLGetVersion()&CompatibleVersionMask) ? &DocTemplateStaticHead : 0;
}

/*__export*/ STDAPI_(TModule**) GetModulePtr(uint32 version)
{
  return (version&CompatibleVersionMask) == (OWLGetVersion()&CompatibleVersionMask) ? &owl::Module : 0;
}

#endif

//#endif

/* ========================================================================== */

