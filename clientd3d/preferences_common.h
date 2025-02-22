// Meridian 59, Copyright 1994-2025 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef PREFERENCES_COMMON_H
#define PREFERENCES_COMMON_H

#include "stdafx.h"

constexpr int MAX_KEYVALUELEN = 128;

constexpr int MODIFIER_NONE = 0;
constexpr int MODIFIER_ALT = 1;
constexpr int MODIFIER_CTRL = 2;
constexpr int MODIFIER_SHIFT = 3;
constexpr int MODIFIER_ANY = 4;

extern const TCHAR* TCModifiers[5];

extern CString strINIFile;
extern BOOL bUpdateINI;

extern TCHAR TCNewkey[MAX_KEYVALUELEN];

extern BOOL bInvert;
extern int iMouselookXscale;
extern int iMouselookYscale;

extern BOOL m_alwaysrun;
extern BOOL m_classic;
extern BOOL m_dynamic;
extern BOOL m_quickchat;
extern BOOL m_software;
extern BOOL m_attackontarget;
extern BOOL m_gpuefficiency;

extern TCHAR TCBroadcast[MAX_KEYVALUELEN];
extern TCHAR TCChat[MAX_KEYVALUELEN];
extern TCHAR TCEmote[MAX_KEYVALUELEN];
extern TCHAR TCSay[MAX_KEYVALUELEN];
extern TCHAR TCTell[MAX_KEYVALUELEN];
extern TCHAR TCWho[MAX_KEYVALUELEN];
extern TCHAR TCYell[MAX_KEYVALUELEN];

extern TCHAR TCAttack[MAX_KEYVALUELEN];
extern TCHAR TCBuy[MAX_KEYVALUELEN];
extern TCHAR TCDeposit[MAX_KEYVALUELEN];
extern TCHAR TCExamine[MAX_KEYVALUELEN];
extern TCHAR TCLook[MAX_KEYVALUELEN];
extern TCHAR TCOffer[MAX_KEYVALUELEN];
extern TCHAR TCOpen[MAX_KEYVALUELEN];
extern TCHAR TCPickup[MAX_KEYVALUELEN];
extern TCHAR TCWithdraw[MAX_KEYVALUELEN];

extern TCHAR TCBackward[MAX_KEYVALUELEN];
extern TCHAR TCFlip[MAX_KEYVALUELEN];
extern TCHAR TCForward[MAX_KEYVALUELEN];
extern TCHAR TCLeft[MAX_KEYVALUELEN];
extern TCHAR TCLookdown[MAX_KEYVALUELEN];
extern TCHAR TCLookstraight[MAX_KEYVALUELEN];
extern TCHAR TCLookup[MAX_KEYVALUELEN];
extern TCHAR TCMouselooktoggle[MAX_KEYVALUELEN];
extern TCHAR TCRight[MAX_KEYVALUELEN];
extern TCHAR TCRunwalk[MAX_KEYVALUELEN];
extern TCHAR TCSlideleft[MAX_KEYVALUELEN];
extern TCHAR TCSlideright[MAX_KEYVALUELEN];

extern TCHAR TCTabbackward[MAX_KEYVALUELEN];
extern TCHAR TCTabforward[MAX_KEYVALUELEN];
extern TCHAR TCTargetclear[MAX_KEYVALUELEN];
extern TCHAR TCTargetnext[MAX_KEYVALUELEN];
extern TCHAR TCTargetprevious[MAX_KEYVALUELEN];
extern TCHAR TCTargetself[MAX_KEYVALUELEN];
extern TCHAR TCSelecttarget[MAX_KEYVALUELEN];

extern TCHAR TCMap[MAX_KEYVALUELEN];
extern TCHAR TCMapzoomin[MAX_KEYVALUELEN];
extern TCHAR TCMapzoomout[MAX_KEYVALUELEN];

BOOL IsModifier(TCHAR* TCValue);
void InitModifierButton(TCHAR* TCButtonstring, CButton* modifier);
void AppendModifier(TCHAR* TCString, int iAppend);
void StripOffModifier(TCHAR* TCValue);
BOOL CheckforDuplicateBind(TCHAR* TCCompare);
void CheckforDuplicateBindM(TCHAR* TCCompare);
void BooltoString(BOOL bValue, TCHAR* TCValue);
BOOL StringtoBool(const TCHAR* TCValue);

#endif // PREFERENCES_COMMON_H
