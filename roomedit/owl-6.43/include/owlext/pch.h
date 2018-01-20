//----------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1998 by Bidus Yura.
// All rights reserved.
//
// Include for OWL Extensions, gets core.h or all.h when precompiled
// headers are enabled.
//----------------------------------------------------------------------------
# include <owl/pch.h>

#if defined(_OWLXPCH) && !defined(__OWLEXT_CORE_H)
#  include <owlext/core.h>
#elif !defined(__OWLEXT_ALL_H)
#  include <owlext/all.h>
#endif

