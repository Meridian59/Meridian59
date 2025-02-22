#include "stdafx.h"
#include "preferences_profanity.h"
#include "client.h"

IMPLEMENT_DYNAMIC(CProfanitySettingsDialog, CDialog)

CProfanitySettingsDialog::CProfanitySettingsDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(CProfanitySettingsDialog::IDD, pParent)
{
}

CProfanitySettingsDialog::~CProfanitySettingsDialog()
{
}

void CProfanitySettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	// Add DDX/DDV calls here
}

BEGIN_MESSAGE_MAP(CProfanitySettingsDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CProfanitySettingsDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CProfanitySettingsDialog::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CProfanitySettingsDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CProfanitySettingsDialog::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CProfanitySettingsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CenterWindow(GetParent());

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT1);
	pEdit->SetLimitText(MAXPROFANETERM);
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1 + !config.ignoreprofane);
	CheckDlgButton(IDC_CHECK1, config.extraprofane);
	GetDlgItem(IDC_RADIO1 + !config.ignoreprofane)->SetFocus();

	return FALSE; // return TRUE unless we set the focus
}

void CProfanitySettingsDialog::OnBnClickedButton1()
{
	char term[MAXPROFANETERM + 1];
	GetDlgItemText(IDC_EDIT1, term, sizeof(term));
	AddProfaneTerm(term);
	SetDlgItemText(IDC_EDIT1, "");
	GetDlgItem(IDC_EDIT1)->SetFocus();
}

void CProfanitySettingsDialog::OnBnClickedButton2()
{
	char term[MAXPROFANETERM + 1];
	GetDlgItemText(IDC_EDIT1, term, sizeof(term));
	RemoveProfaneTerm(term);
	SetDlgItemText(IDC_EDIT1, "");
	GetDlgItem(IDC_EDIT1)->SetFocus();
}

void CProfanitySettingsDialog::OnBnClickedOk()
{
	config.ignoreprofane = IsDlgButtonChecked(IDC_RADIO1);
	config.extraprofane = IsDlgButtonChecked(IDC_CHECK1);
	RecompileAllProfaneExpressions();
	EndDialog(IDOK);
}

void CProfanitySettingsDialog::OnBnClickedCancel()
{
	EndDialog(IDCANCEL);
}
