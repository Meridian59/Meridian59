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

#include <owl/framewin.h>
#include <owl/mdichild.h>
#include <owl/mdi.rh>
#include <memory>

namespace owl {

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
    auto PreProcessMsg(MSG&) -> bool override;
    auto Create() -> bool override;

    /// \name Child factory functions
    /// @{
    virtual TMDIChild*  CreateChild();
    virtual TMDIChild*  InitChild();
    /// @}

    LPCLIENTCREATESTRUCT  GetClientAttr();
    void   SetClientAttr(LPCLIENTCREATESTRUCT clientattr);

  protected:
    auto GetWindowClassName() -> TWindowClassName override;

    // Menu command handlers & enabler
    //
    void                CmCreateChild();        ///< CM_CREATECHILD
    void                CmTileChildren();       ///< CM_TILECHILDREN
    void                CmTileChildrenHoriz();  ///< CM_TILECHILDREN
    void                CmCascadeChildren();    ///< CM_CASCADECHILDREN
    void                CmArrangeIcons();       ///< CM_ARRANGEICONS
    void                CmCloseChildren();      ///< CM_CLOSECHILDREN
    void                CmChildActionEnable(TCommandEnabler& commandEnabler);

    HWND                EvMDICreate(MDICREATESTRUCT& createStruct);
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
  DECLARE_STREAMABLE_OWL(TMDIClient, 1);

  friend class TMDIFrame;  // To allow access to private ctor
};

DECLARE_STREAMABLE_INLINES(owl::TMDIClient);

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

    TMDIFrame(
      LPCTSTR title,
      TResId menuResId,
      std::unique_ptr<TMDIClient> clientWnd = nullptr,
      TModule* = nullptr);

    TMDIFrame(
      const tstring& title,
      TResId menuResId,
      std::unique_ptr<TMDIClient> clientWnd = nullptr,
      TModule* = nullptr);

#if defined(OWL5_COMPAT)

    TMDIFrame(LPCTSTR       title,
              TResId        menuResId,
              TMDIClient&   clientWnd,
              TModule*      module = 0);

    TMDIFrame(
      const tstring& title,
      TResId menuResId,
      TMDIClient& clientWnd,
      TModule* = 0);

#endif

    TMDIFrame(HWND frameHandle, HWND clientHandle, TModule* module = 0);

    // Override virtual functions defined by TFrameWindow or TWindow
    //
    auto SetMenu(HMENU) -> bool override;
    auto GetClientWindow() -> TMDIClient* override;
    auto GetCommandTarget() -> HWND override;

    /// Find & return the child menu of an MDI frame's (or anyone's) menu bar.
    //
    static HMENU FindChildMenu(HMENU);

  protected:
    virtual TResult DefWindowProc(TMsgId, TParam1, TParam2);
    auto PerformCreate() -> THandle override;

  private:
    /// Hidden to prevent accidental copying or assignment
    //
    TMDIFrame(const TMDIFrame&);
    TMDIFrame& operator=(const TMDIFrame&);

  DECLARE_RESPONSE_TABLE(TMDIFrame);
  DECLARE_STREAMABLE_OWL(TMDIFrame, 1);
};

DECLARE_STREAMABLE_INLINES(owl::TMDIFrame);

typedef TMDIFrame TMdiFrame;

/// @}

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
