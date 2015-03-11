//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TModule. TModule defines the base behavior for
/// OWL libraries and applications.
//----------------------------------------------------------------------------

#if defined(__BORLANDC__)
# pragma option -w-inl // Disable warning "Functions containing 'statement' is not expanded inline".
#endif

#include <owl/pch.h>

#include <owl/defs.h>
#include <owl/module.h>
#include <owl/appdict.h>
#include <owl/applicat.h>
#include <owl/pointer.h>
#include <stdio.h>
#include <winnls.h>
#include <owl/template.h>

#if defined(BI_MULTI_THREAD_RTL)
#include <owl/thread.h>
#endif

#include <owl/window.h>
#include <owl/framewin.h>

#if defined(BI_COMP_MSC)
#include <new.h>
#endif

#if OWL_STACKWALKER
# include "stackwalker.h"
#endif

long TlsAddRefs();
long TlsRelease();

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlApp);        // General Application diagnostic group

//
// Global message id for GetWindowPtr call defined in owl.cpp
//
extern _OWLDATA(uint) GetWindowPtrMsgId;

} // OWL namespace

using namespace std;

namespace owl {

//
// TSystemMessage
// ~~~~~~~~~~~~~~
//
TSystemMessage::TSystemMessage()
{
  Error = ::GetLastError();
  Init(LangNeutral);
}

//
//
//
TSystemMessage::TSystemMessage(uint32 error, TLangId langId)
{
  Error = error;
  ::SetLastError(Error);
  Init(langId);
}

//
//
//
int TSystemMessage::MessageBox(TWindow* win, const tstring& msg, const tstring& title, uint flags)
{
  flags |= MB_SYSTEMMODAL;
  
  tstring message = msg;
  message += Message;
  TApplication* appl;
  if(!win && (appl = OWLGetAppDictionary().GetApplication(0))!=0 && appl->GetMainWindow())
    win = appl->GetMainWindow();
  if(win)
    return win->MessageBox(message.c_str(), title, flags);
  return ::MessageBox(0, message.c_str(), title.c_str(), flags);
}

//
//
//
void TSystemMessage::Init(TLangId langId)
{
  LPVOID ptr = 0;
  if(::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, Error, langId, (LPTSTR)&ptr, 0, NULL) != 0)
  {
    Message = (LPTSTR)ptr;
    LocalFree(ptr); // free the memory allocated by FormatMessage
  }
  else{
    tchar buffer[MAX_PATH];
    wsprintf(buffer, _T("Error: %#X occurred (no further info)"), Error);
    Message = buffer;
  }
}
//----------------------------------------------------------------------------

//} // OWL namespace

// Implement
//////////////////////////////////////////////////////////////////////////////////////////////////
// multithread support
/// \cond
class __TModuleList {
  public:
    static TModule*  Next(TModule* module);
    static bool      Find(TModule* module);
    static void      Add(TModule* module);
    static void      Remove(TModule* module);
    static TModule*  FindResModule(TResId id, TResId type);

    static TPtrArray<TModule*>* ModuleArray;

#if defined(BI_MULTI_THREAD_RTL)
    static TMRSWSection*  Lock;
#endif

};
/// \endcond

TPtrArray<TModule*>* __TModuleList::ModuleArray = 0;
#if defined(BI_MULTI_THREAD_RTL)
TMRSWSection*  __TModuleList::Lock = 0;
#endif

#if defined(BI_MULTI_THREAD_RTL)
#define LOCKLIST(s) TMRSWSection::TLock __lock(*__TModuleList::Lock, s)
#else
#define LOCKLIST(s)
#endif
//
TModule* __TModuleList::Next(TModule* module)
{
  if(!ModuleArray)
    return 0;
  LOCKLIST(true);
  if(!module){
    return (*ModuleArray)[0];
  }
  else{
    for(uint i = 0; i < ModuleArray->Size(); i++){
      if(module == (*ModuleArray)[i])
        return (i+1 < ModuleArray->Size()) ? (*ModuleArray)[i+1] : 0;
    }
  }
  return 0;
}
//
void __TModuleList::Add(TModule* module)
{
  if(Find(module))
    return;

  if(!ModuleArray)
  {
#if defined(BI_MULTI_THREAD_RTL)
    Lock = new TMRSWSection;
#endif
    ModuleArray = new TPtrArray<TModule*>;
  }
  {
    LOCKLIST(false);

    // If there is another object, but with the same hInstance, replace it.
    // This usualy happens when the global module is created and added to the list,
    // and afterwards the TApplication-derived object with the same hInstance is created
    for (uint i = 0; i < ModuleArray->Size(); i++)
    {
      if (module->GetHandle() == (*ModuleArray)[i]->GetHandle())
      {
        (*ModuleArray)[i] = module;
        return;
      }
    }


    ModuleArray->AddAt(module,0);
  }
}
//
void __TModuleList::Remove(TModule* module)
{
  if(!Find(module))
    return;

  PRECONDITION(ModuleArray);
  {
    LOCKLIST(false);
    ModuleArray->DetachItem(module);
    if(ModuleArray->Empty())
	{
      delete ModuleArray;
      ModuleArray = 0;
    }
  }
#if defined(BI_MULTI_THREAD_RTL)
  if(!ModuleArray){
    delete Lock;
    Lock = 0;
  }
#endif
}
//
bool __TModuleList::Find(TModule* module)
{
  if(!ModuleArray)
    return false;
  LOCKLIST(true);
  
  for (uint i = 0; i < ModuleArray->Size(); i++){
    if (module == (*ModuleArray)[i])
      return true;
  }
  return false;
}
//
TModule* __TModuleList::FindResModule(TResId id, TResId type)
{
  if(!ModuleArray)
    return 0;

  LOCKLIST(true);

  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  TLangId langId = appl ? appl->GetLangId() : LangNeutral;
  TPtrArray<TModule*>::Iterator Iter1(*ModuleArray);
  while(Iter1){
    if((*Iter1)->FindResourceEx(id, type, langId))
      return *Iter1;
    Iter1++;
  }

  TPtrArray<TModule*>::Iterator Iter(*ModuleArray);
  while(Iter){
    if((*Iter)->FindResource(id, type))
      return *Iter;
    Iter++;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////
//
TModule* TModule::NextModule(TModule* module)
{
  return __TModuleList::Next(module);
}

//namespace owl {

//
// Implementation of Constructors for a TModule object
//

//
/// Constructs a TModule object that is used as an alias for a DLL. If shouldLoad is
/// true, TModule will automatically load and free the DLL. If shouldLoad is false,
/// then the HInstance needs to be set later using InitModule.
///
/// "mustLoad" determines if a load failure should cause an exception throw
//
TModule::TModule(const tstring& name, bool shouldLoad, bool mustLoad, bool addToList)
{
  if (shouldLoad) {
    TErrorMode loadMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    Handle = ::LoadLibrary(name.c_str());
    if (Handle <= HINSTANCE(HINSTANCE_ERROR) && mustLoad) {
      TRACEX(OwlApp, 0, _T("Unable to load DLL '") << name.c_str() << _T('\''));
      TXInvalidModule::Raise(name);  // !CQ use a better exception. This is for windows
    }
  }
  else {
    Handle = 0;
  }

  ShouldFree = shouldLoad;
  SetName(name);

  //
  // Add Module  into global Array
  //
  if(addToList)
    __TModuleList::Add(this);
}

//
/// Constructs a TModule object that is an alias for an already loaded DLL or
/// program with an available HInstance. When the TModule is destructed, the
/// instance isn't automatically freed. name, which is optional, can be 0.
/// No cmdLine is set up
//
TModule::TModule(const tstring& name, HINSTANCE hInstance, bool addToList)
{
  PRECONDITION(hInstance > HINSTANCE(HINSTANCE_ERROR));
  Handle = hInstance;
  ShouldFree = false;
  SetName(name);

  //
  // Add Module  into global Array
  //
  if(addToList)
    __TModuleList::Add(this);
}

//
/// Constructs a TModule object for an ObjectWindows DLL or program from within
/// LibMain or WinMain. Calls InitModule to initialize hInstance and cmdLine.
//
TModule::TModule(const tstring& name, HINSTANCE hInstance, const tstring& cmdLine, bool addToList)
{
  Handle = 0;
  ShouldFree = false;
  SetName(name);
  if (hInstance)
    InitModule(hInstance, cmdLine);

  //
  // Add Module  into global Array
  //
  if(addToList)
    __TModuleList::Add(this);
}

//
/// Destructs a TModule, freeing the instance if appropriate, and deleting
/// new'd strings
//
TModule::~TModule()
{
  if (ShouldFree && Handle > HINSTANCE(HINSTANCE_ERROR))
    ::FreeLibrary(Handle);

  //
  // Detach Module from global ModuleArray
  //
  __TModuleList::Remove(this);
}

//
/// Global  search for  resources
//
TModule*
TModule::FindResModule(TResId id, TResId type)
{
  return __TModuleList::FindResModule(id, type);
}

//
/// Accessor function that sets the name of the module.
//
void
TModule::SetName(const tstring& name)
{
  Name = (name.length() == 0 && Handle > HINSTANCE(HINSTANCE_ERROR)) ?
    GetModuleFileName() : name;
}

//
/// Performs any instance initialization necessary for the module. If the module
/// cannot be created, a TXInvalidModule exception is thrown.
///
/// Copies cmd line, and gets proc
/// instance handles for the standard procs.
//

#if !defined(BI_COMP_GNUC)
#pragma warn -par
#endif

void
TModule::InitModule(THandle hInstance, const tstring& cmdLine)
{
  SetHandle(hInstance);

#if defined(BI_COMP_GNUC)
  // Borland removes the first element of argv (program name) from
  // owl::CmdLine somehow ("the RTL takes care of this" above), but
  // g++ does not. Modifying the cmdLine argument won't have any effect
  // because it's const and is passed as string::c_str().
  char const* cmdLine = TApplication::GetCmdLine().c_str();
  if(cmdLine){
    while (*cmdLine && *cmdLine++ != _T(' '))
      ;
    TApplication::GetCmdLine() = cmdLine;
  }
#endif

  // Register a system-wide "GetWindowPtr" message as GetWindowPtr(hInstance)
  // Each running copy of ObjectWindows will get a unique message Id
  // instance of OWL library. !!!
  if (!GetWindowPtrMsgId) {
    const tchar msgTemplate[] = _T("GetWindowPtr(%X)");
    tchar msgName[COUNTOF(msgTemplate) + 8];
    wsprintf(msgName, msgTemplate, unsigned(hInstance));
    GetWindowPtrMsgId = ::RegisterWindowMessage(msgName);
    CHECK(GetWindowPtrMsgId);
  }
}

#if !defined(BI_COMP_GNUC)
#pragma warn .par
#endif

//
/// Replaceable exception handler; may be redefined to process OWL exceptions
/// if canResume is false, then the user doesn't have the option of ignoring
//
/// Called when fatal exceptions occur, Error takes an xmsg exception
/// object, a resource ID for a message box caption, and an optional resource ID for
/// a user prompt. By default, Error calls HandleGlobalException with the xmsg
/// object and the strings obtained from the resources. An application (derived from
/// TApplication which is derived from TModule) can reimplement this function to
/// provide alternative behavior.
/// A nonzero status code is returned to indicate that an error condition is to be
/// propagated; a zero status indicates that the condition has been handled and that
/// it is OK to proceed. ObjectWindows uses this status code inside its message loop
/// to allow the program to resume. The global error handler (defined in except.h),
/// which displays the message text, is
/// \code
/// int _OWLFUNC HandleGlobalException(xmsg& x, char* caption, char* canResume);
/// \endcode
int
TModule::Error(TXBase& x, unsigned captionResId, unsigned promptResId)
{
  tchar cbuf[80];
  tchar pbuf[80];

  if (!captionResId)
    captionResId = IDS_UNHANDLEDXMSG;
  return HandleGlobalException(x,
    LoadString(captionResId, cbuf, 80) ? cbuf : 0,
    promptResId ?
      (LoadString(promptResId, pbuf, 80) ? pbuf : _T("OK to Resume?"))
      : 0);
}

//
/// For use with CopyText.
//
struct TGetModuleFileName
{
  const TModule& module;
  TGetModuleFileName(const TModule& m) : module(m) {}

  int operator()(LPTSTR buf, int buf_size) const
  {
    int n = module.GetModuleFileName(buf, buf_size);
    WARN(n == buf_size, _T("TModule::GetModuleFileName truncated the result."));
    return n;
  }
};

//
/// String-aware overload
//
tstring TModule::GetModuleFileName() const
{
  return CopyText(_MAX_PATH, TGetModuleFileName(*this));
}

//
/// Set the instance handle for a module that does not yet have one. Cannot
/// be called on a module that already has an instance handle.
//
void
TModule::SetHandle(HINSTANCE hInstance)
{
  PRECONDITION(!ShouldFree && !Handle);
  Handle = hInstance;
}

//
// LoadString replacements which do not generated debug warning output
//
 typedef WCHAR* TResText;
 typedef WCHAR* TResCount;

//
// Wrapper for loading a string resource.
//
/// Loads a string resource identified by id into the buffer pointed to by
/// buff. maxChars indicates the size of the buffer to which the zero-terminated
/// string is copied. A string longer than the length specified in maxChars is
/// truncated. The return value is the number of characters copied into the buffer,
/// or 0 if the string resource does not exist.
int
TModule::LoadString(uint id, LPTSTR buf, int bufSize) const
{
#ifdef MAINWIN
  int resource = ::LoadString( GetInstance(), id, buf, bufSize);
  if (resource)
    return resource;
#else /// MAINWIN
  uint len = 0;
  HRSRC     resHdl;
  HGLOBAL   glbHdl;
  TResText  resData;
  
  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  resHdl = FindResourceEx(id/16+1, RT_STRING, appl ? appl->GetLangId() : LangNeutral);
  if(!resHdl)
    resHdl = FindResource(id/16+1, RT_STRING);
  if(resHdl != 0){
    if((glbHdl = LoadResource(resHdl)) != 0) {
      if ((resData = (TResText)LockResource(glbHdl)) != 0) {
        // get pointer to our string
        int cnt;
        for (cnt = id % 16; len = *(TResCount)resData++, cnt--; resData += len)
          ;
        if (len != 0) {
#  if defined(UNICODE)
          if (len >= uint(bufSize))
            len = bufSize-1;
          wmemset(buf, 0, bufSize);
          ::_tcsncpy(buf, resData, len);
          //::_tcscpy(buf, resData); // Ma, 09.10.2003 string too long
#  else
          len = ::WideCharToMultiByte(CP_ACP, 0, resData, len, buf, bufSize, 0, 0);
          if (len > 0)
            buf[len] = 0;
#  endif
        }
      }
      FreeResource(glbHdl);
      if (len)
        return len;
    }
  }
  else{
    // if not found look in Module List
    TModule* module = TModule::FindResModule(id/16+1, RT_STRING);
    if (module)
      return module->LoadString(id, buf, bufSize);
  }
#endif //else not MAINWIN
  if (bufSize)
    *buf = 0;  // make empty string just in case caller doesn't check return
  return 0;    // indicate string not found
}

//
// Wrapper for loading a string resource.
//
/// Loads a string resource identified by id
//
#ifdef UNIX
tstring
TModule::LoadString(uint id) const
{

// Added by Val Ovechkin 7:18 PM 6/11/98
  char str[1024] = "<empty>";
  int res = ::LoadString( GetInstance(), id, str, sizeof str );
  if( !res ) str[0] = 0;
  return tstring( str );


  uint len = 0;
  HRSRC     resHdl;
  HGLOBAL   glbHdl;
  TResText  resData;
  tstring    retString;

  if ((resHdl = FindResource(id/16+1, RT_STRING)) != 0
   && (glbHdl = LoadResource(resHdl)) != 0) {
    if ((resData = (TResText)LockResource(glbHdl)) != 0) {
      int cnt;
      for (cnt = id % 16; len = *(TResCount)resData++, cnt--; resData += len)
        ;
      if (len != 0) {
#if (0) // This is dangerous unless string is changed to handle non-terminated
        // char arrays
        //
        retString.append(resData, 0, len);
#else
        int n = ::WideCharToMultiByte(CP_ACP, 0, resData, len, 0, 0, 0, 0);
        TAPointer<char> buf = new char[n+1];
        len = ::WideCharToMultiByte(CP_ACP, 0, resData, len, buf, n+1, 0, 0);
  #if   0
        retString.resize(len+1);
        uint i;
        for (i = 0; i < len; i++)
          retString[i] = (char)buf[i];
        retString[i] = 0;
  #else
        // The string object does not need the terminating null of C-strings
        //
        retString.resize(len);
        uint i;
        for (i = 0; i < len; i++)
          retString[i] = (char)buf[i];
  #endif
#endif
      }
    }
    FreeResource(glbHdl);
    if (len)
      return retString;
  }

  if (&GetGlobalModule() != this)                   // look in OWL module if different
    return GetGlobalModule().LoadString(id);

  return retString;    // empty if string not found
}

#else

/// Loads a string resource identified by id
tstring
TModule::LoadString(uint id) const
{
  int len    = 0;
  HRSRC     resHdl;
  HGLOBAL   glbHdl;
  TResText  resData;
   tstring retString;

  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  resHdl = FindResourceEx(id/16+1, RT_STRING, appl ? appl->GetLangId() : LangNeutral);
  if(!resHdl)
    resHdl = FindResource(id/16+1, RT_STRING);
  if (resHdl != 0){
    if((glbHdl = LoadResource(resHdl)) != 0){
      if ((resData = (TResText)LockResource(glbHdl)) != 0) {
        int cnt;
        for (cnt = id % 16; len = *(TResCount)resData++, cnt--; resData += len)
          ;
        if (len != 0) {
#if (0)   // This is dangerous unless string is changed to handle non-terminated
          // char arrays
          retString.append(resData, 0, len);
#else
#    if defined(UNICODE)
          // The string object does not need the terminating null of C-strings
          retString.resize(len); // Ma, 09.10.2003 string is not terminated!

          for (int i = 0; i < len; i++)
            retString[i] = resData[i];
#    else
          int n = ::WideCharToMultiByte(CP_ACP, 0, resData, len, 0, 0, 0, 0);
          TAPointer<tchar> __ClnObj(new tchar[n+1]);
          tchar* buf = __ClnObj;
          len = ::WideCharToMultiByte(CP_ACP, 0, resData, len, buf, n+1, 0, 0);
          // The string object does not need the terminating null of C-strings
          retString.resize(len);
          int i;
          for (i = 0; i < len; i++)
            retString[i] = buf[i];
#    endif
#endif
        }
        UnlockResource(glbHdl);
      }
      if (len)
        return retString;
    }
  }
  else{
    // if not found look in Module List
    TModule* module = TModule::FindResModule(id/16+1, RT_STRING);
    if (module)
      return module->LoadString(id);
  }

  return retString;    // empty if string not found
}
#endif



//
/// Wrapper for the Windows API.
//
/// Loads the bitmap resource specified by id. If the bitmap cannot be found,
/// LoadBitmap returns 0.
//
HBITMAP TModule::LoadBitmap(TResId id) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
#if 0
  HRSRC    resHdl;
  HGLOBAL glbHdl;
  HBITMAP hBitmap=0;

  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  resHdl = FindResourceEx(id, RT_BITMAP, appl ? appl->GetLangId() : LangNeutral);
  
  if(resHdl){
    if((glbHdl = LoadResource(resHdl)) !=0)
      hBitmap = (HBITMAP)LockResource(glbHdl);
  }
#else
  HBITMAP hBitmap = ::LoadBitmap(Handle, id);
#endif
  WARNX(OwlApp, !hBitmap, 0, _T("Bitmap resource not found Id:") << id);

  if(!hBitmap){// if not found look in Module List
     TModule* module = TModule::FindResModule(id, RT_BITMAP);
     if(module)
       return module->LoadBitmap(id);
  }

  CHECKX(hBitmap,_T("Bitmap resource not found even in module list."));
  return hBitmap;
}
//
/// Wrapper for the Windows API.
//
/// Loads the accelerator table resource specified by id. LoadAccelerators loads the
/// table only if it has not been previously loaded. If the table has already been
/// loaded, LoadAccelerators returns a handle to the loaded table.
//
HACCEL TModule::LoadAccelerators(TResId id) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));

#if 0  
  HRSRC resHdl;
  HGLOBAL glbHdl;

  HACCEL hAccel = 0;

  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  resHdl = FindResourceEx(id, RT_ACCELERATOR, appl ? appl->GetLangId() : LangNeutral);
  if(!resHdl)
    resHdl = FindResource(id, RT_ACCELERATOR);
  if(resHdl){
    if((glbHdl = LoadResource(resHdl)) !=0)
      hAccel = (HACCEL)LockResource(glbHdl);
  }
#else  
  HACCEL hAccel = ::LoadAccelerators(Handle, id);
#endif  
  WARNX(OwlApp,!hAccel,0,_T("Accelerator resource not found") << id);

  if(!hAccel){ // if not found look in Module List
    TModule* module = TModule::FindResModule(id, RT_ACCELERATOR);
     if(module)
       return module->LoadAccelerators(id);
  }

  WARNX(OwlApp,!hAccel,0,_T("Accelerator resource not found even in module list") << id);
  //CHECKX(hAccel,_T("Accelerator resource not found even in module list."));

  return hAccel;
}
//----------------------------------------------------------------------------

//
/// Wrapper for the Windows API.
//
/// Loads the menu resource indicated by id into memory. If the menu resource cannot
/// be found, LoadMenu returns 0.
//
HMENU TModule::LoadMenu(TResId id) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
#if 0
  HRSRC resHdl;
  HGLOBAL glbHdl;
   HMENU hMenu = 0;
  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  resHdl = FindResourceEx(id, RT_MENU, appl ? appl->GetLangId() : LangNeutral);
  if(!resHdl)
    resHdl = FindResource(id, RT_MENU);
  if(resHdl){
    if((glbHdl = LoadResource(resHdl)) !=0)
      hMenu = (HMENU)LockResource(glbHdl);
  }
#else  
  HMENU hMenu = ::LoadMenu(Handle, id);
#endif
  WARNX(OwlApp,!hMenu,0,_T("Menu resource not found") << id);

  if (!hMenu){ // if not found look in Module List
    TModule* module = TModule::FindResModule(id, RT_MENU);
     if(module)
       return module->LoadMenu(id);
  }

  CHECKX(hMenu,_T("Menu resource not found even in module list."));
  return hMenu;
}
//----------------------------------------------------------------------------

//
/// Wrapper for the Windows API.
//
/// Loads the cursor resource specified by id into memory and returns a handle to
/// the cursor resource. If the cursor resource cannot be found or identifies a
/// resource that is not a cursor, LoadCursor returns 0.
//
HCURSOR TModule::LoadCursor(TResId id) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
#if 0
  HRSRC resHdl;
  HGLOBAL glbHdl;
   HCURSOR hCursor = 0;

  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  resHdl = FindResourceEx(id, RT_CURSOR, appl ? appl->GetLangId() : LangNeutral);
  
  if(resHdl){
    if((glbHdl = LoadResource(resHdl)) !=0)
      hCursor = (HCURSOR)LockResource(glbHdl);
  }
#else
  HCURSOR hCursor = ::LoadCursor(Handle, id);
#endif
  WARNX(OwlApp,!hCursor,0,_T("Cursor resource not found") << id);

  if (!hCursor){ // if not found look in Module List
    TModule* module = TModule::FindResModule(id, RT_CURSOR);
     if(module)
       return module->LoadCursor(id);
  }

  CHECKX(hCursor,_T("Cursor resource not found even in module list."));
  return hCursor;
}

//----------------------------------------------------------------------------
//
/// Wrapper for the Windows API.
//
/// Loads the icon resource indicated by the parameter, name, into memory. LoadIcon
/// loads the icon only if it has not been previously loaded. If the icon resource
/// cannot be found, LoadIcon returns 0.
/// 
/// LoadIcon can be used to load a predefined Windows icon if name points to one of
/// the Windows IDI_XXXX values.
//
HICON TModule::LoadIcon(TResId name) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
#if 0
  HRSRC resHdl;
  HGLOBAL glbHdl;
   HICON hIcon = 0;

  TApplication* appl = OWLGetAppDictionary().GetApplication(0);
  resHdl = FindResourceEx(TResId(name), RT_ICON, appl ? appl->GetLangId() : LangNeutral);

  if(resHdl){
    if((glbHdl = LoadResource(resHdl)) !=0)
      hIcon = (HICON)LockResource(glbHdl);
  }
#else
  HICON hIcon = ::LoadIcon(Handle, name);
#endif
  WARNX(OwlApp,!hIcon,0,_T("Icon resource not found") << name);
  
  if (!hIcon){ // if not found look in Module List
    TModule* module = TModule::FindResModule(name, RT_ICON);
     if(module)
       return module->LoadIcon(name);
  }

  CHECKX(hIcon,_T("Icon resource not found even in module list"));
  return hIcon;
}

//----------------------------------------------------------------------------
//
/// Loads the given HTML resource and returns it as a narrow string.
/// It is left to the client to detect the character set and encoding of the contents.
/// If the resource is not found in this module, the module list is tried.
/// If that also fails, an assertion exception is thrown in diagnostics mode.
/// In non-diagnostics mode, an empty string is returned on failure.
//
string TModule::LoadHtml(TResId id) const
{
  PRECONDITION(Handle > HINSTANCE(HINSTANCE_ERROR));
  HRSRC r = FindResourceEx(id, RT_HTML);
  WARNX(OwlApp, !r, 0, _T("HTML resource not found: ") << id);
  if (!r)
  { 
    // The resource was not found in this module, so try the module list.
    //
    TModule* module = FindResModule(id, RT_HTML);
    CHECKX(module, _T("HTML resource not found even in the module list."));
    return module ? module->LoadHtml(id) : string();
  }
  HGLOBAL g = LoadResource(r); CHECK(g);
  const char* p = static_cast<const char*>(LockResource(g)); // Note: No unlocking needed (see API doc).
  return string(p, p + SizeofResource(r)); 
}

//----------------------------------------------------------------------------
// Module entry dynamic binding base class

//
/// Constructs a Module entry object given a narrow Windows resource identifier.
/// The resource identifier encodes either a function name or a function ordinal
/// for one of the exported functions in the module.
//
TModuleProc::TModuleProc(const TModule& module, TNarrowResId id)
{
  Proc = module.GetProcAddress(id);
  if (!Proc) // Handle failure.
  {
    tstring msg = id.IsInt() ?
      TXOwl::MakeMessage(IDS_INVALIDMODULEORD, id.GetInt()) :
      TXOwl::MakeMessage(IDS_INVALIDMODULEFCN, id.GetString());
    msg += module.GetName();  // Just tack on the module name.
    TXOwl::Raise(msg);
  }
}

//----------------------------------------------------------------------------
// Wrappers for Windows API
//


  static const tchar userStr[] = _T("USER32");
# if defined(UNICODE)
    static const char LoadIconStr[] = "LoadIconW";
    static const char GetClassInfoStr[] = "GetClassInfoW";
    static const char GetMenuStringStr[]= "GetMenuStringW";
# else
    static const char LoadIconStr[] = "LoadIconA";
    static const char GetClassInfoStr[] = "GetClassInfoA";
    static const char GetMenuStringStr[]= "GetMenuStringA";
# endif

static const char GetMenuStateStr[] = "GetMenuState";
static const char DestroyIconStr[] = "DestroyIcon";

//
// Returns TModule object wrapping the handle of the USER module
//
TModule&
TUser::GetModule()
{
  static TModule userModule(userStr, ::GetModuleHandle(userStr),false);
  return userModule;
}

//
// Invokes 'LoadIcon' indirectly
//
HICON
TUser::LoadIcon(HINSTANCE p1, LPCTSTR p2)
{
  static TModuleProc2<HICON, HINSTANCE, LPCTSTR>
         loadIcon(GetModule(), LoadIconStr);
  return loadIcon(p1, p2);
}

//
// Invokes 'DestroyIcon' indirectly
//
BOOL
TUser::DestroyIcon(HICON p1)
{
  static TModuleProc1<BOOL, HICON> destroyIcon(GetModule(), DestroyIconStr);
  return destroyIcon(p1);
}

//
// Invokes 'GetClassInfo' indirectly
//
BOOL
TUser::GetClassInfo(HINSTANCE p1, LPCTSTR p2, LPWNDCLASS p3)
{
  static TModuleProc3<BOOL, HINSTANCE, LPCTSTR, LPWNDCLASS>
         getClassInfo(GetModule(), GetClassInfoStr);
  return getClassInfo(p1, p2, p3);
}

//
// Invokes 'GetMenuString' indirectly
//
int
TUser::GetMenuString(HMENU p1, UINT p2, LPTSTR p3, int p4, UINT p5)
{
  static TModuleProc5<int, HMENU, uint, LPTSTR, int, int>
         getMenuString(GetModule(), GetMenuStringStr);
  return getMenuString(p1, p2, p3, p4, p5);
}

//
// Invokes 'GetMenuState' indirectly
//
UINT
TUser::GetMenuState(HMENU p1, UINT p2, UINT p3)
{
  static TModuleProc3<UINT, HMENU, UINT, UINT>
         getMenuState(GetModule(), GetMenuStateStr);
  return getMenuState(p1, p2, p3);
}

/////////////////////////////////////////////////////////////
// VERSION.DLL or VER.DLL 
  static const tchar verStr[] = _T("VERSION.DLL");
# if defined(UNICODE)
    static const char GetFileVersionInfoStr[] = "GetFileVersionInfoW";
    static const char GetFileVersionInfoSizeStr[] = "GetFileVersionInfoSizeW";
    static const char VerQueryValueStr[]= "VerQueryValueW";
    static const char VerLanguageNameStr[]  = "VerLanguageNameW";
# else
    static const char GetFileVersionInfoStr[] = "GetFileVersionInfoA";
    static const char GetFileVersionInfoSizeStr[] = "GetFileVersionInfoSizeA";
    static const char VerQueryValueStr[]= "VerQueryValueA";
    static const char VerLanguageNameStr[]  = "VerLanguageNameA";
# endif

//
// Returns TModule object wrapping the handle of the VERSION.DLL module
//
TModule&
TVersion::GetModule()
{
  static TModule verModule(verStr, true, true, false);
  return verModule;
}

//
// Invokes 'GetFileVersionInfo' indirectly
//
BOOL
TVersion::GetFileVersionInfo(LPTSTR p1, DWORD p2, DWORD p3, LPVOID p4)
{
  static TModuleProc4<BOOL,LPTSTR,DWORD,DWORD,LPVOID>
         getFileVersionInfo(GetModule(), GetFileVersionInfoStr);
  return getFileVersionInfo(p1, p2, p3, p4);
}

//
// Invokes 'GetFileVersionInfoSize' indirectly
//
DWORD      
TVersion::GetFileVersionInfoSize(LPTSTR p1, LPDWORD p2)
{
  static TModuleProc2<DWORD,LPTSTR,LPDWORD>
         getFileVersionInfoSize(GetModule(), GetFileVersionInfoSizeStr);
  return getFileVersionInfoSize(p1, p2);
}

//
// Invokes 'VerQueryValue' indirectly
//
BOOL      
TVersion::VerQueryValue(const LPVOID p1,LPTSTR p2,LPVOID p3, uint * p4)
{
  static TModuleProc4<BOOL,const LPVOID,LPTSTR,LPVOID,uint *>
         verQueryValue(GetModule(), VerQueryValueStr);
  return verQueryValue(p1, p2, p3, p4);
}

//
// Invokes 'VerLanguageName' indirectly
//
DWORD 
TVersion::VerLanguageName(DWORD p1, LPTSTR p2, DWORD p3)
{
  static TModuleProc3<DWORD,DWORD,LPTSTR,DWORD>
         verLanguageName(GetModule(), VerLanguageNameStr);
  return verLanguageName(p1, p2, p3);
}

/////////////////////////////////////////////////////////////
//
//  Delay loading OLE libraries 
//
static const tchar oleStr[]            = _T("OLE32.DLL");
static const tchar oleAutStr[]          = _T("OLEAUT32.DLL");

static const char CoCreateInstanceStr[]  = "CoCreateInstance";
static const char SysFreeStringStr[]    = "SysFreeString";
static const char SysStringLenStr[]      = "SysStringLen";
static const char SysAllocStringStr[]    = "SysAllocString";

//
// Returns TModule object wrapping the handle of the OLE32.DLL/COMPOBJ.DLL module
//
TModule&
TOle::GetModule()
{
  static TModule oleModule(oleStr, true, true, false);
  return oleModule;
}

//
// Invokes 'CoCreateInstance' indirectly
//
HRESULT 
TOle::CoCreateInstance(REFCLSID p1, LPUNKNOWN p2, DWORD p3, REFIID p4, LPVOID* p5)
{
  static TModuleProc5<HRESULT,REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*>
         coCreateInstance(GetModule(), CoCreateInstanceStr);
  return coCreateInstance(p1, p2, p3, p4, p5);
};

//
// Returns TModule object wrapping the handle of the OLE32.DLL/COMPOBJ.DLL module
//
TModule&
TOleAuto::GetModule()
{
  static TModule oleautoModule(oleAutStr, true, true, false);
  return oleautoModule;
}

//
// Invokes 'SysFreeString' indirectly
//
HRESULT    
TOleAuto::SysFreeString(BSTR p1)
{
  static TModuleProc1<HRESULT,BSTR> 
         sysFreeString(GetModule(), SysFreeStringStr);
  return sysFreeString(p1);
}

//
// Invokes 'SysStringLen' indirectly
//
UINT      
TOleAuto::SysStringLen(BSTR p1)
{
  static TModuleProc1<UINT,BSTR> 
         sysStringLen(GetModule(), SysStringLenStr);
  return sysStringLen(p1);
}


//
// Invokes 'SysAllocString' indirectly
//
BSTR
TOleAuto::SysAllocString(const OLECHAR * p1)
{
  static TModuleProc1<BSTR,const OLECHAR *>
         sysAllocString(GetModule(), SysAllocStringStr);
  return sysAllocString(p1);
}

//----------------------------------------------------------------------------
//
// Exception class
//

//
/// Creates the Invalid Module exception.
//
TXInvalidModule::TXInvalidModule(const tstring& name)
:
  TXOwl(MakeMessage(IDS_INVALIDMODULE, name))
{
}

//
/// Creates a copy of the exception 
//
TXInvalidModule*
TXInvalidModule::Clone() const
{
  return new TXInvalidModule(*this);
}


//
/// Throws the exception
//
void
TXInvalidModule::Throw()
{
  throw *this;
}

//
/// Throws the exception
//
void
TXInvalidModule::Raise(const tstring& name)
{
  TXInvalidModule(name).Throw();
}
} // OWL namespace


namespace owl {


//----------------------------------------------------------------------------
// TModule streaming
//

IMPLEMENT_STREAMABLE(TModule);

#if !defined(BI_NO_OBJ_STREAMING)

//
/// Extracts the module object from the persistent stream.
//
void*
TModule::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TModule* o = GetObject();
  is >> o->Name;
  is >> o->ShouldFree;
  if (o->ShouldFree)
    o->Handle = ::LoadLibrary(o->Name.c_str());

  return o;
}

//
/// Writes the module into a persistent stream.
//
void
TModule::Streamer::Write(opstream& os) const
{
  TModule* o = GetObject();
  os << o->Name;
  os << o->ShouldFree;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace


//----------------------------------------------------------------------------
//
// Entry (& exit) functions for Owl in a DLL
//

#if defined(_BUILDOWLDLL)

namespace owl {

void InitGlobalModule(HINSTANCE hInstance)
{
	if (Module == NULL)
	{
    tchar buffer[_MAX_PATH];
    GetModuleFileName(hInstance, buffer, _MAX_PATH);
    static TModule module(buffer, hInstance);
    Module = &module;
  	TRACEX(OwlApp, 0, "Global Module " << hInstance << ", " << buffer << " created");
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

//
/// TModule derived class to facilitate streaming pointer to the OWL Library
/// the OWL module must be streamed by reference before any pointers to it
/// the following code simply prevents writing data back over the OWL module
//
class _OWLCLASS TObjectWindowsLibrary : public TModule {
 public:
  TObjectWindowsLibrary(HINSTANCE hInst) : TModule(_T("ObjectWindowsDLL"), hInst){}
  DECLARE_STREAMABLE(_OWLCLASS, TObjectWindowsLibrary, 1);
};


IMPLEMENT_STREAMABLE1(TObjectWindowsLibrary, TModule);

#  if !defined(BI_NO_OBJ_STREAMING)

//
/// Reads the object from the persistent stream.
//
void*
TObjectWindowsLibrary::Streamer::Read(ipstream&, uint32) const
{
  return GetObject();
}

//
/// Writes the object into a persistent stream.
//
void
TObjectWindowsLibrary::Streamer::Write(opstream&) const
{
}

#  endif  // if !defined(BI_NO_OBJ_STREAMING)


//JJH - following stuff is defined in global.cpp, we do not need it here.
//Jogy - it is needed in both places for DLL builds.
//VH - Reason is that the VC and BC build script doesn't link global.obj for the DLL itself, only for the import lib.
//
# if !defined(WINELIB)
extern TModule* Module = 0;   // Global Module ptr in each DLL/EXE
# endif // !WINELIB

} // OWL namespace

#  if !defined(BI_COMP_BORLANDC) && !defined(BI_COMP_GNUC)

int __cdecl xnew_handler(size_t) {throw std::bad_alloc(); return 0;}

static void init_new_handler(void)
{
  _set_new_handler(&xnew_handler);
}

#  endif // !defined(BI_COMP_BORLANDC) && !defined(BI_COMP_GNUC)


extern "C" BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID);
#    if defined(BI_COMP_BORLANDC)  
extern "C" BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID) = RawDllMain;
#    else
extern "C" BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID); //JJH split this to declaration and initialization
BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID) = &RawDllMain;
#    endif

extern "C"
//#    if !defined(BI_COMP_MSC)
//BOOL WINAPI RawDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
//#    else
BOOL WINAPI RawDllMain(HINSTANCE, DWORD dwReason, LPVOID)
//#    endif
{
  if (dwReason == DLL_PROCESS_ATTACH){
#if OWL_STACKWALKER
    //Start gathering memory leaks:
    InitAllocCheck(ACOutput_XML);
#endif

#    ifdef _UNICODE
    // give error message and exit if running Unicode on non-Unicode system
    if (::GetVersion() & 0x80000000){
      // Note: this message is for programmers who can read english
      ::MessageBoxA(NULL,
        "This application or DLL can not be loaded "
        "on Windows 95 or on Windows 3.1.  It takes advantage "
        "of Unicode features only available on Windows NT.",
        "OWL Runtime Module", MB_ICONSTOP|MB_OK);
      return FALSE; // and fail
    }
#    endif
    

    //
    ///! SetErrorMode(GetErrorMode(0)|SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
    // add a reference to thread local storage data
    TlsAddRefs();

  }
  else if (dwReason == DLL_PROCESS_DETACH){
    TlsRelease();
#if OWL_STACKWALKER
    //Stop gathering memory leaks:
    DeInitAllocCheck();
#endif
  }
  return TRUE;    // ok
}

#if !defined(BI_COMP_BORLANDC)
#if defined(WINELIB) //JJH
extern "C"
#endif
BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD reason, LPVOID lpreserved)
#else
int WINAPI
DllEntryPoint(HINSTANCE hInstance, owl::uint32 reason, void*)
#endif
{
  switch (reason) {
    case DLL_PROCESS_ATTACH: {
#if !defined(BI_COMP_BORLANDC) && !defined(BI_COMP_GNUC)
        init_new_handler();
#endif

//JJH Ok, we don't need to initialize owl::Module, since TApplication object
//    does this for us, see also applicat.cpp:420, applicat.h:132
//#if !defined(WINELIB)
        owl::Module = new owl::TObjectWindowsLibrary(hInstance);
//#endif
      break;
    }
    case DLL_PROCESS_DETACH: {
      break;
    }
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////
//
// Here VC bug with vector constructor
//
// Special code to wire up vector deleting destructors
// TODO: Jogy - check if this is needed with modern VC++ compilers
#if defined(BI_COMP_MSC)
static void _OwlForceVectorDelete()
{
#ifdef _DEBUG
  PRECONDITION(false);  // never called
#endif

  new owl::TPoint[2];
  new owl::TPointF[2];
  new owl::TPointL[2];
  new owl::TRect[2];
  new owl::TSize[2];
  new owl::TColor[2];
  new owl::TMenu[2];
  new owl::TResId[2];
  new owl::TFileTime[2];
  new owl::TSystemTime[2];
}
#endif // BI_COM_MSC
#else  // else _BUILDOWLDLL
////////////////////////////////////////////////////////////////////////////////////////
// static lib????
/////////////////////////////////////
#if defined(BI_COMP_BORLANDC)
//
// Doesn't work here -> moved to winmain
//
//
// NOTE: We use priority 31 to come just before/after ctr/dtr of global
//       objects (which are assigned a priorority of 32)
//
void __initOWL()
{
  TlsAddRefs();
}
#pragma startup __initOWL 31

//
void __termOWL()
{
  TlsRelease();
}
#pragma exit __termOWL 31

//
#elif defined(BI_COMP_MSC)


#if defined(BI_COMP_FORTE) || defined(BI_COMP_ACC) //|| defined(__EDG__)
  void __cdecl xnew_handler() { throw std::bad_alloc(); }
#else
  int  __cdecl xnew_handler(size_t) { throw std::bad_alloc(); return 0;}
#endif //else not BI_COMP_FORTE
static void init_new_handler(void)
{
//#ifdef __EDG__
//  std::set_new_handler(&xnew_handler);
//#else
  _set_new_handler(&xnew_handler);
//#endif
}

// force initialization early
#pragma warning(disable: 4073)
#pragma init_seg(lib)

static void __initOWL()
{
  //_hInstance = GetModuleHandleA(NULL);
  TlsAddRefs();

  init_new_handler();

#if defined(BI_DBCS_SUPPORT)
//  _setmbcp(_MB_CP_ANSI); // check
#endif
}

//
#ifdef BI_COMP_FORTE
extern "C"
#endif
void _cdecl __termOWL()
{
  TlsRelease();
}

static char __initOWLS = (char)(__initOWL(), atexit(&__termOWL));

//
# elif defined(BI_COMP_GNUC)

// //JJH
//#if defined(WINELIB)
//void __termOWL()
//#else
void _cdecl __termOWL()
//#endif
{
  TlsRelease();
}

static void __initOWL()
{
  TlsAddRefs();
}

// //JJH
//#if defined(WINELIB)
//typedef void (__attribute__((__stdcall__)) *STDCALL_FN)();
//static char __initOWLS = (char)(__initOWL(),atexit((STDCALL_FN)__termOWL));
//#else
static char __initOWLS = (char)(__initOWL(),atexit(__termOWL));
//#endif

#else
#error Unknown compiler
#endif // #ifdef BI_COMP_FORTE


#endif  // defined(_BUILDOWLDLL)

namespace owl {
//
// Inserter for formated output of instance handle
//
_OWLCFUNC(ostream&)
operator <<(ostream& os, const TModule& m)
{
  return os << hex << uint(m.Handle);
}
} // OWL namespace


/* ========================================================================== */

