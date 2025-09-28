//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Declares an IBClassMgr "factory" which can create Bolero helper objects.
//
//    Since callers of BOleClassManager don't have the class
//    definition, they can't call operator new to create a
//    helper object.
//
//    BOleClassMgr has a pointer to the service, so you need a
//    BOleClassMgr for every Bolero connection you maintain.
//----------------------------------------------------------------------------
#ifndef BOLECMAN_H
#define BOLECMAN_H

#ifndef _BOLECOMP_H
#include "bolecomp.h"
#endif

#ifndef _BOLESVC_H
#include "bolesvc.h"
#endif

class _ICLASS BOleFontObj;

class BOleClassManager : public BOleComponent, public IBClassMgr
{

private:
  BOleService *pSvc;
  BOleFontObj *pFont;  // font cache storage
  int nServerCount;

public:

  // IUnknown methods
  //
  DEFINE_IUNKNOWN(pObjOuter);
  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IClassMgr methods
  //
  virtual HRESULT _IFUNC ComponentCreate(
    PIUnknown FAR* ppRet,
    PIUnknown pAggregator,
    BCID    idClass
    );
  virtual HRESULT _IFUNC ComponentInfoGet(
    PIUnknown FAR* ppInfo,
    PIUnknown pAggregator,
    BCID    idClass
    )
  { *ppInfo = NULLP; return ResultFromScode(E_NOTIMPL); }

  int _IFUNC ServerCount (int nDelta = 0);
  BOleService * GetService()  {return pSvc;};
  BOleFontObj * GetFontCache()  {return pFont;};
  void          SetFontCache(BOleFontObj *pFont);

  BOleClassManager(IBUnknownMain *pObj);
  ~BOleClassManager();
};

#endif
