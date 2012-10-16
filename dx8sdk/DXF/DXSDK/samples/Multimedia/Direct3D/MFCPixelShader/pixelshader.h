//-----------------------------------------------------------------------------
// File: D3DApp.h
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
    HWND                    m_hwndRenderWindow;
    HWND                    m_hwndRenderFullScreen;
    LPDIRECT3DTEXTURE8      m_pTexture0;
    LPDIRECT3DTEXTURE8      m_pTexture1;
    LPDIRECT3DVERTEXBUFFER8 m_pQuadVB;
    DWORD                   m_hPixelShader;
    LPD3DXBUFFER            m_pD3DXBufShader;

    VOID UpdateUIForInstruction( INT iInstruction );
    VOID    UpdateUIForDeviceCapabilites();
    VOID    DisablePixelShaderEditingControls();
    VOID    GeneratePixelShaderOpcodes();
    HRESULT SetPixelShader( TCHAR* strOpcodes );
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
    afx_msg void OnPresets();
    afx_msg void OnOpenPixelShaderFile();
    afx_msg void OnPixelShaderEdit();
	afx_msg void OnChangeInstructions();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()



};




//-----------------------------------------------------------------------------
// Pixel shader instructions
//-----------------------------------------------------------------------------
struct PIXELSHADER_OPCODE
{
    DWORD  opcode;
    DWORD  dwNumArgs;
    BOOL   bInstructionSupportsScaling;
    BOOL   bInstructionSupportsBias;
    BOOL   bInputsSupportModifiers;
    BOOL   bUsesTextureRegisters;
    TCHAR* optext;
    TCHAR* description;
};


struct PIXELSHADER_PARAMETER
{
    DWORD  opcode;
    TCHAR* optext;
    TCHAR* description;
};

#define D3DSIO_PS  0x00010000 // for specifying PixelShader version

// The pixel shader instructions
PIXELSHADER_OPCODE g_PixelShaderInstructions[] =
{ //    opcode          numArgs Scaling Bias InputMods Tex  opText         Description
    {   D3DSIO_NOP,          0, FALSE, FALSE, FALSE, FALSE, "nop",         "No operation" },
    {   D3DSIO_PS,           2, FALSE, FALSE, FALSE, FALSE, "ps",          "Pixel Shader Version" },
    {   D3DSIO_DEF,          5, FALSE, FALSE, FALSE, FALSE, "def",         "Define Constant" },
    {   D3DSIO_MOV,          2,  TRUE,  TRUE,  TRUE, FALSE, "mov",         "Move" },
    {   D3DSIO_ADD,          3,  TRUE,  TRUE,  TRUE, FALSE, "add",         "Add" },
    {   D3DSIO_SUB,          3,  TRUE,  TRUE,  TRUE, FALSE, "sub",         "Subtract" },
    {   D3DSIO_MAD,          4,  TRUE,  TRUE,  TRUE, FALSE, "mad",         "Multiply and add" },
    {   D3DSIO_MUL,          3,  TRUE,  TRUE,  TRUE, FALSE, "mul",         "Multiply" },
    {   D3DSIO_DP3,          3,  TRUE,  TRUE,  TRUE, FALSE, "dp3",         "DotProduct3" },
    {   D3DSIO_LRP,          4,  TRUE,  TRUE,  TRUE, FALSE, "lrp",         "Interpolate (blend)" },
    {   D3DSIO_CND,          4,  TRUE,  TRUE,  TRUE, FALSE, "cnd",         "Conditional" },
    {   D3DSIO_TEXCOORD,     1, FALSE, FALSE, FALSE,  TRUE, "texcoord",    "Texture coord ???" },
    {   D3DSIO_TEXKILL,      1, FALSE, FALSE, FALSE,  TRUE, "texkill",     "Texture kill ???" },
    {   D3DSIO_TEX,          1, FALSE, FALSE, FALSE,  TRUE, "tex",         "Texture" },
    {   D3DSIO_TEXBEM,       2, FALSE, FALSE, FALSE,  TRUE, "texbem",      "Bump env map" },
    {   D3DSIO_TEXBEML,      2, FALSE, FALSE, FALSE,  TRUE, "texbeml",     "Bump env map w/luminance" },
    {   D3DSIO_TEXREG2AR,    2, FALSE, FALSE, FALSE,  TRUE, "texreg2ar",   "???" },
    {   D3DSIO_TEXREG2GB,    2, FALSE, FALSE, FALSE,  TRUE, "texreg2gb",   "???" },
    {   D3DSIO_TEXM3x2PAD,   2, FALSE, FALSE, FALSE,  TRUE, "texm3x2pad",  "3x2 ???" },
    {   D3DSIO_TEXM3x2TEX,   2, FALSE, FALSE, FALSE,  TRUE, "texm3x2tex",  "3x2 ???" },
    {   D3DSIO_TEXM3x3PAD,   2, FALSE, FALSE, FALSE,  TRUE, "texm3x3pad",  "3x3 ???" },
    {   D3DSIO_TEXM3x3TEX,   2, FALSE, FALSE, FALSE,  TRUE, "texm3x3tex",  "3x3 ???" },
    {   D3DSIO_TEXM3x3DIFF,  2, FALSE, FALSE, FALSE,  TRUE, "texm3x3diff", "3x3 ???" },
    {   D3DSIO_TEXM3x3SPEC,  3, FALSE, FALSE, FALSE,  TRUE, "texm3x3spec", "3x3 ???" },
    {   D3DSIO_TEXM3x3VSPEC, 2, FALSE, FALSE, FALSE,  TRUE, "texm3x3vspec","3x3 ???" },
    {   D3DSIO_END,          0, FALSE, FALSE, FALSE, FALSE,  NULL,                NULL },
};




// The source parameters for the pixel shader instructions
PIXELSHADER_PARAMETER g_PixelShaderParameters[] =
{
    {   D3DS_SR0,  "r0", "Register 0" },
    {   D3DS_SR1,  "r1", "Register 1" },
    {   D3DS_SR2,  "r2", "Register 2" },
    {   D3DS_SR3,  "r3", "Register 3" },
    {   D3DS_SR4,  "r4", "Register 4" },
    {   D3DS_SR5,  "r5", "Register 5" },
    {   D3DS_SR6,  "r6", "Register 6" },
    {   D3DS_SR7,  "r7", "Register 7" },
    {   D3DS_T0,   "t0", "Texture 0" },
    {   D3DS_T1,   "t1", "Texture 1" },
    {   D3DS_T2,   "t2", "Texture 2" },
    {   D3DS_T3,   "t3", "Texture 3" },
    {   D3DS_T4,   "t4", "Texture 4" },
    {   D3DS_T5,   "t5", "Texture 5" },
    {   D3DS_T6,   "t6", "Texture 6" },
    {   D3DS_T7,   "t7", "Texture 7" },
    {   D3DS_C0,   "c0", "Constant 0" },
    {   D3DS_C1,   "c1", "Constant 1" },
    {   D3DS_C2,   "c2", "Constant 2" },
    {   D3DS_C3,   "c3", "Constant 3" },
    {   D3DS_C4,   "c4", "Constant 4" },
    {   D3DS_C5,   "c5", "Constant 5" },
    {   D3DS_C6,   "c6", "Constant 6" },
    {   D3DS_C7,   "c7", "Constant 7" },
    {   D3DS_V0,   "v0", "Vertex color 0" },
    {   D3DS_V1,   "v1", "Vertex color 1" },
    {   D3DS_V1,   NULL, NULL },
};



// Define some preset pixel shaders
TCHAR* g_strPixelShaderPresets[10] =
{
    {
        "// Diffuse only\r\n"
        "ps.1.0\r\n"
        "mov r0, v0"
    },
    {
        "// Decal\r\n"
        "ps.1.0\r\n"
        "tex t0\r\n"
        "mov r0, t0"
    },
    {
        "// Modulate texture with diffuse (color0)\r\n"
        "ps.1.0\r\n"
        "tex t0\r\n"
        "mul r0, v0, t0"
    },
    {
        "// Blend between tex0 and tex1\r\n"
        "ps.1.0\r\n"
        "tex t0\r\n"
        "tex t1\r\n"
        "mov r1, t1\r\n"
        "lrp r0, v0, t0, r1"
    },
    {
        "// Scale tex0 by color0 and add to tex1\r\n"
        "ps.1.0\r\n"
        "tex t0\r\n"
        "tex t1\r\n"
        "mov r1, t1\r\n"
        "mad r0, v0, t0, r1"
    },
    {
        "// Add tex0, tex1, color0, and color1\r\n"
        "ps.1.0\r\n"
        "tex t0\r\n"
        "tex t1\r\n"
        "add r0, t0, v1\r\n"
        "add r0, r0, t1\r\n"
        "add r0, r0, v0"
    },
};





#endif


