//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
///  Class definition for TBarDescr.
//
//----------------------------------------------------------------------------

#if !defined(OWL_BARDESCR_H)  // Sentry, use file only if it's not already included.
#define OWL_BARDESCR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/resource.h>

namespace owl {

class TResId;
class _OWLCLASS TGadget;
class _OWLCLASS TGadgetWindow;
class _OWLCLASS TCelArray;

template <class R, class T1, class T2, class T3>  class TFunctor3;
// TGadgetFunctor is a ptr to a fn taking 4 parameters, and returning TGadget*
typedef TFunctor3<TGadget*,int,int, bool&> TGadgetFunctor;

#define TGadget_FUNCTOR(func) TFunctionTranslator3<TGadget*,int,int, bool&, TGadget* (*)(int,int, bool&)>(func)
#define TGadget_MFUNCTOR(obj,memberFunc) TMemberTranslator3<TGadget*,int,int, bool&, TGadget*(obj::*)(int,int, bool&)>(obj, memberFunc);

//
// static TGadget* MyHandle(int cellIdx,int cmdId,bool& usecell)
//{
//  usecell == false;
//  if(cmdId==0)
//     return new TSeparatorGadget(6);
//  usecell == true;
//  return new TButtonGadget(index, id, TButtonGadget::Command,false,TButtonGadget::Up,true);
//}
// Funct = TGadget_FUNCTOR(MyHandle)  - function
//
// or:
//
// TGadget* TMyWindow::MyHandle(int cellIdx,int cmdId,bool& usecell)
//{
//  usecell == false;
//  if(cmdId==0)
//     return new TSeparatorGadget(6);
//  usecell == true;
//  return new TButtonGadget(index, id, TButtonGadget::Command,false,TButtonGadget::Up,true);
//}
// Funct = TGadget_MFUNCTOR(myWndPtr,&TMyWindow::MyHandle)  - member function
// Funct = TGadget_MFUNCTOR(this,&TMyWindow::MyHandle)      - member function
//


// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class TBarDescrGdArray;

/// \addtogroup gadgets
/// @{
//
/// \class TBarDescr
/// Descriptor of Bar Implementation 
//
class _OWLCLASS TBarDescr 
{
  public:
  	
/// Used by TBarDescr, the TGroup enum describes the following constants that define
/// the index of the entry in the GroupCount array.
    enum TGroup 
    {
      FileGroup,				///< Index of the File gadget group count
      EditGroup,				///< Index of the Edit gadget group count
      ContainerGroup,		///< Index of the Container gadget group count
      ObjectGroup,			///< Index of the Object gadget group count
      WindowGroup,			///< Index of the Window gadget group count
      HelpGroup,				///< Index of the Help gadget group count
      NumGroups					///< Total number of groups
    };
  public:
    TBarDescr(); // for internal use
    TBarDescr(TResId id, int fg, int eg, int cg, int og, int wg, int hg,
               TModule* module = &GetGlobalModule());
    TBarDescr(TResId id, TModule* module = &GetGlobalModule());
    virtual ~TBarDescr();

    TBarDescr& operator =(const TBarDescr& original);

    bool      SetBitmap(const TResId newResId, TModule* module);
    TResId  GetId() const;
    int     GetGroupCount(int group) const;
    bool    RemoveGadgets(TGadgetWindow& destWindow);
    void    SetBuilder(const TGadgetFunctor& functor);
    TGadgetFunctor* GetBuilder();

    virtual bool  Create(TGadgetWindow& destWindow);
    virtual bool  Merge(const TBarDescr& sourceBarDescr, TGadgetWindow& destWindow);
    virtual bool  Restore(TGadgetWindow& destWindow);

    // please use: SetBuilder(TGadgetFunctor& functor);
    // properties
#if 0  // deprecated
    class TGadgetBuilder{
      private:
        TBarDescr* GetParent();
      public:
        TGadgetBuilder& operator=(const TGadgetFunctor& funct)
        {
          GetParent()->SetBuilder(funct);
          return *this;
        }
    }GadgetBuilder;
#endif

  protected:
    TBarDescr(const TBarDescr& node);
    bool    ExtractGroups();

  protected_data:
    int                   GroupCount[NumGroups];  ///< An array of values indicating the number of pop-up menus in each group on the
                                                  ///< menu bar.
    TToolbarRes*          BarRes;				///< Points to the TToolbarRes object that owns this TBarDescr.
    TCelArray*            CelArray;			///< Points to the TCelArray object that owns this TBarDescr.
    TResId                Id;           ///< The resource ID for the bar. The resource ID is passed in the constructors to
                                        ///< identify the tool bar resource.
    TModule*              Module;       ///< Points to the TModule object that owns this TBarDescr.
    TGadgetFunctor*        GadgetFunctor;

  private:
    TBarDescrGdArray* Gadgets; // internal usage

  friend _OWLCFUNC(ipstream&) operator >>(ipstream& is, TBarDescr& m);
  friend _OWLCFUNC(opstream&) operator <<(opstream& os, const TBarDescr& m);
};
/// @}


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// inlines
//

/// Gets the control bar resource ID used to construct the bar descriptor.
inline TResId TBarDescr::GetId() const {
  return Id;
}

//
/// Gets the number of gadgets in a specified group within the control bar. There
/// are a maximum of six functional groups as defined by the TGroup enum. These
/// groups include FileGroup,  EditGroup, ContainerGroup, ObjectGroup, WindowGroup,
/// and HelpGroup.
inline int TBarDescr::GetGroupCount(int group) const {
  return GroupCount[group];
}
//
//
//
inline TGadgetFunctor* TBarDescr::GetBuilder(){
  return GadgetFunctor;
}
//
//

} // OWL namespace


#endif // __bardescr_h sentry.


