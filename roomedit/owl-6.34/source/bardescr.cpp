//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Implementation of class TBarDescr
//----------------------------------------------------------------------------

#include <owl/pch.h>

#include <owl/resource.h>
#include <owl/celarray.h>
#include <owl/gadget.h>
#include <owl/buttonga.h>
#include <owl/gadgetwi.h>
#include <owl/functor.h>
#include <owl/bardescr.h>
#include <owl/template.h>
#include <owl/functor.h>
#include <owl/glyphbtn.h>

namespace owl {

OWL_DIAGINFO;
/// \cond
struct __TBarDescrGdNode {
  __TBarDescrGdNode(TGadget* gd, bool ug): Gadget(gd),UseGlyph(ug){}
  ~__TBarDescrGdNode();
  TGadget*  Gadget;
  bool      UseGlyph;
};
//
__TBarDescrGdNode::~__TBarDescrGdNode()
{
  delete Gadget;
}
//
class TBarDescrGdArray:  public TIPtrArray<__TBarDescrGdNode*>{ // internal usage
  public:
    TBarDescrGdArray(int count):TIPtrArray<__TBarDescrGdNode*>(count){}
};
/// \endcond

//
//
//
static TGadget* CreateGadget(int index, int id, bool& usecell)
{
  usecell = false;
  if(id == 0)
    return new TSeparatorGadget;
  usecell = true;
  return new TButtonGadget(index, 
                           id, 
                           TButtonGadget::Command,
                           false,
                           TButtonGadget::Up,true);
}

// -----------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//  class TBarDescr
//  ~~~~~ ~~~~~~~~~

/// \class TBarDescr
/// TBarDescr describes your control bar and its functional groups. TBarDescr
/// provides an easy way for you to group gadgets on your control bar and to add new
/// groups to an existing control bar. It uses a resource ID to identify the bar
/// resource and an array of count values to indicate the number of gadgets in each
/// group on the control bar.
/// 
/// The TGroup enum enumerates the six basic functional groups on a control bar:
/// File, Edit, Container, Object, Window, and Help. TBarDescr's constructors simply
/// initialize the members based on the arguments passed.
/// 
/// TFrameWindow::MergeBar actually performs the real work of merging the gadgets
/// groups.
/// 
/// For a single document application (SDI), the gadgets are merged as soon as you
/// load the application. See the sample program, bardescm, for an example of MDI
/// control bar merging, and bardescr, for an example of SDI control bar merging.
/// 
/// One technique you can use to create a control bar involves invoking the
/// TBarDescr constructor and passing the number of group counts for each gadgets
/// selection.
/// 
/// For example, if your original gadgets groups included these items:
/// \image html bm78.BMP
/// you might use the following group counts:
/// <TABLE>
/// <TR><TD>Group</TD>	<TD>Count</TD>	<TD>Menu</TD></TR>
/// <TR><TD>FileGroup</TD>	<TD>1</TD>	<TD>File</TD></TR>
/// <TR><TD>EditGroup</TD>	<TD>2</TD>	<TD>Edit Search</TD></TR>
/// <TR><TD>ContainerGroup</TD>	<TD>1</TD>	<TD>View</TD></TR>
/// <TR><TD>ObjectGroup</TD>	<TD>3</TD>	<TD>Page Paragraph Word</TD></TR>
/// <TR><TD>WindowGroup</TD>	<TD>1</TD>	<TD>Window</TD></TR>
/// <TR><TD>HelpGroup</TD>	<TD>1</TD>	<TD>Help</TD></TR>
/// </TABLE>
/// You would then invoke the constructor this way:
/// \code
/// TBarDescr(IDM_MYMENU, 1, 2, 1, 3, 1, 1)
/// \endcode
/// You can build the previous control bar by merging two control bars. When a zero
/// is passed in the TBarDescr's constructor, the group indicated by the zero is
/// filled in by the child control bar's group, if an item is specified, when the
/// bar merging occurs.  Set your application's parent frame control bar by
/// specifying these gadget groups:
/// \image html bm79.BMP
/// and passing these group counts in the constructor:
/// \code
/// TBarDescr(IDM_FRAME, 1, 0, 1, 0, 1, 1)
/// \endcode
/// Set the word-processor child control bar this way:
/// \image html bm80.BMP
/// and pass these values in the constructor:
/// \code
/// TBarDescr(IDM_WPROC, 0, 2, 0, 3, 0, 1)
/// \endcode
/// If no child is active, only the frame menu will be active. When the word
/// processor's child window becomes active, the child control bar is merged with
/// the frame bar. Every group that is 0 in the child control bar leaves the
/// parent's group intact. The previous example interleaves every group except for
/// the last group, the Help group, in which the child replaces the frame bar.
/// 
/// By convention, the even groups (File, Container, Window) usually belong to the
/// outside frame or container, and the odd groups (Edit, Object, Help) belong to
/// the child or contained group.
/// 
/// If a -1 is used in a group count, the merger eliminates the parent's group
/// without replacing it. For example, another child control bar, such as a
/// calculator, could be added to your application in this way:
/// \image html bm81.BMP
/// \code
/// TBarDescr(IDM_WCALC, 0, 1, -1, 1, 0, 1)
/// \endcode
/// In this example, the child's control group contributes nothing from the
/// container group, and the parent's container group is removed. This produces a
/// merged bar (with the View control bar selection eliminated as a result of the
/// -1) that looks like this:
/// \image html bm82.BMP
/// 
/// If you want to merge the following parent gadget groups
/// \image html bm79.BMP
/// with these paint window gadget groups,
/// \image html bm83.BMP
/// pass the following group counts in the constructor:
/// \code
/// TBarDescr(IDM_WPAINT, 0, 1, 0, 2, 0, 1)
/// \endcode
/// This produces the following merged control bar:
/// \image html bm84.BMP
/// 
/// The simplest way to add groups to a control bar involves defining the control
/// groups and adding separators in a resource file. Insert the term BUTTON -1
/// between each gadgets group and an additional separator if one of the gadgets
/// groups is not present. For example, the resource file for Step 13 of the
/// ObjectWindows tutorial defines the following gadgets groups and separators:
/// \code
/// IDB_DVMDI TOOLBAR 20, 20
/// { 
///  BUTTON CM_FILENEW
///  BUTTON CM_FILEOPEN
///  BUTTON CM_FILESAVE
///  BUTTON CM_FILESAVEAS
///  SEPARATOR
///  BUTTON -1
///  BUTTON -1
///  BUTTON CM_ABOUT
///  BUTTON -1
/// }
/// IDB_DVMDI BITMAP "dvmain.bmp"
/// \endcode
/// You can see the separators by loading Step13.rc into Resource Workshop.
/// 
/// Step13.cpp uses these commands from the resource file to set the main window and
/// its menu, passing IDM_MDICMNDS as the parameter to SetMenuDescr function, as
/// follows:
/// \code
/// frame->SetBarDescr(new TBarDescr(IDB_DVMDI));
/// \endcode
/// It produces the following menu groups:
/// \image html bm85.BMP
/// 
/// TBarDescr's functions let you perform control bar merging similar to menu
/// merging. That is, you can merge control bars from a MDI frame window,  with
/// those of an the MDI child window. When child is selected, the control bars of
/// the child window merges with that of the frame window.





/// Default constructor for a TBarDescr object. No menu resources or groups are
/// specified. Constructs an empty control bar. For internal use only.
TBarDescr::TBarDescr()
:
  BarRes(0),
  CelArray(0),
  Id(0),
  Module(0)
{
  GadgetFunctor = new TGadgetFunctor;
  SetBuilder(TGadget_FUNCTOR(CreateGadget));
  Gadgets = new TBarDescrGdArray(5);
  for (int i = 0; i < NumGroups; i++)
    GroupCount[i] = 0;

}
// -----------------------------------------------------------------------------
/// Not implemented. Forbidden and protected contructor.
TBarDescr::TBarDescr(const TBarDescr& bar)
:
  Id(bar.Id),
  Module(bar.Module)
{
  GadgetFunctor = new TGadgetFunctor;
  SetBuilder(TGadget_FUNCTOR(CreateGadget));

  Gadgets = new TBarDescrGdArray(5);
  BarRes = new TToolbarRes(*bar.BarRes);

  CHECK(BarRes->IsOK());
  { // map colors
    TBtnBitmap bitmap(BarRes->GetBitmap(), 
                      TBtnBitmap::DefaultFaceColor, 
                      NoAutoDelete);
  }
  CelArray = new TCelArray(&BarRes->GetBitmap(),
                           BarRes->GetCount(),
                           TSize(BarRes->GetWidth(), BarRes->GetHeight()), 
                           TPoint(0, 0), // offset
                           BarRes->GetBitmap().Height()/BarRes->GetHeight());
  ExtractGroups();
}
// -----------------------------------------------------------------------------
/// Creates a control bar descriptor from the controlbar resource specified in the
/// id parameter. Calls the function ExtractGroups to extract the group counts based
/// on the separator items in the control bar.
TBarDescr::TBarDescr(TResId barResId, TModule* module)
:
  Id(barResId),
  Module(module),
  BarRes(0),
  CelArray(0)
{
  GadgetFunctor = new TGadgetFunctor;
  SetBuilder(TGadget_FUNCTOR(CreateGadget));

  Gadgets = new TBarDescrGdArray(5);

  SetBitmap(barResId, module);

  ExtractGroups();
}
// -----------------------------------------------------------------------------
/// Constructs a control bar descriptor from the resource indicated by id. Places
/// the gadgets in groups according the values of the fg, eg, cg, of, wg, and hg
/// parameters. The fg, eg, cg, of, wg, and hg parameters represent the functional
/// groups identified by the TGroup enum. Calls the function ExtractGroups to
/// extract the group counts based on the separator items in the menu bar.
TBarDescr::TBarDescr(TResId id, int fg, int eg, int cg, int og, int wg, 
                     int hg, TModule* module)
:
  Id(id),
  Module(module),
  BarRes(0),
  CelArray(0)
{
  GadgetFunctor = new TGadgetFunctor;
  SetBuilder(TGadget_FUNCTOR(CreateGadget));

  Gadgets = new TBarDescrGdArray(5);
  
  SetBitmap(id, module);
  
  if (!ExtractGroups()) {
    GroupCount[FileGroup]       = fg;
    GroupCount[EditGroup]       = eg;
    GroupCount[ContainerGroup]   = cg;
    GroupCount[ObjectGroup]     = og;
    GroupCount[WindowGroup]     = wg;
    GroupCount[HelpGroup]       = hg;
  }
}
// -----------------------------------------------------------------------------
/// Destroys the TBarDescr object.
TBarDescr::~TBarDescr()
{
  delete Gadgets;
  delete BarRes;
  delete CelArray;
  delete GadgetFunctor;
}
//
void TBarDescr::SetBuilder(const TGadgetFunctor& functor)
{
  *GadgetFunctor = functor;
}
// -----------------------------------------------------------------------------
/// Set new bitmap for use in gadgets.
bool
TBarDescr::SetBitmap(const TResId newResId, TModule* module)
{
  delete BarRes;
  BarRes = 0;
  Module = module;

  // Load toolbar resource
  // NOTE: Don't let TToolbarRes own bitmap, we'll give it to TCelArray
  //       instead.
  BarRes = new TToolbarRes(module ? module->GetHandle() : 0, 
                           newResId, 
                           NoAutoDelete);

  if (BarRes->IsOK()){
    delete CelArray;
    CelArray = 0;
    { // map colors
      TBtnBitmap bitmap(BarRes->GetBitmap(), 
                        TBtnBitmap::DefaultFaceColor, 
                        NoAutoDelete);
    }
    CelArray = new TCelArray(&BarRes->GetBitmap(), BarRes->GetCount(), 
                             TSize(BarRes->GetWidth(), BarRes->GetHeight()), 
                             TPoint(0, 0), // offset
                             BarRes->GetBitmap().Height()/BarRes->GetHeight());
    return true;
  }
  return false;
}
//
static void CreateGadgets(TBarDescrGdArray* array, const TGadgetFunctor& builder, 
                          TToolbarRes* barRes)
{
  for (int i=0,j=0; i < barRes->GetCount(); i++) {
    if(barRes->GetIds()[i] == uint16(-1))
       continue;

    bool usecell;
    TGadget* gd = (builder)(j, barRes->GetIds()[i], usecell);
    array->Add(new __TBarDescrGdNode(gd,usecell));
    
    if(usecell)
      j++;
  }
}
// -----------------------------------------------------------------------------
/// Inserts into destWindow's gadgets.
bool
TBarDescr::Create(TGadgetWindow& destWindow)
{
  // Build toolbar from resource and from descriptor string
  if (BarRes && BarRes->IsOK()) {
    if(!Gadgets->Size())
      CreateGadgets(Gadgets, *GetBuilder(), BarRes);

    int numRows = BarRes->GetBitmap().Height()/BarRes->GetHeight();
    TCelArray* cellArray = new TCelArray(TSize(BarRes->GetWidth(), BarRes->GetHeight()), 
                                         0, BarRes->GetCount(), 5, numRows);
    cellArray->RemoveAll();
    destWindow.SetCelArray(cellArray);
    for (int i=0,j=0; i < (int)Gadgets->Size(); i++){
      destWindow.Insert(*(*Gadgets)[i]->Gadget);
      if((*Gadgets)[i]->UseGlyph)
        cellArray->Add(*CelArray, j++);
    }
    return true;
  }
  return false;
}
// -----------------------------------------------------------------------------
/// Merges this gadgets and sourceBarDescr gadgets and inserts them into
/// destWindow's gadgets.
bool
TBarDescr::Merge(const TBarDescr& srcBarDescr, TGadgetWindow& destWindow)
{
  if (BarRes && BarRes->IsOK()){

    RemoveGadgets(destWindow);

    if(!srcBarDescr.Gadgets->Size())
      CreateGadgets(srcBarDescr.Gadgets, *((TBarDescr&)srcBarDescr).GetBuilder(), 
                    srcBarDescr.BarRes);

    TCelArray& cellArray = destWindow.GetCelArray();

    cellArray.RemoveAll();

    int gdIndex1 = 0;
    int gdIndex2 = 0;
    int cellIndex = 0;
    int cellIndex1 = 0;
    int cellIndex2 = 0;

    for (int g = 0; g < NumGroups; g++){
      if (srcBarDescr.GroupCount[g] > 0){
         for (int i=0; i < srcBarDescr.GroupCount[g]; i++){
          __TBarDescrGdNode* node = (*srcBarDescr.Gadgets)[gdIndex1];
          destWindow.Insert(*node->Gadget);
          if(node->UseGlyph){
            cellArray.Add(*srcBarDescr.CelArray, cellIndex1);
            TButtonGadget* bg = TYPESAFE_DOWNCAST(node->Gadget, TButtonGadget);
            if(bg)
              bg->SetGlyphIndex(cellIndex);
            cellIndex++;
            cellIndex1++;
          }
          gdIndex1++;
        }
      }
      else if (srcBarDescr.GroupCount[g] == 0 && GroupCount[g] > 0) {
         for (int i=0; i < GroupCount[g]; i++){
          __TBarDescrGdNode* node = (*Gadgets)[gdIndex2];
          destWindow.Insert(*node->Gadget);
          if(node->UseGlyph){
            cellArray.Add(*CelArray, cellIndex2);
            TButtonGadget* bg = TYPESAFE_DOWNCAST(node->Gadget, TButtonGadget);
            if(bg)
              bg->SetGlyphIndex(cellIndex);
            cellIndex++;
            cellIndex2++;
          }
          gdIndex2++;
        }
      }
    }
    destWindow.Invalidate();
    destWindow.LayoutSession();
    destWindow.UpdateWindow();
    return true;
  }
  return false;
}
// -----------------------------------------------------------------------------
/// Removes the gadgets in destWindow and then inserts these gadgets.
bool
TBarDescr::Restore(TGadgetWindow& destWindow)
{
  if (BarRes && BarRes->IsOK()){

    RemoveGadgets(destWindow);

    TCelArray& cellArray = destWindow.GetCelArray();
    cellArray.RemoveAll();

    int gdIndex = 0;
    int cellIndex = 0;
    for (int g=0; g < NumGroups; g++){
      if(GroupCount[g]!= 0){
         for (int i=0; i < GroupCount[g]; i++){
          __TBarDescrGdNode* node = (*Gadgets)[gdIndex];
          destWindow.Insert(*node->Gadget);
          if(node->UseGlyph){
            cellArray.Add(*CelArray, cellIndex);
            TButtonGadget* bg = TYPESAFE_DOWNCAST(node->Gadget, TButtonGadget);
            if(bg)
              bg->SetGlyphIndex(cellIndex);
            cellIndex++;
          }
          gdIndex++;
        }
      }
    }
    destWindow.Invalidate();
    destWindow.LayoutSession();
    destWindow.UpdateWindow();
    return true;
  }
  return false;
}
// -----------------------------------------------------------------------------
/// Removes all gadgets from destWindow.
bool
TBarDescr::RemoveGadgets(TGadgetWindow& destWindow)
{
  TGadget* gadget = destWindow.FirstGadget();
  if(gadget && gadget->GetId() == IDG_FLATHANDLE)
    gadget = gadget->NextGadget();
  while (gadget) {
    TGadget*  tmp = gadget;
    gadget = gadget->NextGadget();
    destWindow.Remove(*tmp);
  }
  return true;
}
//
/// Scan bar looking for separators that signify group divisions
/// return whether we found any at all
//
/// Extracts the group counts from the loaded menu bar by counting the number of
/// menus between separator items. After the group counts are extracted, the
/// separators are removed. Returns true if separators (groups) were found; false
/// otherwise.
bool
TBarDescr::ExtractGroups()
{
  if (!BarRes)
    return false;  // no bar to extract from...

  // walk BarRes & remove separators, setting up count as we go.
  //
  int itemCount = BarRes->GetCount();
  int g = 0;
  int count = 0;
  for (int i = 0; i < itemCount;i++) {
    if (BarRes->GetIds()[i] == uint16(-1)){
      if (g < NumGroups)
        GroupCount[g++] = count;
      count = 0;
    }
    else
      count++;
  }
  // Leave if no separators were found
  //
  bool retval = true;
  if (!g)
    retval = false;

  // Get trailing group
  //
  if (g < NumGroups)
    GroupCount[g++] = count;

  // Finish zeroing groups
  //
  for (; g < NumGroups; g++)
    GroupCount[g] = 0;
  return retval;
}

//
//
//
#if !defined(BI_NO_OBJ_STREAMING)
//
/// Extract the menu descriptor from the persistent stream.
//
_OWLCFUNC(ipstream&)
operator >>(ipstream& is, TBarDescr& b)
{
  is >> b.Id;
  is >> b.Module;
  b.BarRes = new TToolbarRes((b.Module ? b.Module->GetHandle() : 0), b.Id, NoAutoDelete);
  { // map colors
    TBtnBitmap bitmap(b.BarRes->GetBitmap(), TBtnBitmap::DefaultFaceColor, NoAutoDelete);
  }
  int numRows = b.BarRes->GetBitmap().Height()/b.BarRes->GetHeight();
  b.CelArray = new TCelArray(&b.BarRes->GetBitmap(),b.BarRes->GetCount(),
                           TSize(b.BarRes->GetWidth(), b.BarRes->GetHeight()),
                           TPoint(0, 0), // offset
                           numRows);

  for (int i = 0; i < TBarDescr::NumGroups; i++)
    is >> b.GroupCount[i];

  return is;
}

//
/// Insert the menu descriptor into the persistent stream.
//
_OWLCFUNC(opstream&)
operator <<(opstream& os, const TBarDescr& b)
{
  os << b.Id;
  os << b.Module;
  for (int i = 0; i < TBarDescr::NumGroups; i++)
    os << b.GroupCount[i];
  return os;
}

#endif  // #if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace

/* ========================================================================== */
