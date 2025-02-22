#include "stdafx.h"
#include "resource.h"
#include "preferences_common.h"

const TCHAR* TCModifiers[5] = { _T("none"), _T("alt"), _T("ctrl"), _T("shift"), _T("any") };

BOOL bUpdateINI = FALSE;

TCHAR TCNewkey[MAX_KEYVALUELEN];

BOOL bInvert;
int iMouselookXscale;
int iMouselookYscale;

BOOL m_alwaysrun;
BOOL m_classic;
BOOL m_dynamic;
BOOL m_quickchat;
BOOL m_software;
BOOL m_attackontarget;
BOOL m_gpuefficiency;

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

BOOL IsModifier(TCHAR* TCValue)
{
	unsigned int i;

	// Skip 'none' and 'any'
	for (i = MODIFIER_ALT; i < MODIFIER_ANY; i++)
	{
		if (lstrcmp(TCValue, TCModifiers[i]) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void InitModifierButton(TCHAR* TCButtonstring, CButton* modifier)
{
	if (IsModifier(TCButtonstring))
	{
		// Disable Modifier Combo
		modifier->EnableWindow(FALSE);
	}
}

void AppendModifier(TCHAR* TCString, int iAppend)
{
	TCHAR TCAppend[128];

	// Only append alt, ctrl, shift, any
	if (iAppend > MODIFIER_NONE && iAppend <= MODIFIER_ANY)
	{
		strcpy(TCAppend, "+");
		strcat(TCAppend, TCModifiers[iAppend]);
	}
	else
	{
		// Invalid iAppend or iAppend == 0 (none). Do nothing.
		return;
	}

	if (strlen(TCString) > 0)
	{
		strcat(TCString, TCAppend);
	}
	else
	{
		strcpy(TCString, TCAppend);
	}
}

void StripOffModifier(TCHAR* TCValue)
{
	unsigned int i;

	if (strlen(TCValue) > 0)
	{
		for (i = 0; i < strlen(TCValue); i++)
		{
			if (TCValue[i] == '+')
			{
				TCValue[i] = 0;
				break;
			}
		}
	}
}

BOOL CheckforDuplicateBind(TCHAR* TCCompare)
{
	if (lstrcmp(TCCompare, TCBroadcast) == 0 && TCCompare != TCBroadcast) return TRUE;
	if (lstrcmp(TCCompare, TCChat) == 0 && TCCompare != TCChat) return TRUE;
	if (lstrcmp(TCCompare, TCEmote) == 0 && TCCompare != TCEmote) return TRUE;
	if (lstrcmp(TCCompare, TCSay) == 0 && TCCompare != TCSay) return TRUE;
	if (lstrcmp(TCCompare, TCTell) == 0 && TCCompare != TCTell) return TRUE;
	if (lstrcmp(TCCompare, TCWho) == 0 && TCCompare != TCWho) return TRUE;
	if (lstrcmp(TCCompare, TCYell) == 0 && TCCompare != TCYell) return TRUE;

	if (lstrcmp(TCCompare, TCAttack) == 0 && TCCompare != TCAttack) return TRUE;
	if (lstrcmp(TCCompare, TCBuy) == 0 && TCCompare != TCBuy) return TRUE;
	if (lstrcmp(TCCompare, TCDeposit) == 0 && TCCompare != TCDeposit) return TRUE;
	if (lstrcmp(TCCompare, TCExamine) == 0 && TCCompare != TCExamine) return TRUE;
	if (lstrcmp(TCCompare, TCLook) == 0 && TCCompare != TCLook) return TRUE;
	if (lstrcmp(TCCompare, TCOffer) == 0 && TCCompare != TCOffer) return TRUE;
	if (lstrcmp(TCCompare, TCOpen) == 0 && TCCompare != TCOpen) return TRUE;
	if (lstrcmp(TCCompare, TCPickup) == 0 && TCCompare != TCPickup) return TRUE;
	if (lstrcmp(TCCompare, TCWithdraw) == 0 && TCCompare != TCWithdraw) return TRUE;

	if (lstrcmp(TCCompare, TCBackward) == 0 && TCCompare != TCBackward) return TRUE;
	if (lstrcmp(TCCompare, TCFlip) == 0 && TCCompare != TCFlip) return TRUE;
	if (lstrcmp(TCCompare, TCForward) == 0 && TCCompare != TCForward) return TRUE;
	if (lstrcmp(TCCompare, TCLeft) == 0 && TCCompare != TCLeft) return TRUE;
	if (lstrcmp(TCCompare, TCLookdown) == 0 && TCCompare != TCLookdown) return TRUE;
	if (lstrcmp(TCCompare, TCLookstraight) == 0 && TCCompare != TCLookstraight) return TRUE;
	if (lstrcmp(TCCompare, TCLookup) == 0 && TCCompare != TCLookup) return TRUE;
	if (lstrcmp(TCCompare, TCMouselooktoggle) == 0 && TCCompare != TCMouselooktoggle) return TRUE;
	if (lstrcmp(TCCompare, TCRight) == 0 && TCCompare != TCRight) return TRUE;
	if (lstrcmp(TCCompare, TCRunwalk) == 0 && TCCompare != TCRunwalk) return TRUE;
	if (lstrcmp(TCCompare, TCSlideleft) == 0 && TCCompare != TCSlideleft) return TRUE;
	if (lstrcmp(TCCompare, TCSlideright) == 0 && TCCompare != TCSlideright) return TRUE;

	if (lstrcmp(TCCompare, TCTabbackward) == 0 && TCCompare != TCTabbackward) return TRUE;
	if (lstrcmp(TCCompare, TCTabforward) == 0 && TCCompare != TCTabforward) return TRUE;
	if (lstrcmp(TCCompare, TCTargetclear) == 0 && TCCompare != TCTargetclear) return TRUE;
	if (lstrcmp(TCCompare, TCTargetnext) == 0 && TCCompare != TCTargetnext) return TRUE;
	if (lstrcmp(TCCompare, TCTargetprevious) == 0 && TCCompare != TCTargetprevious) return TRUE;
	if (lstrcmp(TCCompare, TCTargetself) == 0 && TCCompare != TCTargetself) return TRUE;
	if (lstrcmp(TCCompare, TCSelecttarget) == 0 && TCCompare != TCSelecttarget) return TRUE;

	if (lstrcmp(TCCompare, TCMap) == 0 && TCCompare != TCMap) return TRUE;
	if (lstrcmp(TCCompare, TCMapzoomin) == 0 && TCCompare != TCMapzoomin) return TRUE;
	if (lstrcmp(TCCompare, TCMapzoomout) == 0 && TCCompare != TCMapzoomout) return TRUE;

	return FALSE;
}

void CheckforDuplicateBindM(TCHAR* TCCompare)
{
	if (CheckforDuplicateBind(TCCompare))
	{
		MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, "Duplicate bindings for the same key have been detected.\n"\
			"This could cause some commands to not work on\n"\
			"Meridian 59.  Please double check your bindings.", "Warning", MB_OK);
	}
}

void BooltoString(BOOL bValue, TCHAR* TCValue)
{
	if (bValue)
		strcpy(TCValue, "true");
	else
		strcpy(TCValue, "false");
}

BOOL StringtoBool(const TCHAR* TCValue)
{
	BOOL bValue;

	_tolower(TCValue);

	if (lstrcmp(TCValue, "true") == 0)
		bValue = TRUE;
	else
		bValue = FALSE;

	return bValue;
}
