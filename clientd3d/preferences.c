#include "client.h"

#include <algorithm>
#include <vector>

// Preferences constants

static constexpr int MAX_KEYVALUELEN = 128;

static constexpr int MODIFIER_NONE = 0;
static constexpr int MODIFIER_ALT = 1;
static constexpr int MODIFIER_CTRL = 2;
static constexpr int MODIFIER_SHIFT = 3;
static constexpr int MODIFIER_ANY = 4;

static const char* TCModifiers[5] = { "none", "alt", "ctrl", "shift", "any" };

// Preferences state

static bool bUpdateINI = false;

static char TCNewkey[MAX_KEYVALUELEN];

static bool bInvert;
static int iMouselookXscale;
static int iMouselookYscale;

static bool m_alwaysrun;
static bool m_classic;
static bool m_dynamic;
static bool m_quickchat;
static bool m_software;
static bool m_attackontarget;
static bool m_gpuefficiency;

static char TCBroadcast[MAX_KEYVALUELEN];
static char TCChat[MAX_KEYVALUELEN];
static char TCEmote[MAX_KEYVALUELEN];
static char TCSay[MAX_KEYVALUELEN];
static char TCTell[MAX_KEYVALUELEN];
static char TCWho[MAX_KEYVALUELEN];
static char TCYell[MAX_KEYVALUELEN];

static char TCAttack[MAX_KEYVALUELEN];
static char TCBuy[MAX_KEYVALUELEN];
static char TCDeposit[MAX_KEYVALUELEN];
static char TCExamine[MAX_KEYVALUELEN];
static char TCLook[MAX_KEYVALUELEN];
static char TCOffer[MAX_KEYVALUELEN];
static char TCOpen[MAX_KEYVALUELEN];
static char TCPickup[MAX_KEYVALUELEN];
static char TCWithdraw[MAX_KEYVALUELEN];

static char TCBackward[MAX_KEYVALUELEN];
static char TCFlip[MAX_KEYVALUELEN];
static char TCForward[MAX_KEYVALUELEN];
static char TCLeft[MAX_KEYVALUELEN];
static char TCLookdown[MAX_KEYVALUELEN];
static char TCLookstraight[MAX_KEYVALUELEN];
static char TCLookup[MAX_KEYVALUELEN];
static char TCMouselooktoggle[MAX_KEYVALUELEN];
static char TCRight[MAX_KEYVALUELEN];
static char TCRunwalk[MAX_KEYVALUELEN];
static char TCSlideleft[MAX_KEYVALUELEN];
static char TCSlideright[MAX_KEYVALUELEN];

static char TCTabbackward[MAX_KEYVALUELEN];
static char TCTabforward[MAX_KEYVALUELEN];
static char TCTargetclear[MAX_KEYVALUELEN];
static char TCTargetnext[MAX_KEYVALUELEN];
static char TCTargetprevious[MAX_KEYVALUELEN];
static char TCTargetself[MAX_KEYVALUELEN];
static char TCSelecttarget[MAX_KEYVALUELEN];

static char TCMap[MAX_KEYVALUELEN];
static char TCMapzoomin[MAX_KEYVALUELEN];
static char TCMapzoomout[MAX_KEYVALUELEN];

// Default values

// BOOL values
static const char* const DEF_CLASSIC = "false";
static const char* const DEF_QUICKCHAT = "false";
static const char* const DEF_ALWAYSRUN = "true";
static const char* const DEF_DYNAMIC = "true";
static const char* const DEF_SOFTWARE = "false";
static const char* const DEF_ATTACKONTARGET = "false";
static const char* const DEF_GPU_EFFICIENCY = "true";

// Communication
static const char* const DEF_BROADCAST = "b";
static const char* const DEF_CHAT = "enter";
static const char* const DEF_EMOTE = ";";
static const char* const DEF_SAY = "f";
static const char* const DEF_TELL = "t";
static const char* const DEF_WHO = "w+ctrl";
static const char* const DEF_YELL = "y";

// Interaction
static const char* const DEF_ATTACK = "e+any";
static const char* const DEF_BUY = "b+shift";
static const char* const DEF_DEPOSIT = "i+shift";
static const char* const DEF_EXAMINE = "mouse1+any";
static const char* const DEF_LOOK = "l";
static const char* const DEF_OFFER = "o+ctrl";
static const char* const DEF_OPEN = "space";
static const char* const DEF_PICKUP = "g";
static const char* const DEF_WITHDRAW = "o+shift";

// Movement
static const char* const DEF_BACKWARD = "s";
static const char* const DEF_FLIP = "end";
static const char* const DEF_FORWARD = "w";
static const char* const DEF_LEFT = "left";
static const char* const DEF_LOOKDOWN = "pagedown";
static const char* const DEF_LOOKSTRAIGHT = "home";
static const char* const DEF_LOOKUP = "pageup";
static const char* const DEF_MOUSELOOKTOGGLE = "c+any";
static const char* const DEF_RIGHT = "right";
static const char* const DEF_RUNWALK = "shift";
static const char* const DEF_SLIDELEFT = "a";
static const char* const DEF_SLIDERIGHT = "d";

// Targeting
static const char* const DEF_TABBACKWARD = "tab+shift";
static const char* const DEF_TABFORWARD = "tab";
static const char* const DEF_TARGETCLEAR = "esc";
static const char* const DEF_TARGETNEXT = "]";
static const char* const DEF_TARGETPREVIOUS = "[";
static const char* const DEF_TARGETSELF = "q";
static const char* const DEF_MOUSETARGET = "mouse0";

// Map
static const char* const DEF_MAP = "m+shift";
static const char* const DEF_MAPZOOMIN = "add";
static const char* const DEF_MAPZOOMOUT = "subtract";

// Mouse
static const char* const DEF_INVERT = "false";
static const int DEF_MOUSELOOKXSCALE = 15;
static const int DEF_MOUSELOOKYSCALE = 9;

// Message to tell our sub tabs to refresh their data
#define WM_USER_REINITDIALOG (WM_USER + 1)

// Function declarations
INT_PTR CALLBACK AssignKeyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CommonPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK OptionsPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MovementPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CommunicationPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK InteractionPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TargetingPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MapPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MousePreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Sub tab info strucuture and definitions
struct TabInfo {
    int title;
    int templateId;
    DLGPROC dlgProc;
};
std::vector<std::string> tabTitles;

std::vector<TabInfo> tabs = {
    {IDS_PREFERENCES, IDD_SETTINGS, CommonPreferencesDlgProc},
    {IDS_OPTIONS, IDD_OPTIONS, OptionsPreferencesDlgProc},
    {IDS_MOVE, IDD_MOVEMENT, MovementPreferencesDlgProc},
    {IDS_COMMS, IDD_COMMUNICATION, CommunicationPreferencesDlgProc},
    {IDS_INTERACT, IDD_INTERACTION, InteractionPreferencesDlgProc},
    {IDS_TARGET, IDD_TARGETING, TargetingPreferencesDlgProc},
    {IDS_MAP, IDD_MAP, MapPreferencesDlgProc},
    {IDS_MOUSE, IDD_MOUSE, MousePreferencesDlgProc}
};

// Helpers
static void BoolToString(bool bValue, char *TCValue)
{
    const char* value = bValue ? "true" : "false";
    snprintf(TCValue, sizeof(TCValue), "%s", value);
}

static bool StringToBool(const char *TCValue)
{
    if (TCValue == nullptr) return false;
    std::string strValue = TCValue;
    std::transform(strValue.begin(), strValue.end(), strValue.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return strValue == "true";
}

static bool WritePrivateProfileStringIfChanged(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
    char currentValue[MAX_KEYVALUELEN];
    DWORD nSize = MAX_KEYVALUELEN;

    // Read the current value from the INI file
    GetPrivateProfileString(lpAppName, lpKeyName, "", currentValue, nSize, lpFileName);

    // Compare the current value with the new value
    if (strcmp(currentValue, lpString) != 0)
    {
        // Write the new value to the INI file
        WritePrivateProfileString(lpAppName, lpKeyName, lpString, lpFileName);
        return true;
    }

    return false;
}

static void UpdateINIFile()
{
    // Get the current working directory
    char currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);

    // Construct the full path to the config.ini file
    char strINIFile[MAX_PATH];
    snprintf(strINIFile, sizeof(strINIFile), "%s\\config.ini", currentDir);

    char Value[MAX_KEYVALUELEN];

    const char* strSection = "config";
    DWORD nSize = MAX_KEYVALUELEN;

    bool settingsChanged = false;

    // Write values if they have changed
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "classickeybindings", m_classic ? "true" : "false", strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "quickchat", m_quickchat ? "true" : "false", strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "alwaysrun", m_alwaysrun ? "true" : "false", strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "dynamiclighting", m_dynamic ? "true" : "false", strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "softwarerenderer", m_software ? "true" : "false", strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "attackontarget", m_attackontarget ? "true" : "false", strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "gpuefficiency", m_gpuefficiency ? "true" : "false", strINIFile) || settingsChanged;

    BoolToString(bInvert, Value);
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "invertmouse", Value, strINIFile) || settingsChanged;
    snprintf(Value, sizeof(Value), "%d", iMouselookXscale);
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "mouselookxscale", Value, strINIFile) || settingsChanged;
    snprintf(Value, sizeof(Value), "%d", iMouselookYscale);
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "mouselookyscale", Value, strINIFile) || settingsChanged;

    strSection = "keys";

    // Communication
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "broadcast", TCBroadcast, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "chat", TCChat, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "emote", TCEmote, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "say", TCSay, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "tell", TCTell, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "who", TCWho, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "yell", TCYell, strINIFile) || settingsChanged;

    // Interaction
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "attack", TCAttack, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "buy", TCBuy, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "deposit", TCDeposit, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "examine", TCExamine, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "look", TCLook, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "offer", TCOffer, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "open", TCOpen, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "pickup", TCPickup, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "withdraw", TCWithdraw, strINIFile) || settingsChanged;

    // Movement
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "backward", TCBackward, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "flip", TCFlip, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "forward", TCForward, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "left", TCLeft, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "lookdown", TCLookdown, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "lookstraight", TCLookstraight, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "lookup", TCLookup, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "mouselooktoggle", TCMouselooktoggle, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "right", TCRight, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "run/walk", TCRunwalk, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "slideleft", TCSlideleft, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "slideright", TCSlideright, strINIFile) || settingsChanged;

    // Targeting
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "tabbackward", TCTabbackward, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "tabforward", TCTabforward, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "targetclear", TCTargetclear, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "targetnext", TCTargetnext, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "targetprevious", TCTargetprevious, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "targetself", TCTargetself, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "mousetarget", TCSelecttarget, strINIFile) || settingsChanged;

    // Map
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "map", TCMap, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "mapzoomin", TCMapzoomin, strINIFile) || settingsChanged;
    settingsChanged = WritePrivateProfileStringIfChanged(strSection, "mapzoomout", TCMapzoomout, strINIFile) || settingsChanged;

    // Alert that changes to config.ini will require a client restart (e.g. graphic preferences)
    if (settingsChanged)
    {
        MessageBox(NULL, GetString(hInst, IDS_SETTINGS_RESTART), GetString(hInst, IDS_NOTICE_TITLE), MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
    }
}

static void ReadINIFile()
{
    char strSection[MAX_KEYVALUELEN];
    DWORD nSize = MAX_KEYVALUELEN;
    char ReturnedString[MAX_KEYVALUELEN];

    // Get the current working directory
    char currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);

    // Construct the full path to the config.ini file
    char strINIFile[MAX_PATH];
    snprintf(strINIFile, sizeof(strINIFile), "%s\\config.ini", currentDir);

    strcpy_s(strSection, sizeof(strSection), "config");

    GetPrivateProfileString(strSection, "classickeybindings", "false", ReturnedString, nSize, strINIFile);
    m_classic = strcmp(ReturnedString, "true") == 0;

    GetPrivateProfileString(strSection, "quickchat", "false", ReturnedString, nSize, strINIFile);
    m_quickchat = strcmp(ReturnedString, "true") == 0;

    GetPrivateProfileString(strSection, "alwaysrun", "false", ReturnedString, nSize, strINIFile);
    m_alwaysrun = strcmp(ReturnedString, "true") == 0;

    GetPrivateProfileString(strSection, "dynamiclighting", "false", ReturnedString, nSize, strINIFile);
    m_dynamic = strcmp(ReturnedString, "true") == 0;

    GetPrivateProfileString(strSection, "softwarerenderer", "false", ReturnedString, nSize, strINIFile);
    m_software = strcmp(ReturnedString, "true") == 0;

    GetPrivateProfileString(strSection, "attackontarget", "false", ReturnedString, nSize, strINIFile);
    m_attackontarget = strcmp(ReturnedString, "true") == 0;

    GetPrivateProfileString(strSection, "gpuefficiency", "false", ReturnedString, nSize, strINIFile);
    m_gpuefficiency = strcmp(ReturnedString, "true") == 0;

    GetPrivateProfileString(strSection, "invertmouse", "false", ReturnedString, nSize, strINIFile);
    bInvert = strcmp(ReturnedString, "true") == 0;

    iMouselookXscale = GetPrivateProfileInt(strSection, "mouselookxscale", DEF_MOUSELOOKXSCALE, strINIFile);
    iMouselookYscale = GetPrivateProfileInt(strSection, "mouselookyscale", DEF_MOUSELOOKYSCALE, strINIFile);

    strcpy_s(strSection, sizeof(strSection), "keys");

    // Communication
    GetPrivateProfileString(strSection, "broadcast", DEF_BROADCAST, TCBroadcast, nSize, strINIFile);
    GetPrivateProfileString(strSection, "chat", DEF_CHAT, TCChat, nSize, strINIFile);
    GetPrivateProfileString(strSection, "emote", DEF_EMOTE, TCEmote, nSize, strINIFile);
    GetPrivateProfileString(strSection, "say", DEF_SAY, TCSay, nSize, strINIFile);
    GetPrivateProfileString(strSection, "tell", DEF_TELL, TCTell, nSize, strINIFile);
    GetPrivateProfileString(strSection, "who", DEF_WHO, TCWho, nSize, strINIFile);
    GetPrivateProfileString(strSection, "yell", DEF_YELL, TCYell, nSize, strINIFile);

    // Interaction
    GetPrivateProfileString(strSection, "attack", DEF_ATTACK, TCAttack, nSize, strINIFile);
    GetPrivateProfileString(strSection, "buy", DEF_BUY, TCBuy, nSize, strINIFile);
    GetPrivateProfileString(strSection, "deposit", DEF_DEPOSIT, TCDeposit, nSize, strINIFile);
    GetPrivateProfileString(strSection, "examine", DEF_EXAMINE, TCExamine, nSize, strINIFile);
    GetPrivateProfileString(strSection, "look", DEF_LOOK, TCLook, nSize, strINIFile);
    GetPrivateProfileString(strSection, "offer", DEF_OFFER, TCOffer, nSize, strINIFile);
    GetPrivateProfileString(strSection, "open", DEF_OPEN, TCOpen, nSize, strINIFile);
    GetPrivateProfileString(strSection, "pickup", DEF_PICKUP, TCPickup, nSize, strINIFile);
    GetPrivateProfileString(strSection, "withdraw", DEF_WITHDRAW, TCWithdraw, nSize, strINIFile);

    // Movement
    GetPrivateProfileString(strSection, "backward", DEF_BACKWARD, TCBackward, nSize, strINIFile);
    GetPrivateProfileString(strSection, "flip", DEF_FLIP, TCFlip, nSize, strINIFile);
    GetPrivateProfileString(strSection, "forward", DEF_FORWARD, TCForward, nSize, strINIFile);
    GetPrivateProfileString(strSection, "left", DEF_LEFT, TCLeft, nSize, strINIFile);
    GetPrivateProfileString(strSection, "lookdown", DEF_LOOKDOWN, TCLookdown, nSize, strINIFile);
    GetPrivateProfileString(strSection, "lookstraight", DEF_LOOKSTRAIGHT, TCLookstraight, nSize, strINIFile);
    GetPrivateProfileString(strSection, "lookup", DEF_LOOKUP, TCLookup, nSize, strINIFile);
    GetPrivateProfileString(strSection, "mouselooktoggle", DEF_MOUSELOOKTOGGLE, TCMouselooktoggle, nSize, strINIFile);
    GetPrivateProfileString(strSection, "right", DEF_RIGHT, TCRight, nSize, strINIFile);
    GetPrivateProfileString(strSection, "run/walk", DEF_RUNWALK, TCRunwalk, nSize, strINIFile);
    GetPrivateProfileString(strSection, "slideleft", DEF_SLIDELEFT, TCSlideleft, nSize, strINIFile);
    GetPrivateProfileString(strSection, "slideright", DEF_SLIDERIGHT, TCSlideright, nSize, strINIFile);

    // Targeting
    GetPrivateProfileString(strSection, "tabbackward", DEF_TABBACKWARD, TCTabbackward, nSize, strINIFile);
    GetPrivateProfileString(strSection, "tabforward", DEF_TABFORWARD, TCTabforward, nSize, strINIFile);
    GetPrivateProfileString(strSection, "targetclear", DEF_TARGETCLEAR, TCTargetclear, nSize, strINIFile);
    GetPrivateProfileString(strSection, "targetnext", DEF_TARGETNEXT, TCTargetnext, nSize, strINIFile);
    GetPrivateProfileString(strSection, "targetprevious", DEF_TARGETPREVIOUS, TCTargetprevious, nSize, strINIFile);
    GetPrivateProfileString(strSection, "targetself", DEF_TARGETSELF, TCTargetself, nSize, strINIFile);
    GetPrivateProfileString(strSection, "mousetarget", DEF_MOUSETARGET, TCSelecttarget, nSize, strINIFile);

    // Map
    GetPrivateProfileString(strSection, "map", DEF_MAP, TCMap, nSize, strINIFile);
    GetPrivateProfileString(strSection, "mapzoomin", DEF_MAPZOOMIN, TCMapzoomin, nSize, strINIFile);
    GetPrivateProfileString(strSection, "mapzoomout", DEF_MAPZOOMOUT, TCMapzoomout, nSize, strINIFile);
}

static bool IsModifier(const char *TCValue)
{
    for (unsigned int i = MODIFIER_ALT; i < MODIFIER_ANY; i++)
    {
        if (strcmp(TCValue, TCModifiers[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

static void InitModifierButton(const char *TCButtonstring, HWND modifier)
{
    if (IsModifier(TCButtonstring))
    {
        // Disable Modifier Combo
        EnableWindow(modifier, FALSE);
    }
}

static void AppendModifier(char *TCString, int iAppend)
{
    char TCAppend[128];
    if (iAppend > MODIFIER_NONE && iAppend <= MODIFIER_ANY)
    {
        TCAppend[0] = '+';
        TCAppend[1] = '\0';
        strncat(TCAppend, TCModifiers[iAppend], sizeof(TCAppend) - strlen(TCAppend) - 1);
    }
    else
    {
        return;
    }

    strncat(TCString, TCAppend, MAX_KEYVALUELEN - strlen(TCString) - 1);
}

static void StripOffModifier(char *TCValue)
{
    for (unsigned int i = 0; i < strlen(TCValue); i++)
    {
        if (TCValue[i] == '+')
        {
            TCValue[i] = 0;
            break;
        }
    }
}

static bool CheckforDuplicateBind(const char *TCCompare)
{
    if (strcmp(TCCompare, TCBroadcast) == 0 && TCCompare != TCBroadcast) return true;
    if (strcmp(TCCompare, TCChat) == 0 && TCCompare != TCChat) return true;
    if (strcmp(TCCompare, TCEmote) == 0 && TCCompare != TCEmote) return true;
    if (strcmp(TCCompare, TCSay) == 0 && TCCompare != TCSay) return true;
    if (strcmp(TCCompare, TCTell) == 0 && TCCompare != TCTell) return true;
    if (strcmp(TCCompare, TCWho) == 0 && TCCompare != TCWho) return true;
    if (strcmp(TCCompare, TCYell) == 0 && TCCompare != TCYell) return true;

    if (strcmp(TCCompare, TCAttack) == 0 && TCCompare != TCAttack) return true;
    if (strcmp(TCCompare, TCBuy) == 0 && TCCompare != TCBuy) return true;
    if (strcmp(TCCompare, TCDeposit) == 0 && TCCompare != TCDeposit) return true;
    if (strcmp(TCCompare, TCExamine) == 0 && TCCompare != TCExamine) return true;
    if (strcmp(TCCompare, TCLook) == 0 && TCCompare != TCLook) return true;
    if (strcmp(TCCompare, TCOffer) == 0 && TCCompare != TCOffer) return true;
    if (strcmp(TCCompare, TCOpen) == 0 && TCCompare != TCOpen) return true;
    if (strcmp(TCCompare, TCPickup) == 0 && TCCompare != TCPickup) return true;
    if (strcmp(TCCompare, TCWithdraw) == 0 && TCCompare != TCWithdraw) return true;

    if (strcmp(TCCompare, TCBackward) == 0 && TCCompare != TCBackward) return true;
    if (strcmp(TCCompare, TCFlip) == 0 && TCCompare != TCFlip) return true;
    if (strcmp(TCCompare, TCForward) == 0 && TCCompare != TCForward) return true;
    if (strcmp(TCCompare, TCLeft) == 0 && TCCompare != TCLeft) return true;
    if (strcmp(TCCompare, TCLookdown) == 0 && TCCompare != TCLookdown) return true;
    if (strcmp(TCCompare, TCLookstraight) == 0 && TCCompare != TCLookstraight) return true;
    if (strcmp(TCCompare, TCLookup) == 0 && TCCompare != TCLookup) return true;
    if (strcmp(TCCompare, TCMouselooktoggle) == 0 && TCCompare != TCMouselooktoggle) return true;
    if (strcmp(TCCompare, TCRight) == 0 && TCCompare != TCRight) return true;
    if (strcmp(TCCompare, TCRunwalk) == 0 && TCCompare != TCRunwalk) return true;
    if (strcmp(TCCompare, TCSlideleft) == 0 && TCCompare != TCSlideleft) return true;
    if (strcmp(TCCompare, TCSlideright) == 0 && TCCompare != TCSlideright) return true;

    if (strcmp(TCCompare, TCTabbackward) == 0 && TCCompare != TCTabbackward) return true;
    if (strcmp(TCCompare, TCTabforward) == 0 && TCCompare != TCTabforward) return true;
    if (strcmp(TCCompare, TCTargetclear) == 0 && TCCompare != TCTargetclear) return true;
    if (strcmp(TCCompare, TCTargetnext) == 0 && TCCompare != TCTargetnext) return true;
    if (strcmp(TCCompare, TCTargetprevious) == 0 && TCCompare != TCTargetprevious) return true;
    if (strcmp(TCCompare, TCTargetself) == 0 && TCCompare != TCTargetself) return true;
    if (strcmp(TCCompare, TCSelecttarget) == 0 && TCCompare != TCSelecttarget) return true;

    if (strcmp(TCCompare, TCMap) == 0 && TCCompare != TCMap) return true;
    if (strcmp(TCCompare, TCMapzoomin) == 0 && TCCompare != TCMapzoomin) return true;
    if (strcmp(TCCompare, TCMapzoomout) == 0 && TCCompare != TCMapzoomout) return true;
    return false;
}

static void CheckforDuplicateBindM(const char *TCCompare)
{
    if (CheckforDuplicateBind(TCCompare))
    {
        MessageBox(NULL, GetString(hInst, IDS_DUPLICATE_BINDINGS), GetString(hInst, IDS_WARNING_TITLE), MB_OK | MB_ICONEXCLAMATION);
    }
}

// Function to handle key assignment
static void AssignKey(HWND hDlg, TCHAR* TCButtonstring, int nID) {
    // Show the Assign Key dialog
    if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ASSIGNKEY), hDlg, AssignKeyDlgProc) == IDOK) {
        strcpy_s(TCButtonstring, MAX_KEYVALUELEN, TCNewkey);
        SetDlgItemText(hDlg, nID, TCButtonstring);
        CheckforDuplicateBindM(TCButtonstring);
    }
}

int iModifier;
HHOOK hHook = NULL;

// Dialog procedure for the Modifier dialog
INT_PTR CALLBACK ModifierDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ALT:
            iModifier = MODIFIER_ALT;
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        case IDC_CTRL:
            iModifier = MODIFIER_CTRL;
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        case IDC_SHIFT:
            iModifier = MODIFIER_SHIFT;
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        case IDC_ANY:
            iModifier = MODIFIER_ANY;
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        case IDC_NONE:
            iModifier = MODIFIER_NONE;
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Function to handle key modification
static void ModifyKey(HWND hDlg, TCHAR* TCString, int nID)
{
    // Show the Modifier dialog
    if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MODIFIER), hDlg, ModifierDlgProc) == IDOK)
    {
        strncat(TCString, "+", MAX_KEYVALUELEN - strlen(TCString) - 1);

        const char* modifierStr =
            (iModifier == MODIFIER_ALT) ? "alt" :
            (iModifier == MODIFIER_CTRL) ? "ctrl" :
            (iModifier == MODIFIER_SHIFT) ? "shift" :
            (iModifier == MODIFIER_ANY) ? "any" : "none";

        strncat(TCString, modifierStr, MAX_KEYVALUELEN - strlen(TCString) - 1);
        SetDlgItemText(hDlg, nID, TCString);
    }
}

static void RestoreDefaults(HWND hDlg)
{
    wchar_t szBuffer[256];
    char szBufferA[256];

    MultiByteToWideChar(CP_ACP, 0, DEF_INVERT, -1, szBuffer, 256);
    WideCharToMultiByte(CP_ACP, 0, szBuffer, -1, szBufferA, 256, NULL, NULL);
    bInvert = StringToBool(szBufferA);

    iMouselookXscale = DEF_MOUSELOOKXSCALE;
    iMouselookYscale = DEF_MOUSELOOKYSCALE;

    strcpy_s(TCBroadcast, sizeof(TCBroadcast), DEF_BROADCAST);
    strcpy_s(TCChat, sizeof(TCChat), DEF_CHAT);
    strcpy_s(TCEmote, sizeof(TCEmote), DEF_EMOTE);
    strcpy_s(TCSay, sizeof(TCSay), DEF_SAY);
    strcpy_s(TCTell, sizeof(TCTell), DEF_TELL);
    strcpy_s(TCWho, sizeof(TCWho), DEF_WHO);
    strcpy_s(TCYell, sizeof(TCYell), DEF_YELL);

    strcpy_s(TCAttack, sizeof(TCAttack), DEF_ATTACK);
    strcpy_s(TCBuy, sizeof(TCBuy), DEF_BUY);
    strcpy_s(TCDeposit, sizeof(TCDeposit), DEF_DEPOSIT);
    strcpy_s(TCExamine, sizeof(TCExamine), DEF_EXAMINE);
    strcpy_s(TCLook, sizeof(TCLook), DEF_LOOK);
    strcpy_s(TCOffer, sizeof(TCOffer), DEF_OFFER);
    strcpy_s(TCOpen, sizeof(TCOpen), DEF_OPEN);
    strcpy_s(TCPickup, sizeof(TCPickup), DEF_PICKUP);
    strcpy_s(TCWithdraw, sizeof(TCWithdraw), DEF_WITHDRAW);

    strcpy_s(TCBackward, sizeof(TCBackward), DEF_BACKWARD);
    strcpy_s(TCFlip, sizeof(TCFlip), DEF_FLIP);
    strcpy_s(TCForward, sizeof(TCForward), DEF_FORWARD);
    strcpy_s(TCLeft, sizeof(TCLeft), DEF_LEFT);
    strcpy_s(TCLookdown, sizeof(TCLookdown), DEF_LOOKDOWN);
    strcpy_s(TCLookstraight, sizeof(TCLookstraight), DEF_LOOKSTRAIGHT);
    strcpy_s(TCLookup, sizeof(TCLookup), DEF_LOOKUP);
    strcpy_s(TCMouselooktoggle, sizeof(TCMouselooktoggle), DEF_MOUSELOOKTOGGLE);
    strcpy_s(TCRight, sizeof(TCRight), DEF_RIGHT);
    strcpy_s(TCRunwalk, sizeof(TCRunwalk), DEF_RUNWALK);
    strcpy_s(TCSlideleft, sizeof(TCSlideleft), DEF_SLIDELEFT);
    strcpy_s(TCSlideright, sizeof(TCSlideright), DEF_SLIDERIGHT);

    strcpy_s(TCTabbackward, sizeof(TCTabbackward), DEF_TABBACKWARD);
    strcpy_s(TCTabforward, sizeof(TCTabforward), DEF_TABFORWARD);
    strcpy_s(TCTargetclear, sizeof(TCTargetclear), DEF_TARGETCLEAR);
    strcpy_s(TCTargetnext, sizeof(TCTargetnext), DEF_TARGETNEXT);
    strcpy_s(TCTargetprevious, sizeof(TCTargetprevious), DEF_TARGETPREVIOUS);
    strcpy_s(TCTargetself, sizeof(TCTargetself), DEF_TARGETSELF);
    strcpy_s(TCSelecttarget, sizeof(TCSelecttarget), DEF_MOUSETARGET);

    strcpy_s(TCMap, sizeof(TCMap), DEF_MAP);
    strcpy_s(TCMapzoomin, sizeof(TCMapzoomin), DEF_MAPZOOMIN);
    strcpy_s(TCMapzoomout, sizeof(TCMapzoomout), DEF_MAPZOOMOUT);


    // Update the options dialog with the default settings.
    CheckDlgButton(hDlg, IDC_ALWAYSRUN, m_alwaysrun);
    CheckDlgButton(hDlg, IDC_CLASSIC, m_classic);
    CheckDlgButton(hDlg, IDC_DYNAMIC, m_dynamic);
    CheckDlgButton(hDlg, IDC_QUICKCHAT, m_quickchat);
    CheckDlgButton(hDlg, IDC_SOFTWARE, m_software);
    CheckDlgButton(hDlg, IDC_ATTACKONTARGET, m_attackontarget);
    CheckDlgButton(hDlg, IDC_GPU_EFFICIENCY, m_gpuefficiency);

    // Select the correct options on each preferences tab.
    HWND hParent = GetParent(hDlg);
    for (size_t i = 0; i < tabs.size(); ++i) {
        HWND hTab = PropSheet_IndexToHwnd(hParent, static_cast<int>(i));
        SendMessage(hTab, WM_USER_REINITDIALOG, 0, 0);
    }
}

// Function to initialize mouse settings
static void InitializeMouseSettings(HWND hDlg)
{
    CheckDlgButton(hDlg, IDC_INVERT, bInvert ? BST_CHECKED : BST_UNCHECKED);
    SendMessage(GetDlgItem(hDlg, IDC_MOUSEXSCALE), TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    SendMessage(GetDlgItem(hDlg, IDC_MOUSEXSCALE), TBM_SETPOS, TRUE, iMouselookXscale);
    SendMessage(GetDlgItem(hDlg, IDC_MOUSEYSCALE), TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    SendMessage(GetDlgItem(hDlg, IDC_MOUSEYSCALE), TBM_SETPOS, TRUE, iMouselookYscale);
    SetDlgItemInt(hDlg, IDC_MOUSEXSCALEVALUE, iMouselookXscale, FALSE);
    SetDlgItemInt(hDlg, IDC_MOUSEYSCALEVALUE, iMouselookYscale, FALSE);

    // Setup the tick tracks for this tab view.
    HWND hTrackbarX = GetDlgItem(hDlg, IDC_MOUSEXSCALE);
    HWND hTrackbarY = GetDlgItem(hDlg, IDC_MOUSEYSCALE);

    // Set range
    SendMessage(hTrackbarX, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    SendMessage(hTrackbarY, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));

    // Set tick frequency (try 5 or 10)
    SendMessage(hTrackbarX, TBM_SETTICFREQ, 5, 0);
    SendMessage(hTrackbarY, TBM_SETTICFREQ, 5, 0);

    // Optional: Force a redraw
    InvalidateRect(hTrackbarX, NULL, TRUE);
    InvalidateRect(hTrackbarY, NULL, TRUE);
}

// Dialog procedure for the Mouse Preferences dialog
static INT_PTR CALLBACK MousePreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_USER_REINITDIALOG:
    case WM_INITDIALOG:
        {
        InitializeMouseSettings(hDlg);
        return (INT_PTR)TRUE;
        }
    case WM_HSCROLL:
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_MOUSEXSCALE)) {
            iMouselookXscale = SendMessage(GetDlgItem(hDlg, IDC_MOUSEXSCALE), TBM_GETPOS, 0, 0);
            SetDlgItemInt(hDlg, IDC_MOUSEXSCALEVALUE, iMouselookXscale, FALSE);
        } else if ((HWND)lParam == GetDlgItem(hDlg, IDC_MOUSEYSCALE)) {
            iMouselookYscale = SendMessage(GetDlgItem(hDlg, IDC_MOUSEYSCALE), TBM_GETPOS, 0, 0);
            SetDlgItemInt(hDlg, IDC_MOUSEYSCALEVALUE, iMouselookYscale, FALSE);
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_INVERT) {
            bInvert = IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED;
        }
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY)
        {
            // Save settings
            UpdateINIFile();
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Function to initialize map key bindings
static void InitializeMapKeyBindings(HWND hDlg) {
    SetDlgItemText(hDlg, IDC_MAP, TCMap);
    SetDlgItemText(hDlg, IDC_MAPZOOMIN, TCMapzoomin);
    SetDlgItemText(hDlg, IDC_MAPZOOMOUT, TCMapzoomout);

    // Initialize modifier buttons
    InitModifierButton(TCMap, GetDlgItem(hDlg, IDC_MAP_MOD));
    InitModifierButton(TCMapzoomin, GetDlgItem(hDlg, IDC_MAPZOOMIN_MOD));
    InitModifierButton(TCMapzoomout, GetDlgItem(hDlg, IDC_MAPZOOMOUT_MOD));
}

// Dialog procedure for the Map Preferences dialog
static INT_PTR CALLBACK MapPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_USER_REINITDIALOG:
    case WM_INITDIALOG:
        InitializeMapKeyBindings(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_MAP:
            AssignKey(hDlg, TCMap, IDC_MAP);
            break;
        case IDC_MAPZOOMIN:
            AssignKey(hDlg, TCMapzoomin, IDC_MAPZOOMIN);
            break;
        case IDC_MAPZOOMOUT:
            AssignKey(hDlg, TCMapzoomout, IDC_MAPZOOMOUT);
            break;
        case IDC_MAP_MOD:
            ModifyKey(hDlg, TCMap, IDC_MAP);
            break;
        case IDC_MAPZOOMIN_MOD:
            ModifyKey(hDlg, TCMapzoomin, IDC_MAPZOOMIN);
            break;
        case IDC_MAPZOOMOUT_MOD:
            ModifyKey(hDlg, TCMapzoomout, IDC_MAPZOOMOUT);
            break;
        }
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY)
        {
            // Save settings
            UpdateINIFile();
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static void InitializeTargetingKeyBindings(HWND hDlg) {
    SetDlgItemText(hDlg, IDC_TABBACKWARD, TCTabbackward);
    SetDlgItemText(hDlg, IDC_TABFORWARD, TCTabforward);
    SetDlgItemText(hDlg, IDC_TARGETCLEAR, TCTargetclear);
    SetDlgItemText(hDlg, IDC_TARGETNEXT, TCTargetnext);
    SetDlgItemText(hDlg, IDC_TARGETPREVIOUS, TCTargetprevious);
    SetDlgItemText(hDlg, IDC_TARGETSELF, TCTargetself);
    SetDlgItemText(hDlg, IDC_SELECTTARGET, TCSelecttarget);

    // Initialize modifier buttons
    InitModifierButton(TCTabbackward, GetDlgItem(hDlg, IDC_TABBACKWARD_MOD));
    InitModifierButton(TCTabforward, GetDlgItem(hDlg, IDC_TABFORWARD_MOD));
    InitModifierButton(TCTargetclear, GetDlgItem(hDlg, IDC_TARGETCLEAR_MOD));
    InitModifierButton(TCTargetnext, GetDlgItem(hDlg, IDC_TARGETNEXT_MOD));
    InitModifierButton(TCTargetprevious, GetDlgItem(hDlg, IDC_TARGETPREVIOUS_MOD));
    InitModifierButton(TCTargetself, GetDlgItem(hDlg, IDC_TARGETSELF_MOD));
    InitModifierButton(TCSelecttarget, GetDlgItem(hDlg, IDC_SELECTTARGET_MOD));
}

// Dialog procedure for the Targeting Preferences dialog
static INT_PTR CALLBACK TargetingPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    case WM_USER_REINITDIALOG:
        InitializeTargetingKeyBindings(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_TABBACKWARD:
            AssignKey(hDlg, TCTabbackward, IDC_TABBACKWARD);
            break;
        case IDC_TABFORWARD:
            AssignKey(hDlg, TCTabforward, IDC_TABFORWARD);
            break;
        case IDC_TARGETCLEAR:
            AssignKey(hDlg, TCTargetclear, IDC_TARGETCLEAR);
            break;
        case IDC_TARGETNEXT:
            AssignKey(hDlg, TCTargetnext, IDC_TARGETNEXT);
            break;
        case IDC_TARGETPREVIOUS:
            AssignKey(hDlg, TCTargetprevious, IDC_TARGETPREVIOUS);
            break;
        case IDC_TARGETSELF:
            AssignKey(hDlg, TCTargetself, IDC_TARGETSELF);
            break;
        case IDC_SELECTTARGET:
            AssignKey(hDlg, TCSelecttarget, IDC_SELECTTARGET);
            break;
        case IDC_TABBACKWARD_MOD:
            ModifyKey(hDlg, TCTabbackward, IDC_TABBACKWARD);
            break;
        case IDC_TABFORWARD_MOD:
            ModifyKey(hDlg, TCTabforward, IDC_TABFORWARD);
            break;
        case IDC_TARGETCLEAR_MOD:
            ModifyKey(hDlg, TCTargetclear, IDC_TARGETCLEAR);
            break;
        case IDC_TARGETNEXT_MOD:
            ModifyKey(hDlg, TCTargetnext, IDC_TARGETNEXT);
            break;
        case IDC_TARGETPREVIOUS_MOD:
            ModifyKey(hDlg, TCTargetprevious, IDC_TARGETPREVIOUS);
            break;
        case IDC_TARGETSELF_MOD:
            ModifyKey(hDlg, TCTargetself, IDC_TARGETSELF);
            break;
        case IDC_SELECTTARGET_MOD:
            ModifyKey(hDlg, TCSelecttarget, IDC_SELECTTARGET_MOD);
            break;
        }
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY)
        {
            // Save settings
            UpdateINIFile();
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Function to initialize interaction key bindings
static void InitializeInteractionKeyBindings(HWND hDlg) {
    SetDlgItemText(hDlg, IDC_ATTACK, TCAttack);
    SetDlgItemText(hDlg, IDC_BUY, TCBuy);
    SetDlgItemText(hDlg, IDC_DEPOSIT, TCDeposit);
    SetDlgItemText(hDlg, IDC_EXAMINE, TCExamine);
    SetDlgItemText(hDlg, IDC_LOOK, TCLook);
    SetDlgItemText(hDlg, IDC_OFFER, TCOffer);
    SetDlgItemText(hDlg, IDC_OPEN, TCOpen);
    SetDlgItemText(hDlg, IDC_PICKUP, TCPickup);
    SetDlgItemText(hDlg, IDC_WITHDRAW, TCWithdraw);

    // Initialize modifier buttons
    InitModifierButton(TCAttack, GetDlgItem(hDlg, IDC_ATTACK_MOD));
    InitModifierButton(TCBuy, GetDlgItem(hDlg, IDC_BUY_MOD));
    InitModifierButton(TCDeposit, GetDlgItem(hDlg, IDC_DEPOSIT_MOD));
    InitModifierButton(TCExamine, GetDlgItem(hDlg, IDC_EXAMINE_MOD));
    InitModifierButton(TCLook, GetDlgItem(hDlg, IDC_LOOK_MOD));
    InitModifierButton(TCOffer, GetDlgItem(hDlg, IDC_OFFER_MOD));
    InitModifierButton(TCOpen, GetDlgItem(hDlg, IDC_OPEN_MOD));
    InitModifierButton(TCPickup, GetDlgItem(hDlg, IDC_PICKUP_MOD));
    InitModifierButton(TCWithdraw, GetDlgItem(hDlg, IDC_WITHDRAW_MOD));
}

// Dialog procedure for the Interaction Preferences dialog
static INT_PTR CALLBACK InteractionPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    case WM_USER_REINITDIALOG:
        InitializeInteractionKeyBindings(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ATTACK:
            AssignKey(hDlg, TCAttack, IDC_ATTACK);
            break;
        case IDC_BUY:
            AssignKey(hDlg, TCBuy, IDC_BUY);
            break;
        case IDC_DEPOSIT:
            AssignKey(hDlg, TCDeposit, IDC_DEPOSIT);
            break;
        case IDC_EXAMINE:
            AssignKey(hDlg, TCExamine, IDC_EXAMINE);
            break;
        case IDC_LOOK:
            AssignKey(hDlg, TCLook, IDC_LOOK);
            break;
        case IDC_OFFER:
            AssignKey(hDlg, TCOffer, IDC_OFFER);
            break;
        case IDC_OPEN:
            AssignKey(hDlg, TCOpen, IDC_OPEN);
            break;
        case IDC_PICKUP:
            AssignKey(hDlg, TCPickup, IDC_PICKUP);
            break;
        case IDC_WITHDRAW:
            AssignKey(hDlg, TCWithdraw, IDC_WITHDRAW);
            break;
        case IDC_ATTACK_MOD:
            ModifyKey(hDlg, TCAttack, IDC_ATTACK);
            break;
        case IDC_BUY_MOD:
            ModifyKey(hDlg, TCBuy, IDC_BUY);
            break;
        case IDC_DEPOSIT_MOD:
            ModifyKey(hDlg, TCDeposit, IDC_DEPOSIT);
            break;
        case IDC_EXAMINE_MOD:
            ModifyKey(hDlg, TCExamine, IDC_EXAMINE);
            break;
        case IDC_LOOK_MOD:
            ModifyKey(hDlg, TCLook, IDC_LOOK);
            break;
        case IDC_OFFER_MOD:
            ModifyKey(hDlg, TCOffer, IDC_OFFER);
            break;
        case IDC_OPEN_MOD:
            ModifyKey(hDlg, TCOpen, IDC_OPEN);
            break;
        case IDC_PICKUP_MOD:
            ModifyKey(hDlg, TCPickup, IDC_PICKUP);
            break;
        case IDC_WITHDRAW_MOD:
            ModifyKey(hDlg, TCWithdraw, IDC_WITHDRAW);
            break;
        }
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY)
        {
            // Save settings
            UpdateINIFile();
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static INT_PTR CALLBACK OptionsPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    case WM_USER_REINITDIALOG:
        // Initialize dialog with current settings
        CheckDlgButton(hDlg, IDC_ALWAYSRUN, m_alwaysrun);
        CheckDlgButton(hDlg, IDC_CLASSIC, m_classic);
        CheckDlgButton(hDlg, IDC_DYNAMIC, m_dynamic);
        CheckDlgButton(hDlg, IDC_QUICKCHAT, m_quickchat);
        CheckDlgButton(hDlg, IDC_SOFTWARE, m_software);
        CheckDlgButton(hDlg, IDC_ATTACKONTARGET, m_attackontarget);
        CheckDlgButton(hDlg, IDC_GPU_EFFICIENCY, m_gpuefficiency);
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CLASSIC:
            m_classic = IsDlgButtonChecked(hDlg, IDC_CLASSIC);
            if (m_classic)
            {
                MessageBox(hDlg, GetString(hInst, IDS_CLASSIC_BIND_WARNING), GetString(hInst, IDS_NOTICE_TITLE), MB_OK | MB_ICONEXCLAMATION);
            }
            break;

        case IDC_SOFTWARE:
            m_software = IsDlgButtonChecked(hDlg, IDC_SOFTWARE);
            if (!m_software)
            {
                MessageBox(hDlg, GetString(hInst, IDS_SOFTWARERENDERER_WARN), GetString(hInst, IDS_WARNING_TITLE), MB_OK | MB_ICONEXCLAMATION);
            }
            break;

        case IDC_MAINHELP:
            {
	            char bindingHelpText[2048];
	            LoadString (hInst, IDS_BINDING_HELP, bindingHelpText, 2048);
                MessageBox(hDlg, bindingHelpText, GetString(hInst, IDS_HELP_TITLE), MB_OK | MB_ICONINFORMATION);
            }
            break;

        case IDC_RESTOREDEFAULTS:
            if (MessageBox(hDlg, GetString(hInst, IDS_DEFAULT_ARE_YOU_SURE), GetString(hInst, IDS_RESTORE_DEFAULTS), MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                RestoreDefaults(hDlg);
            }
            break;

        case IDC_ALWAYSRUN:
        case IDC_DYNAMIC:
        case IDC_QUICKCHAT:
        case IDC_ATTACKONTARGET:
        case IDC_GPU_EFFICIENCY:
            // Update the settings based on the checkbox state
            m_alwaysrun = IsDlgButtonChecked(hDlg, IDC_ALWAYSRUN);
            m_dynamic = IsDlgButtonChecked(hDlg, IDC_DYNAMIC);
            m_quickchat = IsDlgButtonChecked(hDlg, IDC_QUICKCHAT);
            m_attackontarget = IsDlgButtonChecked(hDlg, IDC_ATTACKONTARGET);
            m_gpuefficiency = IsDlgButtonChecked(hDlg, IDC_GPU_EFFICIENCY);
            break;
        }
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY)
        {
            // Save settings
            m_alwaysrun = IsDlgButtonChecked(hDlg, IDC_ALWAYSRUN);
            m_classic = IsDlgButtonChecked(hDlg, IDC_CLASSIC);
            m_dynamic = IsDlgButtonChecked(hDlg, IDC_DYNAMIC);
            m_quickchat = IsDlgButtonChecked(hDlg, IDC_QUICKCHAT);
            m_software = IsDlgButtonChecked(hDlg, IDC_SOFTWARE);
            m_attackontarget = IsDlgButtonChecked(hDlg, IDC_ATTACKONTARGET);
            m_gpuefficiency = IsDlgButtonChecked(hDlg, IDC_GPU_EFFICIENCY);

            UpdateINIFile();
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static INT_PTR CALLBACK ProfanitySettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Center the dialog
        RECT rcDlg, rcParent;
        GetWindowRect(hDlg, &rcDlg);
        GetWindowRect(GetParent(hDlg), &rcParent);

        int xPos = (rcParent.left + rcParent.right) / 2 - (rcDlg.right - rcDlg.left) / 2;
        int yPos = (rcParent.top + rcParent.bottom) / 2 - (rcDlg.bottom - rcDlg.top) / 2;

        SetWindowPos(hDlg, NULL, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        SetDlgItemText(hDlg, IDC_EDIT1, "");
        CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1 + !config.ignoreprofane);
        CheckDlgButton(hDlg, IDC_CHECK1, config.extraprofane);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON1:
        {
            char term[MAXPROFANETERM + 1];
            GetDlgItemText(hDlg, IDC_EDIT1, term, sizeof(term));
            AddProfaneTerm(term);
            SetDlgItemText(hDlg, IDC_EDIT1, "");
            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
            return (INT_PTR)TRUE;
        }
        case IDC_BUTTON2:
        {
            char term[MAXPROFANETERM + 1];
            GetDlgItemText(hDlg, IDC_EDIT1, term, sizeof(term));
            RemoveProfaneTerm(term);
            SetDlgItemText(hDlg, IDC_EDIT1, "");
            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
            return (INT_PTR)TRUE;
        }
        case IDOK:
            config.ignoreprofane = IsDlgButtonChecked(hDlg, IDC_RADIO1);
            config.extraprofane = IsDlgButtonChecked(hDlg, IDC_CHECK1);
            RecompileAllProfaneExpressions();
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static INT_PTR CALLBACK CommonPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    case WM_USER_REINITDIALOG:
    {
        // Initialize dialog with current settings
        CheckDlgButton(hDlg, IDC_SCROLLLOCK, config.scroll_lock);
        CheckDlgButton(hDlg, IDC_DRAWNAMES, config.draw_names);
        CheckDlgButton(hDlg, IDC_TOOLTIPS, config.tooltips);
        CheckDlgButton(hDlg, IDC_PAIN, config.pain);
        CheckDlgButton(hDlg, IDC_INVNUM, config.inventory_num);
        CheckDlgButton(hDlg, IDC_SAFETY, config.aggressive);
        CheckDlgButton(hDlg, IDC_BOUNCE, config.bounce);
        CheckDlgButton(hDlg, IDC_TOOLBAR, config.toolbar);
        CheckDlgButton(hDlg, IDS_LATENCY0, config.lagbox);
        CheckDlgButton(hDlg, ID_SPINNING_CUBE, config.spinning_cube);
        CheckDlgButton(hDlg, IDC_PROFANE, config.antiprofane);
        CheckDlgButton(hDlg, IDC_DRAWMAP, config.drawmap);
        CheckDlgButton(hDlg, IDC_MAP_ANNOTATIONS, config.map_annotations);

        CheckDlgButton(hDlg, IDC_MUSIC, config.play_music);
        CheckDlgButton(hDlg, IDC_SOUNDFX, config.play_sound);
        CheckDlgButton(hDlg, IDC_LOOPSOUNDS, config.play_loop_sounds);
        CheckDlgButton(hDlg, IDC_RANDSOUNDS, config.play_random_sounds);

        EnableWindow(GetDlgItem(hDlg, IDC_LOOPSOUNDS), IsDlgButtonChecked(hDlg, IDC_SOUNDFX));
        EnableWindow(GetDlgItem(hDlg, IDC_RANDSOUNDS), IsDlgButtonChecked(hDlg, IDC_SOUNDFX));

        CheckRadioButton(hDlg, IDC_TARGETHALO1, IDC_TARGETHALO3, config.halocolor + IDC_TARGETHALO1);

        CheckDlgButton(hDlg, IDC_COLORCODES, config.colorcodes);

        SendMessage(GetDlgItem(hDlg, IDC_SOUND_VOLUME), TBM_SETRANGE, FALSE, MAKELONG(0, CONFIG_MAX_VOLUME));
        SendMessage(GetDlgItem(hDlg, IDC_SOUND_VOLUME), TBM_SETPOS, TRUE, config.sound_volume);

        SendMessage(GetDlgItem(hDlg, IDC_MUSIC_VOLUME), TBM_SETRANGE, FALSE, MAKELONG(0, CONFIG_MAX_VOLUME));
        SendMessage(GetDlgItem(hDlg, IDC_MUSIC_VOLUME), TBM_SETPOS, TRUE, config.music_volume);

        SendMessage(GetDlgItem(hDlg, IDC_AMBIENT_VOLUME), TBM_SETRANGE, FALSE, MAKELONG(0, CONFIG_MAX_VOLUME));
        SendMessage(GetDlgItem(hDlg, IDC_AMBIENT_VOLUME), TBM_SETPOS, TRUE, config.ambient_volume);

        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_PROFANESETTINGS:
            // Open the profanity settings dialog
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_PROFANESETTINGS), hDlg, ProfanitySettingsDlgProc);
            return (INT_PTR)TRUE;
        }
        break;
    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY)
        {
            // Save settings (config is persisted on close of the meridian executable)
            config.scroll_lock = IsDlgButtonChecked(hDlg, IDC_SCROLLLOCK);
            config.draw_names = IsDlgButtonChecked(hDlg, IDC_DRAWNAMES);
            config.tooltips = IsDlgButtonChecked(hDlg, IDC_TOOLTIPS);
            config.pain = IsDlgButtonChecked(hDlg, IDC_PAIN);
            config.inventory_num = IsDlgButtonChecked(hDlg, IDC_INVNUM);
            config.aggressive = IsDlgButtonChecked(hDlg, IDC_SAFETY);
            config.bounce = IsDlgButtonChecked(hDlg, IDC_BOUNCE);
            config.antiprofane = IsDlgButtonChecked(hDlg, IDC_PROFANE);
            config.drawmap = IsDlgButtonChecked(hDlg, IDC_DRAWMAP);
            config.map_annotations = IsDlgButtonChecked(hDlg, IDC_MAP_ANNOTATIONS);
            config.toolbar = IsDlgButtonChecked(hDlg, IDC_TOOLBAR);
            config.lagbox = IsDlgButtonChecked(hDlg, IDS_LATENCY0);
            config.spinning_cube = IsDlgButtonChecked(hDlg, ID_SPINNING_CUBE);
            config.play_music = IsDlgButtonChecked(hDlg, IDC_MUSIC);
            config.play_sound = IsDlgButtonChecked(hDlg, IDC_SOUNDFX);
            config.play_loop_sounds = IsDlgButtonChecked(hDlg, IDC_LOOPSOUNDS);
            config.play_random_sounds = IsDlgButtonChecked(hDlg, IDC_RANDSOUNDS);
            config.halocolor = IsDlgButtonChecked(hDlg, IDC_TARGETHALO1) == BST_CHECKED ? 0 :
                               IsDlgButtonChecked(hDlg, IDC_TARGETHALO2) == BST_CHECKED ? 1 : 2;
            config.colorcodes = IsDlgButtonChecked(hDlg, IDC_COLORCODES);

            config.sound_volume = SendMessage(GetDlgItem(hDlg, IDC_SOUND_VOLUME), TBM_GETPOS, 0, 0);
            config.music_volume = SendMessage(GetDlgItem(hDlg, IDC_MUSIC_VOLUME), TBM_GETPOS, 0, 0);
            config.ambient_volume = SendMessage(GetDlgItem(hDlg, IDC_AMBIENT_VOLUME), TBM_GETPOS, 0, 0);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Hook procedure to capture keyboard events
static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN || wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            int vkCode = p->vkCode;

            // Handle key down events
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                // Update modifier state
                if (vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) {
                    iModifier = MODIFIER_SHIFT;
                } else if (vkCode == VK_CONTROL || vkCode == VK_LCONTROL || vkCode == VK_RCONTROL) {
                    iModifier = MODIFIER_CTRL;
                } else if (vkCode == VK_MENU || vkCode == VK_LMENU || vkCode == VK_RMENU) {
                    iModifier = MODIFIER_ALT;
                }

                if (vkCode >= 'A' && vkCode <= 'Z') {
                    sprintf(TCNewkey, "%c", (char)vkCode + 32); // +32 changes it to lowercase
                    AppendModifier(TCNewkey, iModifier);
                    EndDialog(GetForegroundWindow(), IDOK);
                    return 1; // Prevent further processing
                }

                switch (vkCode) {
                case VK_BACK:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "backspace");
                    break;
                case VK_TAB:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "tab");
                    break;
                case VK_RETURN:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "enter");
                    break;
                case VK_ESCAPE:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "esc");
                    break;
                case VK_SPACE:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "space");
                    break;
                case VK_PRIOR:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "pageup");
                    break;
                case VK_NEXT:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "pagedown");
                    break;
                case VK_END:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "end");
                    break;
                case VK_HOME:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "home");
                    break;
                case VK_LEFT:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "left");
                    break;
                case VK_UP:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "up");
                    break;
                case VK_RIGHT:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "right");
                    break;
                case VK_DOWN:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "down");
                    break;
                case VK_DELETE:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "delete");
                    break;
                case VK_ADD:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "add");
                    break;
                case VK_SUBTRACT:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "subtract");
                    break;
                case 0xba:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), ";");
                    break;
                case 0xbc:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), ",");
                    break;
                case 0xbe:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), ".");
                    break;
                case 0xdb:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "[");
                    break;
                case 0xdc:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "\\");
                    break;
                case 0xdd:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "]");
                    break;
                case 0xde:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "'");
                    break;
                }

                if (strlen(TCNewkey) > 0) {
                    AppendModifier(TCNewkey, iModifier);
                    EndDialog(GetForegroundWindow(), IDOK);
                    return 1; // Prevent further processing
                }
            }

            // Handle key up events
            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                switch (vkCode) {
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "shift");
                    break;
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "ctrl");
                    break;
                case VK_MENU:
                case VK_LMENU:
                case VK_RMENU:
                    strcpy_s(TCNewkey, sizeof(TCNewkey), "alt");
                    break;
                }

                if (strlen(TCNewkey) > 0) {
                    AppendModifier(TCNewkey, iModifier);
                    EndDialog(GetForegroundWindow(), IDOK);
                    return 1; // Prevent further processing
                }
            }
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

static INT_PTR CALLBACK AssignKeyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        // Initialize variables
        TCNewkey[0] = '\0';
        iModifier = MODIFIER_NONE;
        SetCapture(hDlg);
        SetFocus(hDlg);
        // Set the low-level keyboard hook
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        return (INT_PTR)TRUE;

    case WM_LBUTTONDOWN:
        strcpy_s(TCNewkey, sizeof(TCNewkey), "mouse0");
        EndDialog(hDlg, IDOK);
        return (INT_PTR)TRUE;

    case WM_RBUTTONDOWN:
        strcpy_s(TCNewkey, sizeof(TCNewkey), "mouse1");
        EndDialog(hDlg, IDOK);
        return (INT_PTR)TRUE;

    case WM_MBUTTONDOWN:
        strcpy_s(TCNewkey, sizeof(TCNewkey), "mouse2");
        EndDialog(hDlg, IDOK);
        return (INT_PTR)TRUE;

    case WM_XBUTTONDOWN:
        if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
            strcpy_s(TCNewkey, sizeof(TCNewkey), "mouse3");
        }
        else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2) {
            strcpy_s(TCNewkey, sizeof(TCNewkey), "mouse4");
        }
        if (strlen(TCNewkey) > 0) {
            EndDialog(hDlg, IDOK);
        }
        return (INT_PTR)TRUE;

    case WM_DESTROY:
        ReleaseCapture();
        // Remove the low-level keyboard hook
        UnhookWindowsHookEx(hHook);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Function to initialize key bindings
static void InitializeKeyBindings(HWND hDlg) {
    SetDlgItemText(hDlg, IDC_BROADCAST, TCBroadcast);
    SetDlgItemText(hDlg, IDC_CHAT, TCChat);
    SetDlgItemText(hDlg, IDC_EMOTE, TCEmote);
    SetDlgItemText(hDlg, IDC_SAY, TCSay);
    SetDlgItemText(hDlg, IDC_TELL, TCTell);
    SetDlgItemText(hDlg, IDC_WHO, TCWho);
    SetDlgItemText(hDlg, IDC_YELL, TCYell);
}

static INT_PTR CALLBACK CommunicationPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    case WM_USER_REINITDIALOG:
        InitializeKeyBindings(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BROADCAST:
            AssignKey(hDlg, TCBroadcast, IDC_BROADCAST);
            break;
        case IDC_CHAT:
            AssignKey(hDlg, TCChat, IDC_CHAT);
            break;
        case IDC_EMOTE:
            AssignKey(hDlg, TCEmote, IDC_EMOTE);
            break;
        case IDC_SAY:
            AssignKey(hDlg, TCSay, IDC_SAY);
            break;
        case IDC_TELL:
            AssignKey(hDlg, TCTell, IDC_TELL);
            break;
        case IDC_WHO:
            AssignKey(hDlg, TCWho, IDC_WHO);
            break;
        case IDC_YELL:
            AssignKey(hDlg, TCYell, IDC_YELL);
            break;
        case IDC_BROADCAST_MOD:
            ModifyKey(hDlg, TCBroadcast, IDC_BROADCAST);
            break;
        case IDC_CHAT_MOD:
            ModifyKey(hDlg, TCChat, IDC_CHAT);
            break;
        case IDC_EMOTE_MOD:
            ModifyKey(hDlg, TCEmote, IDC_EMOTE);
            break;
        case IDC_SAY_MOD:
            ModifyKey(hDlg, TCSay, IDC_SAY);
            break;
        case IDC_TELL_MOD:
            ModifyKey(hDlg, TCTell, IDC_TELL);
            break;
        case IDC_WHO_MOD:
            ModifyKey(hDlg, TCWho, IDC_WHO);
            break;
        case IDC_YELL_MOD:
            ModifyKey(hDlg, TCYell, IDC_YELL);
            break;
        }
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY)
        {
            // Save settings
            UpdateINIFile();
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Function to initialize movement key bindings
static void InitializeMovementKeyBindings(HWND hDlg) {
    SetDlgItemText(hDlg, IDC_FORWARD, TCForward);
    SetDlgItemText(hDlg, IDC_BACKWARD, TCBackward);
    SetDlgItemText(hDlg, IDC_LEFT, TCLeft);
    SetDlgItemText(hDlg, IDC_RIGHT, TCRight);
    SetDlgItemText(hDlg, IDC_SLIDELEFT, TCSlideleft);
    SetDlgItemText(hDlg, IDC_SLIDERIGHT, TCSlideright);
    SetDlgItemText(hDlg, IDC_RUNWALK, TCRunwalk);
    SetDlgItemText(hDlg, IDC_LOOKUP, TCLookup);
    SetDlgItemText(hDlg, IDC_LOOKDOWN, TCLookdown);
    SetDlgItemText(hDlg, IDC_LOOKSTRAIGHT, TCLookstraight);
    SetDlgItemText(hDlg, IDC_FLIP, TCFlip);
    SetDlgItemText(hDlg, IDC_MOUSELOOKTOGGLE, TCMouselooktoggle);

    // Initialize modifier buttons
    InitModifierButton(TCForward, GetDlgItem(hDlg, IDC_FORWARD_MOD));
    InitModifierButton(TCBackward, GetDlgItem(hDlg, IDC_BACKWARD_MOD));
    InitModifierButton(TCLeft, GetDlgItem(hDlg, IDC_LEFT_MOD));
    InitModifierButton(TCRight, GetDlgItem(hDlg, IDC_RIGHT_MOD));
    InitModifierButton(TCSlideleft, GetDlgItem(hDlg, IDC_SLIDELEFT_MOD));
    InitModifierButton(TCSlideright, GetDlgItem(hDlg, IDC_SLIDERIGHT_MOD));
    InitModifierButton(TCRunwalk, GetDlgItem(hDlg, IDC_RUNWALK_MOD));
    InitModifierButton(TCLookup, GetDlgItem(hDlg, IDC_LOOKUP_MOD));
    InitModifierButton(TCLookdown, GetDlgItem(hDlg, IDC_LOOKDOWN_MOD));
    InitModifierButton(TCLookstraight, GetDlgItem(hDlg, IDC_LOOKSTRAIGHT_MOD));
    InitModifierButton(TCFlip, GetDlgItem(hDlg, IDC_FLIP_MOD));
    InitModifierButton(TCMouselooktoggle, GetDlgItem(hDlg, IDC_MOUSELOOKTOGGLE_MOD));
}

// Dialog procedure for the Movement Preferences dialog
static INT_PTR CALLBACK MovementPreferencesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        InitializeMovementKeyBindings(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_FORWARD:
            AssignKey(hDlg, TCForward, IDC_FORWARD);
            break;
        case IDC_BACKWARD:
            AssignKey(hDlg, TCBackward, IDC_BACKWARD);
            break;
        case IDC_LEFT:
            AssignKey(hDlg, TCLeft, IDC_LEFT);
            break;
        case IDC_RIGHT:
            AssignKey(hDlg, TCRight, IDC_RIGHT);
            break;
        case IDC_SLIDELEFT:
            AssignKey(hDlg, TCSlideleft, IDC_SLIDELEFT);
            break;
        case IDC_SLIDERIGHT:
            AssignKey(hDlg, TCSlideright, IDC_SLIDERIGHT);
            break;
        case IDC_RUNWALK:
            AssignKey(hDlg, TCRunwalk, IDC_RUNWALK);
            break;
        case IDC_LOOKUP:
            AssignKey(hDlg, TCLookup, IDC_LOOKUP);
            break;
        case IDC_LOOKDOWN:
            AssignKey(hDlg, TCLookdown, IDC_LOOKDOWN);
            break;
        case IDC_LOOKSTRAIGHT:
            AssignKey(hDlg, TCLookstraight, IDC_LOOKSTRAIGHT);
            break;
        case IDC_FLIP:
            AssignKey(hDlg, TCFlip, IDC_FLIP);
            break;
        case IDC_MOUSELOOKTOGGLE:
            AssignKey(hDlg, TCMouselooktoggle, IDC_MOUSELOOKTOGGLE);
            break;
        case IDC_FORWARD_MOD:
            ModifyKey(hDlg, TCForward, IDC_FORWARD);
            break;
        case IDC_BACKWARD_MOD:
            ModifyKey(hDlg, TCBackward, IDC_BACKWARD);
            break;
        case IDC_LEFT_MOD:
            ModifyKey(hDlg, TCLeft, IDC_LEFT);
            break;
        case IDC_RIGHT_MOD:
            ModifyKey(hDlg, TCRight, IDC_RIGHT);
            break;
        case IDC_SLIDELEFT_MOD:
            ModifyKey(hDlg, TCSlideleft, IDC_SLIDELEFT);
            break;
        case IDC_SLIDERIGHT_MOD:
            ModifyKey(hDlg, TCSlideright, IDC_SLIDERIGHT);
            break;
        case IDC_RUNWALK_MOD:
            ModifyKey(hDlg, TCRunwalk, IDC_RUNWALK);
            break;
        case IDC_LOOKUP_MOD:
            ModifyKey(hDlg, TCLookup, IDC_LOOKUP);
            break;
        case IDC_LOOKDOWN_MOD:
            ModifyKey(hDlg, TCLookdown, IDC_LOOKDOWN);
            break;
        case IDC_LOOKSTRAIGHT_MOD:
            ModifyKey(hDlg, TCLookstraight, IDC_LOOKSTRAIGHT);
            break;
        case IDC_FLIP_MOD:
            ModifyKey(hDlg, TCFlip, IDC_FLIP);
            break;
        case IDC_MOUSELOOKTOGGLE_MOD:
            ModifyKey(hDlg, TCMouselooktoggle, IDC_MOUSELOOKTOGGLE);
            break;
        }
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY) {
            // Save settings
            UpdateINIFile();
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return (INT_PTR)TRUE;
        } else if (((LPNMHDR)lParam)->code == PSN_RESET) {
            // Handle Cancel button
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static LRESULT CALLBACK PropSheetSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                       UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    if (uMsg == WM_SHOWWINDOW && wParam == TRUE)
    {
        // Get the parent window (the main application)
        HWND hParent = (HWND)dwRefData;
        if (!hParent) hParent = GetDesktopWindow(); // Fallback to screen center

        // Get window sizes
        RECT rcParent, rcDlg;
        GetWindowRect(hParent, &rcParent);
        GetWindowRect(hWnd, &rcDlg);

        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;

        int parentWidth = rcParent.right - rcParent.left;
        int parentHeight = rcParent.bottom - rcParent.top;

        // Compute the new position (centered)
        int xPos = rcParent.left + (parentWidth - dlgWidth) / 2;
        int yPos = rcParent.top + (parentHeight - dlgHeight) / 2;

        // Move the window to the computed position
        SetWindowPos(hWnd, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

        // Remove the subclass so this runs only once
        RemoveWindowSubclass(hWnd, PropSheetSubclassProc, 0);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

static int CALLBACK PropSheetCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    if (uMsg == PSCB_INITIALIZED && hwndDlg)
    {
        // Remove the Help (?) button from the title bar
        LONG style = GetWindowLong(hwndDlg, GWL_EXSTYLE);
        style &= ~WS_EX_CONTEXTHELP;
        SetWindowLong(hwndDlg, GWL_EXSTYLE, style);

        // Apply the subclass to center the dialog when it's shown
        HWND hParent = GetWindow(hwndDlg, GW_OWNER);
        SetWindowSubclass(hwndDlg, PropSheetSubclassProc, 0, (DWORD_PTR)hParent);
    }
    return 0;
}

void ShowPreferencesDialog(HWND hWndParent)
{
    std::vector<PROPSHEETPAGE> psp(tabs.size());
    PROPSHEETHEADER psh;

    ReadINIFile();

    tabTitles.clear();
    tabTitles.reserve(tabs.size());

    for (size_t i = 0; i < tabs.size(); ++i)
    {
        psp[i].dwSize = sizeof(PROPSHEETPAGE);
        psp[i].dwFlags = PSP_USETITLE;
        psp[i].hInstance = GetModuleHandle(NULL);
        psp[i].pszTemplate = MAKEINTRESOURCE(tabs[i].templateId);
        psp[i].pszIcon = NULL;
        psp[i].pfnDlgProc = tabs[i].dlgProc;
        char tabTitle[2048] = { 0 };
        LoadString(hInst, tabs[i].title, tabTitle, 2048);
		tabTitles.emplace_back(tabTitle);
        psp[i].pszTitle = tabTitles[i].c_str();
        psp[i].lParam = 0;
    }

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = (PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_USEICONID) & ~PSH_HASHELP;
    // Add a callback to modify window styles before the property sheet is shown
    psh.dwFlags |= PSH_USECALLBACK;
    psh.pfnCallback = PropSheetCallback;
    psh.hwndParent = hWndParent;
    psh.hInstance = GetModuleHandle(NULL);
    char propertiesTitle[2048] = { 0 };
    LoadString(hInst, IDS_PREFERENCES_TITLE, propertiesTitle, 2048);
    psh.pszCaption = propertiesTitle;
    psh.nPages = static_cast<UINT>(psp.size());
    psh.nStartPage = 0;
    psh.ppsp = psp.data();
    // As a homage to the late keybind we include its icon (RIP 2001 - March 2025)
    psh.pszIcon = MAKEINTRESOURCE(IDI_KEYBIND);

    // Create the property sheet
    PropertySheet(&psh);
}
