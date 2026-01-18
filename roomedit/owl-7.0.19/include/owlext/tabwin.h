//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TTabWindow
// TWin95TabWindow
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_TABWIN_H)
#define __OWLEXT_TABWIN_H

#ifndef __OWLEXT_CORE_H
#  include <owlext/core.h>              // required for all OWLEXT headers
#endif


#include <owl/template.h>

#ifndef OWL_TABCTRL_H
# include <owl/tabctrl.h>
#endif


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                    TTabWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TTabWindow : public owl::TWindow {
    // Object lifetime methods
    //
public:
    TTabWindow(owl::TWindow* parent, owl::TWindow* array[] = 0);
    TTabWindow(owl::TWindow* parent, owl::uint32 X, owl::uint32 Y, owl::uint32 W, owl::uint32 H, owl::TWindow* array[] = 0);
private:
    TTabWindow();                                 // DISALLOWED METHOD
    TTabWindow(const TTabWindow&);                // DISALLOWED METHOD
    TTabWindow& operator=(const TTabWindow& src); // DISALLOWED METHOD

    // Tabbed-window interface: adding windows, removing windows
    //
public:
    virtual void Attach(owl::TWindow* ptr, LPCTSTR title = 0);
    virtual void Attach(owl::TWindow* windowArray[]);
    virtual owl::TWindow* Detach(owl::int32 index) = 0;
    virtual owl::TWindow* Detach(owl::TWindow* ptr)
        { return Detach(Retrieve(ptr)); }
    virtual owl::TWindow* Retrieve(owl::int32 index)
        { return tabbedWindowArray[index]; }
    virtual owl::int32 Retrieve(owl::TWindow* ptr);
    virtual owl::TWindow* operator[](owl::int32 index)
        { return tabbedWindowArray[index]; }
    virtual owl::int32 operator[](owl::TWindow* win)
        { return Retrieve(win); }

    virtual owl::int32 GetSelectedTab() = 0;
    virtual void SetSelectedTab(owl::int32 index) = 0;
    virtual owl::int32 GetWindowCount()
        { return tabbedWindowArray.GetItemsInContainer(); }

protected:
    enum { ID_TABCTRL = 100 };
    virtual owl::TWindow* GetTabCtrl() = 0;

    // OWL methods
    //
protected:
    DECLARE_RESPONSE_TABLE(TTabWindow);

    // Internal Data
    //
protected:      // so deriveds can see it more easily
    owl::TPtrArray<owl::TWindow*> tabbedWindowArray;
};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                               TWin95TabWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TWin95TabWindow : public TTabWindow {
    // Ctors & Dtors
    //
public:
    TWin95TabWindow(owl::TWindow* parent, owl::TWindow* array[] = 0);
    TWin95TabWindow(owl::TWindow* parent, owl::uint32 X, owl::uint32 Y, owl::uint32 W, owl::uint32 H, owl::TWindow* array[] = 0);
    virtual ~TWin95TabWindow();

    // OWL Methods
    //
protected:
    void SetupWindow() override;
    void EvSize(owl::uint sizeType, const owl::TSize& size);
    void TabSelChange(owl::TNotify &);
    bool TabSelChanging(owl::TNotify &);
    void TabKeyDown(owl::TTabKeyDown &);

    // Tabbed-window interface: adding windows, removing windows
    //
public:
    void Attach(owl::TWindow*, LPCTSTR title = nullptr) override;
    void Attach(owl::TWindow* windowArray[]) override
      { TTabWindow::Attach(windowArray); }
    auto Detach(owl::int32 index) -> owl::TWindow* override;
    auto Detach(owl::TWindow* ptr) -> owl::TWindow* override
      { return TTabWindow::Detach(ptr); }
    auto GetSelectedTab() -> owl::int32 override
      { return m_tabCtrl->GetSel(); }
    void SetSelectedTab(owl::int32 index) override;
    auto GetWindowCount() -> owl::int32 override;

protected:

    auto GetTabCtrl() -> owl::TTabControl* override
      { return m_tabCtrl; }

    virtual void AdjustTabClient();

    // Internal data
    //
private:
    owl::TTabControl* m_tabCtrl;

  DECLARE_RESPONSE_TABLE(TWin95TabWindow);
};


/*
 * By arranging the code this way, we allow for other tabbed-window schemes to
 * customize the actual window-arrangement; for example, the class declaration
 * below would allow us to use the TNoteTab control instead of the TTabControl
 * control to manage the window-flipping.
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                TNoteTabWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
#include <owl/notetab.h>

class OWLEXTCLASS TNoteTabWindow : public TTabWindow
{
    // Ctors & Dtors
    //
public:
    TNoteTabWindow(TWindow* parent, TWindow* array[] = 0);
    TNoteTabWindow(TWindow* parent, owl::uint32 X, owl::uint32 Y, owl::uint32 W, owl::uint32 H, TWindow* array[] = 0);
    virtual ~TNoteTabWindow();

    // OWL Methods
    //
protected:
    virtual void SetupWindow();
    DECLARE_RESPONSE_TABLE(TNoteTabWindow);
    void EvSize(owl::uint sizeType, TSize& size);
    void TabSelChange(TNotify &);
    bool TabSelChanging(TNotify &);
    void TabKeyDown(TTabKeyDown &);

    // Tabbed-window interface: adding windows, removing windows
    //
public:
    virtual void Attach(TWindow* ptr, const char* title = 0);
    virtual void Attach(TWindow* windowArray[])
    { TTabWindow::Attach(windowArray); }
    virtual TWindow* Detach(owl::int32 index);
    virtual TWindow* Detach(TWindow* ptr)
    { return TTabWindow::Detach(ptr); }
    virtual owl::int32 GetSelectedTab()
    { return m_tabCtrl->GetSel(); }
    virtual void SetSelectedTab(owl::int32 index)
    { m_tabCtrl->SetSel(index); }
    virtual owl::int32 GetWindowCount()
    {
        if (m_tabCtrl->GetCount() != tabbedWindowArray.GetItemsInContainer())
            TRACE("This is a problem--tabCtrl->GetCount() != tabbedWindowArray");
        return TTabWindow::GetWindowCount();
    }
protected:
    virtual TNoteTab* GetTabCtrl()
    { return m_tabCtrl; }
    virtual void AdjustTabClient();

private:
    TNoteTab* m_tabCtrl;
};
*/

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif
