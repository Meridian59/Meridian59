//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TAutoVal implementation
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <ocf/autodefs.h>

namespace ocf {

using namespace owl;

//----------------------------------------------------------------------------
// TAutoVal assignment operators (others are inline)
//

//
//
void
TAutoVal::operator =(TAutoString s)
{
  vt = atString;
  bstrVal = ::SysAllocString((const OLECHAR *)s);
  SetLocale(s.GetLangId() ? s.GetLangId() : TLocaleString::NativeLangId);
}

//
//
void
TAutoVal::operator =(TString s)
{
  vt = atString;
  bstrVal = ::SysAllocString((const OLECHAR *)s);
  SetLocale(s.GetLangId() ? s.GetLangId() : TLocaleString::NativeLangId);
}

//
//
void
TAutoVal::operator =(const owl::tstring& s)
{
  vt = atString;
#if defined(_UNICODE)
  bstrVal = ::SysAllocString(s.c_str());
#else
  bstrVal = ::owl::SysAllocString(s.c_str());
#endif
  SetLocale(TLocaleString::NativeLangId);
}


// Provides automatic conversion of const char* to BSTR
//
void TAutoVal::operator =(const char* s)
{
  vt=atString;
  bstrVal = ::owl::SysAllocString(s);
  SetLocale(owl::TLocaleString::NativeLangId);
}

//
// should add operator==(TLocaleString) which translates to proper LangId
// this requires Set/GetLocale to be used for atVoid initialized variants
//

//----------------------------------------------------------------------------
// TAutoVal conversion operators
//

//
//
//
void
TAutoVal::SetLocale(TLocaleId locale)
{
  switch (vt) {
    case atVoid:
    case atString:
    case atObject:
         p.Locale = locale;
  };
}

//
//
//
TLocaleId
TAutoVal::GetLocale() const
{
  switch (vt) {
    case atVoid:
    case atString:
    case atObject:
        return p.Locale;
  };
  return 0;
}

//
//
//
TLangId
TAutoVal::GetLanguage() const
{
  switch (vt) {
    case atVoid:
    case atString:
    case atObject:
      return LANGIDFROMLCID(p.Locale);
  };
  return 0;
}

//
//
//
TAutoVal::operator unsigned char()
{
  unsigned char v;
  switch (vt) {
  case atByte:    return bVal;
  case atShort:   if(::VarUI1FromI2(iVal,  &v)) break; return v;
  case atLong:    if(::VarUI1FromI4(lVal,  &v)) break; return v;
  case atFloat:   if(::VarUI1FromR4(fltVal,&v)) break; return v;
  case atDouble:  if(::VarUI1FromR8(dblVal,&v)) break; return v;
  case atCurrency:if(::VarUI1FromCy(cyVal, &v)) break; return v;
  case atDatetime:if(::VarUI1FromDate(date,&v)) break; return v;
  case atString:  if(::VarUI1FromStr(bstrVal,GetLocale(),0L,&v)) break; return v;
  case atObject:  if(::VarUI1FromDisp(pdispVal,GetLocale(),&v)) break; return v;
  case atBool:    return (unsigned char)(boolVal ? 1 : 0);
  case atByRef+atByte:    return *pbVal;
  case atByRef+atShort:   if(::VarUI1FromI2(*piVal,  &v)) break; return v;
  case atByRef+atLong:    if(::VarUI1FromI4(*plVal,  &v)) break; return v;
  case atByRef+atFloat:   if(::VarUI1FromR4(*pfltVal,&v)) break; return v;
  case atByRef+atDouble:  if(::VarUI1FromR8(*pdblVal,&v)) break; return v;
  case atByRef+atCurrency:if(::VarUI1FromCy(*pcyVal, &v)) break; return v;
  case atByRef+atDatetime:if(::VarUI1FromDate(*pdate,&v)) break; return v;
  case atByRef+atString:  if(::VarUI1FromStr(*pbstrVal,GetLocale(),0L,&v)) break; return v;
  case atByRef+atBool:    return (unsigned char)(*pbool ? 1 : 0);
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
//
TAutoVal::operator short()
{
  short v;
  switch (vt) {
  case atByte:    return (short)bVal;
  case atShort:   return iVal;
///  case atLong:    if (lVal>0x7FFFL || lVal <0xFFFF8000L) break; return (short)lVal;
  case atLong:    if(::VarI2FromI4(lVal, &v))  break; return v;
  case atFloat:   if(::VarI2FromR4(fltVal,&v)) break; return v;
  case atDouble:  if(::VarI2FromR8(dblVal,&v)) break; return v;
  case atCurrency:if(::VarI2FromCy(cyVal, &v)) break; return v;
  case atDatetime:if(::VarI2FromDate(date,&v)) break; return v;
  case atString:  if(::VarI2FromStr(bstrVal,GetLocale(),0L,&v)) break; return v;
  case atObject:  if(::VarI2FromDisp(pdispVal,GetLocale(),&v)) break; return v;
  case atBool:    return short(boolVal ? 1 : 0);
  case atByRef+atByte:    return (short)*pbVal;
  case atByRef+atShort:   return *piVal;
///  case atByRef+atLong:    if (*plVal>0x7FFFL || *plVal <0xFFFF8000L) break; return (short)*plVal;
  case atByRef+atLong:    if(::VarI2FromI4(*plVal, &v)) break; return v;
  case atByRef+atFloat:   if(::VarI2FromR4(*pfltVal,&v)) break; return v;
  case atByRef+atDouble:  if(::VarI2FromR8(*pdblVal,&v)) break; return v;
  case atByRef+atCurrency:if(::VarI2FromCy(*pcyVal, &v)) break; return v;
  case atByRef+atDatetime:if(::VarI2FromDate(*pdate,&v)) break; return v;
  case atByRef+atString:  if(::VarI2FromStr(*pbstrVal,GetLocale(),0L,&v)) break; return v;
  case atByRef+atBool:    return short(*pbool ? 1 : 0);
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
//
TAutoVal::operator long()
{
  long v;
  switch (vt) {
  case atByte:    return (long)bVal;
  case atShort:   return (long)iVal;
  case atLong:    return lVal;
  case atFloat:   if(::VarI4FromR4(fltVal,&v)) break; return v;
  case atDouble:  if(::VarI4FromR8(dblVal,&v)) break; return v;
  case atCurrency:if(::VarI4FromCy(cyVal, &v)) break; return v;
  case atDatetime:if(::VarI4FromDate(date,&v)) break; return v;
  case atString:  if(::VarI4FromStr(bstrVal,GetLocale(),0L,&v)) break; return v;
  case atObject:  if(::VarI4FromDisp(pdispVal,GetLocale(),&v)) break; return v;
  case atBool:    return boolVal ? 1L : 0L;
  case atByRef+atByte:    return (long)*pbVal;
  case atByRef+atShort:   return (long)*piVal;
  case atByRef+atLong:    return *plVal;
  case atByRef+atFloat:   if(::VarI4FromR4(*pfltVal,&v)) break; return v;
  case atByRef+atDouble:  if(::VarI4FromR8(*pdblVal,&v)) break; return v;
  case atByRef+atCurrency:if(::VarI4FromCy(*pcyVal, &v)) break; return v;
  case atByRef+atDatetime:if(::VarI4FromDate(*pdate,&v)) break; return v;
  case atByRef+atString:  if(::VarI4FromStr(*pbstrVal,GetLocale(),0L,&v)) break; return v;
  case atByRef+atBool:    return *pbool ? 1L : 0L;
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
//
TAutoVal::operator TBool()
{
  switch (vt) {
  case atByte:    return static_cast<TBool>(bVal != 0);
  case atShort:   return static_cast<TBool>(iVal != 0);
  case atLong:    return static_cast<TBool>(lVal != 0);
  case atFloat:   return static_cast<TBool>(fltVal != 0);
  case atDouble:  return static_cast<TBool>(dblVal != 0);
  case atCurrency:return static_cast<TBool>(cyVal.Lo!=0 || cyVal.Hi != 0);
  case atDatetime:return static_cast<TBool>(date != 0);
  case atString:  {short v; if(::VarBoolFromStr(bstrVal,GetLocale(),0L,&v)) break; return static_cast<TBool>(v!=0);}
  case atObject:  {short v; if(::VarBoolFromDisp(pdispVal,GetLocale(),&v)) break; return static_cast<TBool>(v!=0);}
  case atBool:    return static_cast<TBool>(boolVal != 0);  // note: VARIANT bool TRUE is -1
  case atByRef+atByte:    return static_cast<TBool>(*pbVal != 0);
  case atByRef+atShort:   return static_cast<TBool>(*piVal != 0);
  case atByRef+atLong:    return static_cast<TBool>(*plVal != 0);
  case atByRef+atFloat:   return static_cast<TBool>(*pfltVal != 0);
  case atByRef+atDouble:  return static_cast<TBool>(*pdblVal != 0);
  case atByRef+atCurrency:return static_cast<TBool>(pcyVal->Lo != 0 || pcyVal->Hi != 0);
  case atByRef+atDatetime:return static_cast<TBool>(*pdate != 0);
  case atByRef+atString:  {short v; if(::VarBoolFromStr(*pbstrVal,GetLocale(),0L,&v)) break; return static_cast<TBool>(v!=0);}
  case atByRef+atBool:    return static_cast<TBool>(*pbool != 0);
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
//
TAutoVal::operator float()
{
  float v;
  switch (vt) {
  case atByte:     if(::VarR4FromUI1(bVal,&v)) break; return v;
  case atShort:    if(::VarR4FromI2(iVal, &v)) break; return v;
  case atLong:     if(::VarR4FromI4(lVal, &v)) break; return v;
  case atFloat:    return fltVal;
  case atDouble:   if(::VarR4FromR8(dblVal,&v)) break; return v;
  case atCurrency: if(::VarR4FromCy(cyVal, &v)) break; return v;
  case atDatetime: if(::VarR4FromDate(date,&v)) break; return v;
  case atString:   if(::VarR4FromStr(bstrVal,GetLocale(),0L,&v)) break; return v;
  case atObject:   if(::VarR4FromDisp(pdispVal,GetLocale(),&v)) break; return v;
  case atBool:     if(::VarR4FromBool(boolVal,&v)) break; return v;
  case atByRef+atByte:    if(::VarR4FromUI1(*pbVal,&v)) break; return v;
  case atByRef+atShort:   if(::VarR4FromI2(*piVal, &v)) break; return v;
  case atByRef+atLong:    if(::VarR4FromI4(*plVal, &v)) break; return v;
  case atByRef+atFloat:   return *pfltVal;
  case atByRef+atDouble:  if(::VarR4FromR8(*pdblVal,&v)) break; return v;
  case atByRef+atCurrency:if(::VarR4FromCy(*pcyVal, &v)) break; return v;
  case atByRef+atDatetime:if(::VarR4FromDate(*pdate,&v)) break; return v;
  case atByRef+atString:  if(::VarR4FromStr(*pbstrVal,GetLocale(),0L,&v)) break; return v;
  case atByRef+atBool:    if(::VarR4FromBool(*pbool,&v)) break; return v;
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
//
TAutoVal::operator double()
{
  double v;
  switch (vt) {
  case atByte:    if(::VarR8FromUI1(bVal,&v)) break; return v;
  case atShort:   if(::VarR8FromI2(iVal, &v)) break; return v;
  case atLong:    if(::VarR8FromI4(lVal, &v)) break; return v;
  case atFloat:   if(::VarR8FromR4(fltVal,&v)) break; return v;
  case atDouble:  return dblVal;
  case atCurrency:if(::VarR8FromCy(cyVal, &v)) break; return v;
  case atDatetime:if(::VarR8FromDate(date,&v)) break; return v;
  case atString:  if(::VarR8FromStr(bstrVal,GetLocale(),0L,&v)) break; return v;
  case atObject:  if(::VarR8FromDisp(pdispVal,GetLocale(),&v)) break; return v;
  case atBool:    if(::VarR8FromBool(boolVal,&v)) break; return v;
  case atByRef+atByte:    if(::VarR8FromUI1(*pbVal,&v)) break; return v;
  case atByRef+atShort:   if(::VarR8FromI2(*piVal, &v)) break; return v;
  case atByRef+atLong:    if(::VarR8FromI4(*plVal, &v)) break; return v;
  case atByRef+atFloat:   if(::VarR8FromR4(*pfltVal,&v)) break; return v;
  case atByRef+atDouble:  return *pdblVal;
  case atByRef+atCurrency:if(::VarR8FromCy(*pcyVal, &v)) break; return v;
  case atByRef+atDatetime:if(::VarR8FromDate(*pdate,&v)) break; return v;
  case atByRef+atString:  if(::VarR8FromStr(*pbstrVal,GetLocale(),0L,&v)) break; return v;
  case atByRef+atBool:    if(::VarR8FromBool(*pbool,&v)) break; return v;
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
//
TAutoVal::operator TAutoDate()
{
  DATE v;
  switch (vt) {
  case atByte:     if(::VarDateFromUI1(bVal,&v)) break; return v;
  case atShort:    if(::VarDateFromI2(iVal, &v)) break; return v;
  case atLong:     if(::VarDateFromI4(lVal, &v)) break; return v;
  case atFloat:    if(::VarDateFromR4(fltVal,&v)) break; return v;
  case atDouble:   if(::VarDateFromR8(dblVal,&v)) break; return v;
  case atDatetime: return date;
  case atString:   if(::VarDateFromStr(bstrVal,GetLocale(),0L,&v)) break; return v;
  case atObject:   if(::VarDateFromDisp(pdispVal,GetLocale(),&v)) break; return v;
  case atBool:     if(::VarDateFromBool(boolVal,&v)) break; return v;
  case atByRef+atByte:    if(::VarDateFromUI1(*pbVal,&v)) break; return v;
  case atByRef+atShort:   if(::VarDateFromI2(*piVal, &v)) break; return v;
  case atByRef+atLong:    if(::VarDateFromI4(*plVal, &v)) break; return v;
  case atByRef+atFloat:   if(::VarDateFromR4(*pfltVal,&v)) break; return v;
  case atByRef+atDouble:  if(::VarDateFromR8(*pdblVal,&v)) break; return v;
  case atByRef+atDatetime:return date;
  case atByRef+atString:  if(::VarDateFromStr(*pbstrVal,GetLocale(),0L,&v)) break; return v;
  case atByRef+atBool:    if(::VarDateFromBool(*pbool,&v)) break; return v;
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
//
TAutoVal::operator TAutoCurrency()
{
  CY v;
  TAutoCurrency& rv = *(TAutoCurrency*)&v;

  HRESULT stat = HR_NOERROR;
  switch (vt) {
  case atByte:     stat = ::VarCyFromUI1(bVal, &v); break;
  case atShort:    stat = ::VarCyFromI2(iVal,  &v); break;
  case atLong:     stat = ::VarCyFromI4(lVal,  &v); break;
  case atFloat:    stat = ::VarCyFromR4(fltVal,&v); break;
  case atDouble:   stat = ::VarCyFromR8(dblVal,&v); break;
//  case atCurrency:break; <=== Was this !!! // Y.B check !!!
  case atCurrency: rv = cyVal; break; // Changed to this!!! RBM  // suggested by Bob Miller

  case atString:   stat = ::VarCyFromStr(bstrVal,GetLocale(),0L,&v); break;
  case atObject:   stat = ::VarCyFromDisp(pdispVal,GetLocale(),&v); break;
  case atBool:     stat = ::VarCyFromBool(boolVal,&v); break;
  case atByRef+atByte:    stat = ::VarCyFromUI1(*pbVal, &v); break;
  case atByRef+atShort:   stat = ::VarCyFromI2(*piVal,  &v); break;
  case atByRef+atLong:    stat = ::VarCyFromI4(*plVal,  &v); break;
  case atByRef+atFloat:   stat = ::VarCyFromR4(*pfltVal,&v); break;
  case atByRef+atDouble:  stat = ::VarCyFromR8(*pdblVal,&v); break;
  //case atByRef+atCurrency:break; // suggested by Bob Miller  // Y.B check !!!
  case atByRef+atCurrency: rv = *pcyVal; break;  // Changed to this!!! RBM

  case atByRef+atString:  stat = ::VarCyFromStr(*pbstrVal,GetLocale(),0L,&v); break;
  case atByRef+atBool:    stat = ::VarCyFromBool(*pbool,&v); break;
  default:         stat = (HRESULT)-1;
  }
  if (stat != HR_NOERROR)
    TXAuto::Raise(TXAuto::xConversionFailure);
  return rv;
}

//
// Convert TAutoVal to TUString, used by TAutoString contructor and assignment
//
TAutoVal::operator TUString*()
{
  BSTR v;
  HRESULT stat;
  LCID lcid = MAKELCID(LangUserDefault, SORT_DEFAULT);

  switch (vt) {
  case atByte:     stat = ::VarBstrFromUI1(bVal, lcid,0, &v); break;
  case atShort:    stat = ::VarBstrFromI2(iVal,  lcid,0, &v); break;
  case atLong:     stat = ::VarBstrFromI4(lVal,  lcid,0, &v); break;
  case atFloat:    stat = ::VarBstrFromR4(fltVal,lcid,0, &v); break;
  case atDouble:   stat = ::VarBstrFromR8(dblVal,lcid,0, &v); break;
  case atCurrency: stat = ::VarBstrFromCy(cyVal ,lcid,0, &v); break;
  case atDatetime: stat = ::VarBstrFromDate(date,lcid,0, &v); break;
  case atString:   if (bstrVal) vt = atLoanedBSTR;
             return (s.Holder=TUString::Create(bstrVal, true, GetLanguage()));
  case atObject:   stat = ::VarBstrFromDisp(pdispVal,GetLocale(),0, &v); break;
  case atBool:     stat = ::VarBstrFromBool(boolVal,lcid,0, &v); break;
  case atByRef+atByte:     stat = ::VarBstrFromUI1(*pbVal, lcid,0, &v); break;
  case atByRef+atShort:    stat = ::VarBstrFromI2(*piVal,  lcid,0, &v); break;
  case atByRef+atLong:     stat = ::VarBstrFromI4(*plVal,  lcid,0, &v); break;
  case atByRef+atFloat:    stat = ::VarBstrFromR4(*pfltVal,lcid,0, &v); break;
  case atByRef+atDouble:   stat = ::VarBstrFromR8(*pdblVal,lcid,0, &v); break;
  case atByRef+atCurrency: stat = ::VarBstrFromCy(*pcyVal ,lcid,0, &v); break;
  case atByRef+atDatetime: stat = ::VarBstrFromDate(*pdate,lcid,0, &v); break;
  case atByRef+atString:   if (*pbstrVal) vt = atByRef+atLoanedBSTR;
           return (s.Holder=TUString::Create(*pbstrVal, true, GetLanguage()));
  case atByRef+atBool:     stat = ::VarBstrFromBool(*pbool,lcid,0,&v);break;
  default:         stat = (HRESULT)-1;
  }
  if (stat != HR_NOERROR)
    TXAuto::Raise(TXAuto::xConversionFailure);
  return TUString::Create(v, false, LANGIDFROMLCID(lcid));
}

//
// Convert TAutoVal to string, high performance if already of string type
//
TAutoVal::operator owl::tstring()
{
  BSTR v;
  HRESULT stat;
  LCID lcid = MAKELCID(LangUserDefault, SORT_DEFAULT);

  switch (vt) {
  case atByte:     stat = ::VarBstrFromUI1(bVal, lcid,0, &v); break;
  case atShort:    stat = ::VarBstrFromI2(iVal,  lcid,0, &v); break;
  case atLong:     stat = ::VarBstrFromI4(lVal,  lcid,0, &v); break;
  case atFloat:    stat = ::VarBstrFromR4(fltVal,lcid,0, &v); break;
  case atDouble:   stat = ::VarBstrFromR8(dblVal,lcid,0, &v); break;
  case atCurrency: stat = ::VarBstrFromCy(cyVal ,lcid,0, &v); break;
  case atDatetime: stat = ::VarBstrFromDate(date,lcid,0, &v); break;
  case atString:   return owl::tstring(TString(bstrVal));
  case atObject:           stat = ::VarBstrFromDisp(pdispVal,GetLocale(),0, &v); break;
  case atBool:             stat = ::VarBstrFromBool(boolVal,lcid,0, &v); break;
  case atByRef+atByte:     stat = ::VarBstrFromUI1(*pbVal, lcid,0, &v); break;
  case atByRef+atShort:    stat = ::VarBstrFromI2(*piVal,  lcid,0, &v); break;
  case atByRef+atLong:     stat = ::VarBstrFromI4(*plVal,  lcid,0, &v); break;
  case atByRef+atFloat:    stat = ::VarBstrFromR4(*pfltVal,lcid,0, &v); break;
  case atByRef+atDouble:   stat = ::VarBstrFromR8(*pdblVal,lcid,0, &v); break;
  case atByRef+atCurrency: stat = ::VarBstrFromCy(*pcyVal ,lcid,0, &v); break;
  case atByRef+atDatetime: stat = ::VarBstrFromDate(*pdate,lcid,0, &v); break;
  case atByRef+atString:   return owl::tstring(TString(*pbstrVal));
  case atByRef+atBool:     stat = ::VarBstrFromBool(*pbool,lcid,0,&v); break;
  default:         stat = (HRESULT)-1;
  }
  if (stat != HR_NOERROR)
    TXAuto::Raise(TXAuto::xConversionFailure);
  owl::tstring s((TString)v);
  ::SysFreeString(v);
  return s;
}


//
//
TAutoVal::operator TAutoString()
{
  TAutoString str(*this);
  return str;
}

//
//
//
TAutoVal::operator TString()
{
  return operator owl::tstring();
}

//
//
//
TAutoVal::operator unsigned short()
{
  long temp = operator long();
  if (temp < 0 || temp > long(USHRT_MAX))
    TXAuto::Raise(TXAuto::xConversionFailure);
  return (unsigned short)temp;
}

//
// Pointer data type conversion operators
//
TAutoVal::operator unsigned char* ()
{
  ConvRef(atByte);
  return pbVal;
}

//
//
TAutoVal::operator short* ()
{
  ConvRef(atShort);
  return piVal;
}

//
//
TAutoVal::operator unsigned short* ()
{
  ConvRef(atShort);
  return (unsigned short*)piVal;    // NOTE: May overflow!
}

//
//
TAutoVal::operator long* ()
{
  ConvRef(atLong);
  return plVal;
}

//
//
TAutoVal::operator unsigned long* ()
{
  ConvRef(atLong);
  return (unsigned long*)plVal;     // NOTE: May overflow!
}

//
//
TAutoVal::operator TBool* ()
{
  ConvRef(atBool);
  return (TBool*)pbool;
}

//
//
TAutoVal::operator float* ()
{
  ConvRef(atFloat);
  return pfltVal;
}

//
//
TAutoVal::operator double* ()
{
  ConvRef(atDouble);
  return pdblVal;
}

//
//
TAutoVal::operator void* ()
{
  ConvRef(atVoid);
  return byref;
}

//
//
TAutoVal::operator TAutoDate* ()
{
  ConvRef(atDatetime);
  return (TAutoDate*)pdate;
}

//
//
TAutoVal::operator TAutoCurrency* ()
{
  ConvRef(atCurrency);
  return pcyVal;
}

//
//
void TAutoVal::ConvRef(int type)
{
// !BB This code looks like it should read
// !BB
// !BB
#if 1
  if (vt != (type | atByRef)) { // !BB New code
#else
  if (type != (vt | atByRef)) { // !BB Old code
#endif
    TXAuto::Raise(TXAuto::xConversionFailure);
  }
}

//
//
TAutoVal::operator IDispatch*()
{
  if (vt == atObject) {
    if (pdispVal)
      pdispVal->AddRef();
    return pdispVal;
  }
  if (vt == (atObject | atByRef)) {
    if (*ppdispVal)
      (*ppdispVal)->AddRef();
    return *ppdispVal;
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
//
TAutoVal::operator IPictureDisp*()
{
  return STATIC_CAST(IPictureDisp*, operator IDispatch*());
}

//
//
//
TAutoVal::operator IFontDisp*()
{
  return STATIC_CAST(IFontDisp*, operator IDispatch*());
}

//
//
TAutoVal::operator IUnknown*()
{
  if (vt == atUnknown) {
    if (punkVal)
      punkVal->AddRef();
    return punkVal;
  }
  if (vt == (atUnknown | atByRef)) {
    if (*ppunkVal)
      (*ppunkVal)->AddRef();
    return *ppunkVal;
  }
  if(vt == atObject){
    if(pdispVal)
      pdispVal->AddRef();
    return pdispVal;
  }
  if (vt == (atObject | atByRef)) {
    if (*ppdispVal)
      (*ppdispVal)->AddRef();
    return *ppdispVal;
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

//
//
TAutoVal::operator IUnknown&()
{
  if (vt == atUnknown && punkVal)
    return *punkVal;
  if (vt == (atUnknown | atByRef) && *ppunkVal)
    return **ppunkVal;
  TXAuto::Raise(TXAuto::xConversionFailure);
  return *punkVal; // for stupid VC++
}

//
//
TAutoVal::operator IDispatch&()
{
  if (vt == atObject && pdispVal)
    return *pdispVal;
  if (vt == (atObject | atByRef) && *ppdispVal)
    return **ppdispVal;
  TXAuto::Raise(TXAuto::xConversionFailure);
  return *pdispVal;// for stupid VC++
}

//
//
TAutoVal::operator IPictureDisp&()
{
  return STATIC_CAST(IPictureDisp&, operator IDispatch&());
}

//
//
TAutoVal::operator IFontDisp&()
{
  return STATIC_CAST(IFontDisp&, operator IDispatch&());
}


// The following operators are added to complement the functionality of TAutoVal

TAutoVal::operator BSTR()
{
  BSTR v;
  HRESULT stat;
  LCID lcid = MAKELCID(LangUserDefault, SORT_DEFAULT);

  switch (vt) {
  case atByte:     stat = ::VarBstrFromUI1 (bVal,   lcid, 0, &v); break;
  case atShort:    stat = ::VarBstrFromI2  (iVal,   lcid, 0, &v); break;
  case atLong:     stat = ::VarBstrFromI4  (lVal,   lcid, 0, &v); break;
  case atFloat:    stat = ::VarBstrFromR4  (fltVal, lcid, 0, &v); break;
  case atDouble:   stat = ::VarBstrFromR8  (dblVal, lcid, 0, &v); break;
  case atCurrency: stat = ::VarBstrFromCy  (cyVal,  lcid, 0, &v); break;
  case atDatetime: stat = ::VarBstrFromDate(date,   lcid, 0, &v); break;
  case atString:   return ::SysAllocString (bstrVal);
  case atObject:   stat = ::VarBstrFromDisp(pdispVal, GetLocale(),    0, &v); break;
  case atBool:     stat = ::VarBstrFromBool(boolVal,  lcid,           0, &v); break;
  case atByRef + atByte:     stat = ::VarBstrFromUI1 (*pbVal,   lcid, 0, &v); break;
  case atByRef + atShort:    stat = ::VarBstrFromI2  (*piVal,   lcid, 0, &v); break;
  case atByRef + atLong:     stat = ::VarBstrFromI4  (*plVal,   lcid, 0, &v); break;
  case atByRef + atFloat:    stat = ::VarBstrFromR4  (*pfltVal, lcid, 0, &v); break;
  case atByRef+atDouble:     stat = ::VarBstrFromR8  (*pdblVal, lcid, 0, &v); break;
  case atByRef + atCurrency: stat = ::VarBstrFromCy  (*pcyVal,  lcid, 0, &v); break;
  case atByRef + atDatetime: stat = ::VarBstrFromDate(*pdate,   lcid, 0, &v); break;
  case atByRef + atString:   return ::SysAllocString (*pbstrVal);
  case atByRef + atBool:     stat = ::VarBstrFromBool(*pbool,   lcid, 0, &v); break;
  default: stat = (HRESULT) - 1;
  }

  if (stat != HR_NOERROR)
    TXAuto::Raise(TXAuto::xConversionFailure);

  return v;
}

TAutoVal::operator BSTR*()
{
  ConvRef(atString);
  return pbstrVal;
}

TAutoVal::operator TBSTR*()
{
  ConvRef(atString);
  return (TBSTR*)pbstrVal;
}

TAutoVal::operator SAFEARRAY*()
{
  if (vt & atSafeArray)
    return((vt & atByRef) ? *pparray : parray);
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

TAutoVal::operator IUnknown**()
{
  if (vt == atUnknown) {
    punkVal->AddRef();
    return &punkVal;
  }
  if (vt == (atUnknown | atByRef)) {
    (*ppunkVal)->AddRef();
    return ppunkVal;
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

TAutoVal::operator IDispatch**()
{
  if (vt == atObject) {
    pdispVal->AddRef();
    return &pdispVal;
  }
  if (vt == (atObject | atByRef)) {
    (*ppdispVal)->AddRef();
    return ppdispVal;
  }
  TXAuto::Raise(TXAuto::xConversionFailure);
  return 0;
}

TAutoVal::operator TAutoVal*()
{
  return this;
}

TAutoVal::operator LARGE_INTEGER()
{
  if (vt == VT_I8)
    return hVal;
  TXAuto::Raise(TXAuto::xConversionFailure);

  // To prevent warning
  LARGE_INTEGER dummy = {0,0};
  return dummy;
}

TAutoVal::operator ULARGE_INTEGER()
{
  if (vt == VT_UI8)
    return uhVal;
  TXAuto::Raise(TXAuto::xConversionFailure);

  // To prevent warning
  ULARGE_INTEGER dummy = {0,0};
  return dummy;
}

#if defined(_WIN64)
TAutoVal::operator ULONG_PTR()
{
  if (vt == VT_UI8)
    return uptrVal;
  TXAuto::Raise(TXAuto::xConversionFailure);

  // To prevent warning
  ULONG_PTR dummy = 0;
  return dummy;
}
#endif
//----------------------------------------------------------------------------
// Localized point to throw a TXAuto exception
//
_TCHAR*
GetAutoExceptionMsg(TXAuto::TError err)
{
  //Jogy - there is no much sense in using a static TAPointer, better to use simple static buffer, or return a tstring
  static TAPointer<_TCHAR> errMsg(new _TCHAR[60]);
  wsprintf(errMsg, _T("Automation Exception, TXAuto::ErrorCode=%d"), err);
  return errMsg;
}

//
//
TXAuto::TXAuto(TXAuto::TError err)
       :TXBase(GetAutoExceptionMsg(err)), ErrorCode(err)
{
}

void
TXAuto::Throw()
{
  throw *this;
}

void
TXAuto::Raise(TXAuto::TError err)
{
  TXAuto(err).Throw();
}

} // OCF namespace

//==============================================================================

