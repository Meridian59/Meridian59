// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Assignkey.cpp : implementation file
//

#include "stdafx.h"
#include "m59bind.h"
#include "Assignkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAssignkey dialog


CAssignkey::CAssignkey(CWnd* pParent /*=NULL*/)
	: CDialog(CAssignkey::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAssignkey)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAssignkey::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAssignkey)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAssignkey, CDialog)
	//{{AFX_MSG_MAP(CAssignkey)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAssignkey message handlers

BOOL CAssignkey::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

  return CDialog::PreTranslateMessage(pMsg);
}

INT_PTR CAssignkey::DoModal() 
{
  MSG pMsg;
  BOOL bGotMsg;
  BOOL bSkip;
  BOOL bGetbind;

  while (bGotMsg = PeekMessage( &pMsg, NULL, 0U, 0U, PM_REMOVE ))
  {
  }

  Create(IDD_ASSIGNKEY);
  ShowWindow(SW_SHOW);

  SetCapture();

  strcpy(TCNewkey,"");
  iModifier=MODIFIER_NONE;
  bSkip=FALSE;
  bGetbind=TRUE;
  do
  {
    WaitMessage();
    while (bGotMsg = PeekMessage( &pMsg, NULL, 0U, 0U, PM_REMOVE ))
    {
#if 0
      TCHAR txt[128];
      sprintf(txt,"%x %x",pMsg.message,pMsg.wParam);
      SetDlgItemText(IDC_ASSIGNFOR,txt);
#endif
      switch (pMsg.message)
      {
      case WM_NCLBUTTONDOWN:
        bSkip=TRUE;
      case WM_LBUTTONDOWN:
        bGetbind=FALSE;
        strcpy(TCNewkey,"mouse0");
        bSkip=TRUE;
        break;
      case WM_NCRBUTTONDOWN:
        bSkip=TRUE;
      case WM_RBUTTONDOWN:
        bGetbind=FALSE;
        strcpy(TCNewkey,"mouse1");
        break;
      case WM_NCMBUTTONDOWN:
        bSkip=TRUE;
      case WM_MBUTTONDOWN:
        bGetbind=FALSE;
        strcpy(TCNewkey,"mouse2");
        break;
      case 0x20b:
        bSkip=TRUE;
        switch(pMsg.wParam)
        {
        case 0x10020:
          bGetbind=FALSE;
          strcpy(TCNewkey,"mouse3");
          break;
        case 0x20040:
          bGetbind=FALSE;
          strcpy(TCNewkey,"mouse4");
          break;
        }
        if(strlen(TCNewkey)>0) bGetbind=FALSE;
        break;
      case WM_SYSKEYDOWN:
        switch (pMsg.wParam)
        {
        case VK_MENU:
          iModifier=MODIFIER_ALT;
          break;
        }
        break;
      case WM_KEYDOWN:
        if(pMsg.wParam>='A' && pMsg.wParam<='Z')
        {
          bGetbind=FALSE;
          sprintf(TCNewkey,"%c",(char) pMsg.wParam+32); // +32 changes it to lowercase
          break;
        }
/*
        if(pMsg.wParam>='0' && pMsg.wParam<='9')
        {
          bGetbind=FALSE;
          sprintf(TCNewkey,"%c",pMsg.wParam);
          break;
        }
*/
        switch (pMsg.wParam)
        {
/*
        case VK_CAPITAL:
          strcpy(TCNewkey,"capslock");
          break;
*/
        case VK_BACK:
          strcpy(TCNewkey,"backspace");
          break;
        case VK_TAB:
          strcpy(TCNewkey,"tab");
          break;
        case VK_RETURN:
          strcpy(TCNewkey,"enter");
          break;
        case VK_ESCAPE:
          strcpy(TCNewkey,"esc");
          break;
        case VK_SPACE:
          strcpy(TCNewkey,"space");
          break;
        case VK_PRIOR:
          strcpy(TCNewkey,"pageup");
          break;
        case VK_NEXT:
          strcpy(TCNewkey,"pagedown");
          break;
        case VK_END:
          strcpy(TCNewkey,"end");
          break;
        case VK_HOME:
          strcpy(TCNewkey,"home");
          break;
        case VK_LEFT:
          strcpy(TCNewkey,"left");
          break;
        case VK_UP:
          strcpy(TCNewkey,"up");
          break;
        case VK_RIGHT:
          strcpy(TCNewkey,"right");
          break;
        case VK_DOWN:
          strcpy(TCNewkey,"down");
          break;
/*
        case VK_INSERT:
          strcpy(TCNewkey,"insert");
          break;
*/
        case VK_DELETE:
          strcpy(TCNewkey,"delete");
          break;
/*
        case VK_NUMPAD0:
          strcpy(TCNewkey,"numpad0");
          break;
        case VK_NUMPAD1:
          strcpy(TCNewkey,"numpad1");
          break;
        case VK_NUMPAD2:
          strcpy(TCNewkey,"numpad2");
          break;
        case VK_NUMPAD3:
          strcpy(TCNewkey,"numpad3");
          break;
        case VK_NUMPAD4:
          strcpy(TCNewkey,"numpad4");
          break;
        case VK_NUMPAD5:
          strcpy(TCNewkey,"numpad5");
          break;
        case VK_NUMPAD6:
          strcpy(TCNewkey,"numpad6");
          break;
        case VK_NUMPAD7:
          strcpy(TCNewkey,"numpad7");
          break;
        case VK_NUMPAD8:
          strcpy(TCNewkey,"numpad8");
          break;
        case VK_NUMPAD9:
          strcpy(TCNewkey,"numpad9");
          break;
        case VK_MULTIPLY:
          strcpy(TCNewkey,"multiply");
          break;
*/
        case VK_ADD:
          strcpy(TCNewkey,"add");
          break;
        case VK_SUBTRACT:
          strcpy(TCNewkey,"subtract");
          break;
/*
        case VK_DECIMAL:
          strcpy(TCNewkey,"decimal");
          break;
        case VK_DIVIDE:
          strcpy(TCNewkey,"divide");
          break;
        case VK_F1:
          strcpy(TCNewkey,"f1");
          break;
        case VK_F2:
          strcpy(TCNewkey,"f2");
          break;
        case VK_F3:
          strcpy(TCNewkey,"f3");
          break;
        case VK_F4:
          strcpy(TCNewkey,"f4");
          break;
        case VK_F5:
          strcpy(TCNewkey,"f5");
          break;
        case VK_F6:
          strcpy(TCNewkey,"f6");
          break;
        case VK_F7:
          strcpy(TCNewkey,"f7");
          break;
        case VK_F8:
          strcpy(TCNewkey,"f8");
          break;
        case VK_F9:
          strcpy(TCNewkey,"f9");
          break;
        case VK_F10:
          strcpy(TCNewkey,"f10");
          break;
        case VK_F11:
          strcpy(TCNewkey,"f11");
          break;
        case VK_F12:
          strcpy(TCNewkey,"f12");
          break;
        case VK_NUMLOCK:
          strcpy(TCNewkey,"numlock");
          break;
        case VK_SCROLL:
          strcpy(TCNewkey,"scrolllock");
          break;
*/
        case VK_SHIFT:
          iModifier=MODIFIER_SHIFT;
          break;
        case VK_CONTROL:
          iModifier=MODIFIER_CTRL;
          break;
        case 0xba:
          strcpy(TCNewkey,";");
          break;
/*
        case 0xbb:
          strcpy(TCNewkey,"=");
          break;
*/
        case 0xbc:
          strcpy(TCNewkey,",");
          break;
/*
        case 0xbd:
          strcpy(TCNewkey,"-");
          break;
*/
        case 0xbe:
          strcpy(TCNewkey,".");
          break;
/*
        case 0xbf:
          strcpy(TCNewkey,"/");
          break;

        case 0xc0:
          strcpy(TCNewkey,"`");
          break;
*/
        case 0xdb:
          strcpy(TCNewkey,"[");
          break;
/*
        case 0xdc:
          strcpy(TCNewkey,"\\");
          break;
*/
        case 0xdd:
          strcpy(TCNewkey,"]");
          break;
        case 0xde:
          strcpy(TCNewkey,"'");
          break;
        }
        if(strlen(TCNewkey)>0) bGetbind=FALSE;
        break;
      case WM_SYSKEYUP:
        switch(pMsg.wParam)
        {
        case VK_MENU:
          strcpy(TCNewkey,"alt");
          break;
        }
        if(strlen(TCNewkey)>0) bGetbind=FALSE;
        break;
      case WM_KEYUP:
        switch(pMsg.wParam)
        {
        case VK_SHIFT:
          strcpy(TCNewkey,"shift");
          break;
        case VK_CONTROL:
          strcpy(TCNewkey,"ctrl");
          break;
        }
        if(strlen(TCNewkey)>0) bGetbind=FALSE;
        break;
      }
      if(!bSkip)
      {
        TranslateMessage(&pMsg);
        DispatchMessage(&pMsg);
      }
      bSkip=FALSE;
    }
   
  } while(bGetbind);

  ReleaseCapture();

  DestroyWindow();

  return FALSE;
//	return CDialog::DoModal();
}
