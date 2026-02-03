//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// This file declares TMessageHook.  This class is used to encapsulate a
// WH_GETMESSAGE Windows Hook.
//
// Original code by Don Griffin; used with permission.
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_MSGHOOK_H)
#define __OWLEXT_MSGHOOK_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
# include <owlext/core.h>
#endif


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

class OWLEXTCLASS TMessageHook{
    protected:
        TMessageHook ();
        ~TMessageHook ();

        void    ActivateMsgHook (bool = true);
        bool    IsMsgHookActive () const { return mActive; }

        //  Call the base class version if you want to let other objects see
        //  the event (and eventually call CallNextHookEx).
        virtual void    OnGetMsg (MSG &);   // PM_REMOVE
        virtual void    OnPeekMsg (MSG &);  // PM_NOREMOVE

    private:
        TMessageHook  * mNext;
        bool            mActive;

        static LRESULT CALLBACK     GetMsgProc (int, WPARAM, LPARAM);
        static owl::uint                 ActiveCount ();
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif  // __OWLEXT_MSGHOOK_H

