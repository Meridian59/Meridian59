//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// PICKEDIT.CPP
//
// TPickEditList Class
//
// Original code by Steve Carr (Compuserve: 100251,1571)
// Written: 20th December 1995
//------------------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/dialog.h>
#include <owlext\pickedit.h>
#include <owlext\browse.h>
#include <owlext\pickedit.rh>

namespace OwlExt {

using namespace owl;
using namespace std;

// private function
TModule* FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type);


class TPickListDlg : public TDialog {
public:
  TPickListDlg(TWindow* parent, LPCTSTR filename);

  void SetupWindow() override;

  TBrowse* list;

protected:
  LPCTSTR fileName;

  void CmOk();
  void EvChange();

  DECLARE_RESPONSE_TABLE(TPickListDlg);
};

DEFINE_RESPONSE_TABLE1(TPickListDlg, TDialog)
EV_COMMAND(IDOK, CmOk),
EV_LBN_DBLCLK(IDC_PICKLIST, CmOk),
EV_LBN_SELCHANGE(IDC_PICKLIST, EvChange),
END_RESPONSE_TABLE;

TPickListDlg::TPickListDlg(TWindow* parent, LPCTSTR aFileName)
:
TDialog(parent, IDD_EXTPICKLIST,
    FindResourceModule(Parent,0,IDD_EXTPICKLIST,RT_DIALOG))
{
  list = new TBrowse(this, IDC_PICKLIST);
  fileName = aFileName;
}


#define COL_BASE   150

void
TPickListDlg::SetupWindow()
{
  TDialog::SetupWindow();

  TRect rc, rcThis = GetWindowRect(), rcParent = Parent->GetWindowRect();

  rcThis.right = rcParent.left + rcThis.Width();
  rcThis.bottom = rcParent.bottom + rcThis.Height();
  rcThis.left = rcParent.left;
  rcThis.top = rcParent.bottom;

  ::GetWindowRect(GetDesktopWindow(), &rc);
  if (rcThis.left < rc.left)
    rcThis.Offset(rc.left-rcThis.left, 0);
  if (rcThis.bottom > rc.bottom)
    rcThis.Offset(0, rc.bottom-rcThis.bottom);
  if (rcThis.right > rc.right)
    rcThis.Offset(rc.right-rcThis.right, 0);
  MoveWindow(rcThis);

  tifstream in(fileName);

  _TCHAR buf[80], match[80];
  int items = 0;
  int totalPct = 0;

  if (in.bad()){
    MessageBox(_T("TPickEditList: Invalid File"),_T("ERROR"), MB_ICONEXCLAMATION|MB_OK); //??????????????????????????????
    return;
  }

  list->ClearList();
  while(in.good()){
    in.getline(buf, sizeof(buf) / sizeof(_TCHAR));
    if (!_tcslen(buf))
      break;
    _TCHAR* pos = _tcsrchr(buf, _T(','));
    if (!pos)    {
      MessageBox(_T("TPickEditList: Invalid Header Description"),        //????????????????????????
        _T("ERROR"), MB_ICONEXCLAMATION|MB_OK);
      return;
    }
    *pos = _T('\0'); pos++; // should point to number
    int columnPct = _ttoi(pos);
    if (columnPct <= 0 || columnPct > 100)
    {
      MessageBox(_T("TPickEditList: Invalid Column Percentage Width"),   //???????????????????????
        _T("ERROR"), MB_ICONEXCLAMATION|MB_OK);
      return;
    }
    totalPct += columnPct;
    list->AddColumn(COL_BASE+items, buf, columnPct, true, TBrowse::Percent);
    items++;
  }
  int index = LB_ERR;

  if (items <= 0 || totalPct > 100){
    list->Reset();
    MessageBox(_T("TPickEditList: Invalid Header Descriptions"), _T("ERROR"),
      MB_ICONEXCLAMATION|MB_OK);
    return;
  }

  TPickEditList* edit = TYPESAFE_DOWNCAST( Parent, TPickEditList );
  edit->GetText(match, sizeof(match));
  while(in.good()){
    int thisIndex = list->AddRow();

    in.getline(buf, sizeof(buf) / sizeof(_TCHAR));
    list->SetColumnText(COL_BASE, buf);
    list->SetColumnStyle(COL_BASE, 1);
    if (index == LB_ERR && _tcslen(match) && !_tcsnicmp(buf, match, _tcslen(match)))
      index = thisIndex;

    for(int i = 1; i < items && in.good(); i++){
      in.getline(buf, sizeof(buf));
      list->SetColumnText(COL_BASE+i, buf)  ;
      list->SetColumnStyle(COL_BASE+i, 2);
    }
    // parse blank line between codes
    in.getline(buf, sizeof(buf) / sizeof(_TCHAR));
  }
  list->SetSelIndex(index);

  ::EnableWindow(GetDlgItem(IDOK), (list->GetSelIndex() != LB_ERR));
}

void
TPickListDlg::CmOk()
{
  TPickEditList* edit = TYPESAFE_DOWNCAST( Parent, TPickEditList );
  int index = list->GetSelIndex();
  owl::tstring s;
  list->GetColumnText(COL_BASE, s, index);
  edit->SetText(s.c_str());
  TDialog::CmOk();
}

void
TPickListDlg::EvChange()
{
  ::EnableWindow(GetDlgItem(IDOK), (list->GetSelIndex() != LB_ERR));
}

TPickEditList::TPickEditList(TWindow* parent, int resourceId, LPCTSTR aFileName,
               TResId bmpId, uint textLen, TModule* module)
               :
TPopupEdit(parent, resourceId, bmpId, textLen, module)
{
  int len = static_cast<int>(_tcslen(aFileName));
  fileName = new _TCHAR[len+1];
  _tcscpy(fileName, aFileName);
}

TPickEditList::TPickEditList(TWindow* parent, int id, LPCTSTR aFileName, LPCTSTR text,
               int x, int y, int w, int h, TResId bmpId,
               uint textLen, TModule* module)
               :
TPopupEdit(parent, id, text, x, y, w, h, bmpId, textLen, module)
{
  int len = static_cast<int>(_tcslen(aFileName));
  fileName = new _TCHAR[len+1];
  _tcscpy(fileName, aFileName);
}

TPickEditList::~TPickEditList()
{
  delete[] fileName;
}

void
TPickEditList::Clicked()
{
  TPickListDlg* dlg = new TPickListDlg(this, fileName);

  dlg->Execute();
}

} // OwlExt namespace
//==============================================================================
