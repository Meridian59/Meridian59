//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// A "busy" cursor.
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_BUSY_H)
#define __OWLEXT_BUSY_H


// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                   TBusyCursor
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TBusyHook;

class OWLEXTCLASS TBusyCursor {
  // Friendship declarations
  //
  public:
    friend class TBusyHook;

  // Object lifetime methods
  //
  public:
    explicit TBusyCursor(owl::tstring message);
    TBusyCursor();
    ~TBusyCursor();

  // Accessors
  //
  public:
    bool Active() const;
    owl::tstring Message() const;
    static bool GloballyActive ();

  // Mutators
  //
  public:
    void Active(bool active);
    void Message(owl::tstring message);
  protected:
    virtual void UpdateMessage (LPCTSTR);
    // Override this to change the text other than in the app's MainWindow's
    // status bar:
    void UpdateMessage(const owl::tstring& s);

  // Internal methods
  //
  private:
    void Init();
    void Activate();
    void Deactivate();

  // Internal data
  //
  private:
    static TBusyCursor* sTop;
    TBusyCursor* mNext;
    HCURSOR mBusyCursor;
    owl::tstring mMessage;
    bool mActive;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations

inline bool TBusyCursor::Active() const
{ return mActive; }

inline owl::tstring TBusyCursor::Message() const
{ return mMessage; }

inline bool TBusyCursor::GloballyActive ()
{ return sTop ? sTop->Active() : false; }

inline void TBusyCursor::UpdateMessage(const owl::tstring& s)
{ UpdateMessage (s.length() > 0 ? s.c_str() : 0); }

} // OwlExt namespace

#endif  // __OWLEXT_BUSY_H

