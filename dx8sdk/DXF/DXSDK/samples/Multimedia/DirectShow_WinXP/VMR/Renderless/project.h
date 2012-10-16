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
    #define CHECK_HR(expr) do { if (FAILED(expr)) __leave; } while(0);
#endif

// {B87BEB7B-8D29-423f-AE4D-6582C10175AC}
//DEFINE_GUID(CLSID_VideoMixingRenderer,0xb87beb7b, 0x8d29, 0x423f, 0xae, 0x4d, 0x65, 0x82, 0xc1, 0x1, 0x75, 0xac);
