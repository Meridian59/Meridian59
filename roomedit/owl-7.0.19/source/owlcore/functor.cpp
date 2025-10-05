//-----------------------------------------------------------------------------
// Visual Database Tools
// Copyright (c) 1996 by Borland International, All Rights Reserved
//
/// \file
/// C++ Functor template implementation
///
/// Based on Functors described in
///  Rich Hickey's 'Callbacks in C++ using template functors' Feb 95 C++ Report
//
//-----------------------------------------------------------------------------

#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/functor.h>

namespace owl {

//
// Functor base class constructor implentation
//
TFunctorBase::TFunctorBase(const void* c, const void* f, size_t sz)
{
  if (c) {
    Callee = const_cast<void*>(c);
    memcpy(MemberFunc, f, sz);
  }
  else {
    Func = f;
  }
}

} // OWL namespace

