//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of a bit set and a character set
//----------------------------------------------------------------------------

#if !defined(OWL_BITSET_H)
#define OWL_BITSET_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/objstrm.h>

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>


namespace owl {

/// \addtogroup utility
/// @{
/// \class TBitSet
// ~~~~~ ~~~~~~~
/// Simplifies setting and testing bits in a 32 count array of uint8 (32 bytes).
//
/// TBitSet sets or clears a single bit or a group of bits. You can use this class
/// to set and clear option flags and to retrieve information about a set of bits.
/// The class TCharSet performs similar operations for a string of characters.

template <class T = uint8>
class _OWLCLASS TBitSet {
  public:
    TBitSet();
    TBitSet(const TBitSet& src);

    bool Has(T item) const;

    TBitSet operator ~() const;

    void DisableItem(T item);
    void EnableItem(T item);
    TBitSet& operator +=(T item);
    TBitSet& operator -=(T item);

    void DisableItem(const TBitSet& bs);
    void EnableItem(const TBitSet& bs);
    TBitSet& operator +=(const TBitSet& bs);
    TBitSet& operator |=(const TBitSet& bs);
    TBitSet& operator -=(const TBitSet& bs);
    TBitSet& operator &=(const TBitSet& bs);

    bool   IsEmpty() const;

    friend TBitSet operator &(const TBitSet& bs1, const TBitSet& bs2);

    friend TBitSet operator |(const TBitSet& bs1, const TBitSet& bs2);

    friend int operator ==(const TBitSet& bs1, const TBitSet& bs2);

    friend int operator !=(const TBitSet& bs1, const TBitSet& bs2);

    friend opstream& operator <<(opstream& out, const TBitSet& bs)
    {
      out.fwriteBytes(bs.Bits, sizeof(bs.Bits));
      return out;
    }
    friend ipstream& operator >>(ipstream& in, TBitSet& bs)
    {
      in.freadBytes(bs.Bits, sizeof(bs.Bits));
      return in;
    }

  private:
    int    Loc(T item) const;
    uint8  Mask(T item) const;

    uint8  Bits[1 << (sizeof(T) * 8 - 3)];
};

template<class T>
int operator ==(const TBitSet<T>& bs1, const TBitSet<T>& bs2)
{
  for (int i = 0; i < sizeof(bs1.Bits); i++)
    if (bs1.Bits[i] != bs2.Bits[i])
      return false;
  return true;
}

template<class T>
TBitSet<T> operator &(const TBitSet<T>& bs1, const TBitSet<T>& bs2)
{
  TBitSet<T> temp(bs1);
  temp &= bs2;
  return temp;
}

template<class T>
TBitSet<T> operator |(const TBitSet<T>& bs1, const TBitSet<T>& bs2)
{
  TBitSet<T> temp(bs1);
  temp |= bs2;
  return temp;
}

template<class T>
int operator !=(const TBitSet<T>& bs1, const TBitSet<T>& bs2)
{
  return !operator ==(bs1, bs2);
}


//
/// \class TCharSet
// ~~~~~ ~~~~~~~~
/// Derived from TBitSet, TCharSet sets and clears bytes for a group of characters.
/// You can use this class to set or clear bits in a group of characters, such as
/// the capital letters from "A" through "Z" or the lowercase letters from "a"
/// through "z". The class TBitSet performs similar operations for a group of bits.
//
class _OWLCLASS TCharSet : public TBitSet<tchar> {
  public:
    TCharSet();
    TCharSet(const TBitSet<tchar>& src);
    TCharSet(LPCTSTR str);
    TCharSet(const tstring& str);
};

/// \class TBitFlags
/// TBitFlags is a *lightweight* class for setting, clearing and querrying
/// bit flags. It's intenteded to be used with a 'short' or 'long' type
/// allowing an easy method to handle 16 and 32 bit flags respectively.
//
/// For example:
/// \code
///    class TMyClass : public TMyBase, public TBitFlags<short> {
/// \endcode
//
template <class T> class /*_OWLCLASS*/ TBitFlags {
  public:
    TBitFlags(T  t = 0);

    // Query, Clear and Set flag bits
    //
    T       Clear(T t);
    T       Set(T t);
    bool    IsSet(T t) const;

  protected:
    T       Bits;
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Initialize the Flags to the specified parameter
//
template <class T>
TBitFlags<T>::TBitFlags(T t)
:
  Bits(t)
{
}

//
/// Activate the bits that are enabled in the specified parameter
//
template <class T>
T TBitFlags<T>::Set(T t)
{
  return Bits |= t;
}

//
/// Clear the bits that are enabled in the specified parameter
//
template <class T>
T TBitFlags<T>::Clear(T t)
{
  return Bits &= ~t;
}

//
/// Return true of the ON bits of the parameter are currently enabled.
/// Return false otherwise.
//
template <class T>
bool TBitFlags<T>::IsSet(T t) const
{
  return (Bits & t) != 0;
}


} // OWL namespace


#endif  // OWL_BITSET_H
