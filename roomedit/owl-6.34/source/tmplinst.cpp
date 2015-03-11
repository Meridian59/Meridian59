//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Module used to explicit expand/export templates used by ObjectWindows
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/template.h>
#include <owl/contain.h>

namespace owl {

OWL_DIAGINFO;

// Generate the necessary instances. ?????????????
#if !defined(__GNUC__) //JJH added pragma option removal
#pragma option -Jgd
#endif

#if defined(BI_COMP_BORLANDC)

/*
typedef _OWLCLASS TObjectArray<string> fake_type1;
typedef _OWLCLASS TSortedObjectArray<string> fake_type2;
typedef _OWLCLASS TObjectArray<int> fake_type4;
typedef _OWLCLASS TObjectArray<uint32> fake_type5;
*/

# if defined(_BUILDOWLDLL)
// The template instances only need to be generated when building the
// ObjectWindows DLL - These instances are exported by OWL and imported
// by user code.
template class _OWLCLASS TObjectArray<tstring>;
template class _OWLCLASS TSortedObjectArray<tstring>;
template class _OWLCLASS TObjectArray<int>;
template class _OWLCLASS TObjectArray<uint32>;
# endif
#endif

} // OWL namespace


