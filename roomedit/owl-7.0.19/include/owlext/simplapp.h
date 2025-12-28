//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software. All rights reserved.
//
// TSimpleApp
//-------------------------------------------------------------------
#ifndef __OWLEXT_SIMPLAPP_H
#define __OWLEXT_SIMPLAPP_H

#ifndef __OWLEXT_CORE_H
#  include <owlext/core.h>              // required for all OWLEXT headers
#endif


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                    TSimpleApp
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TSimpleApp : public owl::TApplication
{
    // Object lifetime methods
    //
public:
    TSimpleApp(owl::TFrameWindow* fwndw, LPCTSTR name = 0);

    // OWL mutators
    //
protected:
    virtual void InitMainWindow();
    virtual bool IdleAction(long idleCount);

    // Internal data
    //
private:
    owl::TFrameWindow* m_window;
    static owl::TAppDictionary s_appDictionary;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace


#endif

