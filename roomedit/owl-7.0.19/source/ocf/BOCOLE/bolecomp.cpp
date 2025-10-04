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
#include "bolecomp.h"

ULONG _IFUNC BOleComponent::AddRefMain()
{
  return ++nReferences;
}

ULONG _IFUNC BOleComponent::ReleaseMain()
{
  return --nReferences ? nReferences : (delete this, 0);
}

HRESULT _IFUNC BOleComponent::QueryInterfaceMain(REFIID iid, LPVOID *ppv)
{
  *ppv = NULL;
  return  IBUnknownMain::QueryInterfaceMain(iid, ppv);
}

BOleComponent::BOleComponent(BOleClassManager *pF, IBUnknownMain *pO)
{
  pFactory = pF;
  if (pO)
    pObjOuter = pO;
  else
    pObjOuter = this;
  nReferences = 1;
}

BOleComponent::~BOleComponent()
{
}

