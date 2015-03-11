//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TBitSet, a set of up to 256 bit flags
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/bitset.h>
#include <owl/objstrm.h>

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a TBitSet object with all bits set to 0.
//
template <class T>
TBitSet<T>::TBitSet()
{
  for (unsigned int i = 0; i < sizeof(Bits); i++)
    Bits[i] = 0;
}

//
/// Constructs a TBitSet object as a copy of another TBitSet.
//
template <class T>
TBitSet<T>::TBitSet(const TBitSet& bs)
{
  for (unsigned int i = 0; i < sizeof(Bits); i++)
    Bits[i] = bs.Bits[i];
}

//
/// Return true if a bit is turned on.
//
template <class T>
bool
TBitSet<T>::Has(T item) const
{
  return (Bits[Loc(item)] & Mask(item)) != 0;
}

//
/// Negate a bit.
//
template <class T>
TBitSet<T>
TBitSet<T>::operator ~() const
{
  TBitSet temp;
  for (unsigned int i = 0; i < sizeof(Bits); i++)
    temp.Bits[i] = uint8(~Bits[i]);
  return temp;
}

//
/// Turn off a specific bit.
//
template <class T>
void
TBitSet<T>::DisableItem(T item)
{
  Bits[Loc(item)] &= uint8(~Mask(item));
}

//
/// Turn on a specific bit.
//
template <class T>
void
TBitSet<T>::EnableItem(T item)
{
  Bits[Loc(item)] |= Mask(item);
}

//
/// Enable a specific bit.
//
template <class T>
TBitSet<T>& TBitSet<T>::operator +=(T item)
{
  EnableItem(item);
  return *this;
}

//
/// Disable a specific bit.
//
template <class T>
TBitSet<T>& TBitSet<T>::operator -=(T item)
{
  DisableItem(item);
  return *this;
}

//
/// Turn off all the given bits.
//
template <class T>
void
TBitSet<T>::DisableItem(const TBitSet<T>& bs)
{
  for (unsigned int i = 0; i < sizeof(Bits); i++)
    Bits[i] &= uint8(~(bs.Bits[i]));
}

//
/// Turn on all the given bits.
//
template <class T>
void
TBitSet<T>::EnableItem(const TBitSet<T>& bs)
{
  for (unsigned int i = 0; i < sizeof(Bits); i++)
    Bits[i] |= bs.Bits[i];
}

//
/// Enable all the given bits.
//
template <class T>
TBitSet<T>& TBitSet<T>::operator +=(const TBitSet<T>& bs)
{
  EnableItem(bs);
  return *this;
}

//
/// Enable all the given bits.
//
template <class T>
TBitSet<T>& TBitSet<T>::operator |=(const TBitSet<T>& bs)
{
  EnableItem(bs);
  return *this;
}

//
/// Disable all the given bits.
//
template <class T>
TBitSet<T>& TBitSet<T>::operator -=(const TBitSet<T>& bs)
{
  DisableItem(bs);
  return *this;
}

//
/// Logically AND each individual bits.
//
template <class T>
TBitSet<T>&
TBitSet<T>::operator &=(const TBitSet<T>& bs)
{
  for (unsigned int i = 0; i < sizeof(Bits); i++)
    Bits[i] &= bs.Bits[i];
  return *this;
}

//
/// Return true if all bits are off.
//
template <class T>
bool
TBitSet<T>::IsEmpty() const
{
  for (unsigned int i = 0; i < sizeof(Bits); i++)
    if (Bits[i] != 0)
      return false;
  return true;
}

namespace detail
{

#if __cplusplus > 199711L // C++11

  template <class T>
  using make_unsigned = std::make_unsigned<T>;

#else

  template <class T>
  struct make_unsigned;

  template <> struct make_unsigned<uint8> {typedef uint8 type;};
  template <> struct make_unsigned<char> {typedef unsigned char type;};
  template <> struct make_unsigned<wchar_t> {typedef wchar_t type;};

#endif

} // namespace

//
/// Return the index of the item searched for.
//
template <class T>
int TBitSet<T>::Loc(T item) const
{
  return static_cast<typename detail::make_unsigned<T>::type>(item) / 8;
}

//
/// Return the mask needed to look for a specific bit.
//
template <class T>
uint8 TBitSet<T>::Mask(T item) const
{
  return static_cast<uint8>(1 << (static_cast<typename detail::make_unsigned<T>::type>(item) % 8));
}

//
// Explicit instantiations. 
// These are the only instantiations that are supported.
// (Otherwise the implementation above needs to be moved to the header,
// so that the user can instantiate it with other template arguments).
//
template class TBitSet<uint8>;
template class TBitSet<tchar>;

//----------------------------------------------------------------------------

//
/// Construct a default character set.
/// All items are set to 0.
//
TCharSet::TCharSet()
:
  TBitSet<tchar>()
{
}

//
/// Copy the bitset into this character set.
//
TCharSet::TCharSet(const TBitSet<tchar>& bs)
:
  TBitSet<tchar>(bs)
{
}

//
/// Construct a character set from a string.
/// The characters act as the index for the bitset.
//
TCharSet::TCharSet(LPCTSTR str)
:
  TBitSet<tchar>()
{
  for (LPCTSTR p = str; *p; p++) {
    if (*p == _T('\\'))
      p++;
    else if (*p == _T('-') && p > str && p[1]) {  // handle "A-Z" type shorthands
      p++;
      for (tchar c = tchar(p[-2]+1); c < *p; c++) // replace "-" with "B..Y"
        EnableItem((uint8)c);
    }
    EnableItem((uint8)*p);
  }
}

//
/// Construct a character set from a string.
/// The characters act as the index for the bitset.
//
TCharSet::TCharSet(const tstring& s)
:
  TBitSet<tchar>()
{
  const LPCTSTR str = s.c_str();
  for (LPCTSTR p = str; *p; p++) {
    if (*p == _T('\\'))
      p++;
    else if (*p == _T('-') && p > str && p[1]) {  // handle "A-Z" type shorthands
      p++;
      for (tchar c = tchar(p[-2]+1); c < *p; c++) // replace "-" with "B..Y"
        EnableItem((uint8)c);
    }
    EnableItem((uint8)*p);
  }
}

} // OWL namespace
//==============================================================================
