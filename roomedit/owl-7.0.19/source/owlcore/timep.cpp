//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TTime class persistent streaming implementation
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/time.h>
#include <owl/objstrm.h>
#include <owl/wsyscls.h>
#include <owl/private/strmdefs.h>

namespace owl {

_OWLCFUNC(opstream  &) operator << ( opstream  & os,
                                            const TTime  & d )
{
    return os << d.Sec;
}

_OWLCFUNC(ipstream  &) operator >> ( ipstream  & is,
                                            TTime  & d )
{
    return is >> d.Sec;
}

//
_OWLCFUNC(opstream &) operator << ( opstream  & s, const TFileTime  & d ){
  s << d.dwLowDateTime << d.dwHighDateTime;
  return s;
}

//
_OWLCFUNC(ipstream &) operator >> ( ipstream  & s, TFileTime  & d )
{
  s >> d.dwLowDateTime >> d.dwHighDateTime;
  return s;
}

} // OWL namespace
/* ========================================================================== */


