//-----------------------------------------------------------------------------
// File: Fog.h
//
// Desc: Header file MFCFog sample app
//
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef FOG_H
#define FOG_H

#if _MSC_VER >= 1000
#pragma once
#endif
#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file
#endif





//-----------------------------------------------------------------------------
// Name: class CAppDoc
// Desc: Overridden CDocument class needed for the CFormView
//-----------------------------------------------------------------------------
class CAppDoc : public CDocument
{
protected:
    DECLARE_DYNCREATE(CAppDoc)

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAppDoc)
    public:
    //}}AFX_VIRTUAL

// Implementation
    //{{AFX_MSG(CAppDoc)
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};




//-----------------------------------------------------------------------------
// Name: class CAppFrameWnd
// Desc: CFrameWnd-based class needed to override the CFormView's window style
//-----------------------------------------------------------------------------
class CAppFrameWnd : public CFrameWnd
{
protected:
    DECLARE_DYNCREATE(CAppFrameWnd)
public:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAppFrameWnd)
    public:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    //}}AFX_VIRTUAL

protected:
    //{{AFX_MSG(CAppFrameWnd)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};




//-----------------------------------------------------------------------------
// Name: class CApp
// Desc: Main MFCapplication class derived from CWinApp.
//-----------------------------------------------------------------------------
class CApp : public CWinApp
{
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CApp)
    public:
    virtual BOOL InitInstance();
    virtual BOOL OnIdle( LONG );
    //}}AFX_VIRTUAL

// Implementation
    //{{AFX_MSG(CApp)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};




//-----------------------------------------------------------------------------
// Name: class CAppForm
// Desc: CFormView-based class which allows the UI to be created with a form
//       (dialog) resource. This class manages all the controls on the form.
//-----------------------------------------------------------------------------
class CAppForm : public CFormView, public CD3DApplication
{
private:
    BOOL    m_bHiResTerrain;
    BOOL    m_bHiResTerrainOld;
    DWORD   m_dwFogColor;
    DWORD   m_dwFogMode;
    BOOL    m_bCanDoTableFog;
    BOOL    m_bCanDoVertexFog;
    BOOL    m_bCanDoWFog;
    BOOL    m_bDeviceUsesWFog;
    BOOL    m_bRangeBasedFog;
    BOOL    m_bUsingTableFog;
    FLOAT   m_fFogStartSlider;
    FLOAT   m_fFogEndSlider;
    FLOAT   m_fFogStartValue;
    FLOAT   m_fFogEndValue;
    FLOAT   m_fFogDensity;
    HWND    m_hwndRenderWindow;
    HWND    m_hwndRenderFullScreen;
    LPDIRECT3DTEXTURE8      m_pFloorTexture;
    LPDIRECT3DVERTEXBUFFER8 m_pTerrainVB;
    DWORD                   m_dwNumTerrainVertices;
    LPDIRECT3DVERTEXBUFFER8 m_pColumnVB;
    DWORD                   m_dwNumColumnVertices;

    HRESULT ConfirmDevice( D3DCAPS8*,DWORD,D3DFORMAT );
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT FrameMove();
    HRESULT Render();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT FinalCleanup();
    virtual HRESULT AdjustWindowForChange();

    VOID    UpdateUIForDeviceCapabilites();
    VOID    SetFogParameters();
    HRESULT GenerateTerrainDisk( LPDIRECT3DDEVICE8 pd3dDevice, DWORD dwNumSegments,
                                 FLOAT fScale );
    HRESULT GenerateColumn( LPDIRECT3DDEVICE8 pd3dDevice, DWORD dwNumSegments,
                            FLOAT fRadius, FLOAT fHeight );


protected:
    DECLARE_DYNCREATE(CAppForm)

             CAppForm();
    virtual  ~CAppForm();

public:
    BOOL IsReady() { return m_bReady; }
    TCHAR* PstrFrameStats() { return m_strFrameStats; }
    VOID RenderScene() { Render3DEnvironment(); }
    HRESULT CheckForLostFullscreen();

    //{{AFX_DATA(CAppForm)
    enum { IDD = IDD_FORMVIEW };
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CAppForm)
    virtual void OnInitialUpdate();
    //}}AFX_VIRTUAL

    //{{AFX_MSG(CAppForm)
    afx_msg void OnToggleFullScreen();
    afx_msg void OnChangeDevice();
    afx_msg void OnHScroll( UINT, UINT, CScrollBar* );
    afx_msg void OnFogColor();
    afx_msg void OnRangeBasedFog();
    afx_msg void OnVertexFog();
    afx_msg void OnTableFog();
    afx_msg void OnFogMode();
    afx_msg void OnTerrainResolution();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};




#endif


