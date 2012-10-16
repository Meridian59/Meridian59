//-----------------------------------------------------------------------------
// File: MFCForm.cpp
//
// Desc: File for the dialog form for the MFC multi-texture demo.
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "MFCTex.h"
#include "TexArgs.h"




//-----------------------------------------------------------------------------
// Global data and objects
//-----------------------------------------------------------------------------
extern CApp g_App;




//-----------------------------------------------------------------------------
// External storage of the texture stage states
//-----------------------------------------------------------------------------
extern WORD  g_wT0COp,   g_wT1COp,   g_wT2COp;
extern WORD  g_wT0CArg1, g_wT1CArg1, g_wT2CArg1;
extern WORD  g_wT0CArg2, g_wT1CArg2, g_wT2CArg2;
extern WORD  g_wT0AOp,   g_wT1AOp,   g_wT2AOp;
extern WORD  g_wT0AArg1, g_wT1AArg1, g_wT2AArg1;
extern WORD  g_wT0AArg2, g_wT1AArg2, g_wT2AArg2;
extern DWORD g_dwTextureFactor;
extern DWORD g_dwDiffuseColor;
extern DWORD g_dwMaxTextureBlendStages ;


VOID CAppForm::InitializeUIControls()
{
    
    // Get ptrs to the combo box controls for texture stage state UI
    m_pTex0ColorArg1 = (CComboBox*)GetDlgItem( IDC_TEX0_COLORARG1 );
    m_pTex0ColorOp   = (CComboBox*)GetDlgItem( IDC_TEX0_COLOROP );
    m_pTex0ColorArg2 = (CComboBox*)GetDlgItem( IDC_TEX0_COLORARG2 );
    m_pTex0AlphaArg1 = (CComboBox*)GetDlgItem( IDC_TEX0_ALPHAARG1 );
    m_pTex0AlphaOp   = (CComboBox*)GetDlgItem( IDC_TEX0_ALPHAOP );
    m_pTex0AlphaArg2 = (CComboBox*)GetDlgItem( IDC_TEX0_ALPHAARG2 );
    m_pTex1ColorArg1 = (CComboBox*)GetDlgItem( IDC_TEX1_COLORARG1 );
    m_pTex1ColorOp   = (CComboBox*)GetDlgItem( IDC_TEX1_COLOROP );
    m_pTex1ColorArg2 = (CComboBox*)GetDlgItem( IDC_TEX1_COLORARG2 );
    m_pTex1AlphaArg1 = (CComboBox*)GetDlgItem( IDC_TEX1_ALPHAARG1 );
    m_pTex1AlphaOp   = (CComboBox*)GetDlgItem( IDC_TEX1_ALPHAOP );
    m_pTex1AlphaArg2 = (CComboBox*)GetDlgItem( IDC_TEX1_ALPHAARG2 );
    m_pTex2ColorArg1 = (CComboBox*)GetDlgItem( IDC_TEX2_COLORARG1 );
    m_pTex2ColorOp   = (CComboBox*)GetDlgItem( IDC_TEX2_COLOROP );
    m_pTex2ColorArg2 = (CComboBox*)GetDlgItem( IDC_TEX2_COLORARG2 );
    m_pTex2AlphaArg1 = (CComboBox*)GetDlgItem( IDC_TEX2_ALPHAARG1 );
    m_pTex2AlphaOp   = (CComboBox*)GetDlgItem( IDC_TEX2_ALPHAOP );
    m_pTex2AlphaArg2 = (CComboBox*)GetDlgItem( IDC_TEX2_ALPHAARG2 );

    GetDlgItem( IDC_TEX0_NAME )->SetWindowText( _T("env2.bmp") );
    GetDlgItem( IDC_TEX1_NAME )->SetWindowText( _T("spotlite.bmp") );
    GetDlgItem( IDC_TEX2_NAME )->SetWindowText( _T("env3.bmp") );
    OnChangeTex();

    GetDlgItem( IDC_BLEND_FACTOR )->SetWindowText( _T("80808080") );
    OnChangeBlendFactor();

    GetDlgItem( IDC_DIFFUSE_COLOR )->SetWindowText( _T("ff0000ff") );
    OnChangeDiffuseColor();

    for( TCHAR** pstrTexOps = g_astrTextureOps; *pstrTexOps; pstrTexOps++ )
    {
        m_pTex0ColorOp->AddString( *pstrTexOps );
        m_pTex0AlphaOp->AddString( *pstrTexOps );
        m_pTex1ColorOp->AddString( *pstrTexOps );
        m_pTex1AlphaOp->AddString( *pstrTexOps );
        m_pTex2ColorOp->AddString( *pstrTexOps );
        m_pTex2AlphaOp->AddString( *pstrTexOps );
    }

    for( TCHAR** pstrArgs1 = g_astrTextureArgs1; *pstrArgs1; pstrArgs1++ )
    {
        m_pTex0ColorArg1->AddString( *pstrArgs1 );
        m_pTex0AlphaArg1->AddString( *pstrArgs1 );
        m_pTex1ColorArg1->AddString( *pstrArgs1 );
        m_pTex1AlphaArg1->AddString( *pstrArgs1 );
        m_pTex2ColorArg1->AddString( *pstrArgs1 );
        m_pTex2AlphaArg1->AddString( *pstrArgs1 );
    }

    for( TCHAR** pstrArgs2 = g_astrTextureArgs2; *pstrArgs2; pstrArgs2++ )
    {
        m_pTex0ColorArg2->AddString( *pstrArgs2 );
        m_pTex0AlphaArg2->AddString( *pstrArgs2 );
        m_pTex1ColorArg2->AddString( *pstrArgs2 );
        m_pTex1AlphaArg2->AddString( *pstrArgs2 );
        m_pTex2ColorArg2->AddString( *pstrArgs2 );
        m_pTex2AlphaArg2->AddString( *pstrArgs2 );
    }

    for( TCHAR** pstrPresets = g_astrPresetEffects; *pstrPresets; pstrPresets++ )
    {
        ((CComboBox*)GetDlgItem( IDC_PRESET_EFFECTS ))->AddString( *pstrPresets );
    }
    OnChangePresetEffects();

    // Store the initial media directory (for loading textures)
    strcpy( m_strInitialDir, DXUtil_GetDXSDKMediaPath() );
}




//-----------------------------------------------------------------------------
// Name: UpdateUIForDeviceCapabilites()
// Desc: Whenever we get a new device, call this function to enable/disable the
//       appropiate UI controls to match the device's capabilities.
//-----------------------------------------------------------------------------
VOID CAppForm::UpdateUIForDeviceCapabilites()
{
    g_dwMaxTextureBlendStages = (DWORD)m_d3dCaps.MaxTextureBlendStages;

    m_pTex1ColorOp->EnableWindow( g_dwMaxTextureBlendStages>1 ? TRUE : FALSE );
    m_pTex1AlphaOp->EnableWindow( g_dwMaxTextureBlendStages>1 ? TRUE : FALSE );
    m_pTex1ColorArg1->EnableWindow( g_dwMaxTextureBlendStages>1 ? TRUE : FALSE );
    m_pTex1ColorArg2->EnableWindow( g_dwMaxTextureBlendStages>1 ? TRUE : FALSE );
    m_pTex1AlphaArg1->EnableWindow( g_dwMaxTextureBlendStages>1 ? TRUE : FALSE );
    m_pTex1AlphaArg2->EnableWindow( g_dwMaxTextureBlendStages>1 ? TRUE : FALSE );

    m_pTex2ColorOp->EnableWindow( g_dwMaxTextureBlendStages>2 ? TRUE : FALSE );
    m_pTex2AlphaOp->EnableWindow( g_dwMaxTextureBlendStages>2 ? TRUE : FALSE );
    m_pTex2ColorArg1->EnableWindow( g_dwMaxTextureBlendStages>2 ? TRUE : FALSE );
    m_pTex2ColorArg2->EnableWindow( g_dwMaxTextureBlendStages>2 ? TRUE : FALSE );
    m_pTex2AlphaArg1->EnableWindow( g_dwMaxTextureBlendStages>2 ? TRUE : FALSE );
    m_pTex2AlphaArg2->EnableWindow( g_dwMaxTextureBlendStages>2 ? TRUE : FALSE );
}




//-----------------------------------------------------------------------------
// Name: OnChangeTex()
// Desc: When the user hits a texture change button
//-----------------------------------------------------------------------------
VOID CAppForm::OnChangeTex()
{
    CString strName0, strName1, strName2;

    ((CEdit*)GetDlgItem( IDC_TEX0_NAME ))->GetWindowText( strName0 );
    ((CEdit*)GetDlgItem( IDC_TEX1_NAME ))->GetWindowText( strName1 );
    ((CEdit*)GetDlgItem( IDC_TEX2_NAME ))->GetWindowText( strName2 );

    SetTextureMaps( (LPCTSTR)strName0, (LPCTSTR)strName1, (LPCTSTR)strName2 );
}



//-----------------------------------------------------------------------------
// Name: OnSelectTexture0Name()
// Desc: When the user hits a texture change button
//-----------------------------------------------------------------------------
VOID CAppForm::OnSelectTexture0Name()
{
    TCHAR strTextureName[MAX_PATH] = _T("");

    // Display the OpenFileName dialog. Then, try to load the specified file
    OPENFILENAME ofn = { sizeof(OPENFILENAME), NULL, NULL,
                         _T("Texture Files (.bmp;.tga)\0*.bmp;*.tga\0\0"), 
                         NULL, 0, 1, strTextureName, MAX_PATH, m_strFileName, MAX_PATH, 
                         m_strInitialDir, _T("Open Texture File"), 
						 OFN_FILEMUSTEXIST, 0, 1, NULL, 0, NULL, NULL };

    if( TRUE == GetOpenFileName( &ofn ) )
    {
        GetDlgItem(IDC_TEX0_NAME)->SetWindowText( m_strFileName );
        OnChangeTex();
    }
}




//-----------------------------------------------------------------------------
// Name: OnSelectTexture1Name()
// Desc: When the user hits a texture change button
//-----------------------------------------------------------------------------
VOID CAppForm::OnSelectTexture1Name()
{
    TCHAR strTextureName[MAX_PATH] = _T("");

    // Display the OpenFileName dialog. Then, try to load the specified file
    OPENFILENAME ofn = { sizeof(OPENFILENAME), NULL, NULL,
                         _T("Texture Files (.bmp;.tga)\0*.bmp;*.tga\0\0"), 
                         NULL, 0, 1, strTextureName, MAX_PATH, m_strFileName, MAX_PATH, 
                         m_strInitialDir, _T("Open Texture File"), 
						 OFN_FILEMUSTEXIST, 0, 1, NULL, 0, NULL, NULL };

    if( TRUE == GetOpenFileName( &ofn ) )
    {
        GetDlgItem(IDC_TEX1_NAME)->SetWindowText( m_strFileName );
        OnChangeTex();
    }
}




//-----------------------------------------------------------------------------
// Name: OnSelectTexture2Name()
// Desc: When the user hits a texture change button
//-----------------------------------------------------------------------------
VOID CAppForm::OnSelectTexture2Name()
{
    TCHAR strTextureName[MAX_PATH] = _T("");

    // Display the OpenFileName dialog. Then, try to load the specified file
    OPENFILENAME ofn = { sizeof(OPENFILENAME), NULL, NULL,
                         _T("Texture Files (.bmp;.tga)\0*.bmp;*.tga\0\0"), 
                         NULL, 0, 1, strTextureName, MAX_PATH, m_strFileName, MAX_PATH, 
                         m_strInitialDir, _T("Open Texture File"), 
						 OFN_FILEMUSTEXIST, 0, 1, NULL, 0, NULL, NULL };

    if( TRUE == GetOpenFileName( &ofn ) )
    {
        GetDlgItem(IDC_TEX2_NAME)->SetWindowText( m_strFileName );
        OnChangeTex();
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateStageColor()
// Desc: Updates a stage's ColorOp parameters in the form's UI.
//-----------------------------------------------------------------------------
VOID CAppForm::UpdateStageColor( WORD stage, LONG op, LONG arg1, LONG arg2 )
{
    WORD wListID = IDC_TEX0_COLOROP;

    if( stage == 1 )
        wListID = IDC_TEX1_COLOROP;
    if( stage == 2 )
        wListID = IDC_TEX2_COLOROP;

    ((CComboBox*)GetDlgItem( wListID+0 ))->SetCurSel( op );
    ((CComboBox*)GetDlgItem( wListID+1 ))->SetCurSel( arg1 );
    ((CComboBox*)GetDlgItem( wListID+2 ))->SetCurSel( arg2 );
}




//-----------------------------------------------------------------------------
// Name: UpdateStageAlpha()
// Desc: Updates a stage's AlphaOp parameters in the form's UI.
//-----------------------------------------------------------------------------
VOID CAppForm::UpdateStageAlpha( WORD stage, LONG op, LONG arg1, LONG arg2 )
{
    WORD wListID = IDC_TEX0_ALPHAOP;

    if( stage == 1 )
        wListID = IDC_TEX1_ALPHAOP;
    if( stage == 2 )
        wListID = IDC_TEX2_ALPHAOP;

    ((CComboBox*)GetDlgItem( wListID+0 ))->SetCurSel( op );
    ((CComboBox*)GetDlgItem( wListID+1 ))->SetCurSel( arg1 );
    ((CComboBox*)GetDlgItem( wListID+2 ))->SetCurSel( arg2 );
}




//-----------------------------------------------------------------------------
// Name: OnChangePresetEffects()
// Desc: When the user selects a preset texture stage effect
//-----------------------------------------------------------------------------
void CAppForm::OnChangePresetEffects()
{
    // Clear all the combo boxes in the UI
    UpdateStageColor( 0, toDISABLE, ta1TEXTURE, ta2CURRENT );
    UpdateStageAlpha( 0, toDISABLE, ta1TEXTURE, ta2CURRENT );
    UpdateStageColor( 1, toDISABLE, ta1TEXTURE, ta2CURRENT );
    UpdateStageAlpha( 1, toDISABLE, ta1TEXTURE, ta2CURRENT );
    UpdateStageColor( 2, toDISABLE, ta1TEXTURE, ta2CURRENT );
    UpdateStageAlpha( 2, toDISABLE, ta1TEXTURE, ta2CURRENT );

    switch( ((CComboBox*)GetDlgItem( IDC_PRESET_EFFECTS ))->GetCurSel() )
    {
        case CB_ERR: // No selection
            UpdateStageColor( 0, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  0: // Modulate
            UpdateStageColor( 0, toMODULATE,   ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  1: // Modulate Alpha
            UpdateStageColor( 0, toMODULATE, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toMODULATE, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  2: // Add
            UpdateStageColor( 0, toADD,        ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  3: // Decal Alpha
            UpdateStageColor( 0, toBLENDTEXTURE, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2,   ta1TEXTURE, ta2DIFFUSE );
            break;
        case  4: // Colored light map
            UpdateStageColor( 0, toMODULATE,   ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toMODULATE,   ta1TEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  5: // Inverse colored light map
            UpdateStageColor( 0, toMODULATE,   ta1INVTEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2, ta1TEXTURE,    ta2DIFFUSE );
            UpdateStageColor( 1, toMODULATE,   ta1INVTEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  6: // Single channel light map
            UpdateStageColor( 0, toSELECTARG1, ta1TEXTURE,      ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toMODULATE,   ta1ALPHATEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  7: // Modulate and late add
            UpdateStageColor( 0, toMODULATE, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toADD,      ta1TEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  8: // Linear blend using texture alpha
            UpdateStageColor( 0, toMODULATE,     ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2,   ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toBLENDTEXTURE, ta1TEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case  9: // Linear blend using diffuse alpha
            UpdateStageColor( 0, toMODULATE,     ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2,   ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toBLENDDIFFUSE, ta1TEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case 10: // Multitexture add w/smooth saturation
            UpdateStageColor( 0, toMODULATE,   ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toADD,        ta1TEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case 11: // Multitexture subtract
            UpdateStageColor( 0, toMODULATE,   ta1TEXTURE,    ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2, ta1TEXTURE,    ta2DIFFUSE );
            UpdateStageColor( 1, toADD,        ta1INVTEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case 12: // Add Diffuse to light map then modulate
            UpdateStageColor( 0, toADD,      ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toMODULATE, ta1TEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 2, toMODULATE, ta1DIFFUSE, ta2CURRENT );
            UpdateStageAlpha( 2, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case 13: // Detail modulate
            UpdateStageColor( 0, toMODULATE,   ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toMODULATE2X, ta1TEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
        case 14: // Detail add
            UpdateStageColor( 0, toMODULATE,   ta1TEXTURE, ta2DIFFUSE );
            UpdateStageAlpha( 0, toSELECTARG2, ta1TEXTURE, ta2DIFFUSE );
            UpdateStageColor( 1, toADDSIGNED,  ta1TEXTURE, ta2CURRENT );
            UpdateStageAlpha( 1, toSELECTARG1, ta1TEXTURE, ta2DIFFUSE );
            break;
    }

    OnChangeStageArgs();
}




//-----------------------------------------------------------------------------
// Name: OnChooseStageArgs()
// Desc: When the user changes a stage arg combo box, or else when called by
//       the app, this function extracts state information from the UI, and
//       saves it in a form for the D3DDevice SetTextureStageState() calls.
//-----------------------------------------------------------------------------
void CAppForm::OnChangeStageArgs()
{
    g_wT0COp = aTexOps[ m_pTex0ColorOp->GetCurSel() + 1 ];
    g_wT0AOp = aTexOps[ m_pTex0AlphaOp->GetCurSel() + 1 ];
    g_wT1COp = aTexOps[ m_pTex1ColorOp->GetCurSel() + 1 ];
    g_wT1AOp = aTexOps[ m_pTex1AlphaOp->GetCurSel() + 1 ];
    g_wT2COp = aTexOps[ m_pTex2ColorOp->GetCurSel() + 1 ];
    g_wT2AOp = aTexOps[ m_pTex2AlphaOp->GetCurSel() + 1 ];
    if( 0 == g_wT0COp ) g_wT0COp = D3DTOP_DISABLE;
    if( 0 == g_wT0AOp ) g_wT0AOp = D3DTOP_DISABLE;
    if( 0 == g_wT1COp ) g_wT1COp = D3DTOP_DISABLE;
    if( 0 == g_wT1AOp ) g_wT1AOp = D3DTOP_DISABLE;
    if( 0 == g_wT2COp ) g_wT2COp = D3DTOP_DISABLE;
    if( 0 == g_wT2AOp ) g_wT2AOp = D3DTOP_DISABLE;

    g_wT0CArg1 = aTexArgs1[ m_pTex0ColorArg1->GetCurSel() + 1 ];
    g_wT0CArg2 = aTexArgs2[ m_pTex0ColorArg2->GetCurSel() + 1 ];
    g_wT0AArg1 = aTexArgs1[ m_pTex0AlphaArg1->GetCurSel() + 1 ];
    g_wT0AArg2 = aTexArgs2[ m_pTex0AlphaArg2->GetCurSel() + 1 ];
    g_wT1CArg1 = aTexArgs1[ m_pTex1ColorArg1->GetCurSel() + 1 ];
    g_wT1CArg2 = aTexArgs2[ m_pTex1ColorArg2->GetCurSel() + 1 ];
    g_wT1AArg1 = aTexArgs1[ m_pTex1AlphaArg1->GetCurSel() + 1 ];
    g_wT1AArg2 = aTexArgs2[ m_pTex1AlphaArg2->GetCurSel() + 1 ];
    g_wT2CArg1 = aTexArgs1[ m_pTex2ColorArg1->GetCurSel() + 1 ];
    g_wT2CArg2 = aTexArgs2[ m_pTex2ColorArg2->GetCurSel() + 1 ];
    g_wT2AArg1 = aTexArgs1[ m_pTex2AlphaArg1->GetCurSel() + 1 ];
    g_wT2AArg2 = aTexArgs2[ m_pTex2AlphaArg2->GetCurSel() + 1 ];
}






//-----------------------------------------------------------------------------
// Name: OnChangeBlendFactor()
// Desc: When the user enters a new blend factor
//-----------------------------------------------------------------------------
VOID CAppForm::OnChangeBlendFactor()
{
    CString strValue;
    (GetDlgItem( IDC_BLEND_FACTOR ))->GetWindowText( strValue );

    const TCHAR* strFactor = (LPCTSTR)strValue;
    if( (strFactor[0] == '0') && ((strFactor[1] == 'x') || (strFactor[1] == 'X')) )
        strFactor += 2;

    sscanf( strFactor, _T("%x"), &g_dwTextureFactor );
}





//-----------------------------------------------------------------------------
// Name: OnChangeDiffuseColor()
// Desc: When the user wants a new diffuse color
//-----------------------------------------------------------------------------
VOID CAppForm::OnChangeDiffuseColor()
{
    CString strValue;
    (GetDlgItem( IDC_DIFFUSE_COLOR ))->GetWindowText( strValue );

    const TCHAR* strColor = (LPCTSTR)strValue;
    if( (strColor[0] == '0') && ((strColor[1] == 'x') || (strColor[1] == 'X')) )
        strColor += 2;

    sscanf( strColor, _T("%x"), &g_dwDiffuseColor );
}





class CViewCodeDlg : public CDialog
{
public:
    CViewCodeDlg() : CDialog(IDD_VIEWCODE) {}
    BOOL OnInitDialog();
};




TCHAR* TranslateTextureOp( DWORD dwTexOp )
{
    switch( dwTexOp )
    {
        case D3DTOP_DISABLE:                return _T("DISABLE");
        case D3DTOP_SELECTARG1:             return _T("SELECTARG1");
        case D3DTOP_SELECTARG2:             return _T("SELECTARG2");
        case D3DTOP_MODULATE:               return _T("MODULATE");
        case D3DTOP_MODULATE2X:             return _T("MODULATE2X");
        case D3DTOP_MODULATE4X:             return _T("MODULATE4X");
        case D3DTOP_ADD:                    return _T("ADD");
        case D3DTOP_ADDSIGNED:              return _T("ADDSIGNED");
        case D3DTOP_ADDSIGNED2X:            return _T("ADDSIGNED2X");
        case D3DTOP_SUBTRACT:               return _T("SUBTRACT");
        case D3DTOP_ADDSMOOTH:              return _T("ADDSMOOTH");
        case D3DTOP_BLENDDIFFUSEALPHA:      return _T("BLENDDIFFUSEALPHA");
        case D3DTOP_BLENDTEXTUREALPHA:      return _T("BLENDTEXTUREALPHA");
        case D3DTOP_BLENDFACTORALPHA:       return _T("BLENDFACTORALPHA");
        case D3DTOP_BLENDTEXTUREALPHAPM:    return _T("BLENDTEXTUREALPHAPM");
        case D3DTOP_BLENDCURRENTALPHA:      return _T("BLENDCURRENTALPHA");
        case D3DTOP_PREMODULATE:            return _T("PREMODULATE");
        case D3DTOP_MODULATEALPHA_ADDCOLOR: return _T("MODULATEALPHA_ADDCOLOR");
        case D3DTOP_MODULATECOLOR_ADDALPHA: return _T("MODULATECOLOR_ADDALPHA");
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR: return _T("MODULATEINVALPHA_ADDCOLOR");
        case D3DTOP_MODULATEINVCOLOR_ADDALPHA: return _T("MODULATEINVCOLOR_ADDALPHA");
        case D3DTOP_BUMPENVMAP:             return _T("BUMPENVMAP");
        case D3DTOP_BUMPENVMAPLUMINANCE:    return _T("BUMPENVMAPLUMINANCE");
        case D3DTOP_DOTPRODUCT3:            return _T("DOTPRODUCT3");
    }
    return _T("DISABLE");
}




TCHAR* TranslateTextureArg( DWORD dwTexArg )
{
    static TCHAR buffer[256];

    switch( dwTexArg & 0x0000000f )
    {
        case D3DTA_DIFFUSE: strcpy( buffer, _T("D3DTA_DIFFUSE") );
                            break;
        case D3DTA_CURRENT: strcpy( buffer, _T("D3DTA_CURRENT") );
                            break;
        case D3DTA_TEXTURE: strcpy( buffer, _T("D3DTA_TEXTURE") );
                            break;
        case D3DTA_TFACTOR: strcpy( buffer, _T("D3DTA_TFACTOR") );
                            break;
        default:            return _T("D3DTA_DIFFUSE");
    }

    if( dwTexArg&D3DTA_COMPLEMENT )
        strcat( buffer, _T(" | D3DTA_COMPLEMENT") );
    if( dwTexArg&D3DTA_ALPHAREPLICATE )
        strcat( buffer, _T(" | D3DTA_ALPHAREPLICATE") );

    return buffer;
}




BOOL CViewCodeDlg::OnInitDialog()
{
    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIEWCODEEDITBOX);

    pEdit->FmtLines(TRUE);
    pEdit->SetWindowText( _T("howdy\015\012doody") );

    TCHAR* strTemplate = _T("   pd3dDevice->SetTextureStageState( %d, D3DTSS_%s%s );\015\012" );

    TCHAR buffer[2048];
    TCHAR strOutText[2048] = _T("");

    strcat( strOutText, _T("HRESULT SetShader( LPDIRECT3DDEVICE8 pd3dDevice )\015\012") );
    strcat( strOutText, _T("{\015\012") );

    sprintf( buffer, strTemplate, 0, _T("COLORARG1, "), TranslateTextureArg( g_wT0CArg1 ) );
    if( g_wT0COp != D3DTOP_DISABLE && g_wT0COp != D3DTOP_SELECTARG2 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 0, _T("COLOROP,   D3DTOP_"), TranslateTextureOp( g_wT0COp ) );
    strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 0, _T("COLORARG2, "), TranslateTextureArg( g_wT0CArg2 ) );
    if( g_wT0COp != D3DTOP_DISABLE && g_wT0COp != D3DTOP_SELECTARG1 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 0, _T("ALPHAARG1, "), TranslateTextureArg( g_wT0AArg1 ) );
    if( g_wT0AOp != D3DTOP_DISABLE && g_wT0AOp != D3DTOP_SELECTARG2 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 0, _T("ALPHAOP,   D3DTOP_"), TranslateTextureOp( g_wT0AOp ) );
    strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 0, _T("ALPHAARG2, "), TranslateTextureArg( g_wT0AArg2 ) );
    if( g_wT0AOp != D3DTOP_DISABLE && g_wT0AOp != D3DTOP_SELECTARG1 )
        strcat( strOutText, buffer );
    strcat( strOutText, _T("\015\012") );

    sprintf( buffer, strTemplate, 1, _T("COLORARG1, "), TranslateTextureArg( g_wT1CArg1 ) );
    if( g_wT1COp != D3DTOP_DISABLE && g_wT1COp != D3DTOP_SELECTARG2 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 1, _T("COLOROP,   D3DTOP_"), TranslateTextureOp( g_wT1COp ) );
    strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 1, _T("COLORARG2, "), TranslateTextureArg( g_wT1CArg2 ) );
    if( g_wT1COp != D3DTOP_DISABLE && g_wT1COp != D3DTOP_SELECTARG1 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 1, _T("ALPHAARG1, "), TranslateTextureArg( g_wT1AArg1 ) );
    if( g_wT1AOp != D3DTOP_DISABLE && g_wT1AOp != D3DTOP_SELECTARG2 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 1, _T("ALPHAOP,   D3DTOP_"), TranslateTextureOp( g_wT1AOp ) );
    strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 1, _T("ALPHAARG2, "), TranslateTextureArg( g_wT1AArg2 ) );
    if( g_wT1AOp != D3DTOP_DISABLE && g_wT1AOp != D3DTOP_SELECTARG1 )
        strcat( strOutText, buffer );
    strcat( strOutText, _T("\015\012") );

    sprintf( buffer, strTemplate, 2, _T("COLORARG1, "), TranslateTextureArg( g_wT2CArg1 ) );
    if( g_wT2COp != D3DTOP_DISABLE && g_wT2COp != D3DTOP_SELECTARG2 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 2, _T("COLOROP,   D3DTOP_"), TranslateTextureOp( g_wT2COp ) );
    strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 2, _T("COLORARG2, "), TranslateTextureArg( g_wT2CArg2 ) );
    if( g_wT2COp != D3DTOP_DISABLE && g_wT2COp != D3DTOP_SELECTARG1 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 2, _T("ALPHAARG1, "), TranslateTextureArg( g_wT2AArg1 ) );
    if( g_wT2AOp != D3DTOP_DISABLE && g_wT2AOp != D3DTOP_SELECTARG2 )
        strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 2, _T("ALPHAOP,   D3DTOP_"), TranslateTextureOp( g_wT2AOp ) );
    strcat( strOutText, buffer );
    sprintf( buffer, strTemplate, 2, _T("ALPHAARG2, "), TranslateTextureArg( g_wT2AArg2 ) );
    if( g_wT2AOp != D3DTOP_DISABLE && g_wT2AOp != D3DTOP_SELECTARG1 )
        strcat( strOutText, buffer );
    strcat( strOutText, _T("\015\012") );
    strcat( strOutText, _T("   DWORD dwNumPasses;\015\012") );
    strcat( strOutText, _T("   return pd3dDevice->ValidateDevice( &dwNumPasses );\015\012") );
    strcat( strOutText, _T("}\015\012") );

    pEdit->SetWindowText( strOutText );

    return CDialog::OnInitDialog();
}




//-----------------------------------------------------------------------------
// Name: OnViewCode()
// Desc: When the user hits the "view code" button
//-----------------------------------------------------------------------------
void CAppForm::OnViewCode()
{
    CViewCodeDlg dlg;
    dlg.DoModal();
}




