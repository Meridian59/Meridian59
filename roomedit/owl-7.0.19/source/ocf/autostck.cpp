//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// OLE Automation - Implementation of TAutoStack
//----------------------------------------------------------------------------
#include <ocf/pch.h>
#include <ocf/appdesc.h>
#include <ocf/ocreg.h>
#include <ocf/occtrl.h>

namespace ocf {

//----------------------------------------------------------------------------
// TAutoStack implementation
//

TAutoStack::TAutoStack(DISPID id, VARIANT *stack, TLocaleId locale,
                       int argcount, int named, long * map,
                       TServedObject* owner)
:
  Owner(owner),
  Stack((TAutoVal*)stack),
  LangId(LANGIDFROMLCID(locale)),
  ArgCount(argcount),
  NamedCount(named),
  NamedIds(map),
  CurrentArg(-1),
  DispId(id)
{
}

TAutoStack::~TAutoStack()
{
  TAutoVal* val;
  for (val = Stack; ArgCount; --*const_cast<int*>(&ArgCount), val++)
    val->Restore();
}

TAutoVal& TAutoStack::operator [](int index)
{
  TAutoSymbol* argSymbol = nullptr;
  if (index < ArgSymbolCount)
    argSymbol = index >= 0 ? Symbol + index + 1 : Symbol;
  int vIndex = ArgCount - index - 1;        // index if not a named argument
  if (index == TAutoStack::SetValue ||      // property value to set
      index >= ArgCount-NamedCount) {       // named or out of range
    for (vIndex = NamedCount; --vIndex >= 0; )
      if (NamedIds[vIndex] == index) {
        break;
      }
  }
  TAutoVal* val;
  if (vIndex >= 0) {
    CurrentArg = vIndex;   // save index for error return
    val = Stack[vIndex].DereferenceVariant();
    val->SetLocale(LangId);
    if ((val->GetDataType() == atString) &&
        ((argSymbol) && (argSymbol->IsEnum()) &&
         (argSymbol->GetEnum()->Convert(*val, Default))))
      return Default;
  }
  else if (index == TAutoStack::SetValue) {
    TXAuto::Raise(TXAuto::xParameterMissing);
  }
  else {
    val = &Default;
    val->SetLocale(LangId);
    if (!argSymbol)
      TXAuto::Raise(TXAuto::xNoArgSymbol);
    const _TCHAR* dfltStr = argSymbol->Doc.Translate(LangId); // load default
    if (!dfltStr)
      TXAuto::Raise(TXAuto::xNoDefaultValue);
    Default = dfltStr;    // makes a BSTR in order to use OLE conversions
  }
  return *val;
}


} // OCF namespace

//==============================================================================

