//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// OLE Automation Client Implementation
//----------------------------------------------------------------------------
#include <ocf/pch.h>
#include <ocf/autodefs.h>


namespace ocf {

using namespace owl;


DIAG_DECLARE_GROUP(OcfDll);

//
//
void TAutoProxy::Bind(LPCTSTR progid)
{
  PRECONDITION(progid);

  GUID guid;
  TXOle::Check(CLSIDFromProgID(OleStr(progid), &guid), progid);
  Bind(guid);
}

//
//
void TAutoProxy::Bind(const GUID& guid)
{
  IUnknown* unk;
  HRESULT stat = ::CoCreateInstance(guid,0,CLSCTX_SERVER,IID_IUnknown,(void **)&unk);
  if (stat != HR_NOERROR) {
    _TCHAR guidBuf[60];
    TClassId copy(guid);
    _tcscpy(guidBuf, copy);
    TXOle::Check(stat, guidBuf);
  }
  Bind(unk);
}

//
//
void TAutoProxy::Bind(IUnknown* unk)
{
  if (!unk)
    return;
  IDispatch* dsp;
  HRESULT stat = unk->QueryInterface(IID_IDispatch, (void * *)&dsp);
  unk->Release();
  TXOle::Check(stat, _T("IUnknown"));
  Bind(dsp);
}

//
//
void TAutoProxy::Bind(IDispatch* dsp)
{
  Unbind();
  That = dsp;
}

//
//
void TAutoProxy::Bind(IUnknown& unk)
{
  IDispatch* dsp;
  TXOle::Check(unk.QueryInterface(IID_IDispatch, (void * *)&dsp), _T("IUnknown"));
  Bind(dsp);
}

//
//
void TAutoProxy::Bind(IDispatch& obj)
{
  Unbind();
  obj.AddRef();
  That = &obj;
}

//
//
void TAutoProxy::Bind(TAutoVal& val)
{
  if (val.GetDataType() == atVoid)
    That = 0;
  else
    // NOTE: The following call throws exception in case of failure
    //
    Bind((IDispatch&)val);
}

//
//
void TAutoProxy::MustBeBound()
{
  if (!That)
    TXAuto::Raise(TXAuto::xNotIDispatch);
}

//
//
IDispatch* TAutoProxy::GetObject(LPCTSTR progid)
{
  CLSID clsid;
  TXOle::Check(CLSIDFromProgID(OleStr(progid), &clsid));

  LPUNKNOWN  punk  = 0;
  LPDISPATCH pdisp = 0;
  HRESULT hr = GetActiveObject(clsid, 0, &punk);
  if (SUCCEEDED(hr)) {
    punk->QueryInterface(IID_IDispatch,
                         (void**)&pdisp);
    punk->Release();
  }
  return pdisp;
}

//
//
long TAutoProxy::Lookup(LPCTSTR name)
{
  LPCTSTR failure = _T("(null)");
  if (!name)
    name = failure;  // force controlled failure
  long id;

  MustBeBound();
#if !defined(_UNICODE)
  TString tsname(name);
  wchar_t* wname = tsname;
  TXOle::Check(That->GetIDsOfNames(IID_NULL, &wname, 1, Lang, &id), name);
#else
  TXOle::Check(That->GetIDsOfNames(IID_NULL, (LPOLESTR*)&name, 1, Lang, &id), name);
#endif
  return id;
}

//
//
//
void TAutoProxy::Lookup(LPCTSTR names, long* ids, unsigned count)
{
  MustBeBound();

#if !defined(_UNICODE)
  const char * pc = names;
  int i;
  for (i = 0; i < (int)count; i++)
    pc += strlen(pc) + 1;

  wchar_t* wnames = TUString::ConvertAtoW(names, pc-names-1);
  wchar_t** args = new wchar_t *[count];
  wchar_t* pw = wnames;
  for (i = 0; i < (int)count; i++) {
    args[i] = pw;
    pw += lstrlenW(pw) + 1;
  }
  HRESULT stat = That->GetIDsOfNames(IID_NULL, args, count, Lang, ids);
  delete wnames;
#else
  _TCHAR ** args = new _TCHAR *[count];
  const _TCHAR * pc = names;
  for (int i = 0; i < (int)count; i++) {
    args[i] = (_TCHAR *)pc; // for non-unicode, names separated by null bytes
    pc += _tcslen(pc) + 1;
  }
  HRESULT stat = That->GetIDsOfNames(IID_NULL, args, count, Lang, ids);
#endif

  delete[] args;
  if (stat) {
    int bad = 0;
    if ((stat = HR_DISP_UNKNOWNNAME) != HR_NOERROR)
      while (bad < (int)count && ids[bad] != DISPID_UNKNOWN) {
        bad++;
        names += (_tcslen(names) + 1);
      }
    TXOle::Check(stat, names);
  }
}

//
//
//
TAutoVal&
TAutoProxy::Invoke(uint16 attr, TAutoProxyArgs& args,
                   long* ids, unsigned named)
{
  // Check that we're bound
  //
  MustBeBound();

  VARIANT* retval = 0;
  if (!(attr & (acPropSet | acVoidRet))) {

    // NOTE: The first TAutoVal/VARIANT of TAutoProxyArgs (really TAutoArgs)
    //       is for the return value...
    retval = args;

    // !BB ::VariantInit(retval);
    // !BB Don't need this anymore now that TAutoVal had ctrs/dtrs

  }

  DISPID funcId = ids[0];
  DISPPARAMS params;
  params.cArgs = args;
  params.cNamedArgs = named;
  params.rgvarg = args;
  params.rgdispidNamedArgs = ids;

  if (attr & acPropSet) {
    ids[0] = DISPID_PROPERTYPUT;
    params.cNamedArgs++;
    params.cArgs++;
  }
  else {
    params.rgdispidNamedArgs++;
    params.rgvarg++;
  }

  unsigned int errarg = 0;
  EXCEPINFO errinfo = {0};
  HRESULT stat = That->Invoke(funcId, IID_NULL, Lang,
                              uint16(attr & (acMethod | acPropSet | acPropGet)),
                              &params, retval, &errinfo, &errarg);

  // Restore function id incase PropSet
  //
  ids[0] = funcId;

  // Check for failure
  //
  if (stat != HR_NOERROR) {
     TAPointer<_TCHAR> errMsg(new _TCHAR[errinfo.bstrDescription ?\
                                     (wcslen(errinfo.bstrDescription) + 100)\
                                     : 100]);
    // Is there additional informatiol in the EXCEPINFO structure
    //
    if (stat == ResultFromScode(DISP_E_EXCEPTION)) {
#if defined(_UNICODE)
      wsprintf(errMsg, _T("Invoke ID= %ld/%lX (%s) %s=%X"), funcId, funcId,\
                       errinfo.bstrDescription,\
                       (errinfo.wCode ? _T("wCode") : _T("scode")),\
                       (errinfo.wCode ? (int)errinfo.wCode : (int)errinfo.scode));
#else
      TString str(errinfo.bstrDescription);
      const _TCHAR *errDesc = errinfo.bstrDescription ? (const _TCHAR*)str : _T("");
      wsprintf(errMsg, _T("Invoke ID= %ld/%lX (%s) %s=%X"), funcId, funcId,\
                       errDesc,\
                       (errinfo.wCode ? _T("wCode") : _T("scode")),\
                       (errinfo.wCode ? (int)errinfo.wCode : (int)errinfo.scode));
#endif
    }
    else if (stat == ResultFromScode(DISP_E_TYPEMISMATCH) ||
             stat == ResultFromScode(DISP_E_PARAMNOTFOUND)){
      wsprintf(errMsg, _T("Invoke ID= %ld/%lX, arg Index:%d"), funcId, funcId,
                        errarg);
    }
    else
      wsprintf(errMsg, _T("Invoke Id= %ld/%lX"), funcId, funcId);

    TXOle::Check(stat, errMsg);
  }
  return args;
}

//
//
//
TAutoEnumeratorBase::TAutoEnumeratorBase(const TAutoEnumeratorBase& copy)
{
  Current.Copy(copy.Current);
  Iterator = copy.Iterator;
  if (Iterator)
    TXOle::Check(Iterator->Clone(&Iterator));
}

//
//
//
void TAutoEnumeratorBase::operator =(const TAutoEnumeratorBase& copy)
{
  Current.Copy(copy.Current);
  Unbind();
  Iterator = copy.Iterator;
  if (Iterator)
    TXOle::Check(Iterator->Clone(&Iterator));
}

//
//
//
void TAutoEnumeratorBase::Bind(TAutoVal& val)
{
  Unbind();
  IUnknown& unk = val;
  TXOle::Check(unk.QueryInterface(IID_IEnumVARIANT, (void * *) &Iterator),
               _T("_NewEnum"));
}

//
//
//
bool TAutoEnumeratorBase::Step()
{
  Clear();
  if (!Iterator)
    TXOle::Check(HR_NOINTERFACE, _T("_NewEnum"));
  else if (HRIsOK(Iterator->Next(1,(VARIANT *)&Current,0)))
    return true;
  return false;
}

} // OCF namespace

//==============================================================================

