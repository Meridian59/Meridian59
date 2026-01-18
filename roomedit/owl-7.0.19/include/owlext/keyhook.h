//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// This file defines the abstract TKeyHook class.  The intended usage of
// TKeyHook is to derive from it and provide an implementation of the
// OnKeystroke() method.
//
// Original code by Don Griffin; used with permission.
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_KEYHOOK_H)
#define __OWLEXT_KEYHOOK_H

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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                      TKeyHook
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TKeyHook
{
    protected:
        TKeyHook ();
        ~TKeyHook ();

        void    ActivateKeyHook (bool = true);
        bool    IsKeyHookActive () const { return mActive; }

        //  Call the base class version if you want to let other objects see
        //  the event (and eventually call CallNextHookEx).  Return true to
        //  eat the keystroke, false to let it pass.
        virtual bool    OnKeystroke (owl::uint vkey, owl::uint32 flags);

    private:
        TKeyHook  * mNext;
        bool        mActive;
        static LRESULT CALLBACK     KeyboardProc (int, WPARAM, LPARAM);
        static owl::uint                 ActiveCount ();
};


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace


#endif  // __OWLEXT_KEYHOOK_H

