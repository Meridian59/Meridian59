//----------------------------------------------------------------------------
//  ObjectWindow- OWL NExt
//  Copyright 1999. Yura Bidus. 
//  All Rights reserved.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// 
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//  OVERVIEW
//  ~~~~~~~~
//  Syntax coloring text editor.
//----------------------------------------------------------------------------
#if !defined(COOLEDIT_INL)
#define COOLEDIT_INL

//
// inlines 
// 
inline void TCoolTextBuffer::SetDirty(bool dirty)
{ dirty ? Flags |= bfModified : Flags &= ~bfModified; }
inline bool TCoolTextBuffer::IsDirty() const 
{ return (Flags & bfModified) != 0; }
inline TCoolTextBuffer::TCrLfStyle TCoolTextBuffer::GetCRLFMode() const
{  return CRLFMode;          }
inline void TCoolTextBuffer::SetCRLFMode(TCrLfStyle style)
{  CRLFMode = style;        }
inline bool TCoolTextBuffer::IsReadOnly() const
{  return (Flags & bfReadOnly) != 0;  }
inline void TCoolTextBuffer::EnableReadOnly(bool enable)
{  enable ? Flags |= bfReadOnly : Flags &= ~bfReadOnly; }


inline TCoolTextBuffer::TUndoNode* TCoolTextBuffer::GetTopUndoNode()
{  return UndoList;          }
inline TCoolTextBuffer::TRedoNode* TCoolTextBuffer::GetTopRedoNode()
{  return RedoList;          }
inline bool TCoolTextBuffer::CanUndo() const 
{  return UndoList != 0;    }
inline bool TCoolTextBuffer::CanRedo() const 
{  return RedoList != 0;    }
inline TCoolTextBuffer::TSyntaxDescr& 
TCoolTextBuffer::GetSyntaxDescr(int index) const
{  return SyntaxArray[index];}
inline bool TCoolTextBuffer::SyntaxDescrExist() const
{  return SyntaxArray != 0; }
inline void TCoolTextBuffer::SetUndoSize(owl::uint newmax)
{  UndoSize = newmax;        }
inline owl::uint TCoolTextBuffer::GetUndoSize()
{  return UndoSize;          }
inline owl::uint TCoolTextBuffer::GetUndoCnt()
{  return UndoCnt;            }
inline void TCoolTextBuffer::SetRedoSize(owl::uint newmax)
{  RedoSize = newmax;        }
inline owl::uint TCoolTextBuffer::GetRedoSize()
{  return RedoSize;          }
inline owl::uint TCoolTextBuffer::GetRedoCnt()
{  return RedoCnt;            }
inline void TCoolTextBuffer::GetFont(LOGFONT& lf) const
{ lf = BaseFont; }
inline void TCoolTextBuffer::SetFont(const LOGFONT& lf)
{ BaseFont = lf; }
////////////////////////////////////////////////////////////////////
//
inline int TCoolFileBuffer::GetLineCount() const
{ return (int)LineArray.Size(); }
inline int TCoolFileBuffer::GetLineLength(int index) const
{
  /*Jogy - in an empty/new file there are no lines ...
  PRECONDITION(index < (int)LineArray.Size());
  return LineArray[index]->Length;
  */
  return index < (int)LineArray.Size() ? LineArray[index]->Length : 0;
}
inline LPTSTR TCoolFileBuffer::GetLineText(int index)
{
  /*Jogy - in an empty/new file there are no lines ...
  PRECONDITION(index < (int)LineArray.Size());
  return LineArray[index]->Text;
  */
  return index < (int)LineArray.Size() ? LineArray[index]->Text : 0;
}
inline owl::uint32 TCoolFileBuffer::GetLineFlags(int index) const
{  
  PRECONDITION(index < (int)LineArray.Size());
  return (TCoolTextBuffer::TLineFlags)LineArray[index]->Flags;
}
inline void TCoolFileBuffer::SetLineFlags(owl::uint index, owl::uint32 flags)
{  
  PRECONDITION(index<LineArray.Size());
  LineArray[index]->Flags = flags; 
}
inline TCoolTextBuffer::TLineInfo* TCoolFileBuffer::GetLine(int index)
{  
  PRECONDITION(index < (int)LineArray.Size());
  return LineArray[index]; 
}

////////////////////////////////////////////////////////////////////
//
inline TCoolTextWnd* 
TCoolTextWnd::TTextSelection::GetParent(){
  return Parent;
}
inline bool TCoolTextWnd::TTextSelection::IsEmpty(){
  return GetStart() == GetEnd();
}
inline UINT TCoolTextWnd::TTextSelection::GetFormat(){
  return TCoolTextWnd::SelClipFormat;
}
inline TCoolTextBuffer* TCoolTextWnd::TTextSelection::GetBuffer(){
  return Parent->GetBuffer();
}

//
//
//
inline TCoolTextWnd::TSelType TCoolTextWnd::TLineSelection::GetType()
{ return TCoolTextWnd::stLineSelection; }
inline bool TCoolTextWnd::TLineSelection::HasSelection(int line){
  return (line >= Start && line < End) || (line < Start && line >= End);
}
inline TEditPos TCoolTextWnd::TLineSelection::GetStart(){ 
  return TEditPos(0,Start);
}
inline TEditPos TCoolTextWnd::TLineSelection::GetEnd(){ 
  return TEditPos(GetParent()->GetLineLength(End),End);
}
inline void TCoolTextWnd::TLineSelection::Extend(const TEditPos& pos){
  End = pos.row;
}
//
//
//
inline TCoolTextWnd::TSelType TCoolTextWnd::TStreamSelection::GetType()
{ return TCoolTextWnd::stStreamSelection; }
inline TCoolTextWnd::TStreamSelection::TStreamSelection(
  TCoolTextWnd* parent, const TEditRange& range) 
  : TTextSelection(parent), Range(range)
{
}
inline bool TCoolTextWnd::TStreamSelection::HasSelection(int line){
  return (line >= Range.srow && line <= Range.erow) || 
    (line <= Range.srow && line >= Range.erow);
}
inline TEditPos TCoolTextWnd::TStreamSelection::GetStart(){
  return Range.Start(); 
}
inline TEditPos TCoolTextWnd::TStreamSelection::GetEnd(){
  return Range.End(); 
}
inline void TCoolTextWnd::TStreamSelection::Extend(const TEditPos& pos){
  Range.ecol = pos.col;
  Range.erow = pos.row;
}
//
//
//
inline TCoolTextWnd::TSelType TCoolTextWnd::TColumnSelection::GetType()
{ return TCoolTextWnd::stColumnSelection; }
//
//
//
inline bool TCoolTextWnd::IsFlagSet(owl::uint flag) const{
  return (Flags&flag) != 0;
}
inline void TCoolTextWnd::SetFlag(owl::uint flag){
  Flags |= flag;
}
inline void TCoolTextWnd::ClearFlag(owl::uint flag){
  Flags &= ~flag;
}
inline int TCoolTextWnd::GetNumLines() const{
  PRECONDITION(CONST_CAST(TCoolTextWnd*,this)->GetBuffer());
  return CONST_CAST(TCoolTextWnd*,this)->GetBuffer()->GetLineCount();
}
inline LPCTSTR TCoolTextWnd::GetLineText(int lineIndex){
  PRECONDITION(GetBuffer());
  return GetBuffer()->GetLineText(lineIndex);
}
inline int TCoolTextWnd::GetLineLength(int index) const
{
  PRECONDITION(CONST_CAST(TCoolTextWnd*,this)->GetBuffer());
  return CONST_CAST(TCoolTextWnd*,this)->GetBuffer()->GetLineLength(index);
}
inline bool TCoolTextWnd::IsTabsVisible() const 
{  return IsFlagSet(cfViewTabs);}
inline bool TCoolTextWnd::IsCaretEnable() const 
{  return !IsFlagSet(cfNoUseCaret);}
inline bool TCoolTextWnd::IsSelectionMargin() const
{  return IsFlagSet(cfSelMargin);}
inline bool TCoolTextWnd::IsSmartCursor() const
{  return IsFlagSet(cfSmartCursor);}
inline bool TCoolTextWnd::IsShowInactiveSel() const
{  return IsFlagSet(cfShowInactiveSel);}
inline bool TCoolTextWnd::IsDragDrop() const 
{  return !IsFlagSet(cfNoDragDrop);}
inline bool TCoolTextWnd::IsAutoIndent() const
{  return IsFlagSet(cfAutoIntend);}
//
inline bool TCoolTextWnd::IsLineNumbers() const
{  return IsFlagSet(cfShowLineNumber);}
//
inline bool TCoolTextWnd::IsOverType() const 
{  return IsFlagSet(cfOverType);}
//
inline TSyntaxParser* TCoolTextWnd::GetSyntaxParser()
{  return SyntaxParser;}
//
inline TCoolTextBuffer::TSyntaxDescr& 
TCoolTextWnd::GetSyntaxDescr(int index) const{
  PRECONDITION(CONST_CAST(TCoolTextWnd*,this)->GetBuffer());
  return CONST_CAST(TCoolTextWnd*,this)->GetBuffer()->GetSyntaxDescr(index);
}
//
inline owl::TColor TCoolTextWnd::GetTxColor(int index) const
{  return GetSyntaxDescr(index).BkColor;}
//
inline owl::TColor TCoolTextWnd::GetBkColor(int index) const
{  return GetSyntaxDescr(index).BkColor;}
//
inline bool TCoolTextWnd::IsUnderline(int index) const
{  return GetSyntaxDescr(index).FontIndex & 0x04;}
//
inline bool TCoolTextWnd::IsItalic(int index) const
{  return GetSyntaxDescr(index).FontIndex & 0x02;}
//
inline bool TCoolTextWnd::IsBold(int index) const
{  return GetSyntaxDescr(index).FontIndex & 0x01;}
//
inline void TCoolTextWnd::GetFont(LOGFONT& lf) const{
  PRECONDITION(CONST_CAST(TCoolTextWnd*,this)->GetBuffer());
  CONST_CAST(TCoolTextWnd*,this)->GetBuffer()->GetFont(lf);
}
//
inline owl::TFont* TCoolTextWnd::GetFont(int fontindex){
  PRECONDITION(fontindex < 4 && fontindex >=0);
  return CreateFont(fontindex);
}
//
inline int TCoolTextWnd::GetTabSize() const{
  PRECONDITION(TabSize >= 0 && TabSize <= 64);
  return TabSize;
}
inline int TCoolTextWnd::GetLineDigits() const
{  return LineDigits; }
inline TEditPos TCoolTextWnd::GetCursorPos()
{  return CursorPos;}
inline void TCoolTextWnd::CopySelection(){
  PRECONDITION(Selection);
  if(Selection)
    Selection->Copy();
}
inline void TCoolTextWnd::CmEditCopy()
{  CopySelection();}
inline void TCoolTextWnd::CeSelectEnable(owl::TCommandEnabler& ce)
{  ce.Enable(Selection);}
inline bool TCoolTextWnd::IsStreamSelMode() const
{  return SelType == stStreamSelection;}
inline bool TCoolTextWnd::IsLineSelMode() const
{  return SelType == stLineSelection;}
inline bool TCoolTextWnd::IsColumnSelMode() const
{  return SelType == stColumnSelection;}
inline bool TCoolTextWnd::IsSelection() const
{  return Selection != 0;}
inline int TCoolTextWnd::GetLineNumberWidth() const
{  return LineDigits; }
// 
inline LPCTSTR TSyntaxParser::GetLineText(int index)
{ return Parent->GetLineText(index); }
//
inline LPCTSTR TSyntaxParser::GetBreakChars() const
{ return Parent->GetBreakChars(); }
//
// Return the search data used for the common dialog.
//
inline TCoolFindDlg::TData& TCoolSearchWnd::GetSearchData(){
  return SearchData;
}

//
// Return the common dialog pointer.
//
inline TCoolFindDlg* TCoolSearchWnd::GetSearchDialog() {
  return SearchDialog;
}

//
// Use new common dialog pointer.
//
inline void TCoolSearchWnd::SetSearchDialog(TCoolFindDlg* searchdialog){
  SearchDialog = searchdialog;
}

//
// Return the user selected command that brought up the search dialog.
//
inline owl::uint TCoolSearchWnd::GetSearchCmd() {
  return SearchCmd;
}

//
// Remember the command the user selected to bring up the search dialog.
//
inline void TCoolSearchWnd::SetSearchCmd(owl::uint searchcmd) {
  SearchCmd = searchcmd;
}
//
inline bool TCoolEdit::IsReadOnly() const {
  return CONST_CAST(TCoolEdit*,this)->GetBuffer()->IsReadOnly();
}
//
inline void TCoolEdit::EnableReadOnly(bool enable)
{ GetBuffer()->EnableReadOnly(enable); }
//
inline void TCoolEdit::CmEditCut()
{ Cut(); }
//
inline void TCoolEdit::CmEditPaste()
{ Paste(); }
//
inline void TCoolEdit::CmEditClear()
{ ClearAll(); }
//
inline LPCTSTR TCoolEditFile::GetFileName() const
{  return FileName; }
//
inline owl::TOpenSaveDialog::TData& TCoolEditFile::GetFileData() 
{  return FileData; }
//
inline void TCoolEditFile::SetFileData(const owl::TOpenSaveDialog::TData& fd) 
{  FileData = fd; }
//
inline void TCoolEditFile::CmFileNew()
{  FileNew(); }
//
inline void TCoolEditFile::CmFileSave()
{  FileSave(); }
//
inline void TCoolEditFile::CmFileOpen()
{    FileOpen(0); }
//
inline void TCoolEditFile::CmFileSaveAs()
{   FileSaveAs(); }

// class TCoolSearchView
// ~~~~~ ~~~~~~~~~~~~~~~
inline TCoolSearchView::~TCoolSearchView() 
{}
//
inline LPCTSTR TCoolSearchView::StaticName()
{  return _T("CoolSearch View");}
//
inline bool TCoolSearchView::CanClose()
{ return TCoolSearchWnd::CanClose() && (Doc->NextView(this) || 
                    Doc->NextView(0) != this || Doc->CanClose()); }
//
inline LPCTSTR TCoolSearchView::GetViewName() 
{  return StaticName(); }
//
inline owl::TWindow* TCoolSearchView::GetWindow() 
{  return (owl::TWindow*)this; }
//
inline bool TCoolSearchView::VnIsWindow(HWND hWnd)
{ return hWnd == GetHandle() || IsChild(hWnd); }
//
inline bool TCoolSearchView::SetDocTitle(LPCTSTR docname, int index) 
{  return TCoolSearchWnd::SetDocTitle(docname, index); }
//
// class TCoolEditView
// ~~~~~ ~~~~~~~~~~~~~
inline TCoolEditView::~TCoolEditView() 
{}
//
inline LPCTSTR TCoolEditView::StaticName()
{  return _T("CoolEdit View");}
//
inline bool TCoolEditView::CanClose()
{ return TCoolEdit::CanClose() && (Doc->NextView(this) || 
                   Doc->NextView(0) != this || Doc->CanClose()); }
//
inline LPCTSTR TCoolEditView::GetViewName() 
{  return StaticName(); }
//
inline owl::TWindow* TCoolEditView::GetWindow() 
{  return (owl::TWindow*)this; }
//
inline bool TCoolEditView::VnIsWindow(HWND hWnd)
{ return hWnd == GetHandle() || IsChild(hWnd); }
//
inline bool TCoolEditView::SetDocTitle(LPCTSTR docname, int index) 
{  return TCoolEdit::SetDocTitle(docname, index); }


/*============================================================================*/
#endif // COOLEDIT_INL
