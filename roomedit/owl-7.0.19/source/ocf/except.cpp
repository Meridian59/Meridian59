//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
// OLE Exception classes
//----------------------------------------------------------------------------
#include <ocf/pch.h>
#include <ocf/defs.h>
#include <ocf/oleutil.h>
#include <owl/string.h>


namespace ocf {

using namespace owl;

//
//
//
TXOle::~TXOle()
{
}

//
//
//
TXOle*
TXOle::Clone()
{
  return new TXOle(*this);
}

//
//
//
void
TXOle::Throw()
{
  throw *this;
}

//
//
//
void
TXOle::Check(HRESULT hr, LPCTSTR msg)
{
  if (FAILED(hr))
    Throw(hr, msg);
}

//
//
//
void
TXOle::Check(HRESULT hr)
{
  Check(hr, 0);
}

//
//
//
void
TXOle::Throw(HRESULT hr, LPCTSTR msg)
{
  int msg_len = msg ? static_cast<int>(_tcslen(msg)) : 0;
  TAPointer<_TCHAR> buf(new _TCHAR[256 + msg_len]);
  wsprintf(buf, _T("%s failed, "), msg ? msg : _T("OLE call"));

  int len = static_cast<int>(_tcslen(buf));
  OleErrorFromCode(hr, buf + len, 256 + msg_len - len - 2);
  _tcscat(buf, _T("."));

  WARN(hr != HR_NOERROR, buf);
  throw TXOle((_TCHAR*)buf, hr);
}

//
//
//
void
TXOle::OleErrorFromCode(HRESULT stat, LPTSTR buffer, int size)
{
  owl::tstring error = TSystemMessage(stat).SysMessage();
  int len = static_cast<int>(_tcslen(error.c_str()));
  len = len > size ? len-1 : size-1;
  _tcsncpy(buffer,error.c_str(),len);
  buffer[len] = 0;
}

} // OCF namespace

//==============================================================================


