//------------------------------------------------------------------------------88
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus
//
/// \file
/// Definition of container classes used and made available by OWL
//------------------------------------------------------------------------------

#if !defined(OWL_TEMPLATE_H)
#define OWL_TEMPLATE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/defs.h>
#include <owl/private/wsysinc.h>
#include <new>
#include <algorithm>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup utility
/// @{

//-------------------------------------------------------------------------
//
/// Provides class-specific operator new and operator delete that simply call
/// the global operator new and operator delete. That is, TStandardAllocator
/// does not provide any specialized behavior. It is used in the non-managed
/// versions of the parametrized containers.
//
class TStandardAllocator
{
public:

  void operator delete(void* ptr);
  void* operator new(size_t sz);

  void* Alloc(size_t sz);
  void Free(void* ptr);
  void* ReAlloc(void* ptr, size_t sz);

};

typedef TStandardAllocator TSharedAllocator;

/// @}

inline void TStandardAllocator::operator delete(void* ptr)
{
  ::operator delete(ptr);
}

inline void* TStandardAllocator::operator new(size_t sz)
{
  return ::operator new(sz);
}

inline void* TStandardAllocator::Alloc(size_t sz)
{
  return ::malloc(sz);
}

inline void TStandardAllocator::Free(void* ptr)
{
  ::free(ptr);
}

inline void* TStandardAllocator::ReAlloc(void* ptr, size_t sz)
{
  return ::realloc(ptr, sz);
}

/// \addtogroup utility
/// @{

//----------------------------------------------------------------------------
//
/// Provides class-specific operator new and operator delete that allocate from
/// local memory.
//
class TLocalAllocator
{
public:

  void* operator new(size_t sz);
  void operator delete(void* ptr);

  void* Alloc(size_t sz);
  void Free(void* ptr);
  void* ReAlloc(void* ptr, size_t sz);

};

/// @}

inline void* TLocalAllocator::operator new(size_t sz)
{
  return static_cast<void*>(::LocalAlloc(LPTR, sz));
}

inline void TLocalAllocator::operator delete(void* ptr)
{
  ::LocalFree(static_cast<HLOCAL>(ptr));
}

inline void* TLocalAllocator::Alloc(size_t sz)
{
  return static_cast<void*>(::LocalAlloc(LPTR, sz));
}

inline void TLocalAllocator::Free(void* ptr)
{
  ::LocalFree(static_cast<HLOCAL>(ptr));
}

inline void* TLocalAllocator::ReAlloc(void* ptr, size_t sz)
{
  return static_cast<void*>(::LocalReAlloc(static_cast<HLOCAL>(ptr), sz, LPTR));
}

/// \addtogroup utility
/// @{

//----------------------------------------------------------------------------
//
/// Provides class-specific operator new and operator delete that allocate from
/// global memory.
//
class TGlobalAllocator
{
public:

  void* operator new(size_t sz);
  void operator delete(void* ptr);

  void* Alloc(size_t sz);
  void Free(void* ptr);
  void* ReAlloc(void* ptr, size_t sz);

};

/// @}

inline void* TGlobalAllocator::operator new(size_t sz)
{
  return static_cast<void*>(::GlobalLock(::GlobalAlloc(GPTR, sz)));
}

inline void TGlobalAllocator::operator delete(void* ptr)
{
  HGLOBAL hMem = static_cast<HGLOBAL>(GlobalHandle(ptr));
  if (GlobalUnlock(hMem) == 0)
    GlobalFree(hMem);
}

inline void* TGlobalAllocator::Alloc(size_t sz)
{
  return static_cast<void*>(::GlobalLock(::GlobalAlloc(GPTR, sz)));
}

inline void TGlobalAllocator::Free(void* ptr)
{
  HGLOBAL hMem = static_cast<HGLOBAL>(GlobalHandle(ptr));
  if (GlobalUnlock(hMem) == 0)
    GlobalFree(hMem);
}

inline void* TGlobalAllocator::ReAlloc(void* ptr, size_t sz)
{
  HGLOBAL hMem = static_cast<HGLOBAL>(GlobalHandle(ptr));
  if (hMem && GlobalUnlock(hMem) == 0)
    return static_cast<void*>(::GlobalLock(GlobalReAlloc(hMem, sz, GPTR)));
  return nullptr;
}

/// \addtogroup utility
/// @{

//--------------------------------------------------------------------------
//
/// Used internally.
//
template <class A>
class TMBaseMemBlocks;

template <class A>
class TMBlockList
{
public:

  TMBlockList(TMBlockList*);
  ~TMBlockList() {}

private:

  void* operator new(size_t, size_t, const A&);
  void operator delete(void*);

  void operator delete(void* p, size_t, const A&);

  TMBlockList* Next;
  friend class TMBaseMemBlocks<A>;

};

/// @}

template <class A>
inline TMBlockList<A>::TMBlockList(TMBlockList<A>* next)
  : Next(next)
{}

template <class A>
inline void* TMBlockList<A>::operator new(size_t sz, size_t extra, const A&)
{
  return A::operator new(sz + extra);
}

template <class A>
inline void TMBlockList<A>::operator delete(void* ptr)
{
  A::operator delete (ptr);
}

template <class A>
inline void TMBlockList<A>::operator delete(void* p, size_t, const A&)
{
  A::operator delete (p);
}

/// \addtogroup utility
/// @{

//--------------------------------------------------------------------------
//
/// Used internally.
//
class TBlockList
  : public TMBlockList<TStandardAllocator>
{
public:

  TBlockList(TBlockList* blk) : TMBlockList<TStandardAllocator>(blk) {}

};

//--------------------------------------------------------------------------
//
/// Used internally.
//
template <class A>
class TMBaseMemBlocks
  : public A
{
public:

  TMBaseMemBlocks(size_t = 8192);
  ~TMBaseMemBlocks();

  char* Block() const { return REINTERPRET_CAST(char*, CurBlock); }
  unsigned Count() const { return BlockCount; }
  int AllocBlock(size_t);
  void FreeTo(unsigned);
  size_t GetBlockSize() const { return BlockSize; }

private:

  TMBlockList<A>* CurBlock;
  const size_t BlockSize;
  unsigned BlockCount;

};

/// @}

template <class A>
inline TMBaseMemBlocks<A>::TMBaseMemBlocks(size_t sz)
  : CurBlock(0), BlockSize(sz), BlockCount(0)
{
  CHECK(sz != 0);
}

template <class A>
inline TMBaseMemBlocks<A>::~TMBaseMemBlocks()
{

#if !defined( BI_WINDOWS_WEP_BUG )

  FreeTo(0);

#endif

}

template <class A>
int TMBaseMemBlocks<A>::AllocBlock(size_t sz)
{
  TMBlockList<A>* temp = new(std::max(sz, BlockSize), *this) TMBlockList<A>(CurBlock);
  CurBlock = temp + 1;
  BlockCount++;
  return 1;
}

template <class A>
void TMBaseMemBlocks<A>::FreeTo(unsigned term)
{
  PRECONDITION(BlockCount >= term);
  while (BlockCount > term)
  {
    TMBlockList<A>* temp = CurBlock - 1;
    CurBlock = temp->Next;

    delete temp;
    BlockCount--;
  }
}

/// \addtogroup utility
/// @{

//--------------------------------------------------------------------------
//
/// Used internally.
//
class TBaseMemBlocks
  : public TMBaseMemBlocks<TStandardAllocator>
{
public:

  TBaseMemBlocks(size_t sz = 8192)
    : TMBaseMemBlocks<TStandardAllocator>(sz)
  {}

};

//--------------------------------------------------------------------------
//
/// Managed memory stack.
/// Implements mark and release style memory management, using the allocator A.
//
template <class A>
class TMMarker;

template <class A>
class TMMemStack
{
public:

  TMMemStack(size_t = 8192);
  void* Allocate(size_t);

  friend class TMMarker<A>;

private:

  TMBaseMemBlocks<A> Data;
  size_t CurLoc;

};

/// @}

template <class A>
inline TMMemStack<A>::TMMemStack(size_t sz)
  : Data(sz), CurLoc(sz)
{
  CHECK(sz != 0);
}

template <class A>
void* TMMemStack<A>::Allocate(size_t sz)
{
  sz = std::max(static_cast<size_t>(1), sz);
  if (sz > Data.GetBlockSize() - CurLoc)
  {
    if (Data.AllocBlock(sz) == 0)
      return nullptr;
    else
      CurLoc = 0;
  }
  void* temp = Data.Block() + CurLoc;
  CurLoc += sz;
  return temp;
}

/// \addtogroup utility
/// @{

//--------------------------------------------------------------------------
//
/// Provides the mark for TMMemStack.
//
template <class A>
class TMMarker
{
public:

  TMMarker(TMMemStack<A>& ms)
    : Memstk(ms), Blk(ms.Data.Count()), CurLoc(ms.CurLoc)
  {}

  ~TMMarker()
  {
    const auto ok = Blk < Memstk.Data.Count() || (Blk == Memstk.Data.Count() && CurLoc <= Memstk.CurLoc);
    WARN(!ok, _T("TMMarker::~TMMarker: Terminating due to failed precondition."));
    if (!ok) std::terminate();

    Memstk.Data.FreeTo(Blk);
    Memstk.CurLoc = CurLoc;
  }

private:

  TMMemStack<A>& Memstk;
  const unsigned Blk;
  const size_t CurLoc;

};

//--------------------------------------------------------------------------
//
/// Implements mark and release style memory management using the standard allocator.
//
class TMemStack
  : public TMMemStack<TStandardAllocator>
{
public:
  TMemStack(size_t sz = 8192) : TMMemStack<TStandardAllocator>(sz) {}
};

//--------------------------------------------------------------------------
//
/// Provides the mark for TMemStack.
//
class TMarker
  : public TMMarker<TStandardAllocator>
{
public:

  TMarker(TMMemStack<TStandardAllocator>& ms)
    : TMMarker<TStandardAllocator>(ms)
  {}

};

//--------------------------------------------------------------------------
//
/// Managed single-size block allocator.
/// Allocates blocks of the size specified in the constructor, using the memory
/// manager specified by A.
//
template <class A>
class TMMemBlocks
{
public:

  TMMemBlocks(size_t sz, unsigned int count = 100);

  void* Allocate(size_t sz);
  void Free(void* block);

private:

  void* FreeList;
  TMMemStack<A> Mem;
  size_t Size;

};

/// @}

template <class A>
inline TMMemBlocks<A>::TMMemBlocks(size_t sz, unsigned int count)
  : FreeList(nullptr), Mem(sz* count), Size(std::max(sz, sizeof(void*)))
{
  CHECK(sz != 0 && count != 0);
}

template <class A>
inline void* TMMemBlocks<A>::Allocate(size_t sz)
{
  PRECONDITION(Size == std::max(sz, sizeof(void *))); InUse(sz);
  if (FreeList == nullptr)
    return Mem.Allocate(Size);
  else
  {
    void *temp = FreeList;
    FreeList = *static_cast<void**>(temp);
    return temp;
  }
}

template <class A>
inline void TMMemBlocks<A>::Free(void* block)
{
  *static_cast<void **>(block) = FreeList;
  FreeList = block;
}

/// \addtogroup utility
/// @{

//--------------------------------------------------------------------------
/// Single-size block allocator.
/// Allocates blocks of the size specified in the constructor, using the global operator new
/// and operator delete.
//
class TMemBlocks
  : public TMMemBlocks<TStandardAllocator>
{
public:

  TMemBlocks(size_t sz, unsigned n = 100)
    : TMMemBlocks<TStandardAllocator>(sz, n)
  {}

};

/// @}

typedef TMMemBlocks<TStandardAllocator> TStandardBlocks;

template <class T> inline T* __owl_construct(void* P)
{
  return new(P)T;
}

template <class T> inline T* __owl_construct(void* P, const T& t)
{
  return new(P)T(t);
}

template <class T> inline void __owl_destruct(T* t)
{
  static_cast<void>(t); // Suppress "not in use" warning.
  t->~T();
}

//
// specializations
// chars
//
inline char* __owl_construct(void* p, char c) { *(static_cast<char*>(p)) = c; return static_cast<char*>(p); }
inline void __owl_destruct(char* /*t*/) {}

//
// integers
//
inline int* __owl_construct(void* p, int c) { *(static_cast<int*>(p)) = c; return static_cast<int*>(p); }
inline unsigned int* __owl_construct(void* p, unsigned int c) { *(static_cast<unsigned int*>(p)) = c; return static_cast<unsigned int*>(p); }
inline void __owl_destruct(int* /*t*/) {}

//
// long
//
inline long* __owl_construct(void* p, long c) { *(static_cast<long*>(p)) = c; return static_cast<long*>(p); }
inline void __owl_destruct(long* /*t*/) {}

/// \addtogroup utility
/// @{

//--------------------------------------------------------------------------
// Container classes
//

//
/// Iterator for TObjectContainer
//
template <class T, class T1>
class TObjArrayIterator
{
public:

  TObjArrayIterator(T1& array) :Vect(array), Cur(0) {}

  operator bool() const { return Cur < static_cast<int>(Vect.Size()); }
  const T& operator ++(int) { Cur++; return *Vect.Data[Cur - 1]; }
  const T* operator ++() { return Cur < static_cast<int>(Vect.Size()) ? Vect.Data[++Cur] : 0; }
  const T& operator *() const { return *Vect.Data[Cur]; }
  T& operator *() { return *Vect.Data[Cur]; }
  const T& Current() const { return *Vect.Data[Cur]; }
  T& Current() { return *Vect.Data[Cur]; }
  void Restart() { Cur = 0; }

protected:

  T1& Vect;
  int Cur;

};

//
/// Iterator for Pointer Container
//
template <class T, class T1>
class TPtrArrayIterator
{
public:

  TPtrArrayIterator(T1& array) :Vect(array), Cur(0) {}

  operator bool() const { return Cur < static_cast<int>(Vect.Size()); }
  T operator ++(int) { Cur++; return Vect.Data[Cur - 1]; }
  const T* operator ++() { return Cur < static_cast<int>(Vect.Size()) ? &Vect.Data[++Cur] : 0; }
  const T operator *() const { return Vect.Data[Cur]; }
  T operator *() { return Vect.Data[Cur]; }
  const T Current() const { return Vect.Data[Cur]; }
  T Current() { return Vect.Data[Cur]; }
  void Restart() { Cur = 0; }

protected:

  T1& Vect;
  int Cur;

};

//
/// Map node.
/// Usage:
/// typedef TMapNode<string,string> TStringMapNode;
/// typedef TSortedObjectArray<TStringMapNode> TStringMap;
/// typedef TSortedObjectArray<TStringMapNode>::Iterator TStringMapIterator;
//
template <class T1, class T2>
class TMapNode
{
public:

  TMapNode(T1& name, T2& val) :Name(name), Value(val) {}
  TMapNode(T1& name) :Name(name) {}
  TMapNode(const TMapNode<T1, T2>& node) :Name(node.Name), Value(node.Value) {}
  TMapNode() {}
  ~TMapNode() {}

  bool operator <(const TMapNode& ms) const { return Name < ms.Name; }
  bool operator ==(const TMapNode& ms) const { return Name == ms.Name; }
  TMapNode& operator =(const TMapNode& other)
  {
    Name = other.Name;
    Value = other.Value;
    return *this;
  }

  T1 Name;
  T2 Value;

};

//------------------------------------------------------------------------------
//
/// Base array class.
/// Reduces code size.
//
class TArrayBase
{
public:

  TArrayBase() :ItemCnt(0), Reserved(0) {}

  uint GetItemsInContainer() const { return Size(); }
  int LowerBound() const { return 0; }
  int UpperBound() const { return Size() - 1; }
  uint ArraySize() const { return Reserved; }
  bool IsFull() const { return ItemCnt >= Reserved - 1; }
  bool IsEmpty() const { return Empty(); }
  uint Size() const { return ItemCnt; }
  uint Count() const { return ItemCnt; }
  bool Empty() const { return ItemCnt == 0; }

  //
  // Support STL naming convention.
  //
  uint size() const { return ItemCnt; }
  bool empty() const { return Empty(); }

  static const int NPOS = -1;

protected:

  uint ItemCnt;
  uint Reserved;

};


//------------------------------------------------------------------------------
//
/// Holds array pointers to strings.
//
template <class T, class A = TStandardAllocator>
class TMObjectArray
  : public A, public TArrayBase
{
public:

  typedef void (*IterFunc)(T&, void*);
  typedef bool (*CondFunc)(const T&, void*);

  typedef TObjArrayIterator<T, TMObjectArray<T, A> > Iterator;
  friend class TObjArrayIterator<T, TMObjectArray<T, A> >;
  typedef Iterator iterator;

  TMObjectArray(int upper, int lower = 0, int /*delta*/ = 0)
    : Mem(sizeof(T), upper - lower + 1), Data(0) { Resize(upper); }
  TMObjectArray() : Data{nullptr}, Mem{sizeof(T), 10} {}
  ~TMObjectArray();
  TMObjectArray(const TMObjectArray<T, A>& array);
  TMObjectArray<T, A>& operator=(const TMObjectArray<T, A>& array);
  TMMemBlocks<A>& GetAllocator() { return Mem; }

  void RemoveEntry(int loc) { Remove(loc); }
  void SqueezeEntry(unsigned loc) { Remove(loc); }
  void Grow(int index);

  int Add(const T& t);
  void AddAt(const T& t, uint index);

  bool DestroyItem(const T& t);
  bool Destroy(int loc);
  bool HasMember(const T& t) const;
  int Find(const T& t) const;

  T& operator [](int loc);
  const T& operator [](int loc) const;

  void ForEach(IterFunc iter, void* args);
  T* FirstThat(CondFunc cond, void* args) const;
  T* LastThat(CondFunc cond, void* args) const;

  void Flush();

protected:

  void Resize(int delta);
  void Remove(int index);

protected:

  T** Data;
  TMMemBlocks<A> Mem;

};

//------------------------------------------------------------------------------

template <class T>
class TObjectArray
  : public TMObjectArray<T, TStandardAllocator>
{
public:

  typedef void (*IterFunc)(T&, void*);
  typedef bool (*CondFunc)(const T&, void*);

  typedef TObjArrayIterator<T, TObjectArray<T> > Iterator;
  typedef Iterator iterator;
  friend Iterator;

  TObjectArray(int upper, int lower = 0, int delta = 0)
    : TMObjectArray<T, TStandardAllocator>(upper, lower, delta) {}
  TObjectArray() {}
  ~TObjectArray() {}
  TObjectArray(const TObjectArray<T>& array) :TMObjectArray<T, TStandardAllocator>(array) {}

  TObjectArray<T>& operator=(const TObjectArray<T>& array)
  {
    TMObjectArray<T, TStandardAllocator>::operator=(array);
    return *this;
  }

};

//------------------------------------------------------------------------------

template <class T, class A = TStandardAllocator>
class TMSortedObjectArray
  : public TMObjectArray<T, A>
{
public:

  typedef TObjArrayIterator<T, TMSortedObjectArray<T, A> > Iterator;
  friend class TObjArrayIterator<T, TMSortedObjectArray<T, A> >;
  typedef Iterator iterator;

  TMSortedObjectArray(int upper, int lower = 0, int delta = 0)
    : TMObjectArray<T, A>(upper, lower, delta) {}
  TMSortedObjectArray() : TMObjectArray<T, A>() {}
  ~TMSortedObjectArray() {}
  TMSortedObjectArray(const TMSortedObjectArray<T, A>& array);

  TMSortedObjectArray<T, A>& operator=(const TMSortedObjectArray<T, A>& array);

  int Add(const T& t);
  bool DestroyItem(const T& t);
  bool HasMember(const T& t) const;
  int Find(const T& t) const;

private:

  int AddAt(const T& t, int index) { return 0; } // hide
  bool DetachItem(const T& t);

};

//------------------------------------------------------------------------------

template <class T>
class TSortedObjectArray
  : public TMSortedObjectArray<T, TStandardAllocator>
{
public:

  typedef void (*IterFunc)(T&, void*);
  typedef bool (*CondFunc)(const T&, void*);

  typedef TObjArrayIterator<T, TSortedObjectArray<T> > Iterator;
  typedef Iterator iterator;
  friend Iterator;

  TSortedObjectArray(int upper, int lower = 0, int delta = 0)
    : TMSortedObjectArray<T, TStandardAllocator>(upper, lower, delta) {}
  TSortedObjectArray() {}
  ~TSortedObjectArray() {}
  TSortedObjectArray(const TSortedObjectArray<T>& array) :TMSortedObjectArray<T, TStandardAllocator>(array) {}

  TSortedObjectArray<T>& operator=(const TSortedObjectArray<T>& array)
  {
    TMSortedObjectArray<T, TStandardAllocator>::operator=(array);
    return *this;
  }

};

//------------------------------------------------------------------------------
//
/// Stores pointer to object.
/// \note Jogy: Incorrect, stores simple types without assuming that they are pointer to objects.
//
template <class T, class R, class A = TStandardAllocator>
class TTypedArray
  : public A, public TArrayBase
{
public:

  typedef void (*IterFunc)(R, void*);
  typedef bool (*CondFunc)(R, void*);

  typedef TPtrArrayIterator<R, TTypedArray<T, R, A> > Iterator;
  friend class TPtrArrayIterator<R, TTypedArray<T, R, A> >;
  typedef Iterator iterator;

  TTypedArray() : Data(nullptr) {}
  TTypedArray(int upper, int /*lower*/ = 0, int /*delta*/ = 0)
    : Data(nullptr)
  {
    Resize(upper);
  }

  ~TTypedArray() { if (Data) A::Free(Data); }

  TTypedArray(const TTypedArray<T, R, A>& array);
  TTypedArray<T, R, A>& operator=(const TTypedArray<T, R, A>& array);

  void RemoveEntry(int loc) { Remove(loc); }
  void SqueezeEntry(unsigned loc) { Remove(loc); }
  void Grow(int index);

  int Add(R t);
  void AddAt(R t, uint index);
  bool DetachItem(R t);
  bool Detach(int loc);
  bool DestroyItem(R t);
  bool Destroy(int loc);
  bool HasMember(R t) const;
  int Find(R t) const;

  T& operator [](int loc);
  const T& operator [](int loc) const;

  void ForEach(IterFunc iter, void* args);
  T* FirstThat(CondFunc cond, void* args) const;
  T* LastThat(CondFunc cond, void* args) const;

  void Flush();

protected:

  T* Data;

  void Resize(int delta);
  void Remove(int index);

};

//------------------------------------------------------------------------------

template <class T, class R, class A = TStandardAllocator>
class TSTypedArray
  : public TTypedArray<T, R, A>
{
public:

  typedef TPtrArrayIterator<R, TSTypedArray<T, R, A> > Iterator;
  friend class TPtrArrayIterator<R, TSTypedArray<T, R, A> >;
  typedef Iterator iterator;

  TSTypedArray() : TTypedArray<T, R, A>() {}
  TSTypedArray(int upper, int lower = 0, int delta = 0)
    : TTypedArray<T, R, A>(upper, lower, delta)
  {}

  ~TSTypedArray() {}

  TSTypedArray(const TSTypedArray<T, R, A>& array);
  TSTypedArray<T, R, A>& operator=(const TSTypedArray<T, R, A>& array);

  int Add(R t);
  bool DetachItem(R t);
  bool DestroyItem(R t);
  bool HasMember(R t) const;
  int Find(R t) const;

private:

  int AddAt(R t, int index); // hide

};

//------------------------------------------------------------------------------
//
/// Array of pointers of simple types.
//
template <class T>
class TPtrArray
  : public TTypedArray<T, T, TStandardAllocator>
{
public:

  TPtrArray() :TTypedArray<T, T, TStandardAllocator>() {}
  TPtrArray(int upper, int lower = 0, int delta = 0)
    : TTypedArray<T, T, TStandardAllocator>(upper, lower, delta)
  {}

  TPtrArray(const TPtrArray<T>& array) :TTypedArray<T, T, TStandardAllocator>(array) {}

};

//------------------------------------------------------------------------------
//
/// Sorted array of pointers of simple types
//
template <class T>
class TSortedPtrArray
  : public TSTypedArray<T, T, TStandardAllocator>
{
public:

  TSortedPtrArray() :TSTypedArray<T, T, TStandardAllocator>() {}
  TSortedPtrArray(int upper, int lower = 0, int delta = 0)
    : TSTypedArray<T, T, TStandardAllocator>(upper, lower, delta)
  {}

  TSortedPtrArray(const TSortedPtrArray<T>& array) :TSTypedArray<T, T, TStandardAllocator>(array) {}

};

//------------------------------------------------------------------------------

template <class T, class R, class A = TStandardAllocator>
class TMIPtrArray
  : public TTypedArray<T, R, A>
{
public:

  typedef TPtrArrayIterator<R, TMIPtrArray<T, R, A> > Iterator;
  friend class TPtrArrayIterator<R, TMIPtrArray<T, R, A> >;
  typedef Iterator iterator;

  TMIPtrArray();
  TMIPtrArray(int upper, int lower = 0, int delta = 0);
  ~TMIPtrArray();

  TMIPtrArray(const TMIPtrArray<T, R, A>& array);
  TMIPtrArray<T, R, A>& operator=(const TMIPtrArray<T, R, A>& array);

  bool DestroyItem(R t);
  bool Destroy(int loc);
  void Flush(bool del = true);

};

//------------------------------------------------------------------------------

template <class T>
class TIPtrArray
  : public TPtrArray<T>
{
public:

  typedef TPtrArrayIterator<T, TIPtrArray<T> > Iterator;
  friend class TPtrArrayIterator<T, TIPtrArray<T> >;
  typedef Iterator iterator;

  TIPtrArray();
  TIPtrArray(int upper, int lower = 0, int delta = 0);
  ~TIPtrArray();

  TIPtrArray(const TIPtrArray<T>& array);
  TIPtrArray<T>& operator=(const TIPtrArray<T>& array);

  bool DestroyItem(T t);
  bool Destroy(int loc);
  void Flush(bool del = true);

};

template <class T>
class TISortedPtrArray
  : public TSortedPtrArray<T>
{
public:

  typedef TPtrArrayIterator<T, TISortedPtrArray<T> > Iterator;
  friend class TPtrArrayIterator<T, TISortedPtrArray<T> >;
  typedef Iterator iterator;

  TISortedPtrArray();
  TISortedPtrArray(int upper, int lower = 0, int delta = 0);

  ~TISortedPtrArray();

  TISortedPtrArray(const TISortedPtrArray<T>& array);
  TISortedPtrArray<T>& operator=(const TISortedPtrArray<T>& array);

  bool DestroyItem(T t);
  bool Destroy(int loc);
  void Flush(bool del = true);

};

//------------------------------------------------------------------------------

template <class T>
class TBaseNode
{
public:

  TBaseNode(const T& data) :Next(0), Prev(0), Data(data) {}

  TBaseNode* Next;
  TBaseNode* Prev;
  T Data;

};

template <class T>
class TBaseList;

template <class T>
class TBaseListIterator
{
public:

  TBaseListIterator(TBaseList<T>& list) : List(list), Cur(list.First) {}

  operator bool() const { return Cur != 0; }
  const T& operator ++(int) { TBaseNode<T>* tmp = Cur; Cur = Cur->Next; return tmp->Data; }
  const T* operator ++() { Cur = Cur->Next; return Cur ? &Cur->Data : 0; } //?????
  const T& operator --(int) { TBaseNode<T>* tmp = Cur; Cur = Cur->Prev; return tmp->Data; }
  const T* operator --() { Cur = Cur->Prev; return Cur ? &Cur->Data : 0; } //?????
  const T& operator *() const { return Cur->Data; }
  const T& Current() const { return Cur->Data; }
  void Restart() { Cur = List.First; }

  TBaseList<T>& List;
  TBaseNode<T>* Cur;

};

//------------------------------------------------------------------------------

template <class T>
class TBaseList
{
public:

  typedef void (*IterFunc)(T&, void*);
  typedef bool (*CondFunc)(const T&, void*);

  typedef TBaseListIterator<T> Iterator;
  friend class TBaseListIterator<T>;
  typedef Iterator iterator;

  TBaseList();
  TBaseList(int /*upper*/, int lower = 0, int delta = 0);
  ~TBaseList();

  int LowerBound() const { return 0; }
  uint Size() const { return ItemSize; }
  int UpperBound() const { return Size() - 1; }
  uint ArraySize() const { return Size(); }
  bool IsFull() const { return true; }
  bool Empty() const { return First == nullptr; }
  bool IsEmpty() const { return Empty(); }
  uint GetItemsInContainer() const { return Size(); }


  void Add(const T& data);
  void Add(TBaseListIterator<T>* iter, const T& data);
  void Push(const T& data);
  void PushBack(const T& data);
  T Pop();
  const T& Top();
  const T& Bottom();

  void Flush();
  bool DetachItem(const T& t);
  bool Detach(int loc);
  bool HasMember(const T& t) const;
  TBaseNode<T>* Find(const T& t) const;
  void ForEach(IterFunc iter, void* args);
  T* FirstThat(CondFunc cond, void* args) const;
  T* LastThat(CondFunc cond, void* args) const;

protected:

  bool Remove(TBaseNode<T>* t);
  int Add(TBaseNode<T>* node, const T& data);
  int Add(TBaseNode<T>* node);
  int Add(TBaseNode<T>* item, TBaseNode<T>* node);

protected:

  TBaseNode<T>* First;
  TBaseNode<T>* Last;
  uint ItemSize; ///< optimization for Size()

  static const int NPOS = -1;
};

//------------------------------------------------------------------------------

template <class T, class T1>
class TTernaryNode
{
public:

  TTernaryNode(T split, const T1& data)
    : Data(data), Split(split), LoId(0), EqId(0), HiId(0)
  {}

  T Split;
  TTernaryNode* LoId;
  TTernaryNode* EqId;
  TTernaryNode* HiId;
  T1 Data;

};

template <class T, class T1>
class TTernaryTree
{
public:

  TTernaryTree() :Root(0) {}
  ~TTernaryTree() {}

  T1* Find(T* s) { return RSearch(Root, s); }
  void Insert(T* s, const T1& data) { Root = RInsert(Root, s, data); }
  void Flush() { Flush(Root); }

protected:

  T1* RSearch(TTernaryNode<T, T1>* p, T* s)
  {
    if (!p)
      return 0;
    if (*s < p->Split)
      return RSearch(p->LoId, s);
    else if (*s > p->Split)
      return RSearch(p->HiId, s);
    else
    {
      if (*s == 0)
        return &p->Data;
      return RSearch(p->EqId, ++s);
    }
  }

  TTernaryNode<T, T1>* RInsert(TTernaryNode<T, T1>* p, T* s, const T1& data)
  {
    if (!p)
      p = new TTernaryNode<T, T1>(*s, data);
    if (*s < p->Split)
      p->LoId = RInsert(p->LoId, s, data);
    else if (*s == p->Split)
    {
      if (*s != 0)
        p->EqId = RInsert(p->EqId, ++s, data);
    }
    else
      p->HiId = RInsert(p->HiId, s, data);
    ItemCnt++;
    return p;
  }

  void Flush(TTernaryNode<T, T1>* p)
  {
    if (p)
    {
      Flush(p->LoId);
      Flush(p->EqId);
      Flush(p->HiId);
      delete p;
    }
  }

protected:

  TTernaryNode<T, T1>* Root;
  uint ItemCnt; /// item count

};

/// @}

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inlines
//

template<class T, class A> TMObjectArray<T, A>::TMObjectArray(const TMObjectArray& array)
  : Data{nullptr}, Mem{sizeof(T), std::max(10u, array.Reserved / 10u)}
{
  if (array.Reserved)
    Resize(array.Reserved);
  for (int i = 0; i < static_cast<int>(array.ItemCnt); i++)
  {
    Data[i] = __owl_construct<T>(Mem.Allocate(sizeof(T)), *array.Data[i]);
  }
  ItemCnt = array.ItemCnt;
}

template <class T, class A>
inline TMObjectArray<T, A>::~TMObjectArray()
{
  Flush();
  if (Data)
    A::Free(Data);
}

template <class T, class A>
TMObjectArray<T, A>& TMObjectArray<T, A>::operator=(const TMObjectArray<T, A>& array)
{
  if (this == &array)
    return *this;

  Flush();
  if (array.ItemCnt >= Reserved)
    Resize(array.ItemCnt - Reserved);
  for (int i = 0; i < static_cast<int>(array.ItemCnt); i++)
  {
    Data[i] = __owl_construct<T>(Mem.Allocate(sizeof(T)), *array.Data[i]);
  }
  ItemCnt = array.ItemCnt;

  return *this;
}

template <class T, class A>
inline int TMObjectArray<T, A>::Add(const T& t)
{
  if (ItemCnt >= Reserved)
    Resize(ItemCnt + 1);
  Data[ItemCnt] = __owl_construct<T>(Mem.Allocate(sizeof(T)), t);
  return ItemCnt++; // if fail -> throw xalloc
}

template <class T, class A>
void TMObjectArray<T, A>::AddAt(const T& t, uint index)
{
  if (index == Size())
  {
    Add(t);
    return;
  }
  if (index >= Reserved)
    Resize(index + 1); // on error -> throw xalloc
  if (ItemCnt >= Reserved)
    Resize(ItemCnt + 1); // on error -> throw xalloc
  if (index < ItemCnt && ItemCnt)
    memmove(&Data[index + 1], &Data[index], sizeof(T*) * (ItemCnt - index));
  Data[index] = __owl_construct<T>(Mem.Allocate(sizeof(T)), t);
  ItemCnt++;
}

template <class T, class A>
inline bool TMObjectArray<T, A>::DestroyItem(const T& t)
{
  const auto index = this->Find(t);
  if (index != this->NPOS)
  {
    T* tmp = Data[index];
    Remove(index);
    __owl_destruct(tmp);// call destructor
    Mem.Free(tmp);
    return true;
  }
  return false;
}

template <class T, class A>
inline bool TMObjectArray<T, A>::Destroy(int loc)
{
  if (loc < static_cast<int>(Size()))
  {
    T* tmp = Data[loc];
    Remove(loc);
    __owl_destruct(tmp);// call destructor
    Mem.Free(tmp);
    return true;
  }
  return false;
}

template <class T, class A>
inline bool TMObjectArray<T, A>::HasMember(const T& t) const
{
  return this->Find(t) != this->NPOS;
}

template <class T, class A>
int TMObjectArray<T, A>::Find(const T& t) const
{
  for (int i = 0; i < static_cast<int>(Size()); i++)
    if (*Data[i] == t)
      return i;
  return this->NPOS;
}

template <class T, class A>
inline T& TMObjectArray<T, A>::operator [](int loc)
{
  PRECONDITION(loc < static_cast<int>(Size()));
  return *Data[loc];
}

template <class T, class A>
inline const T& TMObjectArray<T, A>::operator [](int loc) const
{
  PRECONDITION(loc < static_cast<int>(Size()));
  return *Data[loc];
}

template <class T, class A>
void TMObjectArray<T, A>::ForEach(IterFunc iter, void* args)
{
  for (int i = 0; i < static_cast<int>(Size()); i++)
    (iter)(*Data[i], args);
}

template <class T, class A>
T* TMObjectArray<T, A>::FirstThat(CondFunc cond, void* args) const
{
  for (int i = 0; i < static_cast<int>(Size()); i++)
    if ((cond)(*Data[i], args))
      return Data[i];
  return 0;
}

template <class T, class A>
T* TMObjectArray<T, A>::LastThat(CondFunc cond, void* args) const
{
  for (int i = static_cast<int>(Size()) - 1; i >= 0; i--)
    if ((cond)(*Data[i], args))
      return Data[i];
  return 0;
}

template <class T, class A>
void TMObjectArray<T, A>::Flush()
{
  for (int i = 0; i < static_cast<int>(Size()); i++)
  {
    __owl_destruct(Data[i]);// call destructor
    Mem.Free(Data[i]);
  }
  ItemCnt = 0;
}

template <class T, class A>
inline void TMObjectArray<T, A>::Grow(int index)
{
  if (static_cast<int>(Reserved) > index)
    return;
  Resize(index - Reserved);
}

template <class T, class A>
void TMObjectArray<T, A>::Resize(int delta)
{
  if (Data)
  {
    T** data = static_cast<T**>(A::ReAlloc(static_cast<void*>(Data), (Reserved + delta) * sizeof(T**)));
    if (!data)
    {
      data = static_cast<T**>(A::Alloc((Reserved + delta) * sizeof(T**)));
      memcpy(data, Data, Reserved * sizeof(T**));
      A::Free(Data);
    }
    Data = data;
  }
  else
    Data = static_cast<T**>(A::Alloc((Reserved + delta) * sizeof(T**)));

  memset(reinterpret_cast<uint8*>(Data) + Reserved * sizeof(T**), 0, delta * sizeof(T**));
  Reserved += delta;
}

template <class T, class A>
inline void TMObjectArray<T, A>::Remove(int index)
{
  memmove(&Data[index], &Data[index + 1], sizeof(T*) * (ItemCnt - index - 1));
  ItemCnt--;
}

//----------------------------------------------------------------------------

template <class T, class A>
inline TMSortedObjectArray<T, A>::TMSortedObjectArray(const TMSortedObjectArray<T, A>& array)
  : TMObjectArray<T, A>(array)
{}

template <class T, class A>
inline TMSortedObjectArray<T, A>& TMSortedObjectArray<T, A>::operator=(const TMSortedObjectArray<T, A>& array)
{
  if (this == &array)
    return *this;
  TMObjectArray<T, A>::operator=(array);
  return *this;
}

template <class T, class A>
int TMSortedObjectArray<T, A>::Add(const T& t)
{
  if (this->ItemCnt >= this->Reserved)
    this->Resize(this->ItemCnt + 1); // on error -> throw xalloc
  unsigned loc = this->ItemCnt++;
  while (loc > 0 && t < *(this->Data[loc - 1]))
  {
    this->Data[loc] = this->Data[loc - 1];
    loc--;
  }
  this->Data[loc] = __owl_construct<T>(this->Mem.Allocate(sizeof(T)), t);
  return loc;
}

template <class T, class A>
inline bool TMSortedObjectArray<T, A>::DestroyItem(const T& t)
{
  const auto index = this->Find(t);
  if (index != this->NPOS)
  {
    T* tmp = this->Data[index];
    this->Remove(index);
    __owl_destruct(tmp);// call destructor
    this->Mem.Free(tmp);
    return true;
  }
  return false;
}

template <class T, class A>
inline bool TMSortedObjectArray<T, A>::HasMember(const T& t) const
{
  return this->Find(t) != this->NPOS;
}

template <class T, class A>
int TMSortedObjectArray<T, A>::Find(const T& t) const
{
  auto lower = 0;
  auto upper = static_cast<int>(this->Size()) - 1;
  while (lower <= upper)
  {
    const auto middle = (lower + upper) / 2;
    if (*(this->Data[middle]) == t)
      return middle;
    if (*(this->Data[middle]) < t)
      lower = middle + 1;
    else
      upper = middle - 1;
  }
  return this->NPOS;
}

//----------------------------------------------------------------------------

template <class T, class R, class A>
TTypedArray<T, R, A>::TTypedArray(const TTypedArray<T, R, A>& array)
  : Data(nullptr)
{
  if (array.Reserved)
    Resize(array.Reserved);
  for (int i = 0; i < static_cast<int>(array.ItemCnt); i++)
    Data[i] = array.Data[i];
  ItemCnt = array.ItemCnt;
}

template <class T, class R, class A>
TTypedArray<T, R, A>& TTypedArray<T, R, A>::operator=(const TTypedArray<T, R, A>& array)
{
  if (this == &array)
    return *this;

  Flush();
  if (array.ItemCnt >= Reserved)
    Resize(array.ItemCnt - Reserved);
  for (int i = 0; i < static_cast<int>(array.ItemCnt); i++)
    Data[i] = array.Data[i];
  ItemCnt = array.ItemCnt;

  return *this;
}

template <class T, class R, class A>
inline int TTypedArray<T, R, A>::Add(R t)
{
  if (ItemCnt >= Reserved)
    Resize(ItemCnt + 1);
  Data[ItemCnt++] = t;
  return ItemCnt - 1; // if fail -> throw xalloc
}

template <class T, class R, class A>
void TTypedArray<T, R, A>::AddAt(R t, uint index)
{
  if (index == Size())
  {
    Add(t);
    return;
  }
  if (index >= Reserved)
    Resize(index + 1); // on error -> throw xalloc
  if (ItemCnt >= Reserved)
    Resize(ItemCnt + 1); // on error -> throw xalloc
  if (index < ItemCnt && ItemCnt)
    memmove(&Data[index + 1], &Data[index], sizeof(T) * (ItemCnt - index));
  Data[index] = t;
  ItemCnt++;
}

template <class T, class R, class A>
bool TTypedArray<T, R, A>::DetachItem(const R t)
{
  const auto index = this->Find(t);
  if (index != this->NPOS)
  {
    Remove(index);
    return true;
  }
  return false;
}

template <class T, class R, class A>
bool TTypedArray<T, R, A>::Detach(int loc)
{
  if (loc < static_cast<int>(Size()))
  {
    Remove(loc);
    return true;
  }
  return false;
}

template <class T, class R, class A>
inline bool TTypedArray<T, R, A>::DestroyItem(R t)
{
  return DetachItem(t);
}

template <class T, class R, class A>
inline bool TTypedArray<T, R, A>::Destroy(int loc)
{
  return Detach(loc);
}

template <class T, class R, class A>
inline bool TTypedArray<T, R, A>::HasMember(R t) const
{
  return this->Find(t) != this->NPOS;
}

template <class T, class R, class A>
int TTypedArray<T, R, A>::Find(R t) const
{
  for (int i = 0; i < static_cast<int>(Size()); i++)
    if(Data[i] == t)
      return i;
  return this->NPOS;
}

template <class T, class R, class A>
inline T& TTypedArray<T, R, A>::operator [](int loc)
{
  PRECONDITION(loc < static_cast<int>(Size()));
  return Data[loc];
}

template <class T, class R, class A>
inline const T& TTypedArray<T, R, A>::operator [](int loc) const
{
  PRECONDITION(loc < static_cast<int>(Size()));
  return Data[loc];
}

template <class T, class R, class A>
void TTypedArray<T, R, A>::ForEach(IterFunc iter, void* args)
{
  for (int i = 0; i < static_cast<int>(Size()); i++)
    (iter)(Data[i], args);
}

template <class T, class R, class A>
T* TTypedArray<T, R, A>::FirstThat(CondFunc cond, void* args) const
{
  for (int i = 0; i < static_cast<int>(Size()); i++)
    if ((cond)(Data[i], args))
      return &Data[i];
  return 0;
}

template <class T, class R, class A>
T* TTypedArray<T, R, A>::LastThat(CondFunc cond, void* args) const
{
  for (int i = static_cast<int>(Size()) - 1; i >= 0; i--)
    if ((cond)(Data[i],args))
      return &Data[i];
  return 0;
}

template <class T, class R, class A>
inline void TTypedArray<T, R, A>::Flush()
{
  ItemCnt = 0;
}

template <class T, class R, class A>
inline void TTypedArray<T, R, A>::Grow(int index)
{
  if (Reserved > static_cast<uint>(index))
    return;
  Resize(index - Reserved);
}

template<class T, class R, class A> void
TTypedArray<T, R, A>::Resize(int delta)
{
  if (Data)
  {
    T* data = static_cast<T*>(this->ReAlloc(static_cast<void*>(Data), (Reserved + delta) * sizeof(T)));
    if (!data) //no ReAlloc, try Alloc instead
    {
      data = static_cast<T*>(this->Alloc((Reserved + delta) * sizeof(T)));
      if (data)
      {
        memcpy(data, Data, Reserved * sizeof(T));
        A::Free(Data);
        Data = data;
      }
      else
        return;
    }
    else
      Data = data;
  }
  else // if (Data)
    Data = static_cast<T*>(this->Alloc((Reserved + delta) * sizeof(T)));

  memset(reinterpret_cast<uint8*>(Data) + Reserved * sizeof(T), 0, delta * sizeof(T));
  Reserved += delta;
}

template <class T, class R, class A>
inline void TTypedArray<T, R, A>::Remove(int index)
{
  memmove(&Data[index], &Data[index + 1], sizeof(T) * (ItemCnt - index - 1));
  ItemCnt--;
}

//----------------------------------------------------------------------------

template <class T, class R, class A>
inline TSTypedArray<T, R, A>::TSTypedArray(const TSTypedArray<T, R, A>& array)
  : TTypedArray<T, R, A>(array)
{}

template <class T, class R, class A>
inline TSTypedArray<T, R, A>& TSTypedArray<T, R, A>::operator=(const TSTypedArray<T, R, A>& array)
{
  if (this == &array)
    return *this;
  TTypedArray<T, R, A>::operator=(array);
  return *this;
}

template <class T, class R, class A>
int TSTypedArray<T, R, A>::Add(R t)
{
  if (this->ItemCnt >= this->Reserved)
    this->Resize(this->ItemCnt + 1); // on error -> throw xalloc
  unsigned loc = this->ItemCnt++;
  while (loc > 0 && t < this->Data[loc - 1])
  {
    this->Data[loc] = this->Data[loc - 1];
    loc--;
  }
  this->Data[loc] = t;
  return loc;
}

template <class T, class R, class A>
bool TSTypedArray<T, R, A>::DetachItem(R t)
{
  const auto index = this->Find(t);
  if (index != this->NPOS)
  {
    this->Remove(index);
    return true;
  }
  return false;
}

template <class T, class R, class A>
inline bool TSTypedArray<T, R, A>::DestroyItem(R t)
{
  return DetachItem(t);
}

template <class T, class R, class A>
inline bool TSTypedArray<T, R, A>::HasMember(R t) const
{
  return this->Find(t) != this->NPOS;
}

template <class T, class R, class A>
int TSTypedArray<T, R, A>::Find(R t) const
{
  auto lower = 0;
  auto upper = static_cast<int>(this->Size()) - 1;
  while (lower <= upper)
  {
    const auto middle = (lower + upper) / 2;
    if (this->Data[middle] == t)
      return middle;
    if (this->Data[middle] < t)
      lower = middle + 1;
    else
      upper = middle - 1;
  }
  return this->NPOS;
}

//----------------------------------------------------------------------------

template <class T, class R, class A>
inline TMIPtrArray<T, R, A>::TMIPtrArray(int upper, int lower, int delta)
  : TTypedArray<T, R, A>(upper, lower, delta)
{}

template <class T, class R, class A>
inline TMIPtrArray<T, R, A>::TMIPtrArray(const TMIPtrArray<T, R, A>& array)
  : TTypedArray<T, R, A>(array)
{}

template <class T, class R, class A>
inline TMIPtrArray<T, R, A>::TMIPtrArray()
{}

template <class T, class R, class A>
inline TMIPtrArray<T, R, A>::~TMIPtrArray()
{
  Flush(true);
}

template <class T, class R, class A>
inline TMIPtrArray<T, R, A>& TMIPtrArray<T, R, A>::operator=(const TMIPtrArray<T, R, A>& array)
{
  if (this == &array)
    return *this;
  Flush(true);
  TTypedArray<T, R, A>::operator=(array);
  return *this;
}

template <class T, class R, class A>
inline bool TMIPtrArray<T, R, A>::DestroyItem(R t)
{
  const auto index = this->Find(t);
  if (index != this->NPOS)
  {
    R tmp = this->Data[index];
    this->Remove(index);
    delete tmp;// using global delete because we didn't allocate it
    return true;
  }
  return false;
}

template <class T, class R, class A>
inline bool TMIPtrArray<T, R, A>::Destroy(int loc)
{
  if (loc < static_cast<int>(this->Size()))
  {
    R tmp = this->Data[loc];
    this->Remove(loc);
    delete tmp;// using global delete because we didn't allocate it
    return true;
  }
  return false;
}

template <class T, class R, class A>
void TMIPtrArray<T, R, A>::Flush(bool del)
{
  if (del)
  {
    for (int i = 0; i < static_cast<int>(this->Size()); i++)
      delete this->Data[i];// using global delete because we didn't allocate it
  }
  TTypedArray<T, R, A>::Flush();
}

//----------------------------------------------------------------------------

template <class T>
inline TIPtrArray<T>::TIPtrArray(int upper, int lower, int delta)
  : TPtrArray<T>(upper, lower, delta)
{}

template <class T>
inline TIPtrArray<T>::TIPtrArray(const TIPtrArray<T>& array)
  : TPtrArray<T>(array)
{}

template <class T>
inline TIPtrArray<T>& TIPtrArray<T>::operator=(const TIPtrArray<T>& array)
{
  if (this == &array)
    return *this;
  Flush(true);
  TPtrArray<T>::operator=(array);
  return *this;
}

template <class T>
inline TIPtrArray<T>::TIPtrArray()
{}

template <class T>
inline TIPtrArray<T>::~TIPtrArray()
{
  Flush(true);
}

template <class T>
void TIPtrArray<T>::Flush(bool del)
{
  if (del)
  {
    for (int i = 0; i < static_cast<int>(this->Size()); i++)
      delete this->Data[i];// using global delete because we didn't allocate it
  }
  TPtrArray<T>::Flush();
}

template <class T>
inline bool TIPtrArray<T>::DestroyItem(T t)
{
  const auto index = this->Find(t);
  if (index != this->NPOS)
  {
    T tmp = this->Data[index];
    this->Remove(index);
    delete tmp;// using global delete because we didn't allocate it
    return true;
  }
  return false;
}

template <class T>
inline bool TIPtrArray<T>::Destroy(int loc)
{
  if (loc < static_cast<int>(this->Size()))
  {
    T tmp = this->Data[loc];
    this->Remove(loc);
    delete tmp;// using global delete because we didn't allocate it
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------

template <class T>
inline TISortedPtrArray<T>::TISortedPtrArray(int upper, int lower, int delta)
  : TSortedPtrArray<T>(upper, lower, delta)
{}

template <class T>
inline TISortedPtrArray<T>::TISortedPtrArray()
{}

template <class T>
inline TISortedPtrArray<T>::~TISortedPtrArray()
{
  Flush(true);
}

template <class T>
inline TISortedPtrArray<T>::TISortedPtrArray(const TISortedPtrArray<T>& array)
  : TSortedPtrArray<T>(array)
{}

template <class T>
inline TISortedPtrArray<T>& TISortedPtrArray<T>::operator=(const TISortedPtrArray<T>& array)
{
  if (this == &array)
    return *this;

  Flush(true);
  TSortedPtrArray<T>::operator=(array);
  return *this;
}

template <class T>
void TISortedPtrArray<T>::Flush(bool del)
{
  if (del)
  {
    for (int i = 0; i < static_cast<int>(this->Size()); i++)
      delete this->Data[i];// using global delete because we didn't allocate it
  }
  TSortedPtrArray<T>::Flush();
}

template <class T>
inline bool TISortedPtrArray<T>::DestroyItem(T t)
{
  const auto index = this->Find(t);
  if (index != this->NPOS)
  {
    T tmp = this->Data[index];
    this->Remove(index);
    delete tmp;// using global delete because we didn't allocate it
    return true;
  }
  return false;
}

template <class T>
inline bool TISortedPtrArray<T>::Destroy(int loc)
{
  if (loc < static_cast<int>(this->Size()))
  {
    T tmp = this->Data[loc];
    this->Remove(loc);
    delete tmp;// using global delete because we didn't allocate it
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------

template <class T>
inline TBaseList<T>::TBaseList()
  : First(nullptr), Last(nullptr), ItemSize(0)
{}

template <class T>
inline TBaseList<T>::TBaseList(int /*upper*/, int /*lower*/, int /*delta*/)
  : First(nullptr), Last(nullptr), ItemSize(0)
{}

template <class T>
inline TBaseList<T>::~TBaseList()
{
  Flush();
}

template <class T>
inline void TBaseList<T>::Add(const T& data)
{
  TBaseNode<T>* node = new TBaseNode<T>(data);
  Add(node);
}

template <class T>
inline void TBaseList<T>::Add(TBaseListIterator<T>* iter, const T& data)
{
  Add(iter ? iter->Cur : 0, new TBaseNode<T>(data));
}

template <class T>
inline void TBaseList<T>::Push(const T& data)
{
  Add(static_cast<TBaseNode<T>*>(nullptr), data);
}

template <class T>
inline void TBaseList<T>::PushBack(const T& data)
{
  Add(Last, data);
}

template <class T>
inline T TBaseList<T>::Pop()
{
  TBaseNode<T>* tmp = First;
  Remove(tmp);
  T data = tmp->Data; delete tmp;
  return data;
}

template <class T>
inline const T& TBaseList<T>::Top()
{
  return First->Data;
}

template <class T>
inline const T& TBaseList<T>::Bottom()
{
  return Last->Data;
}

template <class T>
void TBaseList<T>::Flush()
{
  for (TBaseNode<T>* node = First; node;)
  {
    TBaseNode<T>* tmp = node;
    node = node->Next;
    delete tmp;
  }
  First = Last = nullptr;
  ItemSize = 0;
}

template <class T>
bool TBaseList<T>::DetachItem(const T& t)
{
  TBaseNode<T>* item = this->Find(t);
  if (item && Remove(item))
  {
    delete item;
    return true;
  }
  return false;
}

template <class T>
bool TBaseList<T>::Detach(int loc)
{
  if (loc < static_cast<int>(Size()))
  {
    int counter = 0;
    for (TBaseNode<T>* i = First; i; i = i->Next, counter++)
    {
      if (counter == loc)
      {
        Remove(i);
        return true;
      }
    }
  }
  return false;
}

template <class T>
inline bool TBaseList<T>::HasMember(const T& t) const
{
  return this->Find(t) != 0;
}

template <class T>
TBaseNode<T>* TBaseList<T>::Find(const T& t) const
{
  for (TBaseNode<T>* i = First; i; i = i->Next)
    if (i->Data == t)
      return i;
  return nullptr;
}

template <class T>
void TBaseList<T>::ForEach(IterFunc iter, void* args)
{
  for (TBaseNode<T>* i = First; i; i = i->Next)
    (iter) (i->Data, args);
}

template <class T>
T* TBaseList<T>::FirstThat(CondFunc cond, void* args) const
{
  for (TBaseNode<T>* i = First; i; i = i->Next)
    if ((cond) (i->Data, args))
      return &i->Data;
  return nullptr;
}

template <class T>
T* TBaseList<T>::LastThat(CondFunc cond, void* args) const
{
  for (TBaseNode<T>* i = Last; i; i = i->Prev)
    if ((cond) (i->Data, args))
      return &i->Data;
  return 0;
}

template <class T>
bool TBaseList<T>::Remove(TBaseNode<T>* node)
{
  if (!node)
    return false;
  if (node == First)
  {
    First = node->Next;
    node->Next = nullptr;
    if (First)
      First->Prev = nullptr;
  }
  else if (node == Last)
  {
    Last = node->Prev;
    node->Prev = nullptr;
    if (Last)
      Last->Next = nullptr;
  }
  else
  {
    node->Prev->Next = node->Next;
    node->Next->Prev = node->Prev;
    node->Prev = nullptr;
    node->Next = nullptr;
  }

  ItemSize--;
  return true;
}

template <class T>
inline int TBaseList<T>::Add(TBaseNode<T>* node, const T& data)
{
  return Add(node, new TBaseNode<T>(data));
}

template <class T>
int TBaseList<T>::Add(TBaseNode<T>* node)
{
  if (!node)
    return this->NPOS;
  if (!First)
  {
    First = node;
    Last = node;
  }
  else
  {
    node->Prev = Last;
    Last->Next = node;
    Last = node;
  }
  return ++ItemSize;
}

template <class T>
int TBaseList<T>::Add(TBaseNode<T>* item, TBaseNode<T>* node)
{
  if (!node)
    return this->NPOS;
  if (!First)
  {
    return Add(node);
  }
  if (!item)
  {
    node->Next = First;
    First->Prev = node;
    First = node;
  }
  else
  {
    if (item == Last)
      Last = node;
    node->Prev = item;
    node->Next = item->Next;
    item->Next = node;
    if (node->Next)
      node->Next->Prev = node;
  }
  return ++ItemSize;
}

} // OWL namespace

//----------------------------------------------------------------------------
// Inlined overloads of global new
//

inline void* operator new(size_t sz, const owl::TStandardAllocator&)
{
  return ::operator new(sz);
}

inline void* operator new(size_t sz, const owl::TLocalAllocator&)
{
  return static_cast<void*>(::LocalAlloc(LPTR, sz));
}

inline void* operator new(size_t sz, const owl::TGlobalAllocator&)
{
  return static_cast<void*>(::GlobalLock(::GlobalAlloc(GPTR, sz)));
}

template <class A>
inline void* operator new(size_t sz, owl::TMMemStack<A>& m)
{
  return m.Allocate(sz);
}

#endif
