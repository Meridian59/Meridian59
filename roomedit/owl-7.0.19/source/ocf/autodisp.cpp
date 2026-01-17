//----------------------------------------------------------------------------
// ObjectConnections
// Copyright (c) 1994, 1996 by Borland International
/// \file
/// OLE Automation - Implementation of TDispatch, a light-weight version
///                  of IDispatch.
/// \note See TServedObject for a richer implementation of IDispatch.
//----------------------------------------------------------------------------
#include <ocf/pch.h>
#include <ocf/appdesc.h>
#include <ocf/ocreg.h>
#include <ocf/occtrl.h>

namespace ocf {

using namespace owl;

DIAG_DECLARE_GROUP(OcfDll);

//
//
//
TUnknown* TDispatchCreator::CreateObject(TObjectDescriptor objDesc,
                                         IUnknown* outer)
{
  return new TDispatch(objDesc, outer);
}

//
//
//
IDispatch* TDispatchCreator::CreateDispatch(TObjectDescriptor objDesc,
                                            IUnknown* outer)
{
  TDispatch* obj = new TDispatch(objDesc, outer);
  return *obj;
}

DEFINE_QI_OLEBASE(IDispatch,    0x20400L)
DEFINE_COMBASES1 (TDispatchCOM, IDispatch)

//
//
//
TDispatch::TDispatch(const TObjectDescriptor& objDesc, IUnknown* outer)
:
  Object(const_cast<void*>(objDesc.Object)),
  Class(objDesc.Class)
{
  SetOuter(outer ? outer
       : &objDesc.Class->Aggregate(const_cast<void*>(objDesc.Object), *this));
  //
  // NOTE: RefCnt = 0 on creation, will ++ in TAutoVal operator(IDispatch*)
  //
}

// IDispatch implementation

HRESULT _IFUNC
TDispatch::GetTypeInfoCount(unsigned int * pctinfo)
{
  *pctinfo = 0;
  return HR_NOERROR;
}

HRESULT _IFUNC
TDispatch::GetTypeInfo(unsigned int, LCID, ITypeInfo* *)
{
  return HR_NOTIMPL;
}

HRESULT _IFUNC
TDispatch::GetIDsOfNames(const IID & riid, OLECHAR * * names,
                          unsigned int cNames, LCID lcid, DISPID * dispIds)
{
  if (riid != IID_NULL){
    WARNX(OcfDll,  riid != IID_NULL, 1,  _T("TDispatch::GetIDsOfNames() riid != IID_NULL"));
    return HR_DISP_UNKNOWNINTERFACE;
  }

  HRESULT retval = HR_NOERROR;
  TAutoSymbol* symbol;
  for (int i = 0; i < (int)cNames; i++) {
    dispIds[i] = -1;
    if (i == 0) {
    symbol = Class->Lookup(OleStr(names[0]), LANGIDFROMLCID(lcid),
                           asAnyCommand, dispIds[0]);
      if (!symbol)
        retval = HR_DISP_UNKNOWNNAME;
    }
    else if (symbol) {
      if (!Class->LookupArg(OleStr(names[i]), LANGIDFROMLCID(lcid), symbol, dispIds[i]))
        retval = HR_DISP_UNKNOWNNAME;
    }
  }
  return retval;
}

//
//
//
HRESULT _IFUNC
TDispatch::Invoke(DISPID dispidMember, const IID& /*riid*/, LCID lcid,
                  unsigned short wFlags, DISPPARAMS* dispparams,
                  VARIANT* varResult, EXCEPINFO* exceptInfo,
                  unsigned int* retArgErr)
{
  // Make a copy of the object in case there's this pointer adjustment
  //
  ObjectPtr object = Object;

  // Make sure our object is still valid
  //
  if (!object) {
    WARNX(OcfDll, !object, 1, _T("TDispatch::Invoke() Object == 0"));
    return HR_DISP_MEMBERNOTFOUND;
  }

  // Build an object representing the data passed in
  // NOTE: Here lies the difference between the 'Invoke' of TServedObject
  //       and TDispatch: the last parameter to TAutoStack's constructor.
  //
  TAutoStack stack(dispidMember, dispparams->rgvarg, lcid, dispparams->cArgs,
                   dispparams->cNamedArgs, dispparams->rgdispidNamedArgs, 0);

  // Find the symbol we're asked to dispatch to
  //
  stack.Symbol = Class->FindId(dispidMember, object);

  if (!stack.Symbol) {
    // NOTE: This is a 'hack' that allows TServedObject to expose a generic method
    //       that's used for cases when the object does not provide a method
    //       that matches the dispId invoked.
    //       It is used by our container support for sinking non-standard events.
    //
    if ((stack.Symbol = Class->FindId(DISPID_CATCH_ALL, object)) == nullptr)
      return HR_DISP_MEMBERNOTFOUND;
  }

  // Check the attribute bits to ensure we support the type
  //
  if (!stack.Symbol->TestFlag(wFlags)) {
    TRACEX(OcfDll, 1, _T("TDispatch::Invoke() type unsupported"));
    return HR_DISP_MEMBERNOTFOUND;
  }

  // Check if we need return result
  // NOTE: Some servers [such as Word.Basic] seem to be very picky about
  //       the distinction between a function and procedure. They are not
  //       as flexible as we are here..
  //
  if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
    varResult = 0;


  // Check that the number of arguments sent matches what we're expecting
  //
  // Again here we'll have to relax a little on the param count check since in the
  // case of the DISPID_CATCH_ALL, the handler is a generic one that can handle
  // any number of parameters.
  //
  if (((stack.ArgSymbolCount = Class->GetArgCount(*stack.Symbol)) +
      ((wFlags & (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)) != 0) < stack.ArgCount) &&
      (stack.Symbol->DispId != DISPID_CATCH_ALL)){
    TRACEX(OcfDll, 1, _T("TDispatch::Invoke() BadParamCount"));
    return HR_DISP_BADPARAMCOUNT;
  }


  // Dispatch via the command object and hope we're OK
  // NOTE: TDispatchCreator() vs. Creator of TDispatch & TServedObject
  //
  TDispatchCreator myDispatcher=TDispatchCreator();
  switch(Class->Dispatch(object, myDispatcher, *this, wFlags, stack, (TAutoVal*)varResult)) {
  case TXAuto::xNoError:
          return HR_NOERROR;

    case TXAuto::xNotIDispatch:
    case TXAuto::xForeignIDispatch:
          return HR_DISP_BADVARTYPE;

    case TXAuto::xValidateFailure:
         *retArgErr = stack.CurrentArg;
          return HR_DISP_OVERFLOW;

    case TXAuto::xConversionFailure:
    case TXAuto::xTypeMismatch:
         *retArgErr = stack.CurrentArg;
          return HR_DISP_TYPEMISMATCH;

    case TXAuto::xNoArgSymbol:
         *retArgErr = stack.CurrentArg;
          return HR_DISP_PARAMNOTFOUND;

    case TXAuto::xParameterMissing:
    case TXAuto::xNoDefaultValue:
          return HR_DISP_PARAMNOTOPTIONAL;

    case TXAuto::xErrorStatus:
      if (exceptInfo) {
        exceptInfo->wCode = (unsigned short)stack.ErrorCode;
        exceptInfo->wReserved = 0;
        exceptInfo->bstrSource = 0;   // cmp vs. TServedObject's support
        exceptInfo->bstrDescription = TOleAuto::SysAllocString((const OLECHAR *)stack.ErrorMsg);
        exceptInfo->bstrHelpFile = 0;
        exceptInfo->pfnDeferredFillIn = 0;

        // INVESTIGATE: Is there a method to relay better SCODEs ?
        //
        exceptInfo->scode = E_FAIL;
      }
      return HR_DISP_EXCEPTION;

  case TXAuto::xExecutionFailure:
  default:
        // INVESTIGATE: Is there a better error code here ?
        //
        return HR_DISP_OVERFLOW;
  }
}


} // OCF namespace

//==============================================================================

