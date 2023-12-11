// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// m59bind.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "m59bind.h"

#include "m59bindDlg.h"
#include "Assignkey.h"
#include "Modifier.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString strINIFile;
BOOL bUpdateINI=FALSE;

TCHAR TCNewkey[MAX_KEYVALUELEN];

int iModifier;

BOOL bInvert;
int iMouselookXscale;
int iMouselookYscale;

TCHAR TCBroadcast[MAX_KEYVALUELEN];
TCHAR TCChat[MAX_KEYVALUELEN];
TCHAR TCEmote[MAX_KEYVALUELEN];
TCHAR TCSay[MAX_KEYVALUELEN];
TCHAR TCTell[MAX_KEYVALUELEN];
TCHAR TCWho[MAX_KEYVALUELEN];
TCHAR TCYell[MAX_KEYVALUELEN];

TCHAR TCAttack[MAX_KEYVALUELEN];
TCHAR TCBuy[MAX_KEYVALUELEN];
TCHAR TCDeposit[MAX_KEYVALUELEN];
TCHAR TCExamine[MAX_KEYVALUELEN];
TCHAR TCLook[MAX_KEYVALUELEN];
TCHAR TCOffer[MAX_KEYVALUELEN];
TCHAR TCOpen[MAX_KEYVALUELEN];
TCHAR TCPickup[MAX_KEYVALUELEN];
TCHAR TCWithdraw[MAX_KEYVALUELEN];

TCHAR TCBackward[MAX_KEYVALUELEN];
TCHAR TCFlip[MAX_KEYVALUELEN];
TCHAR TCForward[MAX_KEYVALUELEN];
TCHAR TCLeft[MAX_KEYVALUELEN];
TCHAR TCLookdown[MAX_KEYVALUELEN];
TCHAR TCLookstraight[MAX_KEYVALUELEN];
TCHAR TCLookup[MAX_KEYVALUELEN];
TCHAR TCMouselooktoggle[MAX_KEYVALUELEN];
TCHAR TCRight[MAX_KEYVALUELEN];
TCHAR TCRunwalk[MAX_KEYVALUELEN];
TCHAR TCSlideleft[MAX_KEYVALUELEN];
TCHAR TCSlideright[MAX_KEYVALUELEN];

TCHAR TCTabbackward[MAX_KEYVALUELEN];
TCHAR TCTabforward[MAX_KEYVALUELEN];
TCHAR TCTargetclear[MAX_KEYVALUELEN];
TCHAR TCTargetnext[MAX_KEYVALUELEN];
TCHAR TCTargetprevious[MAX_KEYVALUELEN];
TCHAR TCTargetself[MAX_KEYVALUELEN];
TCHAR TCSelecttarget[MAX_KEYVALUELEN];

TCHAR TCMap[MAX_KEYVALUELEN];
TCHAR TCMapzoomin[MAX_KEYVALUELEN];
TCHAR TCMapzoomout[MAX_KEYVALUELEN];

/////////////////////////////////////////////////////////////////////////////
// CM59bindApp

BEGIN_MESSAGE_MAP(CM59bindApp, CWinApp)
	//{{AFX_MSG_MAP(CM59bindApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CM59bindApp construction

CM59bindApp::CM59bindApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
  strINIFile=".\\config.ini";
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CM59bindApp object

CM59bindApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CM59bindApp initialization

BOOL CM59bindApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#endif

  CM59bindDlg dlg;

	m_pMainWnd = &dlg;
	auto nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
    bUpdateINI=TRUE;
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
    bUpdateINI=FALSE;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


BOOL CM59bindApp::PreTranslateMessage(MSG* pMsg) 
{
  switch(pMsg->message)
  {
  case WM_KEYDOWN:
    switch(pMsg->wParam)
    {
    case VK_ESCAPE:
      return FALSE; // Disable ESC to exit main program
    }
    break;
  }
 
	return CWinApp::PreTranslateMessage(pMsg);
}

// Removes the modifier (+any for example) from passed string
void StripOffModifier(TCHAR *TCValue)
{
  unsigned int i;

  if(strlen(TCValue)>0)
  {
    for(i=0;i<strlen(TCValue);i++)
    {
      if(TCValue[i]=='+')
      {
        TCValue[i]=0;
        break;
      }
    }
  }
}

// Removes the modifier (+any for example) without changing first passed string
// and puts result in second passed string.
void ExcludeModifier(TCHAR *TCValue, TCHAR *TCExcluded)
{
  unsigned int i;

  strcpy(TCExcluded,TCValue);

  if(strlen(TCExcluded)>0)
  {
    for(i=0;i<strlen(TCExcluded);i++)
    {
      if(TCExcluded[i]=='+')
      {
        TCExcluded[i]=0;
        break;
      }
    }
  }
}

const TCHAR *TCModifiers[5] = {"none","alt","ctrl","shift","any"};

// Appends '+<modifier>' to end of passed string based on passed int value
void AppendModifier(TCHAR *TCString, int iAppend)
{
  TCHAR TCAppend[128];

  // Only append alt,ctrl,shift,any
  if(iAppend>MODIFIER_NONE && iAppend<=MODIFIER_ANY)
  {
    strcpy(TCAppend,"+");
    strcat(TCAppend,TCModifiers[iAppend]);
  }
  else
  {
    // Invalid iAppend or iAppend==0(none). Do nothing.
    return;
  }

  if(strlen(TCString)>0)
  {
    strcat(TCString,TCAppend);
  }
  else
  {
    strcpy(TCString,TCAppend);
  }
}

// Isolates modifier from passed string then returns it's int number
// (Does NOT modify passed string)
int ModifierNum(TCHAR *TCValue)
{
  TCHAR TCModifier[128];
  unsigned int i, j;
  BOOL bFound;

  if(strlen(TCValue)>0)
  {
    bFound=FALSE;
    j=0;
    for(i=0;i<strlen(TCValue);i++)
    {
      if(bFound)
      {
        TCModifier[j]=TCValue[i];
        j++;
      }
      else
      {
        if(TCValue[i]=='+')
        {
          bFound=TRUE;
        }
      }
    }
    TCModifier[j]=0;

    for(i=MODIFIER_NONE;i<MODIFIER_ANY+1;i++)
    {
      if(lstrcmp(TCModifier,TCModifiers[i])==0)
      {
        return i;
      }
    }
  }
  
  return 0;
}

BOOL IsModifier(TCHAR *TCValue)
{
  unsigned int i;

  // Skip 'none' and 'any'
  for(i=MODIFIER_ALT;i<MODIFIER_ANY;i++)
  {
    if(lstrcmp(TCValue,TCModifiers[i])==0)
    {
      return TRUE;
    }
  }

  return FALSE;
}

BOOL CheckforDuplicateBind(TCHAR *TCCompare)
{
  if(lstrcmp(TCCompare,TCBroadcast)==0 && TCCompare!=TCBroadcast) return TRUE;
  if(lstrcmp(TCCompare,TCChat)==0 && TCCompare!=TCChat) return TRUE;
  if(lstrcmp(TCCompare,TCEmote)==0 && TCCompare!=TCEmote) return TRUE;
  if(lstrcmp(TCCompare,TCSay)==0 && TCCompare!=TCSay) return TRUE;
  if(lstrcmp(TCCompare,TCTell)==0 && TCCompare!=TCTell) return TRUE;
  if(lstrcmp(TCCompare,TCWho)==0 && TCCompare!=TCWho) return TRUE;
  if(lstrcmp(TCCompare,TCYell)==0 && TCCompare!=TCYell) return TRUE;

  if(lstrcmp(TCCompare,TCAttack)==0 && TCCompare!=TCAttack) return TRUE;
  if(lstrcmp(TCCompare,TCBuy)==0 && TCCompare!=TCBuy) return TRUE;
  if(lstrcmp(TCCompare,TCDeposit)==0 && TCCompare!=TCDeposit) return TRUE;
  if(lstrcmp(TCCompare,TCExamine)==0 && TCCompare!=TCExamine) return TRUE;
  if(lstrcmp(TCCompare,TCLook)==0 && TCCompare!=TCLook) return TRUE;
  if(lstrcmp(TCCompare,TCOffer)==0 && TCCompare!=TCOffer) return TRUE;
  if(lstrcmp(TCCompare,TCOpen)==0 && TCCompare!=TCOpen) return TRUE;
  if(lstrcmp(TCCompare,TCPickup)==0 && TCCompare!=TCPickup) return TRUE;
  if(lstrcmp(TCCompare,TCWithdraw)==0 && TCCompare!=TCWithdraw) return TRUE;

  if(lstrcmp(TCCompare,TCBackward)==0 && TCCompare!=TCBackward) return TRUE;
  if(lstrcmp(TCCompare,TCFlip)==0 && TCCompare!=TCFlip) return TRUE;
  if(lstrcmp(TCCompare,TCForward)==0 && TCCompare!=TCForward) return TRUE;
  if(lstrcmp(TCCompare,TCLeft)==0 && TCCompare!=TCLeft) return TRUE;
  if(lstrcmp(TCCompare,TCLookdown)==0 && TCCompare!=TCLookdown) return TRUE;
  if(lstrcmp(TCCompare,TCLookstraight)==0 && TCCompare!=TCLookstraight) return TRUE;
  if(lstrcmp(TCCompare,TCLookup)==0 && TCCompare!=TCLookup) return TRUE;
  if(lstrcmp(TCCompare,TCMouselooktoggle)==0 && TCCompare!=TCMouselooktoggle) return TRUE;
  if(lstrcmp(TCCompare,TCRight)==0 && TCCompare!=TCRight) return TRUE;
  if(lstrcmp(TCCompare,TCRunwalk)==0 && TCCompare!=TCRunwalk) return TRUE;
  if(lstrcmp(TCCompare,TCSlideleft)==0 && TCCompare!=TCSlideleft) return TRUE;
  if(lstrcmp(TCCompare,TCSlideright)==0 && TCCompare!=TCSlideright) return TRUE;

  if(lstrcmp(TCCompare,TCTabbackward)==0 && TCCompare!=TCTabbackward) return TRUE;
  if(lstrcmp(TCCompare,TCTabforward)==0 && TCCompare!=TCTabforward) return TRUE;
  if(lstrcmp(TCCompare,TCTargetclear)==0 && TCCompare!=TCTargetclear) return TRUE;
  if(lstrcmp(TCCompare,TCTargetnext)==0 && TCCompare!=TCTargetnext) return TRUE;
  if(lstrcmp(TCCompare,TCTargetprevious)==0 && TCCompare!=TCTargetprevious) return TRUE;
  if(lstrcmp(TCCompare,TCTargetself)==0 && TCCompare!=TCTargetself) return TRUE;
  if(lstrcmp(TCCompare,TCSelecttarget)==0 && TCCompare!=TCSelecttarget) return TRUE;

  if(lstrcmp(TCCompare,TCMap)==0 && TCCompare!=TCMap) return TRUE;
  if(lstrcmp(TCCompare,TCMapzoomin)==0 && TCCompare!=TCMapzoomin) return TRUE;
  if(lstrcmp(TCCompare,TCMapzoomout)==0 && TCCompare!=TCMapzoomout) return TRUE;

  return FALSE;
}

void CheckforDuplicateBindM(TCHAR *TCCompare)
{
  if(CheckforDuplicateBind(TCCompare))
  {
    MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd,"Duplicate bindings for the same key have been detected.\n"\
                                               "This could cause some commands to not work on\n"\
                                               "Meridian 59.  Please double check your bindings.","Warning",MB_OK);
  }
}

void ProcessButtonPressed(TCHAR *TCButtonstring, CButton *modifier, int nID, HWND m_hWnd)
{
//  int iOldModifier;

  CAssignkey dlg;
  dlg.DoModal();

//  iOldModifier=ModifierNum(TCButtonstring); // Save modifier
  strcpy(TCButtonstring,TCNewkey);

  if(IsModifier(TCButtonstring))
  {
    // Disable Modifier button
    modifier->EnableWindow(FALSE);
  }
  else
  {
    AppendModifier(TCButtonstring,iModifier);
    // Enable Modifier button
    modifier->EnableWindow(TRUE);
  }

  SetDlgItemText(m_hWnd,nID,TCButtonstring);

  CheckforDuplicateBindM(TCButtonstring);
}

void InitModifierButton(TCHAR *TCButtonstring, CButton *modifier)
{
  if(IsModifier(TCButtonstring))
  {
    // Disable Modifier Combo
    modifier->EnableWindow(FALSE);
  }
}

void ProcessModifierPressed(TCHAR *TCString, int nID, HWND m_hWnd)
{
  CModifier dlg;

  iModifier=MODIFIER_NONE;
	auto nResponse = dlg.DoModal();
	if (nResponse == IDCANCEL)
	{
    return;
	}

  StripOffModifier(TCString);
  AppendModifier(TCString,iModifier);

  SetDlgItemText(m_hWnd,nID,TCString);

  CheckforDuplicateBindM(TCString);
}

void BooltoString(BOOL bValue, TCHAR *TCValue)
{
  if(bValue)
	  strcpy(TCValue,"true");
  else
	  strcpy(TCValue,"false");
}

BOOL StringtoBool(TCHAR *TCValue)
{
  BOOL bValue;

  _tolower(TCValue);

  if(lstrcmp(TCValue,"true")==0)
    bValue=TRUE;
  else
    bValue=FALSE;

  return bValue;
}

