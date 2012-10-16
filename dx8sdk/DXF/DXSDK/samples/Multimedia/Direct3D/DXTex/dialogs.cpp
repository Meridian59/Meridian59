// Dialogs.cpp : implementation file
//

#include "stdafx.h"
#include "dxtex.h"
#include "dialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewTextureDlg dialog


CNewTextureDlg::CNewTextureDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CNewTextureDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CNewTextureDlg)
    m_iTexType = 0;
    m_dwWidth = 256;
    m_dwHeight = 256;
    m_dwDepth = 8;
    m_iFmt = 0;
    m_strFmtDesc = _T("");
    m_numMips = 1;
    //}}AFX_DATA_INIT
}


void CNewTextureDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CNewTextureDlg)
    DDX_Radio(pDX, IDC_TEXTURE, m_iTexType);
    DDX_Text(pDX, IDC_WIDTH, m_dwWidth);
    DDV_MinMaxInt(pDX, m_dwWidth, 1, 1024);
    DDX_Text(pDX, IDC_HEIGHT, m_dwHeight);
    DDV_MinMaxInt(pDX, m_dwHeight, 1, 1024);
    DDX_Text(pDX, IDC_DEPTH, m_dwDepth);
    DDV_MinMaxInt(pDX, m_dwDepth, 2, 1024);
    DDX_Radio(pDX, IDC_A8R8G8B8, m_iFmt);
    DDX_Text(pDX, IDC_FMTDESC, m_strFmtDesc);
    DDX_Text(pDX, IDC_MIPCOUNT, m_numMips);
    DDV_MinMaxInt(pDX, m_numMips, 1, 20);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewTextureDlg, CDialog)
    //{{AFX_MSG_MAP(CNewTextureDlg)
    ON_BN_CLICKED(IDC_TEXTURE, OnChangeTextureType)
    ON_BN_CLICKED(IDC_A8R8G8B8, OnChangeFormat)
    ON_BN_CLICKED(IDC_VOLUMETEXTURE, OnChangeTextureType)
    ON_BN_CLICKED(IDC_CUBEMAP, OnChangeTextureType)
    ON_BN_CLICKED(IDC_A4R4G4B4, OnChangeFormat)
    ON_BN_CLICKED(IDC_A1R5G5B5, OnChangeFormat)
    ON_BN_CLICKED(IDC_R5G6B5, OnChangeFormat)
    ON_BN_CLICKED(IDC_R8G8B8, OnChangeFormat)
    ON_BN_CLICKED(IDC_X8R8G8B8, OnChangeFormat)
    ON_BN_CLICKED(IDC_X1R5G5B5, OnChangeFormat)
    ON_BN_CLICKED(IDC_R3G3B2, OnChangeFormat)
    ON_BN_CLICKED(IDC_A8R3G3B2, OnChangeFormat)
    ON_BN_CLICKED(IDC_X4R4G4B4, OnChangeFormat)
    ON_BN_CLICKED(IDC_DXT1, OnChangeFormat)
    ON_BN_CLICKED(IDC_DXT2, OnChangeFormat)
    ON_BN_CLICKED(IDC_DXT3, OnChangeFormat)
    ON_BN_CLICKED(IDC_DXT4, OnChangeFormat)
    ON_BN_CLICKED(IDC_DXT5, OnChangeFormat)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewTextureDlg message handlers

BOOL CNewTextureDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    OnChangeTextureType();
    OnChangeFormat();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


void CNewTextureDlg::OnChangeTextureType() 
{
    UpdateData(TRUE);

    if (m_iTexType == 2)
    {
        // Volume Tex
        GetDlgItem(IDC_VOLUMEDEPTHLABEL)->EnableWindow(TRUE);
        GetDlgItem(IDC_DEPTH)->EnableWindow(TRUE);
    }
    else
    {
        // Normal or Cube Tex
        GetDlgItem(IDC_VOLUMEDEPTHLABEL)->EnableWindow(FALSE);
        GetDlgItem(IDC_DEPTH)->EnableWindow(FALSE);
    }
    UpdateData(FALSE);
    OnChangeFormat();
}



void CNewTextureDlg::OnChangeFormat() 
{
    UpdateData(TRUE);
    switch (m_iFmt)
    {
    case 0:
        m_fmt = D3DFMT_A8R8G8B8;
        m_strFmtDesc.LoadString(IDS_FMTDESC_A8R8G8B8);
        break;
    case 1:
        m_fmt = D3DFMT_A1R5G5B5;
        m_strFmtDesc.LoadString(IDS_FMTDESC_A1R5G5B5);
        break;
    case 2:
        m_fmt = D3DFMT_A4R4G4B4;
        m_strFmtDesc.LoadString(IDS_FMTDESC_A4R4G4B4);
        break;
    case 3:
        m_fmt = D3DFMT_R8G8B8;
        m_strFmtDesc.LoadString(IDS_FMTDESC_R8G8B8);
        break;
    case 4:
        m_fmt = D3DFMT_R5G6B5;
        m_strFmtDesc.LoadString(IDS_FMTDESC_R5G6B5);
        break;
    case 5:
        m_fmt = D3DFMT_X8R8G8B8;
        m_strFmtDesc.LoadString(IDS_FMTDESC_X8R8G8B8);
        break;
    case 6:
        m_fmt = D3DFMT_X1R5G5B5;
        m_strFmtDesc.LoadString(IDS_FMTDESC_X1R5G5B5);
        break;
    case 7:
        m_fmt = D3DFMT_R3G3B2;
        m_strFmtDesc.LoadString(IDS_FMTDESC_R3G3B2);
        break;
    case 8:
        m_fmt = D3DFMT_A8R3G3B2;
        m_strFmtDesc.LoadString(IDS_FMTDESC_A8R3G3B2);
        break;
    case 9:
        m_fmt = D3DFMT_X4R4G4B4;
        m_strFmtDesc.LoadString(IDS_FMTDESC_X4R4G4B4);
        break;
    case 10:
        m_fmt = D3DFMT_DXT1;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT1);
        break;
    case 11:
        m_fmt = D3DFMT_DXT2;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT2);
        break;
    case 12:
        m_fmt = D3DFMT_DXT3;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT3);
        break;
    case 13:
        m_fmt = D3DFMT_DXT4;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT4);
        break;
    case 14:
        m_fmt = D3DFMT_DXT5;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT5);
        break;
    }
    UpdateData(FALSE);
}

void CNewTextureDlg::OnOK() 
{
    UpdateData(TRUE);

    // TODO: Need to do lots of validation of width/height/depth/mipcount here

    if (m_iTexType != 2)
        m_dwDepth = 0;

    switch (m_iFmt)
    {
    case 0:  m_fmt = D3DFMT_A8R8G8B8;   break;
    case 1:  m_fmt = D3DFMT_A1R5G5B5;   break;
    case 2:  m_fmt = D3DFMT_A4R4G4B4;   break;
    case 3:  m_fmt = D3DFMT_R8G8B8;     break;
    case 4:  m_fmt = D3DFMT_R5G6B5;     break;
    case 5:  m_fmt = D3DFMT_X8R8G8B8;   break;
    case 6:  m_fmt = D3DFMT_X1R5G5B5;   break;
    case 7:  m_fmt = D3DFMT_R3G3B2;     break;
    case 8:  m_fmt = D3DFMT_A8R3G3B2;   break;
    case 9:  m_fmt = D3DFMT_X4R4G4B4;   break;
    case 10: m_fmt = D3DFMT_DXT1;       break;
    case 11: m_fmt = D3DFMT_DXT2;       break;
    case 12: m_fmt = D3DFMT_DXT3;       break;
    case 13: m_fmt = D3DFMT_DXT4;       break;
    case 14: m_fmt = D3DFMT_DXT5;       break;
    }

    CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CCubeMapDlg dialog


CCubeMapDlg::CCubeMapDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CCubeMapDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CCubeMapDlg)
    m_iFace = 0;
    //}}AFX_DATA_INIT
}


void CCubeMapDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCubeMapDlg)
    DDX_Radio(pDX, IDC_POSX, m_iFace);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCubeMapDlg, CDialog)
    //{{AFX_MSG_MAP(CCubeMapDlg)
        // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CVolumeMapDlg dialog


CVolumeMapDlg::CVolumeMapDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CVolumeMapDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CVolumeMapDlg)
    m_powLayers = 0;
    //}}AFX_DATA_INIT
}


void CVolumeMapDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CVolumeMapDlg)
    DDX_Radio(pDX, IDC_RADIO2, m_powLayers);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVolumeMapDlg, CDialog)
    //{{AFX_MSG_MAP(CVolumeMapDlg)
        // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVolumeMapDlg message handlers


/////////////////////////////////////////////////////////////////////////////
// CChangeFmtDlg dialog


CChangeFmtDlg::CChangeFmtDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CChangeFmtDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CChangeFmtDlg)
    m_iFmt = -1;
    m_strFmtDesc = _T("");
    //}}AFX_DATA_INIT
}


void CChangeFmtDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CChangeFmtDlg)
    DDX_Radio(pDX, IDC_A8R8G8B8, m_iFmt);
    DDX_Text(pDX, IDC_FMTDESC, m_strFmtDesc);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeFmtDlg, CDialog)
    //{{AFX_MSG_MAP(CChangeFmtDlg)
    ON_BN_CLICKED(IDC_A1R5G5B5, OnChangeFmt)
    ON_BN_CLICKED(IDC_A4R4G4B4, OnChangeFmt)
    ON_BN_CLICKED(IDC_A8R8G8B8, OnChangeFmt)
    ON_BN_CLICKED(IDC_R5G6B5, OnChangeFmt)
    ON_BN_CLICKED(IDC_R8G8B8, OnChangeFmt)
    ON_BN_CLICKED(IDC_X8R8G8B8, OnChangeFmt)
    ON_BN_CLICKED(IDC_X1R5G5B5, OnChangeFmt)
    ON_BN_CLICKED(IDC_R3G3B2, OnChangeFmt)
    ON_BN_CLICKED(IDC_A8R3G3B2, OnChangeFmt)
    ON_BN_CLICKED(IDC_X4R4G4B4, OnChangeFmt)
    ON_BN_CLICKED(IDC_DXT1, OnChangeFmt)
    ON_BN_CLICKED(IDC_DXT2, OnChangeFmt)
    ON_BN_CLICKED(IDC_DXT3, OnChangeFmt)
    ON_BN_CLICKED(IDC_DXT4, OnChangeFmt)
    ON_BN_CLICKED(IDC_DXT5, OnChangeFmt)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeFmtDlg message handlers

BOOL CChangeFmtDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    switch (m_fmt)
    {
    case D3DFMT_A8R8G8B8:
        m_iFmt = 0;
        break;
    case D3DFMT_A1R5G5B5:
        m_iFmt = 1;
        break;
    case D3DFMT_A4R4G4B4:
        m_iFmt = 2;
        break;
    case D3DFMT_R8G8B8:
        m_iFmt = 3;
        break;
    case D3DFMT_R5G6B5:
        m_iFmt = 4;
        break;
    case D3DFMT_X8R8G8B8:
        m_iFmt = 5;
        break;
    case D3DFMT_X1R5G5B5:
        m_iFmt = 6;
        break;
    case D3DFMT_R3G3B2:
        m_iFmt = 7;
        break;
    case D3DFMT_A8R3G3B2:
        m_iFmt = 8;
        break;
    case D3DFMT_X4R4G4B4:
        m_iFmt = 9;
        break;
    case D3DFMT_DXT1:
        m_iFmt = 10;
        break;
    case D3DFMT_DXT2:
        m_iFmt = 11;
        break;
    case D3DFMT_DXT3:
        m_iFmt = 12;
        break;
    case D3DFMT_DXT4:
        m_iFmt = 13;
        break;
    case D3DFMT_DXT5:
        m_iFmt = 14;
        break;
    }

    UpdateFmtDesc();    

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CChangeFmtDlg::OnChangeFmt() 
{
    UpdateData(TRUE);
    UpdateFmtDesc();
}

void CChangeFmtDlg::UpdateFmtDesc()
{
    switch (m_iFmt)
    {
    case 0:
        m_fmt = D3DFMT_A8R8G8B8;
        m_strFmtDesc.LoadString(IDS_FMTDESC_A8R8G8B8);
        break;
    case 1:
        m_fmt = D3DFMT_A1R5G5B5;
        m_strFmtDesc.LoadString(IDS_FMTDESC_A1R5G5B5);
        break;
    case 2:
        m_fmt = D3DFMT_A4R4G4B4;
        m_strFmtDesc.LoadString(IDS_FMTDESC_A4R4G4B4);
        break;
    case 3:
        m_fmt = D3DFMT_R8G8B8;
        m_strFmtDesc.LoadString(IDS_FMTDESC_R8G8B8);
        break;
    case 4:
        m_fmt = D3DFMT_R5G6B5;
        m_strFmtDesc.LoadString(IDS_FMTDESC_R5G6B5);
        break;
    case 5:
        m_fmt = D3DFMT_X8R8G8B8;
        m_strFmtDesc.LoadString(IDS_FMTDESC_X8R8G8B8);
        break;
    case 6:
        m_fmt = D3DFMT_X1R5G5B5;
        m_strFmtDesc.LoadString(IDS_FMTDESC_X1R5G5B5);
        break;
    case 7:
        m_fmt = D3DFMT_R3G3B2;
        m_strFmtDesc.LoadString(IDS_FMTDESC_R3G3B2);
        break;
    case 8:
        m_fmt = D3DFMT_A8R3G3B2;
        m_strFmtDesc.LoadString(IDS_FMTDESC_A8R3G3B2);
        break;
    case 9:
        m_fmt = D3DFMT_X4R4G4B4;
        m_strFmtDesc.LoadString(IDS_FMTDESC_X4R4G4B4);
        break;
    case 10:
        m_fmt = D3DFMT_DXT1;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT1);
        break;
    case 11:
        m_fmt = D3DFMT_DXT2;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT2);
        break;
    case 12:
        m_fmt = D3DFMT_DXT3;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT3);
        break;
    case 13:
        m_fmt = D3DFMT_DXT4;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT4);
        break;
    case 14:
        m_fmt = D3DFMT_DXT5;
        m_strFmtDesc.LoadString(IDS_FMTDESC_DXT5);
        break;
    }
    UpdateData(FALSE);
}

