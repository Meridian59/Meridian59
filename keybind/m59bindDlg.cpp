// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// m59bindDlg.cpp : implementation file
//

#include "stdafx.h"
#include "m59bind.h"
#include "m59bindDlg.h"

#include "defaults.h"
#include "Keybindsps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CM59bindDlg dialog

CM59bindDlg::CM59bindDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CM59bindDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CM59bindDlg)
	m_alwaysrun = FALSE;
	m_classic = FALSE;
	m_dynamic = FALSE;
	m_quickchat = FALSE;
	m_software = FALSE;
	m_attackontarget = FALSE;
	m_gpuefficiency = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_pModeless = NULL;

  m_pPropSheet = NULL;
	m_pPageCommunication = NULL;
	m_pPageInteraction = NULL;
	m_pPageMap = NULL;
	m_pPageMovement = NULL;
	m_pPageTargeting = NULL;
	m_pPageMouse = NULL;

  ReadINIFile();
}

CM59bindDlg::~CM59bindDlg()
{
   if(bUpdateINI) UpdateINIFile();

  // explicitly delete sheet and pages
	delete m_pPropSheet;
	delete m_pPageCommunication;
	delete m_pPageInteraction;
	delete m_pPageMap;
	delete m_pPageMovement;
	delete m_pPageTargeting;
	delete m_pPageMouse;
}

void CM59bindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CM59bindDlg)
	DDX_Check(pDX, IDC_ALWAYSRUN, m_alwaysrun);
	DDX_Check(pDX, IDC_CLASSIC, m_classic);
	DDX_Check(pDX, IDC_DYNAMIC, m_dynamic);
	DDX_Check(pDX, IDC_QUICKCHAT, m_quickchat);
	DDX_Check(pDX, IDC_SOFTWARE, m_software);
	DDX_Check(pDX, IDC_ATTACKONTARGET, m_attackontarget);
	DDX_Check(pDX, IDC_GPU_EFFICIENCY, m_gpuefficiency);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CM59bindDlg, CDialog)
	//{{AFX_MSG_MAP(CM59bindDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CLASSIC, OnClassic)
	ON_BN_CLICKED(IDC_SOFTWARE, OnSoftware)
	ON_BN_CLICKED(IDC_MAINHELP, OnMainhelp)
	ON_BN_CLICKED(IDC_RESTOREDEFAULTS, OnRestoredefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CM59bindDlg message handlers

BOOL CM59bindDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_pPageCommunication = new CCommunication;
	m_pPageInteraction = new CInteraction;
	m_pPageMap = new CThemap;
	m_pPageMovement = new CMovement;
	m_pPageTargeting = new CTargeting;
	m_pPageMouse = new CMouse;

	// create the Property sheet and add pages

	m_pPropSheet = new CPropertySheet;
	m_pPropSheet->AddPage(m_pPageMovement);
	m_pPropSheet->AddPage(m_pPageCommunication);
	m_pPropSheet->AddPage(m_pPageInteraction);
	m_pPropSheet->AddPage(m_pPageTargeting);
	m_pPropSheet->AddPage(m_pPageMap);
	m_pPropSheet->AddPage(m_pPageMouse);

	// create a modeless property page
	if (!m_pPropSheet->Create(this,
			DS_CONTEXTHELP | DS_SETFONT | WS_CHILD | WS_VISIBLE))
	{
		DestroyWindow();
		return FALSE;
	}


  m_pPropSheet->SetWindowPos(NULL, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);

  // This fixes an old Windows95 SetFocus bug when a Property Sheet is used in a Dialog Box
  // (Without this line, the program will hang)
	m_pPropSheet->ModifyStyleEx(NULL,WS_EX_CONTROLPARENT);

  // Read INI file bool values and update Check buttons
  CString strSection;

  TCHAR ReturnedString[MAX_KEYVALUELEN];
  DWORD nSize=MAX_KEYVALUELEN;
  CButton* pCheck;

  strSection="config";

  GetPrivateProfileString(strSection,"classickeybindings",DEF_CLASSIC,ReturnedString,nSize,strINIFile);
	pCheck = (CButton*) GetDlgItem(IDC_CLASSIC);

  if(StringtoBool(ReturnedString))
  {
	  pCheck->SetCheck(1);
  }
	
  GetPrivateProfileString(strSection,"invertmouse",DEF_INVERT,ReturnedString,nSize,strINIFile);
  if(StringtoBool(ReturnedString))
    bInvert=TRUE;
  else
    bInvert=FALSE;

  GetPrivateProfileString(strSection,"quickchat",DEF_QUICKCHAT,ReturnedString,nSize,strINIFile);
	pCheck = (CButton*) GetDlgItem(IDC_QUICKCHAT);

  if(StringtoBool(ReturnedString))
  {
	  pCheck->SetCheck(1);
  }
	
  GetPrivateProfileString(strSection,"alwaysrun",DEF_ALWAYSRUN,ReturnedString,nSize,strINIFile);
	pCheck = (CButton*) GetDlgItem(IDC_ALWAYSRUN);

  if(StringtoBool(ReturnedString))
  {
	  pCheck->SetCheck(1);
  }
	
  GetPrivateProfileString(strSection,"dynamiclighting",DEF_DYNAMIC,ReturnedString,nSize,strINIFile);
	pCheck = (CButton*) GetDlgItem(IDC_DYNAMIC);

  if(StringtoBool(ReturnedString))
  {
	  pCheck->SetCheck(1);
  }
	
  GetPrivateProfileString(strSection,"softwarerenderer",DEF_SOFTWARE,ReturnedString,nSize,strINIFile);
	pCheck = (CButton*) GetDlgItem(IDC_SOFTWARE);

  if(StringtoBool(ReturnedString))
  {
	  pCheck->SetCheck(1);
  }

  GetPrivateProfileString(strSection,"attackontarget",DEF_ATTACKONTARGET,ReturnedString,nSize,strINIFile);
	pCheck = (CButton*) GetDlgItem(IDC_ATTACKONTARGET);

  if(StringtoBool(ReturnedString))
  {
	  pCheck->SetCheck(1);
  }

  GetPrivateProfileString(strSection, "gpuefficiency", DEF_GPU_EFFICIENCY, ReturnedString, nSize, strINIFile);
  pCheck = (CButton*)GetDlgItem(IDC_GPU_EFFICIENCY);

  if (StringtoBool(ReturnedString))
  {
	  pCheck->SetCheck(1);
  }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CM59bindDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CM59bindDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CM59bindDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CM59bindDlg::ReadINIFile(void)
{
  CString strSection;
  DWORD nSize=MAX_KEYVALUELEN;

  strSection="config";

  iMouselookXscale=GetPrivateProfileInt(strSection,"mouselookxscale",DEF_MOUSELOOKXSCALE,strINIFile);
  iMouselookYscale=GetPrivateProfileInt(strSection,"mouselookyscale",DEF_MOUSELOOKYSCALE,strINIFile);

  strSection="keys";

  // Communication
  GetPrivateProfileString(strSection,"broadcast",DEF_BROADCAST,TCBroadcast,nSize,strINIFile);
  GetPrivateProfileString(strSection,"chat",DEF_CHAT,TCChat,nSize,strINIFile);
  GetPrivateProfileString(strSection,"emote",DEF_EMOTE,TCEmote,nSize,strINIFile);
  GetPrivateProfileString(strSection,"say",DEF_SAY,TCSay,nSize,strINIFile);
  GetPrivateProfileString(strSection,"tell",DEF_TELL,TCTell,nSize,strINIFile);
  GetPrivateProfileString(strSection,"who",DEF_WHO,TCWho,nSize,strINIFile);
  GetPrivateProfileString(strSection,"yell",DEF_YELL,TCYell,nSize,strINIFile);

  // Interaction
  GetPrivateProfileString(strSection,"attack",DEF_ATTACK,TCAttack,nSize,strINIFile);
  GetPrivateProfileString(strSection,"buy",DEF_BUY,TCBuy,nSize,strINIFile);
  GetPrivateProfileString(strSection,"deposit",DEF_DEPOSIT,TCDeposit,nSize,strINIFile);
  GetPrivateProfileString(strSection,"examine",DEF_EXAMINE,TCExamine,nSize,strINIFile);
  GetPrivateProfileString(strSection,"look",DEF_LOOK,TCLook,nSize,strINIFile);
  GetPrivateProfileString(strSection,"offer",DEF_OFFER,TCOffer,nSize,strINIFile);
  GetPrivateProfileString(strSection,"open",DEF_OPEN,TCOpen,nSize,strINIFile);
  GetPrivateProfileString(strSection,"pickup",DEF_PICKUP,TCPickup,nSize,strINIFile);
  GetPrivateProfileString(strSection,"withdraw",DEF_WITHDRAW,TCWithdraw,nSize,strINIFile);

  // Movement
  GetPrivateProfileString(strSection,"backward",DEF_BACKWARD,TCBackward,nSize,strINIFile);
  GetPrivateProfileString(strSection,"flip",DEF_FLIP,TCFlip,nSize,strINIFile);
  GetPrivateProfileString(strSection,"forward",DEF_FORWARD,TCForward,nSize,strINIFile);
  GetPrivateProfileString(strSection,"left",DEF_LEFT,TCLeft,nSize,strINIFile);
  GetPrivateProfileString(strSection,"lookdown",DEF_LOOKDOWN,TCLookdown,nSize,strINIFile);
  GetPrivateProfileString(strSection,"lookstraight",DEF_LOOKSTRAIGHT,TCLookstraight,nSize,strINIFile);
  GetPrivateProfileString(strSection,"lookup",DEF_LOOKUP,TCLookup,nSize,strINIFile);
  GetPrivateProfileString(strSection,"mouselooktoggle",DEF_MOUSELOOKTOGGLE,TCMouselooktoggle,nSize,strINIFile);
  GetPrivateProfileString(strSection,"right",DEF_RIGHT,TCRight,nSize,strINIFile);
  GetPrivateProfileString(strSection,"run/walk",DEF_RUNWALK,TCRunwalk,nSize,strINIFile);
  GetPrivateProfileString(strSection,"slideleft",DEF_SLIDELEFT,TCSlideleft,nSize,strINIFile);
  GetPrivateProfileString(strSection,"slideright",DEF_SLIDERIGHT,TCSlideright,nSize,strINIFile);

  // Targeting
  GetPrivateProfileString(strSection,"tabbackward",DEF_TABBACKWARD,TCTabbackward,nSize,strINIFile);
  GetPrivateProfileString(strSection,"tabforward",DEF_TABFORWARD,TCTabforward,nSize,strINIFile);
  GetPrivateProfileString(strSection,"targetclear",DEF_TARGETCLEAR,TCTargetclear,nSize,strINIFile);
  GetPrivateProfileString(strSection,"targetnext",DEF_TARGETNEXT,TCTargetnext,nSize,strINIFile);
  GetPrivateProfileString(strSection,"targetprevious",DEF_TARGETPREVIOUS,TCTargetprevious,nSize,strINIFile);
  GetPrivateProfileString(strSection,"targetself",DEF_TARGETSELF,TCTargetself,nSize,strINIFile);
  GetPrivateProfileString(strSection,"mousetarget",DEF_MOUSETARGET,TCSelecttarget,nSize,strINIFile);

  // Map
  GetPrivateProfileString(strSection,"map",DEF_MAP,TCMap,nSize,strINIFile);
  GetPrivateProfileString(strSection,"mapzoomin",DEF_MAPZOOMIN,TCMapzoomin,nSize,strINIFile);
  GetPrivateProfileString(strSection,"mapzoomout",DEF_MAPZOOMOUT,TCMapzoomout,nSize,strINIFile);
}

void CM59bindDlg::UpdateINIFile(void)
{
  CString strSection;

  TCHAR Value[MAX_KEYVALUELEN];

  strSection="config";

  WritePrivateProfileString(strSection,"initialized","true",strINIFile);

  BooltoString(m_classic,Value);
  WritePrivateProfileString(strSection,"classickeybindings",Value,strINIFile);

  BooltoString(bInvert,Value);
  WritePrivateProfileString(strSection,"invertmouse",Value,strINIFile);

  BooltoString(m_quickchat,Value);
  WritePrivateProfileString(strSection,"quickchat",Value,strINIFile);

  BooltoString(m_alwaysrun,Value);
  WritePrivateProfileString(strSection,"alwaysrun",Value,strINIFile);

  BooltoString(m_dynamic,Value);
  WritePrivateProfileString(strSection,"dynamiclighting",Value,strINIFile);
	
  BooltoString(m_software,Value);
  WritePrivateProfileString(strSection,"softwarerenderer",Value,strINIFile);

  BooltoString(m_attackontarget,Value);
  WritePrivateProfileString(strSection,"attackontarget",Value,strINIFile);

  BooltoString(m_gpuefficiency, Value);
  WritePrivateProfileString(strSection, "gpuefficiency", Value, strINIFile);

  sprintf(Value,"%d",iMouselookXscale);
  WritePrivateProfileString(strSection,"mouselookxscale",Value,strINIFile);
  sprintf(Value,"%d",iMouselookYscale);
  WritePrivateProfileString(strSection,"mouselookyscale",Value,strINIFile);

  strSection="keys";

  // Communication
  WritePrivateProfileString(strSection,"broadcast",TCBroadcast,strINIFile);
  WritePrivateProfileString(strSection,"chat",TCChat,strINIFile);
  WritePrivateProfileString(strSection,"emote",TCEmote,strINIFile);
  WritePrivateProfileString(strSection,"say",TCSay,strINIFile);
  WritePrivateProfileString(strSection,"tell",TCTell,strINIFile);
  WritePrivateProfileString(strSection,"who",TCWho,strINIFile);
  WritePrivateProfileString(strSection,"yell",TCYell,strINIFile);

  // Interaction
  WritePrivateProfileString(strSection,"attack",TCAttack,strINIFile);
  WritePrivateProfileString(strSection,"buy",TCBuy,strINIFile);
  WritePrivateProfileString(strSection,"deposit",TCDeposit,strINIFile);
  WritePrivateProfileString(strSection,"examine",TCExamine,strINIFile);
  WritePrivateProfileString(strSection,"look",TCLook,strINIFile);
  WritePrivateProfileString(strSection,"offer",TCOffer,strINIFile);
  WritePrivateProfileString(strSection,"open",TCOpen,strINIFile);
  WritePrivateProfileString(strSection,"pickup",TCPickup,strINIFile);
  WritePrivateProfileString(strSection,"withdraw",TCWithdraw,strINIFile);

  // Movement
  WritePrivateProfileString(strSection,"backward",TCBackward,strINIFile);
  WritePrivateProfileString(strSection,"flip",TCFlip,strINIFile);
  WritePrivateProfileString(strSection,"forward",TCForward,strINIFile);
  WritePrivateProfileString(strSection,"left",TCLeft,strINIFile);
  WritePrivateProfileString(strSection,"lookdown",TCLookdown,strINIFile);
  WritePrivateProfileString(strSection,"lookstraight",TCLookstraight,strINIFile);
  WritePrivateProfileString(strSection,"lookup",TCLookup,strINIFile);
  WritePrivateProfileString(strSection,"mouselooktoggle",TCMouselooktoggle,strINIFile);
  WritePrivateProfileString(strSection,"right",TCRight,strINIFile);
  WritePrivateProfileString(strSection,"run/walk",TCRunwalk,strINIFile);
  WritePrivateProfileString(strSection,"slideleft",TCSlideleft,strINIFile);
  WritePrivateProfileString(strSection,"slideright",TCSlideright,strINIFile);

  // Targeting
  WritePrivateProfileString(strSection,"tabbackward",TCTabbackward,strINIFile);
  WritePrivateProfileString(strSection,"tabforward",TCTabforward,strINIFile);
  WritePrivateProfileString(strSection,"targetclear",TCTargetclear,strINIFile);
  WritePrivateProfileString(strSection,"targetnext",TCTargetnext,strINIFile);
  WritePrivateProfileString(strSection,"targetprevious",TCTargetprevious,strINIFile);
  WritePrivateProfileString(strSection,"targetself",TCTargetself,strINIFile);
  WritePrivateProfileString(strSection,"mousetarget",TCSelecttarget,strINIFile);

  // Map
  WritePrivateProfileString(strSection,"map",TCMap,strINIFile);
  WritePrivateProfileString(strSection,"mapzoomin",TCMapzoomin,strINIFile);
  WritePrivateProfileString(strSection,"mapzoomout",TCMapzoomout,strINIFile);
}

void CM59bindDlg::OnClassic() 
{
  UpdateData(TRUE);
  if(m_classic)
  {
    MessageBox("Activating \"Classic Key Bindings\" will cause the game to ignore the key bindings!","Notice",MB_OK);
  }
}

void CM59bindDlg::OnSoftware() 
{
  UpdateData(TRUE);
  if(!m_software)
  {
    MessageBox("Software renderer may have been activated due to a problem with your video setup.\n\t\t Disabling may cause the game not to run.","Warning",MB_OK);
  }
}

BOOL CM59bindDlg::PreTranslateMessage(MSG* pMsg) 
{
  switch(pMsg->message)
  {
  case WM_KEYDOWN:
    switch(pMsg->wParam)
    {
    case VK_ESCAPE:
      return FALSE;
    }
    break;
  }
	return CDialog::PreTranslateMessage(pMsg);
}

void CM59bindDlg::OnMainhelp() 
{
  MessageBox("To bind a command:\n\n"\
             "* Click on the button next to the command you want to bind.  This button will\n"\
             "  show the key currently bound to the command\n\n"\
             "* When the popup appears, press and release the key and/or mouse button\n"\
             "  you want to bind to that command.  For example, pressing the button next to\n"\
             "  \"forward\" and pressing the W key will make that the new walk forward key.\n\n"\
             "* Modifier keys such as shift, alt, control may be used in combination with the\n"\
             "  main key when binding a command (example: W+shift).\n\n"\
             "* If you wish to change only the modifier key of a command, press the button\n"\
             "  marked with a '+' next to the binding button and select a modifier key from\n"\
             "  the list shown.\n\n"\
             "* The \"Any\" modifier means that any modifier key can be pressed with the key\n"\
             "  and it will still work.  So, W+any means that W+shift, W+alt, W+ctrl, and just\n"\
             "  W will do the specified command.\n\n"
             "* If you make a mistake you may press the \"Restore Defaults\" button and all of\n"\
             "  the bindings will be set to the default.\n\n"
             "* Once you have finished your settings, press \"OK\" to save and quit.  Or you\n"\
             "  can press \"Cancel\" to quit without saving your changes.",
             "Help",MB_OK);
}

void CM59bindDlg::OnOK() 
{
	MessageBox("You must restart the Meridian 59 client before the new bindings or settings will take effect.","Notice",MB_OK|MB_APPLMODAL);
	
	CDialog::OnOK();
}

void RestoreDefaults(void)
{
  bInvert=StringtoBool(DEF_INVERT);

  iMouselookXscale=DEF_MOUSELOOKXSCALE;
  iMouselookYscale=DEF_MOUSELOOKYSCALE;

  strcpy(TCBroadcast,DEF_BROADCAST);
  strcpy(TCChat,DEF_CHAT);
  strcpy(TCEmote,DEF_EMOTE);
  strcpy(TCSay,DEF_SAY);
  strcpy(TCTell,DEF_TELL);
  strcpy(TCWho,DEF_WHO);
  strcpy(TCYell,DEF_YELL);

  strcpy(TCAttack,DEF_ATTACK);
  strcpy(TCBuy,DEF_BUY);
  strcpy(TCDeposit,DEF_DEPOSIT);
  strcpy(TCExamine,DEF_EXAMINE);
  strcpy(TCLook,DEF_LOOK);
  strcpy(TCOffer,DEF_OFFER);
  strcpy(TCOpen,DEF_OPEN);
  strcpy(TCPickup,DEF_PICKUP);
  strcpy(TCWithdraw,DEF_WITHDRAW);

  strcpy(TCBackward,DEF_BACKWARD);
  strcpy(TCFlip,DEF_FLIP);
  strcpy(TCForward,DEF_FORWARD);
  strcpy(TCLeft,DEF_LEFT);
  strcpy(TCLookdown,DEF_LOOKDOWN);
  strcpy(TCLookstraight,DEF_LOOKSTRAIGHT);
  strcpy(TCLookup,DEF_LOOKUP);
  strcpy(TCMouselooktoggle,DEF_MOUSELOOKTOGGLE);
  strcpy(TCRight,DEF_RIGHT);
  strcpy(TCRunwalk,DEF_RUNWALK);
  strcpy(TCSlideleft,DEF_SLIDELEFT);
  strcpy(TCSlideright,DEF_SLIDERIGHT);

  strcpy(TCTabbackward,DEF_TABBACKWARD);
  strcpy(TCTabforward,DEF_TABFORWARD);
  strcpy(TCTargetclear,DEF_TARGETCLEAR);
  strcpy(TCTargetnext,DEF_TARGETNEXT);
  strcpy(TCTargetprevious,DEF_TARGETPREVIOUS);
  strcpy(TCTargetself,DEF_TARGETSELF);
  strcpy(TCSelecttarget,DEF_MOUSETARGET);

  strcpy(TCMap,DEF_MAP);
  strcpy(TCMapzoomin,DEF_MAPZOOMIN);
  strcpy(TCMapzoomout,DEF_MAPZOOMOUT);
}

void CM59bindDlg::OnRestoredefaults() 
{
  if(MessageBox("Are you sure you want to restore all settings to their default?","Restore Defaults",MB_YESNO)==IDYES)
  {
    RestoreDefaults();

    m_pPageCommunication->UpdateDialogData();
    m_pPageInteraction->UpdateDialogData();
    m_pPageMouse->UpdateDialogData();
    m_pPageMovement->UpdateDialogData();
    m_pPageTargeting->UpdateDialogData();
    m_pPageMap->UpdateDialogData();
  }
}
