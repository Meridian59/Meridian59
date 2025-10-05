// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// gadgdesc.cpp: implementation file
// Version:      1.5
// Date:         08.11.1998
// Author:       Dieter Windau
//
// Freeware OWL classes that extents the dockable and gadget system
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02, OWL6 patch #3 and with Windows
// NT 4.0 SP3 but I think the class should work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This package contains many classes and source that are based on other OWL
// developers work. Very special thanks to Alan Chambers, Christopher Kohlhoff,
// Jo Parrello, Mark Hatsell, Michael Mogensen and Yura Bidus
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
//
// ****************************************************************************

#include <owlext\pch.h>
#pragma hdrstop

#include <owl/bitmapga.h>
#include <owl/menugadg.h>

#include <owlext/gadgdesc.h>
#include <owlext/gadgetex.h>

DIAG_DEFINE_GROUP(GadgetDesc, 1, 0);

using namespace owl;

namespace OwlExt {

TGadgetDescriptors* GadgetDescriptors = 0;

// ******************** TGadgetDesc *******************************************

TGadgetDesc::TGadgetDesc(TType type,
             int   id,
             uint  attr)
{
  Type = type;
  Id = id;
  Attr = attr;
}

// ******************** TInvisibleGadgetDec ***********************************

TInvisibleGadgetDesc::TInvisibleGadgetDesc():
TGadgetDesc(InvisibleGadgetDesc, CM_INVISIBLEGADGET, GADG_NOOPERATION)
{
}

TGadget* TInvisibleGadgetDesc::ConstructGadget()
{
  return new TInvisibleGadgetEx();
}

// ******************** TButtonGadgetDesc *************************************

TButtonGadgetDesc::TButtonGadgetDesc(uint32 bmpResId,
                   int  id,
                   TButtonGadget::TType buttonGadgetType,
                   bool enabled,
                   TButtonGadget::TState buttonGadgetState,
                   bool sharedGlyph,
                   uint attr):
TGadgetDesc(ButtonGadgetDesc, id, attr)
{
  PRECONDITION (Id);
  BmpResId = bmpResId;
  ButtonGadgetType = buttonGadgetType;
  Enabled = enabled;
  ButtonGadgetState = buttonGadgetState;
  SharedGlyph = sharedGlyph;
}

TGadget* TButtonGadgetDesc::ConstructGadget()
{
  return new TButtonGadgetEx(BmpResId, Id, ButtonGadgetType, Enabled,
    ButtonGadgetState, SharedGlyph);
}

// ******************** TButtonTextGadgetDesc *********************************

TButtonTextGadgetDesc::TButtonTextGadgetDesc(
  LPCTSTR                text,
  TDisplayType           disptype,
  uint32                 bmpResId,
  int                    id,
  TButtonGadget::TType   buttonGadgetType,
  bool                   enabled,
  TButtonGadget::TState  buttonGadgetState,
  bool                   sharedGlyph,
  uint                   attr):
TButtonGadgetDesc(bmpResId, id, buttonGadgetType,
          enabled, buttonGadgetState, sharedGlyph, attr)
{
  Type = ButtonTextGadgetDesc;
  if (text)
    Text = text;
  else
    Text = TEXT("");
  DispType = disptype;
}

TGadget* TButtonTextGadgetDesc::ConstructGadget()
{
  return new TButtonTextGadgetEx(Text.c_str(), DispType, BmpResId, Id,
    ButtonGadgetType, Enabled, ButtonGadgetState, SharedGlyph);
}

// ******************** TMenuButtonGadgetDesc *********************************

TMenuButtonGadgetDesc::TMenuButtonGadgetDesc(
  TPopupType             popupType,
  HMENU                  hmenu,
  TWindow*               cmdTarget,
  LPCTSTR                text,
  TDisplayType           disptype,
  uint32                 bmpResId,
  int                    id,
  TButtonGadget::TType   buttonGadgetType,
  bool                   enabled,
  TButtonGadget::TState  buttonGadgetState,
  bool                   sharedGlyph,
  uint                   attr):
TButtonTextGadgetDesc(text, disptype, bmpResId, id,
            buttonGadgetType, enabled, buttonGadgetState, sharedGlyph, attr)
{
  Type = MenuButtonGadgetDesc;
  PopupType = popupType;
  hMenu = hmenu;
  CmdTarget = cmdTarget;
}

TGadget* TMenuButtonGadgetDesc::ConstructGadget()
{
  return new TMenuButtonGadgetEx(PopupType, hMenu, CmdTarget, Text.c_str(),
    DispType, BmpResId, Id,  ButtonGadgetType, Enabled, ButtonGadgetState,
    SharedGlyph);
}

// ******************** TRecentFilesGadgetDesc ********************************

TRecentFilesGadgetDesc::TRecentFilesGadgetDesc(
  LPCTSTR                text,
  TDisplayType           disptype,
  uint32                 bmpResId,
  int                    id,
  TButtonGadget::TType   buttonGadgetType,
  bool                   enabled,
  TButtonGadget::TState  buttonGadgetState,
  bool                   sharedGlyph,
  uint                   attr):
TButtonTextGadgetDesc(text, disptype, bmpResId, id,
            buttonGadgetType, enabled, buttonGadgetState, sharedGlyph, attr)
{
  Type = RecentFilesGadgetDesc;
}

TGadget* TRecentFilesGadgetDesc::ConstructGadget()
{
  return new TRecentFilesGadgetEx(Text.c_str(), DispType, BmpResId, Id,
    ButtonGadgetType, Enabled, ButtonGadgetState, SharedGlyph);
}

// ******************** TColorButtonGadgetDesc ********************************

TColorButtonGadgetDesc::TColorButtonGadgetDesc(
  TPopupType             popupType,
  TColorPickerData&      data,
  TColor                 startColorSel,
  TRect*                 fillRect,
  TWindow*               parentWindow,
  LPCTSTR                text,
  TDisplayType           disptype,
  uint32                 bmpResId,
  int                    id,
  TButtonGadget::TType   buttonGadgetType,
  bool                   enabled,
  TButtonGadget::TState  buttonGadgetState,
  bool                   sharedGlyph,
  uint                   attr):
TButtonTextGadgetDesc(text, disptype, bmpResId, id,
            buttonGadgetType, enabled, buttonGadgetState, sharedGlyph, attr),
            Data(data)
{
  Type = ColorButtonGadgetDesc;
  PopupType = popupType;
  StartColorSel = startColorSel;
  FillRect = fillRect;
  ParentWindow = parentWindow;
}

TGadget* TColorButtonGadgetDesc::ConstructGadget()
{
  return new TColorButtonGadgetEx(PopupType, Data, StartColorSel,
    FillRect, ParentWindow, Text.c_str(), DispType, BmpResId, Id,
    ButtonGadgetType, Enabled, ButtonGadgetState, SharedGlyph);
}

// ******************** TSeparatorGadgetDesc **********************************

TSeparatorGadgetDesc::TSeparatorGadgetDesc(int  size,
                       int  id):
TGadgetDesc(SeparatorGadgetDesc, id, GADG_NOOPERATION)
{
  Size = size;
}

TGadget* TSeparatorGadgetDesc::ConstructGadget()
{
  return new TSeparatorGadget(Size, Id, false);
}

// ******************** TControlGadgetExDesc ************************************

TControlGadgetDesc::TControlGadgetDesc(TWindow& control,
                     TGadget::TBorderStyle gadgetBorderStyle,
                     uint attr):
TGadgetDesc(ControlGadgetDesc, control.GetId(), attr)
{
  PRECONDITION (Id);
  CHECK (Id != CM_INVISIBLEGADGET);

  Control = &control;
  GadgetBorderStyle = gadgetBorderStyle;
  Count = 0;
}

TControlGadgetDesc::~TControlGadgetDesc()
{
  if (Count == 0) {
    if (Control->GetHandle())
      Control->Destroy(0);
    delete Control;
  }
}

TGadget* TControlGadgetDesc::ConstructGadget()
{
  PRECONDITION(Control);
  return new TControlGadgetEx(*Control, GadgetBorderStyle);
}

// ******************** TTextGadgetDesc ***************************************

TTextGadgetDesc::TTextGadgetDesc(int    id,
                 TGadget::TBorderStyle gadgetBorderStyle,
                 TTextGadget::TAlign   textGadgetAlign,
                 uint    numChars,
                 LPCTSTR text,
                 uint    attr):
TGadgetDesc(TextGadgetDesc, id, attr)
{
  PRECONDITION (Id);
  CHECK (Id != CM_INVISIBLEGADGET);

  GadgetBorderStyle = gadgetBorderStyle;
  TextGadgetAlign = textGadgetAlign;
  NumChars = numChars;
  Text     = text;
}

TGadget* TTextGadgetDesc::ConstructGadget()
{
  return new TTextGadget(Id, GadgetBorderStyle, TextGadgetAlign,
    NumChars, Text.c_str());
}

// ******************** TMenuGadgetDesc ***************************************

TMenuGadgetDesc::TMenuGadgetDesc(TMenu*   menu,
                 TWindow* window,
                 int      id,
                 TGadget::TBorderStyle gadgetBorderStyle,
                 LPCTSTR  text,
                 TFont*   font,
                 uint     attr):
TGadgetDesc(MenuGadgetDesc, id, attr)
{
  PRECONDITION (Id);
  CHECK (Id != CM_INVISIBLEGADGET);

  Menu = menu;
  Window = window;
  GadgetBorderStyle = gadgetBorderStyle;
  Text = text;
  Font = font ? font : new TGadgetWindowFont;
}

TMenuGadgetDesc::~TMenuGadgetDesc()
{
  delete Font;
}

TGadget* TMenuGadgetDesc::ConstructGadget()
{
  return new TMenuGadget(*Menu, Window, Id, GadgetBorderStyle,
    (LPTSTR)Text.c_str(), new TFont(*Font));
}

// ******************** TTimeGadgetDesc ***************************************

TTimeGadgetDesc::TTimeGadgetDesc(TTimeGadget::TGetTimeFunc timeFunc,
                 int     id,
                 TGadget::TBorderStyle gadgetBorderStyle,
                 TTextGadget::TAlign   textGadgetAlign,
                 uint    numChars,
                 LPCTSTR text,
                 TFont*  font,
                 uint    attr):
TTextGadgetDesc(id, gadgetBorderStyle, textGadgetAlign, numChars, text, attr)
{
  TimeFunc = timeFunc;
  Type = TimeGadgetDesc;
  Font = font ? font : new TGadgetWindowFont;
}

TTimeGadgetDesc::~TTimeGadgetDesc()
{
  delete Font;
}

TGadget* TTimeGadgetDesc::ConstructGadget()
{
  return new TTimeGadget(TimeFunc, Id, GadgetBorderStyle,
    TextGadgetAlign, NumChars, Text.c_str(), new TFont(*Font));
}

// ******************** TBitmapGadgetDesc *************************************

TBitmapGadgetDesc::TBitmapGadgetDesc(TResId imageResIdOrIndex,
                   int  id,
                   TGadget::TBorderStyle gadgetBorderStyle,
                   int  numImages,
                   int  startImage,
                   bool sharedCels,
                   uint attr):
TGadgetDesc(BitmapGadgetDesc, id, attr)
{
  PRECONDITION (Id);
  CHECK (Id != CM_INVISIBLEGADGET);

  ImageResIdOrIndex = imageResIdOrIndex;
  GadgetBorderStyle = gadgetBorderStyle;
  NumImages = numImages;
  StartImage = startImage;
  SharedCels = sharedCels;
}

TGadget* TBitmapGadgetDesc::ConstructGadget()
{
  return new TBitmapGadget(ImageResIdOrIndex, Id, GadgetBorderStyle,
    NumImages, StartImage, SharedCels);
}

// ******************** TCommandCategory **************************************

TCommandCategory::TCommandCategory(LPCTSTR name):
Array(5,0,5)
{
  PRECONDITION(name);
  Name = name;
}

void TCommandCategory::CheckConsistence()
// Delete all duplicate id's in this group
{
  uint i,j, num = Array.GetItemsInContainer();
  int id, ifound;
  for (i=0; i<num; i++) {
    id = Array[i];
    ifound = 0;
    for (j=0; j<num; j++) {
      if (Array[j] == id)
        ifound++;
      if (ifound > 1) {
        Array.Destroy(j);
        num--;
        j--;
        ifound--;
        TRACEX(GadgetDesc, 1, "TCommandCategory @" << (void*)this <<
          " Delete duplicate id: " << id);
      }
    }
  }
}

bool TCommandCategory::operator ==(const TCommandCategory& g) const
{
  if (Name == g.Name &&
    Array.GetItemsInContainer() == g.Array.GetItemsInContainer()) {
      for (uint i=0; i<Array.GetItemsInContainer(); i++) {
        if (Array[i] != g.Array[i])
          return false;
      }
      return true;
  }
  return false;
}

// ******************** TButtonGadgetGruop ************************************

TButtonGadgetGroup::TButtonGadgetGroup(TButtonGadget::TType buttonGadgetType):
Array(5,0,5)
{
  CHECK(buttonGadgetType != TButtonGadget::Command);
  CHECK(buttonGadgetType != TButtonGadget::NonExclusive);

  // Without diagnostic set ButtonGadgetType to Exclusive
  //
  if (buttonGadgetType == TButtonGadget::Command ||
    buttonGadgetType == TButtonGadget::NonExclusive)
    ButtonGadgetType = TButtonGadget::Exclusive;
  else
    ButtonGadgetType = buttonGadgetType;
}

void TButtonGadgetGroup::CheckConsistence()
// Delete all duplicate id's in this group
{
  uint i,j, num = Array.GetItemsInContainer();
  int id, ifound;
  for (i=0; i<num; i++) {
    id = Array[i];
    ifound = 0;
    for (j=0; j<num; j++) {
      if (Array[j] == id)
        ifound++;
      if (ifound > 1) {
        Array.Destroy(j);
        num--;
        j--;
        ifound--;
        TRACEX(GadgetDesc, 1, "TButtonGadgetGroup @" << (void*)this <<
          " Delete duplicate id: " << id);
      }
    }
  }
}

bool TButtonGadgetGroup::operator ==(const TButtonGadgetGroup& g) const
{
  if (ButtonGadgetType == g.ButtonGadgetType &&
    Array.GetItemsInContainer() == g.Array.GetItemsInContainer()) {
      for (uint i=0; i<Array.GetItemsInContainer(); i++) {
        if (Array[i] != g.Array[i])
          return false;
      }
      return true;
  }
  return false;
}

// ******************** TGadgetDescriptors ************************************

TGadgetDescriptors::TGadgetDescriptors():
Array(5,0,5),
Groups(5,0,5),
Categories(5,0,5)
{
  Array.Add(new TSeparatorGadgetDesc());
  Array.Add(new TInvisibleGadgetDesc());
}

TGadget* TGadgetDescriptors::ConstructGadget(int id)
{
  uint i;
  for (i=0; i<Array.GetItemsInContainer(); i++)
    if (Array[i]->Id == id)
      return Array[i]->ConstructGadget();
  return 0;
}

TGadgetDesc* TGadgetDescriptors::Find(int id)
{
  uint i;
  for (i=0; i<Array.GetItemsInContainer(); i++)
    if (Array[i]->Id == id)
      return Array[i];
  return 0;
}

TButtonGadgetGroup* TGadgetDescriptors::FindGroup(int id)
{
  uint i,j;
  TButtonGadgetDesc* desc = TYPESAFE_DOWNCAST(Find(id), TButtonGadgetDesc);
  if (desc) {
    for (i=0; i<Groups.GetItemsInContainer(); i++)
      for (j=0; j<Groups[i]->Array.GetItemsInContainer(); j++)
        if (Groups[i]->Array[j] == id)
          return Groups[i];
  }
  return 0;
}

// return the first category where a gadget with id is found
//
TCommandCategory* TGadgetDescriptors::FindCategory(int id)
{
  uint i,j;
  TButtonGadgetDesc* desc = TYPESAFE_DOWNCAST(Find(id), TButtonGadgetDesc);
  if (desc) {
    for (i=0; i<Categories.GetItemsInContainer(); i++)
      for (j=0; j<Categories[i]->Array.GetItemsInContainer(); j++)
        if (Categories[i]->Array[j] == id)
          return Categories[i];
  }
  return 0;
}

// return the category with name
//
TCommandCategory* TGadgetDescriptors::FindCategory(owl::tstring name)
{
  for (uint i=0; i<Categories.GetItemsInContainer(); i++) {
    if (Categories[i]->Name == name)
      return Categories[i];
  }
  return 0;
}

void TGadgetDescriptors::CheckConsistence()
// Checks the consistence of all TButtonGadgetDesc and TButtonGadgetGroups
// Should be called after all members are added
{
  uint i, j, k, num, num2;
  int id, ifound;
  TButtonGadgetDesc* buttonDesc;
  TButtonGadgetGroup* group;

  // If there exist more than one GadgetDesc with the same id,
  // delete the second one
  //
  num = Array.GetItemsInContainer();
  for (i=0; i<num; i++) {
    id = Array[i]->Id;
    ifound = 0;
    for (j=0; j<num; j++) {
      if (Array[j]->Id == id)
        ifound++;
      if (ifound > 1) {
        Array.Destroy(j);
        num--;
        j--;
        ifound--;
        TRACEX(GadgetDesc, 1, "TGadgetDescriptor: Delete duplicate id: " << id);
      }
    }
  }

  // Delete duplicate id's in every group
  //
  num = Groups.GetItemsInContainer();
  for (i=0; i<num; i++) {
    Groups[i]->CheckConsistence();
  }

  // Delete duplicate id's in every category
  //
  num = Categories.GetItemsInContainer();
  for (i=0; i<num; i++) {
    Categories[i]->CheckConsistence();
  }

  // If the id of the ButtonGadgetDesc exist more than one
  // time in the ButtonGadgetGroups. Delete the id in the second group
  //
  num = Array.GetItemsInContainer();
  for (i=0; i<num; i++) {
    buttonDesc = TYPESAFE_DOWNCAST(Array[i], TButtonGadgetDesc);
    if (buttonDesc) {
      id = Array[i]->Id;
      ifound = 0;
      for (j=0; j<Groups.GetItemsInContainer(); j++) {
        num2 = Groups[j]->Array.GetItemsInContainer();
        for (k=0; k<num2; k++) {
          if (Groups[j]->Array[k] == id)
            ifound ++;
          if (ifound > 1) {
            Groups[j]->Array.Destroy(k);
            ifound--;
            k--;
            num2--;
            TRACEX(GadgetDesc, 1, "TButtonGadgetGroup @" << (void*)Groups[j] <<
              " Delete duplicate id: " << id);
          }
        }
      }
    }
  }

  // If the id of the GadgetDesc exist more than one
  // time in the CommandCategories. Delete the id in the second category
  //
  num = Array.GetItemsInContainer();
  for (i=0; i<num; i++) {
    if (Array[i]) {
      id = Array[i]->Id;
      ifound = 0;
      for (j=0; j<Categories.GetItemsInContainer(); j++) {
        num2 = Categories[j]->Array.GetItemsInContainer();
        for (k=0; k<num2; k++) {
          if (Categories[j]->Array[k] == id)
            ifound ++;
          if (ifound > 1) {
            Categories[j]->Array.Destroy(k);
            ifound--;
            k--;
            num2--;
            TRACEX(GadgetDesc, 1, "TCommandCategoy @" << (void*)Categories[j] <<
              " Delete duplicate id: " << id);
          }
        }
      }
    }
  }

  // If there exist a id in a group, that has no associated TButtonGadgetDesc
  // delete the id in the group
  //
  num = Groups.GetItemsInContainer();
  for (i=0; i<num; i++) {
    num2 = Groups[i]->Array.GetItemsInContainer();
    for (j=0; j<num2; j++) {
      id = Groups[i]->Array[j];
      buttonDesc = TYPESAFE_DOWNCAST(Find(id),
        TButtonGadgetDesc);
      if (buttonDesc == 0) {
        Groups[i]->Array.Destroy(j);
        j--;
        num2--;
        TRACEX(GadgetDesc, 1, "TButtonGadgetGroup @" << (void*)Groups[i] <<
          " Delete id: " << id <<
          " Reason: Is not a button gadget");
      }
    }
  }

  // If there exist a id in a category, that is not a command gadget
  // delete the id in the category
  //
  num = Categories.GetItemsInContainer();
  for (i=0; i<num; i++) {
    num2 = Categories[i]->Array.GetItemsInContainer();
    for (j=0; j<num2; j++) {
      id = Categories[i]->Array[j];
      TGadgetDesc* desc = TYPESAFE_DOWNCAST(Find(id), TGadgetDesc);
      if (!(desc && (desc->Attr & GADG_ISCOMMAND))) {
        Categories[i]->Array.Destroy(j);
        j--;
        num2--;
        TRACEX(GadgetDesc, 1, "TCommandCategory @" << (void*)Categories[i] <<
          " Delete id: " << id <<
          " Reason: Is not a command gadget");
      }
    }
  }

  // Destroy all groups, that are empty
  //
  num = Groups.GetItemsInContainer();
  for (i=0; i<num; i++) {
    if (Groups[i]->Array.GetItemsInContainer() == 0) {
      TRACEX(GadgetDesc, 1, "Delete empty TButtonGadgetGroup @" <<
        (void*)Groups[i]);
      Groups.Destroy(i);
      i--;
      num--;
    }
  }

  // Destroy all categories, that are empty
  //
  num = Categories.GetItemsInContainer();
  for (i=0; i<num; i++) {
    if (Categories[i]->Array.GetItemsInContainer() == 0) {
      TRACEX(GadgetDesc, 1, "Delete empty TCommandCategory @" <<
        (void*)Categories[i]);
      Categories.Destroy(i);
      i--;
      num--;
    }
  }

  // Checks the consistence of ButtonGadgetType between
  // TButtonGadgetDesc and the TButtonGadgetGroup
  //
  num = Array.GetItemsInContainer();
  for (i=0; i<num; i++) {
    buttonDesc = TYPESAFE_DOWNCAST(Array[i], TButtonGadgetDesc);
    if (buttonDesc) {
      id = buttonDesc->Id;
      group = FindGroup(id);
      if (group) {
        if (buttonDesc->ButtonGadgetType != group->ButtonGadgetType) {
          buttonDesc->ButtonGadgetType = group->ButtonGadgetType;
        }
      }
    }
  }

  // If there is a ButtonGadget with Type Exclusive or SemiExclusive and no
  // associated group change the Type of ButtonGadget
  //
  num = Array.GetItemsInContainer();
  for (i=0; i<num; i++) {
    buttonDesc = TYPESAFE_DOWNCAST(Array[i], TButtonGadgetDesc);
    if (buttonDesc && (
      buttonDesc->ButtonGadgetType == TButtonGadget::Exclusive ||
      buttonDesc->ButtonGadgetType == TButtonGadget::SemiExclusive)) {
        id = buttonDesc->Id;
        group = FindGroup(id);
        if (group == 0) {

          // I'm not sure. What is the best error handling for this situation
          // I change them to Command if State is Up and to NonExclusive if Down
          //
          if (buttonDesc->ButtonGadgetState == TButtonGadget::Up)
            buttonDesc->ButtonGadgetType = TButtonGadget::Command;
          else
            buttonDesc->ButtonGadgetType = TButtonGadget::NonExclusive;
        }
    }
  }

  num = Groups.GetItemsInContainer();
  for (i=0; i<num; i++) {
    int iPressed = 0;
    if (Groups[i]->ButtonGadgetType != TButtonGadget::Command &&
      Groups[i]->ButtonGadgetType != TButtonGadget::NonExclusive) {

        // If there is a group with Type Exclusive or SemiExclusive,
        // then make sure that only one button is pressed
        //
        for (j=0; j<Groups[i]->Array.GetItemsInContainer(); j++) {
          buttonDesc = TYPESAFE_DOWNCAST(Find(Groups[i]->Array[j]),
            TButtonGadgetDesc);
          if (buttonDesc->ButtonGadgetState == TButtonGadget::Down)
            iPressed++;
          if (iPressed > 1) {
            buttonDesc->ButtonGadgetState = TButtonGadget::Up;
            iPressed--;
          }
        }

        // If there is a Exclusive group and no pressed button,
        // then press the first button
        //
        if (Groups[i]->ButtonGadgetType == TButtonGadget::Exclusive &&
          iPressed == 0) {
            buttonDesc = TYPESAFE_DOWNCAST(Find(Groups[i]->Array[0]),
              TButtonGadgetDesc);
            buttonDesc->ButtonGadgetState = TButtonGadget::Down;
        }
    }
  }
}

} // OwlExt namespace
//===============================================================================================