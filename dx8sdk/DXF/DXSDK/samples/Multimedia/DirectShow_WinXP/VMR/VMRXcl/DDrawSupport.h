//----------------------------------------------------------------------------
//  File:   DDrawSupport.h
//
//  Desc:   DirectShow sample code
//          Prototype of DDrawObject that provides basic DDraw functionality
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------

#ifndef DDRAWSUPPORT_HEADER
#define DDRAWSUPPORT_HEADER

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
        DbgLog((LOG_ERROR, 0, \
                TEXT("FAILED: %s\nat Line:%d of %s"), \
                TEXT(#expr), __LINE__, TEXT(__FILE__) ));__leave; } else
#endif


#define SCRN_WIDTH      640
#define SCRN_HEIGHT     480
#define SCRN_BITDEPTH   32

//----------------------------------------------------------------------------
//  CDDrawObject
//
//  Class provides basic DDraw functionality
//----------------------------------------------------------------------------
class CDDrawObject {

public:
    CDDrawObject() :
        m_hwndApp(NULL),
        m_pDDObject(NULL),
        m_pPrimary(NULL),
        m_pBackBuff(NULL)
    {
        ZeroMemory(&m_RectScrn, sizeof(m_RectScrn));
    }

    HRESULT Initialize(HWND hwndApp);
    HRESULT Terminate();
    LPDIRECTDRAW7 GetDDObj() {return m_pDDObject;};
    LPDIRECTDRAWSURFACE7 GetFB() {return m_pPrimary;};
    LPDIRECTDRAWSURFACE7 GetBB() {return m_pBackBuff;};

private:
    HWND                    m_hwndApp;
    RECT                    m_RectScrn;
    LPDIRECTDRAW7           m_pDDObject;
    LPDIRECTDRAWSURFACE7    m_pPrimary;
    LPDIRECTDRAWSURFACE7    m_pBackBuff;
};

#endif