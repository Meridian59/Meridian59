#include "stdafx.h"
#include "preferences.h"

// BOOL values
const char* const DEF_CLASSIC = "false";
const char* const DEF_QUICKCHAT = "false";
const char* const DEF_ALWAYSRUN = "true";
const char* const DEF_DYNAMIC = "true";
const char* const DEF_SOFTWARE = "false";
const char* const DEF_ATTACKONTARGET = "false";
const char* const DEF_GPU_EFFICIENCY = "true";

// Communication
const char* const DEF_BROADCAST = "b";
const char* const DEF_CHAT = "enter";
const char* const DEF_EMOTE = ";";
const char* const DEF_SAY = "f";
const char* const DEF_TELL = "t";
const char* const DEF_WHO = "w+ctrl";
const char* const DEF_YELL = "y";

// Interaction
const char* const DEF_ATTACK = "e+any";
const char* const DEF_BUY = "b+shift";
const char* const DEF_DEPOSIT = "i+shift";
const char* const DEF_EXAMINE = "mouse1+any";
const char* const DEF_LOOK = "l";
const char* const DEF_OFFER = "o+ctrl";
const char* const DEF_OPEN = "space";
const char* const DEF_PICKUP = "g";
const char* const DEF_WITHDRAW = "o+shift";

// Movement
const char* const DEF_BACKWARD = "s";
const char* const DEF_FLIP = "end";
const char* const DEF_FORWARD = "w";
const char* const DEF_LEFT = "left";
const char* const DEF_LOOKDOWN = "pagedown";
const char* const DEF_LOOKSTRAIGHT = "home";
const char* const DEF_LOOKUP = "pageup";
const char* const DEF_MOUSELOOKTOGGLE = "c+any";
const char* const DEF_RIGHT = "right";
const char* const DEF_RUNWALK = "shift";
const char* const DEF_SLIDELEFT = "a";
const char* const DEF_SLIDERIGHT = "d";

// Targeting
const char* const DEF_TABBACKWARD = "tab+shift";
const char* const DEF_TABFORWARD = "tab";
const char* const DEF_TARGETCLEAR = "esc";
const char* const DEF_TARGETNEXT = "]";
const char* const DEF_TARGETPREVIOUS = "[";
const char* const DEF_TARGETSELF = "q";
const char* const DEF_MOUSETARGET = "mouse0";

// Map
const char* const DEF_MAP = "m+shift";
const char* const DEF_MAPZOOMIN = "add";
const char* const DEF_MAPZOOMOUT = "subtract";

// Mouse
const char* const DEF_INVERT = "false";
//   Mouselook default X & Y Scale
const int DEF_MOUSELOOKXSCALE = 15;
const int DEF_MOUSELOOKYSCALE = 9;

bool WritePrivateProfileStringIfChanged(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
    TCHAR currentValue[MAX_KEYVALUELEN];
    DWORD nSize = MAX_KEYVALUELEN;

    // Read the current value from the INI file
    GetPrivateProfileString(lpAppName, lpKeyName, _T(""), currentValue, nSize, lpFileName);

    // Compare the current value with the new value
    if (_tcscmp(currentValue, lpString) != 0)
    {
        // Write the new value to the INI file
        WritePrivateProfileString(lpAppName, lpKeyName, lpString, lpFileName);
        return true;
    }

    return false;
}

IMPLEMENT_DYNAMIC(CPreferencesPropertySheet, CPropertySheet)

CPreferencesPropertySheet::CPreferencesPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    AddPage(&m_GeneralSettingsPage);
    AddPage(&m_OptionsPage);
    AddPage(&m_CommunicationPage);
    AddPage(&m_MovementPage);
    AddPage(&m_InteractionPage);
    AddPage(&m_TargetingPage);
    AddPage(&m_MapPage);
    AddPage(&m_MousePage);
}

CPreferencesPropertySheet::CPreferencesPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
    AddPage(&m_GeneralSettingsPage);
    AddPage(&m_OptionsPage);
    AddPage(&m_CommunicationPage);
    AddPage(&m_MovementPage);
	AddPage(&m_InteractionPage);
    AddPage(&m_TargetingPage);
    AddPage(&m_MapPage);
    AddPage(&m_MousePage);
}

BEGIN_MESSAGE_MAP(CPreferencesPropertySheet, CPropertySheet)
    ON_BN_CLICKED(IDOK, &CPreferencesPropertySheet::OnOkButtonClicked)
    ON_BN_CLICKED(IDCANCEL, &CPreferencesPropertySheet::OnCloseButtonClicked)
END_MESSAGE_MAP()


void CPreferencesPropertySheet::ReadINIFile()
{
    CString strSection;
    DWORD nSize=MAX_KEYVALUELEN;
    TCHAR ReturnedString[MAX_KEYVALUELEN];

    // Get the current working directory
    TCHAR currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);

    // Construct the full path to the config.ini file
    CString strINIFile;
    strINIFile.Format(_T("%s\\config.ini"), currentDir);

    strSection="config";

    GetPrivateProfileString(strSection, _T("classickeybindings"), _T("false"), ReturnedString, nSize, strINIFile);
    m_classic = _tcscmp(ReturnedString, _T("true")) == 0;

    GetPrivateProfileString(strSection, _T("quickchat"), _T("false"), ReturnedString, nSize, strINIFile);
    m_quickchat = _tcscmp(ReturnedString, _T("true")) == 0;

    GetPrivateProfileString(strSection, _T("alwaysrun"), _T("false"), ReturnedString, nSize, strINIFile);
    m_alwaysrun = _tcscmp(ReturnedString, _T("true")) == 0;

    GetPrivateProfileString(strSection, _T("dynamiclighting"), _T("false"), ReturnedString, nSize, strINIFile);
    m_dynamic = _tcscmp(ReturnedString, _T("true")) == 0;

    GetPrivateProfileString(strSection, _T("softwarerenderer"), _T("false"), ReturnedString, nSize, strINIFile);
    m_software = _tcscmp(ReturnedString, _T("true")) == 0;

    GetPrivateProfileString(strSection, _T("attackontarget"), _T("false"), ReturnedString, nSize, strINIFile);
    m_attackontarget = _tcscmp(ReturnedString, _T("true")) == 0;

    GetPrivateProfileString(strSection, _T("gpuefficiency"), _T("false"), ReturnedString, nSize, strINIFile);
    m_gpuefficiency = _tcscmp(ReturnedString, _T("true")) == 0;

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

BOOL CPreferencesPropertySheet::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    // Remove the default buttons
    GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);
    GetDlgItem(IDHELP)->ShowWindow(SW_HIDE);

    ReadINIFile();

    return bResult;
}

void CPreferencesPropertySheet::OnClose()
{
}

void CPreferencesPropertySheet::UpdateINIFile()
{
    // Get the current working directory
    TCHAR currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);

    // Construct the full path to the config.ini file
    CString strINIFile;
    strINIFile.Format(_T("%s\\config.ini"), currentDir);

    TCHAR Value[MAX_KEYVALUELEN];

    CString strSection = _T("config");
    DWORD nSize = MAX_KEYVALUELEN;

    bool settingsChanged = false;

    // Write values if they have changed
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("classickeybindings"), m_classic ? _T("true") : _T("false"), strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("quickchat"), m_quickchat ? _T("true") : _T("false"), strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("alwaysrun"), m_alwaysrun ? _T("true") : _T("false"), strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("dynamiclighting"), m_dynamic ? _T("true") : _T("false"), strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("softwarerenderer"), m_software ? _T("true") : _T("false"), strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("attackontarget"), m_attackontarget ? _T("true") : _T("false"), strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("gpuefficiency"), m_gpuefficiency ? _T("true") : _T("false"), strINIFile);

    BooltoString(bInvert, Value);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("invertmouse"), Value, strINIFile);
    sprintf(Value, "%d", iMouselookXscale);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("mouselookxscale"), Value, strINIFile);
    sprintf(Value, "%d", iMouselookYscale);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("mouselookyscale"), Value, strINIFile);

    strSection = _T("keys");

    // Communication
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("broadcast"), TCBroadcast, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("chat"), TCChat, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("emote"), TCEmote, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("say"), TCSay, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("tell"), TCTell, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("who"), TCWho, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("yell"), TCYell, strINIFile);

    // Interaction
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("attack"), TCAttack, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("buy"), TCBuy, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("deposit"), TCDeposit, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("examine"), TCExamine, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("look"), TCLook, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("offer"), TCOffer, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("open"), TCOpen, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("pickup"), TCPickup, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("withdraw"), TCWithdraw, strINIFile);

    // Movement
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("backward"), TCBackward, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("flip"), TCFlip, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("forward"), TCForward, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("left"), TCLeft, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("lookdown"), TCLookdown, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("lookstraight"), TCLookstraight, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("lookup"), TCLookup, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("mouselooktoggle"), TCMouselooktoggle, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("right"), TCRight, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("run/walk"), TCRunwalk, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("slideleft"), TCSlideleft, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("slideright"), TCSlideright, strINIFile);

    // Targeting
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("tabbackward"), TCTabbackward, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("tabforward"), TCTabforward, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("targetclear"), TCTargetclear, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("targetnext"), TCTargetnext, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("targetprevious"), TCTargetprevious, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("targetself"), TCTargetself, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("mousetarget"), TCSelecttarget, strINIFile);

    // Map
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("map"), TCMap, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("mapzoomin"), TCMapzoomin, strINIFile);
    settingsChanged |= WritePrivateProfileStringIfChanged(strSection, _T("mapzoomout"), TCMapzoomout, strINIFile);

    // Alert that changes to config.ini will require a client restart (e.g. graphic preferences)
    if (settingsChanged)
    {
        MessageBox("You must restart the Meridian 59 client before the new bindings or settings will take effect.", "Notice", MB_OK | MB_APPLMODAL);
    }
}

void CPreferencesPropertySheet::OnOkButtonClicked()
{
    // Update meridian.ini
    m_GeneralSettingsPage.SaveSettings();

    // Update config.ini
    UpdateINIFile();

    // Close the dialog and return IDOK
    EndDialog(IDOK);
}

void CPreferencesPropertySheet::OnCloseButtonClicked()
{
    // Close the dialog and return IDOK
    EndDialog(IDOK);
}

void CPreferencesPropertySheet::UpdateAllPreferences()
{
    m_MovementPage.UpdateDialogData();
    m_InteractionPage.UpdateDialogData();
    m_TargetingPage.UpdateDialogData();
    m_MapPage.UpdateDialogData();
    m_MousePage.UpdateDialogData();
}

void CPreferencesPropertySheet::RestoreDefaults(void)
{
    wchar_t szBuffer[256];  // Use wchar_t explicitly instead of TCHAR

    MultiByteToWideChar(CP_ACP, 0, DEF_INVERT, -1, szBuffer, 256);
    bInvert=StringtoBool(reinterpret_cast<TCHAR*>(szBuffer));

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


CPreferencesPropertySheet::~CPreferencesPropertySheet()
{
}
