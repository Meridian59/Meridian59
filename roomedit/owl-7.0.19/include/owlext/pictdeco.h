//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TPictDecorator
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_PICTDECO_H)
#define __OWLEXT_PICTDECO_H

#ifndef __OWLEXT_CORE_H
#  include <owlext/core.h>              // required for all OWLEXT headers
#endif


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                TPictDecorator
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class OWLEXTCLASS TPictDecorator : public owl::TWindow
{
  // Object lifetime methods
  //
public:
  TPictDecorator(owl::TDib* dib, owl::TWindow& parent);
  virtual ~TPictDecorator();

  // OWL overrides
  //
protected:
  DECLARE_RESPONSE_TABLE(TPictDecorator);
  virtual bool EvEraseBkgnd(HDC hdc);
  virtual void EvSize(owl::uint sizeType, const owl::TSize& size);

  /*
  // This was how the code was originally written; however, in order to give you
  // a demonstration of how to use the TProperty<> mechanism in PROPERTY.H, I've
  // chosen to use the TProperty<> mechanism to control the m_pdib member.

  // Accessors
  //
public:
  owl::TDib* GetDib()
    { return m_pdib; }

  // Mutators
  //
public:
  owl::TDib* SetDib(owl::TDib* newDib);

  // Internal data
  //
private:
  owl::TDib* m_pdib;
  */

  // Properties
  //
public:
  owl::TProperty<owl::TDib*> pDib;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif