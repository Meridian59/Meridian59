//------------------------------------------------------------------------------
// File: project.h
//
// Desc: DirectShow sample code - main header file for Renderless player
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "app.h"
#include "vcdplyer.h"
#include "resource.h"


#ifndef __RELEASE_DEFINED
#define __RELEASE_DEFINED
template<typename T>
__inline void RELEASE( T* &p )
{
    if( p ) {
        p->Release();
        p = NULL;
    }
}
#endif

#ifndef CHECK_HR
    #define CHECK_HR(expr) { if (FAILED(expr)) __leave; };
#endif

