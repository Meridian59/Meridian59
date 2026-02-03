//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// This header declares TTimer, a class that enapsulates SetTimer API.
// To use this class, derive from it and implement Tick().  Tick() will
// be called approximatley every "duration" milliseconds.  As with any
// Windows based timer, there is no precision guaranteed and only limited
// processing should be performed on each tick.
//
// Original code by Don Griffin; used with permission.
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_TIMER_H)
#define __OWLEXT_TIMER_H

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


class OWLEXTCLASS TTimer{
    public:
        static void     DoTicks ();

        bool            IsTimer () { return IsTimer (this); }
        static bool     IsTimer (TTimer *timer);

    protected:
        explicit TTimer (owl::uint duration);
        virtual ~TTimer ();

        virtual void Tick () = 0;

    private:
        static TTimer         * sFirst;
        TTimer                * mNext;
        const owl::uint              mDuration;
        owl::uint32                  mLastTick;
        bool                    mProcessing;

        void        DoTick (owl::uint32 tickCount);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif  // __OWLEXT_TTIMER_H

