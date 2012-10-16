//----------------------------------------------------------------------------
//  File:   project.h
//
//  Desc:   DirectShow sample code
//          Master header file that includes all the other header files 
//          used by the project.
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------
#include <streams.h>
#include <commdlg.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <process.h>
#include <time.h>
#include <objbase.h>
#include <ddraw.h>
#include <atlbase.h>
#include <atlconv.cpp>
#include <mmreg.h>
#include <commctrl.h>
#include <strmif.h>
#include <combase.h>
#include <stdarg.h>
#include <stdio.h>
#include <uuids.h>


#include "app.h"
#include "vcdplyer.h"
#include "ddrawsupport.h"
#include "resource.h"
#include "utils.h"


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
    #define CHECK_HR(expr) if (FAILED(expr)) {\
        OutputDebugString( hresultNameLookup(expr)+"\n");\
        DbgLog((LOG_ERROR, 0, \
                TEXT("FAILED: %s\nat Line:%d of %s"), \
                TEXT(#expr), __LINE__, TEXT(__FILE__) ));__leave; } else
#endif




