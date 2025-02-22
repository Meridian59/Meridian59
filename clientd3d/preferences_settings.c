#include "stdafx.h"
#include "preferences_settings.h"
#include "preferences_profanity.h"

IMPLEMENT_DYNAMIC(CGeneralSettingsPage, CPropertyPage)

CGeneralSettingsPage::CGeneralSettingsPage() : CPropertyPage(CGeneralSettingsPage::IDD)
{
	m_psp.pszTitle = _T("Preferences");
	m_psp.dwFlags |= PSP_USETITLE;
}

CGeneralSettingsPage::~CGeneralSettingsPage()
{
}

void CGeneralSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	// Add DDX/DDV calls here
}

BEGIN_MESSAGE_MAP(CGeneralSettingsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_SOUNDFX, &CGeneralSettingsPage::OnBnClickedSfx)
	ON_BN_CLICKED(IDC_PROFANESETTINGS, &CGeneralSettingsPage::OnBnClickedProfaneSettings)
END_MESSAGE_MAP()

BOOL CGeneralSettingsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CenterWindow(GetParent());

	CheckDlgButton(IDC_SCROLLLOCK, config.scroll_lock);
	CheckDlgButton(IDC_DRAWNAMES, config.draw_names);
	CheckDlgButton(IDC_TOOLTIPS, config.tooltips);
	CheckDlgButton(IDC_PAIN, config.pain);
	CheckDlgButton(IDC_INVNUM, config.inventory_num);
	CheckDlgButton(IDC_SAFETY, config.aggressive);
	CheckDlgButton(IDC_BOUNCE, config.bounce);
	CheckDlgButton(IDC_TOOLBAR, config.toolbar);
	CheckDlgButton(IDS_LATENCY0, config.lagbox);
	CheckDlgButton(ID_SPINNING_CUBE, config.spinning_cube);
	CheckDlgButton(IDC_PROFANE, config.antiprofane);
	CheckDlgButton(IDC_DRAWMAP, config.drawmap);
	CheckDlgButton(IDC_MAP_ANNOTATIONS, config.map_annotations);

	CheckDlgButton(IDC_MUSIC, config.play_music);
	CheckDlgButton(IDC_SOUNDFX, config.play_sound);
	CheckDlgButton(IDC_LOOPSOUNDS, config.play_loop_sounds);
	CheckDlgButton(IDC_RANDSOUNDS, config.play_random_sounds);

	GetDlgItem(IDC_LOOPSOUNDS)->EnableWindow(IsDlgButtonChecked(IDC_SOUNDFX));
	GetDlgItem(IDC_RANDSOUNDS)->EnableWindow(IsDlgButtonChecked(IDC_SOUNDFX));

	CheckRadioButton(IDC_TARGETHALO1, IDC_TARGETHALO3, config.halocolor + IDC_TARGETHALO1);

	CheckDlgButton(IDC_COLORCODES, config.colorcodes);

	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SOUND_VOLUME);
	pSlider->SetRange(0, CONFIG_MAX_VOLUME, FALSE);
	pSlider->SetPos(config.sound_volume);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_MUSIC_VOLUME);
	pSlider->SetRange(0, CONFIG_MAX_VOLUME, FALSE);
	pSlider->SetPos(config.music_volume);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_AMBIENT_VOLUME);
	pSlider->SetRange(0, CONFIG_MAX_VOLUME, FALSE);
	pSlider->SetPos(config.ambient_volume);

	return TRUE;
}

void CGeneralSettingsPage::OnBnClickedSfx()
{
	GetDlgItem(IDC_LOOPSOUNDS)->EnableWindow(IsDlgButtonChecked(IDC_SOUNDFX));
	GetDlgItem(IDC_RANDSOUNDS)->EnableWindow(IsDlgButtonChecked(IDC_SOUNDFX));
}

void CGeneralSettingsPage::OnBnClickedProfaneSettings()
{
	CProfanitySettingsDialog dlg(this);
	if (dlg.DoModal() == IDOK)
		CheckDlgButton(IDC_PROFANE, TRUE);
}

void CGeneralSettingsPage::SaveSettings()
{
	Bool toolbar_changed, lagbox_changed, temp;
	int new_val;

	config.scroll_lock = IsDlgButtonChecked(IDC_SCROLLLOCK);
	config.draw_names = IsDlgButtonChecked(IDC_DRAWNAMES);
	config.tooltips = IsDlgButtonChecked(IDC_TOOLTIPS);
	config.pain = IsDlgButtonChecked(IDC_PAIN);
	config.inventory_num = IsDlgButtonChecked(IDC_INVNUM);
	config.aggressive = IsDlgButtonChecked(IDC_SAFETY);
	config.bounce = IsDlgButtonChecked(IDC_BOUNCE);
	config.antiprofane = IsDlgButtonChecked(IDC_PROFANE);
	config.drawmap = IsDlgButtonChecked(IDC_DRAWMAP);
	config.map_annotations = IsDlgButtonChecked(IDC_MAP_ANNOTATIONS);
	temp = IsDlgButtonChecked(IDC_TOOLBAR);
	toolbar_changed = (temp != config.toolbar);
	config.toolbar = temp;
	temp = IsDlgButtonChecked(IDS_LATENCY0);
	lagbox_changed = (temp != config.lagbox);
	config.lagbox = temp;

	config.spinning_cube = IsDlgButtonChecked(ID_SPINNING_CUBE);

	if (IsDlgButtonChecked(IDC_MUSIC) != config.play_music)
		UserToggleMusic(config.play_music);
	config.play_music = IsDlgButtonChecked(IDC_MUSIC);

	config.play_sound = IsDlgButtonChecked(IDC_SOUNDFX);
	config.play_loop_sounds = IsDlgButtonChecked(IDC_LOOPSOUNDS);
	config.play_random_sounds = IsDlgButtonChecked(IDC_RANDSOUNDS);
	if (!config.play_sound)
		SoundAbort();

	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_MUSIC_VOLUME);
	new_val = pSlider->GetPos();
	if (new_val != config.music_volume)
	{
		config.music_volume = new_val;
		ResetMusicVolume();
	}

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SOUND_VOLUME);
	config.sound_volume = pSlider->GetPos();

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_AMBIENT_VOLUME);
	config.ambient_volume = pSlider->GetPos();

	if (IsDlgButtonChecked(IDC_TARGETHALO1) == BST_CHECKED)
		config.halocolor = 0;
	else if (IsDlgButtonChecked(IDC_TARGETHALO2) == BST_CHECKED)
		config.halocolor = 1;
	else
		config.halocolor = 2;

	config.colorcodes = IsDlgButtonChecked(IDC_COLORCODES);

	// Redraw main window to reflect new settings
	if (toolbar_changed || lagbox_changed)
	{
		ResizeAll();
	}
	else
	{
		InvalidateRect(NULL, TRUE);
		RedrawAll();
	}

	EditBoxSetNormalFormat();

}