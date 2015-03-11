//----------------------------------------------------------------------------
// Borland Class Library
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// TDate class persistant streaming implementation
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/date.h>
#include <owl/objstrm.h>

namespace owl {

_OWLCFUNC(opstream  &) operator << ( opstream  & os, const TDate  & d )
{
    return os << d.Julnum;
}

_OWLCFUNC(ipstream  &) operator >> ( ipstream  & is, TDate  & d )
{
    return is >> d.Julnum;
}

} // OWL namespace

