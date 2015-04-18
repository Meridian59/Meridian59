//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Definition of class TComboBoxEx.
//----------------------------------------------------------------------------

#if !defined(OWL_COMBOBEX_H)
#define OWL_COMBOBEX_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>
#include <owl/commctrl.h>
#include <owl/combobox.h>
#include <owl/property.h>

namespace owl {

class _OWLCLASS TImageList;
class _OWLCLASS TComboBoxEx;
class _OWLCLASS TComboBoxExData;

#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{

//
/// \class TComboBoxExItem
/// Encapsulates an item in an extended combo box (COMBOBOXEXITEM).
//
class _OWLCLASS TComboBoxExItem 
{
  public:

    TComboBoxExItem();
    TComboBoxExItem(const tstring& str, INT_PTR item = -1,int image = -1);
    TComboBoxExItem(const tstring& str, INT_PTR item,int image, int selectedImage, int overlayImage = -1, int indent = -1, LPARAM param = 0);
    TComboBoxExItem(const TComboBoxExItem& item);
    TComboBoxExItem& operator =(const TComboBoxExItem& item);

    /// \name Comparison operators for templates
    /// @{
    bool operator <(const TComboBoxExItem& item) {return this < &item;}
    bool operator ==(const TComboBoxExItem& item) {return this == &item;}
    /// @}

    /// \name Properties
    /// @{
    TProperty<int> Mask; ///< Band image index (into rebar image list): don't use -1
    TProperty<INT_PTR> Item; ///< == -1 to add at end
    TObjProperty<tstring> Text; ///< Band text label
    TProperty<int> Image; ///< The item image
    TProperty<int> Selected; ///< The item selected image
    TProperty<int> Overlay; ///< Band colors
    TProperty<int> Indent; ///< Band colors
    TProperty<LPARAM> Param; ///< Additional data
    /// @}

  protected:

    /// Initialises all data members to zero
    //
    void Init();

    /// Initializes native item with this.
    //
    void InitItem(COMBOBOXEXITEM& item, uint tsize = 0) const;

    /// Initializes this with native item.
    //
    void ReFillItem(const COMBOBOXEXITEM& item);

  friend class _OWLCLASS TComboBoxEx;
};

//
/// \class TComboBoxEx
//
/// ComboBoxEx controls are combo box controls that provide native support for item images.
//
class _OWLCLASS TComboBoxEx 
  : public TComboBox 
{
  public:

    TComboBoxEx(TWindow* parent, int id, int x, int y, int w, int h, uint32 style, uint textLimit, TModule* module = 0);
    TComboBoxEx(TWindow* parent, int resourceId, uint textLen = 0, TModule* module = 0);
    TComboBoxEx(THandle hWnd, TModule* module = 0);
    ~TComboBoxEx();

    HWND GetEditControl();
    HWND GetComboBoxCtrl();
    bool HasEditChanged();

    uint32 GetExtendedStyle() const;
    uint32 SetExtendedStyle(uint32 mask, uint32 style);

    TImageList* GetImageList();
    void SetImageList(TImageList* list, TAutoDelete = AutoDelete);

    void SetUnicode(bool unicode);
    bool GetUnicode();

    int DeleteItem(int index);
    bool GetItem(TComboBoxExItem& item, INT_PTR index=-1);
    TComboBoxExItem GetItem(INT_PTR index);
    int InsertItem(const TComboBoxExItem& item);
    bool SetItem(const TComboBoxExItem& item, INT_PTR index=-1);

    /// Safe overload
    //
    void Transfer(TComboBoxExData& data, TTransferDirection op) 
    {Transfer(&data, op);}

  protected:

    // Override TWindow virtual member functions
    //
    virtual uint Transfer(void* buffer, TTransferDirection);
    virtual TGetClassNameReturnType GetClassName();
    virtual void SetupWindow();

    // Override TControl
    //
    void DeleteItem (DELETEITEMSTRUCT & deleteInfo);

    // Do nothing handlers
    //
    int EvCompareItem(uint ctrlId, const COMPAREITEMSTRUCT& comp);
    void EvDeleteItem(uint ctrlId, const DELETEITEMSTRUCT& del);
    void EvDrawItem(uint ctrlId, const DRAWITEMSTRUCT& draw);
    void EvMeasureItem(uint ctrlId, MEASUREITEMSTRUCT & meas);

  protected_data:

    TImageList* ImageList;
    bool ShouldDelete;

  private:

    // Hidden to prevent accidental copying or assignment
    //
    TComboBoxEx(const TComboBoxEx&);
    TComboBoxEx& operator =(const TComboBoxEx&);

  DECLARE_RESPONSE_TABLE(TComboBoxEx);
};


typedef TObjectArray<TComboBoxExItem> TComboBoxExItemArray;
typedef TComboBoxExItemArray::Iterator TComboBoxExItemArrayIter;

//
/// \class TComboBoxExData
//
class _OWLCLASS TComboBoxExData 
{
  public:
    TComboBoxExData();
   ~TComboBoxExData();

    int AddItem(const TComboBoxExItem& item);
    int DeleteItem(int index);
    TComboBoxExItem& GetItem(int index);

    void Clear();
    uint Size();
    void Select(int index);
    void SelectString(LPCTSTR str);
    void SelectString(const tstring& str) {SelectString(str.c_str());}
    tstring& GetSelection();
    const tstring& GetSelection() const;
    int GetSelIndex() const;
    int GetSelCount() const;
    void ResetSelections();

    int GetSelStringLength() const;
    void GetSelString(LPTSTR buffer, int bufferSize) const;
    const tstring&  GetSelString() const {return Selection;}

  protected:
    TComboBoxExItemArray* Items;
    tstring Selection;
    int SelIndex;
};

/// @}

#include <owl/posclass.h>

//
// inlines
//

//
inline HWND TComboBoxEx::GetEditControl(){
  PRECONDITION(GetHandle());
  return (HWND)SendMessage(CBEM_GETEDITCONTROL,0);
}

//
inline HWND TComboBoxEx::GetComboBoxCtrl(){
  PRECONDITION(GetHandle());
  return (HWND)SendMessage(CBEM_GETCOMBOCONTROL, 0);
}

//
inline uint32 TComboBoxEx::GetExtendedStyle() const{
  PRECONDITION(GetHandle());
  return (uint32)CONST_CAST(TComboBoxEx*,this)->SendMessage(CBEM_GETEXTENDEDSTYLE, 0);
}

//
inline uint32 TComboBoxEx::SetExtendedStyle(uint32 mask, uint32 style){
  PRECONDITION(GetHandle());
  return (uint32)SendMessage(CBEM_SETEXTENDEDSTYLE, TParam1(mask), TParam2(style));
}

//
inline int TComboBoxEx::DeleteItem(int index){
  PRECONDITION(GetHandle());
  return (int)SendMessage(CBEM_DELETEITEM, TParam1(index));
}

//
inline bool TComboBoxEx::HasEditChanged(){
  PRECONDITION(GetHandle());
  return SendMessage(CBEM_HASEDITCHANGED,0)==0;
}

//
inline TImageList* TComboBoxEx::GetImageList(){
  return ImageList;
}

//
inline void TComboBoxEx::SetUnicode(bool unicode){
  PRECONDITION(GetHandle());
  SendMessage(CBEM_SETUNICODEFORMAT,TParam1(unicode));
}

//
inline bool TComboBoxEx::GetUnicode(){
  PRECONDITION(GetHandle());
  return SendMessage(CBEM_GETUNICODEFORMAT,0)!=0;
}

//
inline void TComboBoxEx::DeleteItem (DELETEITEMSTRUCT & deleteInfo){
  DefaultProcessing();
}

//
inline int TComboBoxEx::EvCompareItem(uint, const COMPAREITEMSTRUCT&){
  return static_cast<int>(DefaultProcessing());
}

//
inline void TComboBoxEx::EvDeleteItem(uint, const DELETEITEMSTRUCT&){
  DefaultProcessing();
}

//
inline void TComboBoxEx::EvDrawItem(uint, const DRAWITEMSTRUCT&){
  DefaultProcessing();
}

//
inline void TComboBoxEx::EvMeasureItem(uint, MEASUREITEMSTRUCT&){
  DefaultProcessing();
}

//
/// Selects an item at a given index.
//
inline void TComboBoxExData::Select(int index){
  SelIndex = index;
  Selection = SelIndex >= 0 ? GetItem(SelIndex).Text() : tstring();
}

//
inline int TComboBoxExData::GetSelIndex() const{
  return SelIndex;
}

//
inline tstring& TComboBoxExData::GetSelection(){
  return Selection;
}

inline const tstring& TComboBoxExData::GetSelection() const{
  return Selection;
}

//
inline int TComboBoxExData::GetSelCount() const{
  return SelIndex == CB_ERR ? 0 : 1;
}

//
inline void TComboBoxExData::ResetSelections(){
  SelIndex = CB_ERR;
  Selection = _T("");
}

//
/// Returns the length of the selection string excluding the terminating 0
//
inline int TComboBoxExData::GetSelStringLength() const{
  return static_cast<int>(Selection.length());
}



} // OWL namespace

#endif  // OWL_COMBOBEX_H
