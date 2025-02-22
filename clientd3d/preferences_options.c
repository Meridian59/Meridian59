#include "stdafx.h"
#include "preferences_options.h"
#include "preferences_common.h"
#include "preferences.h"

IMPLEMENT_DYNAMIC(COptionsPage, CPropertyPage)

COptionsPage::COptionsPage() : CPropertyPage(COptionsPage::IDD)
{
}

COptionsPage::~COptionsPage()
{
}

void COptionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ALWAYSRUN, m_alwaysrun);
	DDX_Check(pDX, IDC_CLASSIC, m_classic);
	DDX_Check(pDX, IDC_DYNAMIC, m_dynamic);
	DDX_Check(pDX, IDC_QUICKCHAT, m_quickchat);
	DDX_Check(pDX, IDC_SOFTWARE, m_software);
	DDX_Check(pDX, IDC_ATTACKONTARGET, m_attackontarget);
	DDX_Check(pDX, IDC_GPU_EFFICIENCY, m_gpuefficiency);
}

BEGIN_MESSAGE_MAP(COptionsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_CLASSIC, OnClassic)
	ON_BN_CLICKED(IDC_SOFTWARE, OnSoftware)
	ON_BN_CLICKED(IDC_MAINHELP, OnMainhelp)
	ON_BN_CLICKED(IDC_RESTOREDEFAULTS, OnRestoredefaults)
	ON_BN_CLICKED(IDC_ALWAYSRUN, OnAlwaysRun)
	ON_BN_CLICKED(IDC_DYNAMIC, OnDynamic)
	ON_BN_CLICKED(IDC_QUICKCHAT, OnQuickChat)
	ON_BN_CLICKED(IDC_ATTACKONTARGET, OnAttackOnTarget)
	ON_BN_CLICKED(IDC_GPU_EFFICIENCY, OnGpuEfficiency)
END_MESSAGE_MAP()

void COptionsPage::OnRestoredefaults()
{
	if (MessageBox("Are you sure you want to restore all settings to their default?", "Restore Defaults", MB_YESNO) == IDYES)
	{
		CPreferencesPropertySheet* pParentSheet = (CPreferencesPropertySheet*)GetParent();
		if (pParentSheet)
		{
			pParentSheet->RestoreDefaults();
			pParentSheet->UpdateAllPreferences();
		}
	}
}

void COptionsPage::OnMainhelp()
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
		"Help", MB_OK);
}

void COptionsPage::OnAlwaysRun()
{
	UpdateData(TRUE);
}

void COptionsPage::OnDynamic()
{
	UpdateData(TRUE);
}

void COptionsPage::OnQuickChat()
{
	UpdateData(TRUE);
}

void COptionsPage::OnAttackOnTarget()
{
	UpdateData(TRUE);
}

void COptionsPage::OnGpuEfficiency()
{
	UpdateData(TRUE);
}

void COptionsPage::OnClassic()
{
	UpdateData(TRUE);
	if (m_classic)
	{
		MessageBox("Activating \"Classic Key Bindings\" will cause the game to ignore the key bindings!", "Notice", MB_OK);
	}
}

void COptionsPage::OnSoftware()
{
	UpdateData(TRUE);
	if (!m_software)
	{
		MessageBox("Software renderer may have been activated due to a problem with your video setup.\n\t\t Disabling may cause the game not to run.", "Warning", MB_OK);
	}
}

BOOL COptionsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	UpdateData(FALSE);

	return TRUE;
}

void COptionsPage::OnOK()
{
	UpdateData(TRUE); // Save data from controls to member variables
	CPropertyPage::OnOK(); // Call base class implementation
}
