#ifndef _PREFERENCES_H
#define _PREFERENCES_H

#pragma once
#include <afxdlgs.h>
#include "preferences_settings.h"
#include "preferences_common.h"
#include "preferences_options.h"
#include "preferences_movement.h"
#include "preferences_interaction.h"
#include "preferences_targeting.h"
#include "preferences_map.h"
#include "preferences_mouse.h"

class CPreferencesPropertySheet : public CPropertySheet
{
    DECLARE_DYNAMIC(CPreferencesPropertySheet)

public:
    CPreferencesPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CPreferencesPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    void UpdateINIFile();
    void ReadINIFile();
    void UpdateAllPreferences();
    void RestoreDefaults();
    virtual ~CPreferencesPropertySheet();

protected:
    CGeneralSettingsPage m_GeneralSettingsPage;
    COptionsPage m_OptionsPage;
    CMovementPage m_MovementPage;
    CInteractionPage m_InteractionPage;
    CTargetingPage m_TargetingPage;
    CMapPage m_MapPage;
    CMousePage m_MousePage;

    virtual BOOL OnInitDialog();
    afx_msg void OnCloseButtonClicked();
    afx_msg void OnOkButtonClicked();
    virtual void OnClose();

    DECLARE_MESSAGE_MAP()
};

#endif	/* #ifndef _PREFERENCES_H */
