//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Provides a default IUnknownMain implementation for all Bolero components
//  (helper objects and some private objects.
//
//    IUnknownMain is a way to create aggregatable components which support
//    IUnknown-based interfaces via multiple-inheritance.
//----------------------------------------------------------------------------
#ifndef _BOLECOMP_H
#define _BOLECOMP_H

#ifndef _BOLE_H
#include "bole.h"
#endif

#ifndef _OLEDEBUG_H
#include "oledebug.h"
#endif

class _ICLASS BOleClassManager;

PREDECLARE_INTERFACE( BOleComponent );

class _ICLASS BOleComponent : public IBUnknownMain
#ifdef OLEDBG
, public cDebug
#endif
{
public:
  BOleClassManager * pFactory;

private:
  unsigned long nReferences;

protected:
  IBUnknownMain * pObjOuter;
  BOleComponent(BOleClassManager *pF, PIBUnknownMain pObj);
  virtual ~BOleComponent();

public:

  // IUnknown methods delegate aggregator's IUnknownMain
  //
  DEFINE_IUNKNOWN(pObjOuter);

  // IUnknownMain methods
  //
  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);
  virtual ULONG _IFUNC AddRefMain();
  virtual ULONG _IFUNC ReleaseMain();
};

#endif
