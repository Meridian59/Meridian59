//-----------------------------------------------------------------------------
// File: MFCTex.h
//
// Desc: Header file for a D3DIM app that uses MFC
//
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef D3D_MFC_APP_H
#define D3D_MFC_APP_H

#if _MSC_VER >= 1000
#pragma once
#endif
#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file
#endif

#include "resource.h"




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
    CD3DFont*  m_pFont;              // Font for drawing text
    HWND       m_hwndRenderWindow;
    HWND       m_hwndRenderFullScreen;

    TCHAR      m_strFileName[MAX_PATH];
    TCHAR      m_strInitialDir[MAX_PATH];

    CComboBox *m_pTex0ColorArg1, *m_pTex0ColorOp, *m_pTex0ColorArg2;
    CComboBox *m_pTex0AlphaArg1, *m_pTex0AlphaOp, *m_pTex0AlphaArg2;
    CComboBox *m_pTex1ColorArg1, *m_pTex1ColorOp, *m_pTex1ColorArg2;
    CComboBox *m_pTex1AlphaArg1, *m_pTex1AlphaOp, *m_pTex1AlphaArg2;
    CComboBox *m_pTex2ColorArg1, *m_pTex2ColorOp, *m_pTex2ColorArg2;
    CComboBox *m_pTex2AlphaArg1, *m_pTex2AlphaOp, *m_pTex2AlphaArg2;

    PDIRECT3DVERTEXBUFFER8 m_pVBWalls;
    PDIRECT3DVERTEXBUFFER8 m_pVBFloorCeiling;

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
    VOID    InitializeUIControls();
    VOID    UpdateStageColor( WORD stage, LONG op, LONG arg1, LONG arg2 );
    VOID    UpdateStageAlpha( WORD stage, LONG op, LONG arg1, LONG arg2 );
    VOID    SetTextureMaps( const TCHAR*, const TCHAR*, const TCHAR* );

public:
    BOOL IsReady() { return m_bReady; }
    TCHAR* PstrFrameStats() { return m_strFrameStats; }
    VOID RenderScene() { Render3DEnvironment(); }
    HRESULT CheckForLostFullscreen();

protected:
    DECLARE_DYNCREATE(CAppForm)

             CAppForm();
    virtual  ~CAppForm();
public:
    //{{AFX_DATA(CAppForm)
    enum { IDD = IDD_FORMVIEW };
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CAppForm)
    virtual void OnInitialUpdate();
    //}}AFX_VIRTUAL

    //{{AFX_MSG(CAppForm)
    afx_msg VOID OnToggleFullScreen();
    afx_msg VOID OnViewCode();
    afx_msg VOID OnChangeDevice();
    afx_msg VOID OnChangePresetEffects();
    afx_msg VOID OnChangeTex();
    afx_msg VOID OnSelectTexture0Name();
    afx_msg VOID OnSelectTexture1Name();
    afx_msg VOID OnSelectTexture2Name();
    afx_msg VOID OnChangeBlendFactor();
    afx_msg VOID OnChangeDiffuseColor();
    afx_msg VOID OnChangeStageArgs();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};




#endif


