//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// Credits:
//      The Property pattern is taken from "Patterns in Practice: A Property
// Template for C++", from "C++ Report", Nov/Dec 1995, p. 28-33, by Colin
// Hastie. I've in turn extended it to allow for "true" Delphi-like property
// syntax by allowing you to assign normal T objects to the TProperty<T>, as well
// as provide an implict conversion-to-T operator. Note that this implicit
// conversion is NOT highly held in regard by many C++-ers, and therefore can
// be #defined out of existence by #define-ing STRICT_PROPERTY_SYNTAX before
// #include-ing this file.
//      Delphi (and C++Builder) also allow for methods to be used instead of
// direct access to data members; this, too, is allowed, by making use of the
// callback mechanism within VDBT/BDTF.H. TProperty<T> can be constructed to
// take two callbacks, one returning T taking void (get), and the other returning
// void and taking a const T& (set). These methods will be called in place of
// accessing the m_content method of Property<>, below. Note that I'm assuming
// that if you write the 'get', you'll also write the 'set', and that both methods
// provide their own storage for T (instead of relying on m_content within the
// TProperty<T>).
//
// Requirements; TProperty assumes that the type T has meaningful orthodox
// canonical form semantics (default ctor, copy ctor, assignment op, dtor)
//
//-------------------------------------------------------------------
#ifndef __OWLEXT_PROPERTY_H
#define __OWLEXT_PROPERTY_H

#pragma message ("<owlext/property.h> has been replaced by <owl/property.h>")
#include <owl/property.h>

#endif
