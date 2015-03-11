//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TMDIClient and TMDIFrame classes
//----------------------------------------------------------------------------

#if !defined(OWL_MDI_H)
#define OWL_MDI_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/framewin.h>
#include <owl/mdichild.h>
#include <owl/mdi.rh>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup frame
/// @{

class _OWLCLASS TMDIFrame;

//
/// \class TMDIClient
// ~~~~~ ~~~~~~~~~~
/// Multiple Document Interface (MDI) client windows (represented by a TMDIClient
/// object) manage the MDI child windows of a TMDIFrame parent. TMDIClient is a
/// streamable class.
class _OWLCLASS TMDIClient : public virtual TWindow {
  public:
    TMDIClient(TModule* module = 0);
   ~TMDIClient();

    virtual bool        CloseChildren();
    TMDIChild*          GetActiveMDIChild();

    // Member functions to arrange the MDI children
    //
    virtual void        ArrangeIcons();
    virtual void        CascadeChildren();
    virtual void        TileChildren(int tile = MDITILE_VERTICAL);

    // Override member functions defined by TWindow
    //
    bool                PreProcessMsg(MSG& msg);
    bool                Create();

    /// \name Child factory functions
    /// @{
    virtual TMDIChild*  CreateChild();
    virtual TMDIChild*  InitChild();
    /// @}

    LPCLIENTCREATESTRUCT  GetClientAttr();
    void   SetClientAttr(LPCLIENTCREATESTRUCT clientattr);

  protected:
    virtual TGetClassNameReturnType GetClassName();

    // Menu command handlers & enabler
    //
    void                CmCreateChild();        ///< CM_CREATECHILD
    void                CmTileChildren();       ///< CM_TILECHILDREN
    void                CmTileChildrenHoriz();  ///< CM_TILECHILDREN
    void                CmCascadeChildren();    ///< CM_CASCADECHILDREN
    void                CmArrangeIcons();       ///< CM_ARRANGEICONS
    void                CmCloseChildren();      ///< CM_CLOSECHILDREN
    void                CmChildActionEnable(TCommandEnabler& commandEnabler);

    TResult             EvMDICreate(MDICREATESTRUCT & createStruct);
    void                EvMDIDestroy(HWND hWnd);

    void                EvDropFiles(TDropInfo);

  public_data:
/// ClientAttr holds a pointer to a structure of the MDI client window's attributes.
    LPCLIENTCREATESTRUCT  ClientAttr;

  private:
    TMDIClient(HWND hWnd, TModule*   module = 0);

    // Hidden to prevent accidental copying or assignment
    //
    TMDIClient(const TMDIClient&);
    TMDIClient& operator =(const TMDIClient&);

  DECLARE_RESPONSE_TABLE(TMDIClient);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TMDIClient, 1);
  DECLARE_STREAMABLE_OWL(TMDIClient, 1);
  
  friend class TMDIFrame;  // To allow access to private ctor
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TMDIClient );

typedef TMDIClient TMdiClient;

//
/// \class TMDIFrame
// ~~~~~ ~~~~~~~~~
/// Multiple Document Interface (MDI) frame windows, represented by TMDIFrame, are
/// overlapped windows that serve as main windows of MDI-compliant applications.
/// TMDIFrame objects automatically handle creating and initializing an MDI client
/// window (represented by a TMDIClient object) required by Windows. TMDIFrame sets
/// window style WS_CLIPCHILDREN by default so that minimal flicker occurs when the
/// MDI frame erases its background and the backgrounds of its children. TMDIFrame
/// is a streamable class.
/// Because TMDIFrame is derived from TFrameWindow, it inherits keyboard navigation.
/// As a result, all children of the MDI frame acquire keyboard navigation. However,
/// it's best to enable keyboard navigation only for those children who require it.
/// 
/// To create an OLE-enabled MDI frame window, use TOleMDIFrame, which inherits
/// functionality from both TMDIFrame and TOleFrame.
//
class _OWLCLASS TMDIFrame : virtual public TFrameWindow {
  public:
    TMDIFrame(LPCTSTR       title,
              TResId        menuResId,
              TMDIClient&   clientWnd = *new TMDIClient,
              TModule*      module = 0);

    TMDIFrame(
      const tstring& title,
      TResId menuResId,
      TMDIClient& client = *new TMDIClient,
      TModule* = 0);

    TMDIFrame(THandle frameHandle, HWND clientHandle, TModule* module = 0);

    // Override virtual functions defined by TFrameWindow or TWindow
    //
    bool         SetMenu(HMENU);

    TMDIClient*  GetClientWindow();

    virtual HWND GetCommandTarget();

    /// Find & return the child menu of an MDI frame's (or anyone's) menu bar.
    //
    static HMENU FindChildMenu(HMENU);

  protected:
    virtual TResult DefWindowProc(TMsgId, TParam1, TParam2);
    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);

  private:
    /// Hidden to prevent accidental copying or assignment
    //
    TMDIFrame(const TMDIFrame&);
    TMDIFrame& operator=(const TMDIFrame&);

  DECLARE_RESPONSE_TABLE(TMDIFrame);
  //DECLARE_STREAMABLE(_OWLCLASS, TMDIFrame, 1);
  DECLARE_STREAMABLE_OWL(TMDIFrame, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TMDIFrame );

typedef TMDIFrame TMdiFrame;

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the client create struct for the MDI client.
//
inline LPCLIENTCREATESTRUCT TMDIClient::GetClientAttr() {
  return ClientAttr;
}

//
/// Sets the client create struct for the MDI client.
/// \todo Assumes ownership of the pointer, so it will be deleted ? And the old one is orphaned and leaks?
//
inline void TMDIClient::SetClientAttr(LPCLIENTCREATESTRUCT clientattr) {
  ClientAttr = clientattr;
}

//
/// Calls CreateChild to produce a new child window in response to a menu selection
/// with a menu ID of CM_CREATECHILD.
inline void TMDIClient::CmCreateChild() {
  CreateChild();
}

//
/// Calls TileChildren in response to a menu selection with an ID of
/// CM_TILECHILDREN.
inline void TMDIClient::CmTileChildren() {
  TileChildren();
}

//
/// Calls TileChildren in response to a menu selection with an ID of CM_TILECHILDREN
/// and passes MDI child tile flag as MDITILE_HORIZONTAL.
inline void TMDIClient::CmTileChildrenHoriz() {
  TileChildren(MDITILE_HORIZONTAL);
}

//
/// Calls CascadeChildren in response to a menu selection with an ID of
/// CM_CASCADECHILDREN.
inline void TMDIClient::CmCascadeChildren() {
  CascadeChildren();
}

//
/// Calls ArrangeIcons in response to a menu selection with an ID of
/// CM_ARRANGEICONS.
inline void TMDIClient::CmArrangeIcons() {
  ArrangeIcons();
}

//
/// Calls CloseChildren in response to a menu selection with an ID of
/// CM_CLOSECHILDREN.
inline void TMDIClient::CmCloseChildren() {
  CloseChildren();
}


} // OWL namespace


#endif  // OWL_MDI_H
