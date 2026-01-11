//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// General Registry access & registration implementation
///   TRegKey, TRegValue, TRegKeyIterator, TRegValueIterator
///   TXRegistry
///   TRegItem, TRegList, TRegLink - associative array of localizable string parameters
///   OC registry functions
//----------------------------------------------------------------------------

#if !defined(OWL_REGISTRY_H)
#define OWL_REGISTRY_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/private/checks.h>
#include <owl/except.h>
#include <owl/wsysinc.h>
#include <owl/lclstrng.h>
#include <owl/private/memory.h>
#include <owl/private/strmdefs.h>
#include <vector>
#include <optional>
#include <type_traits>
#include <cstddef>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup winsys
/// @{

//----------------------------------------------------------------------------
// Direct registry access classes. Can read & iterate as well as create and
// write keys and value-data pairs
//

class _OWLCLASS TRegKey;
class _OWLCLASS TRegValueIterator;

//
/// Encapsulates a value-data entry within one registration key.
//
class _OWLCLASS TRegValue
{
public:
  TRegValue(const TRegKey& key, LPCTSTR name);
  TRegValue(const TRegKey& key, const tstring& name);
  explicit TRegValue(const TRegValueIterator& iter);

#if defined(BI_COMP_BORLANDC)

  // Circumvent bug in std::optional (requires default constructor for the contained object type).
  // TODO: Remove this when the library is standard compliant.
  //
  TRegValue();

#endif

  /// \name Accessors
  /// @{

  LPCTSTR GetName() const;
  uint32 GetDataType() const;
  uint32 GetDataSize() const;
  const uint8* GetData() const;

  operator uint32() const;
  operator uint64() const;
  operator LPCTSTR() const;

  /// @}
  /// \name Mutators
  /// @{

  long Set(uint32 type, const uint8* data, uint32 dataSize);
  long Set(uint32 value);
  long Set(uint64 value);
  long Set(LPCTSTR value);
  long Set(const tstring& value);

  TRegValue& operator =(uint32 value);
  TRegValue& operator =(uint64 value);
  TRegValue& operator =(LPCTSTR value);
  TRegValue& operator =(const tstring& value);

  long Delete();

  /// @}

private:
  const TRegKey& Key; ///< Key that this value lives in
  tstring Name; ///< Value name; if empty, this object represents the default value for the associated key.

  uint32 DataType; ///< Type code for value data
  mutable std::vector<uint8> Data; ///< Value data; mutable to support lazy retrieval (see RetrieveOnDemand).
  uint32 DataSize; ///< Size in bytes of Data

  long QueryTypeAndSize();
  void RetrieveOnDemand() const;
};

class _OWLCLASS TRegKeyIterator;

//
/// Encapsulates a registration key in the Windows Registry.
//
class _OWLCLASS TRegKey
{
public:
  typedef HKEY THandle;

  //
  /// Enumeration used to specify whether a key should be created (or simply opened).
  //
  enum TCreateOK
  {
    CreateOK, ///< Create key if it does not exist.
    NoCreate ///< Don't create, simply open.
  };

  /// \name Accessors to system predefined registry keys
  /// @{

  static auto GetClassesRoot() -> TRegKey&;
  static auto GetClassesRootClsid() -> TRegKey&;
  static auto GetCurrentConfig() -> TRegKey&;
  static auto GetCurrentUser() -> TRegKey&;
  static auto GetLocalMachine() -> TRegKey&;
  static auto GetPerformanceData() -> TRegKey&;
  static auto GetUsers() -> TRegKey&;

  /// @}

#if defined(OWL5_COMPAT)

    /// \name Deprecated alternative accessors to system predefined registry keys
    /// @{

  static auto ClassesRoot() -> TRegKey& { return GetClassesRoot(); }
  static auto ClassesRootClsid() -> TRegKey& { return GetClassesRootClsid(); }
  static auto CurrentConfig() -> TRegKey& { return GetCurrentConfig(); }
  static auto CurrentUser() -> TRegKey& { return GetCurrentUser(); }
  static auto LocalMachine() -> TRegKey& { return GetLocalMachine(); }
  static auto PerformanceData() -> TRegKey& { return GetPerformanceData(); }
  static auto Users() -> TRegKey& { return GetUsers(); }

  /// @}

#endif

  TRegKey(THandle baseKey, tstring keyName, REGSAM samDesired = KEY_ALL_ACCESS, TCreateOK createOK = CreateOK);
  TRegKey(THandle baseKey, LPCTSTR keyName, REGSAM samDesired = KEY_ALL_ACCESS, TCreateOK createOK = CreateOK);
  explicit TRegKey(const TRegKeyIterator& iter, REGSAM samDesired = KEY_ALL_ACCESS);
  TRegKey(THandle aliasKey, bool shouldClose, tstring keyName);
  explicit TRegKey(THandle aliasKey, bool shouldClose = false, LPCTSTR keyName = nullptr);

  TRegKey(TRegKey&&) noexcept;
  TRegKey(const TRegKey&) = delete;
  TRegKey& operator =(const TRegKey&) = delete;

#if defined(BI_COMP_BORLANDC)

  // Circumvent bug in std::optional (requires default constructor for the contained object type).
  // TODO: Remove this when the library is standard compliant.
  //
  TRegKey() = default;

#endif

  ~TRegKey();

  operator THandle() const;
  auto GetHandle() const -> THandle { return Key; }

  long DeleteKey(LPCTSTR subKeyName);
  long DeleteKey(const tstring& subKeyName)
  { return DeleteKey(subKeyName.c_str()); }

  long NukeKey(LPCTSTR subKeyName);
  long NukeKey(const tstring& subKeyName)
  { return NukeKey(subKeyName.c_str()); }

  long Flush() const;

  /// \name Security attributes
  /// @{

  //
  /// Encapsulates a SECURITY_DESCRIPTOR.
  ///
  /// This class is used by TRegKey::GetSecurity(SECURITY_INFORMATION) const and 
  /// TRegKey::SetSecurity(SECURITY_INFORMATION, const TSecurityDescriptor&) const.
  ///
  /// \sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-security_descriptor">
  /// SECURITY_DESCRIPTOR</a> in the Windows API.
  ///
  class _OWLCLASS TSecurityDescriptor
  {
  public:
    TSecurityDescriptor(const TRegKey& key, SECURITY_INFORMATION infoRequested);

    auto IsValid() const noexcept -> bool;

    auto GetControl() const -> SECURITY_DESCRIPTOR_CONTROL;
    auto GetLength() const -> DWORD;
    auto GetDacl() const -> std::optional<PACL>;
    auto GetGroup() const -> PSID;
    auto GetOwner() const -> PSID;
    auto GetRevision() const -> DWORD;
    auto GetRmControl() const -> std::optional<UCHAR>;
    auto GetSacl() const -> std::optional<PACL>;

    void Initialize(DWORD revision = SECURITY_DESCRIPTOR_REVISION);
    void RemoveDacl();
    void RemoveGroup();
    void RemoveOwner();
    void RemoveRmControl();
    void RemoveSacl();
    void SetDacl(PACL dacl, bool daclDefaulted = false);
    void SetGroup(PSID group, bool groupDefaulted = false);
    void SetOwner(PSID owner, bool ownerDefaulted = false);
    void SetRmControl(UCHAR rmControl);
    void SetSacl(PACL sacl, bool saclDefaulted = false);

    /// \name Security descriptor buffer access
    /// @{

    auto GetData() noexcept -> SECURITY_DESCRIPTOR* { return reinterpret_cast<SECURITY_DESCRIPTOR*>(Data.data()); }
    auto GetData() const noexcept -> const SECURITY_DESCRIPTOR* { return reinterpret_cast<const SECURITY_DESCRIPTOR*>(Data.data()); }
    auto GetDataSize() const noexcept -> int { return static_cast<int>(Data.size()); }

    /// @}
    /// \name SECURITY_DESCRIPTOR_CONTROL queries
    /// @{

    auto IsFlagSet(SECURITY_DESCRIPTOR_CONTROL mask) const { return (GetControl() & mask) == mask; } ///< Returns true if all the set bits in the given mask are set in the SECURITY_DESCRIPTOR_CONTROL structure.

    auto IsAbsolute() const { return !IsFlagSet(SE_SELF_RELATIVE); } ///< Returns the inverse of the SE_SELF_RELATIVE flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsDaclAutoInheritRequired() const { return IsFlagSet(SE_DACL_AUTO_INHERIT_REQ); } ///< Returns the SE_DACL_AUTO_INHERIT_REQ flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsDaclAutoInherited() const { return IsFlagSet(SE_DACL_AUTO_INHERITED); } ///< Returns the SE_DACL_AUTO_INHERITED flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsDaclDefaulted() const { return IsFlagSet(SE_DACL_DEFAULTED); } ///< Returns the SE_DACL_DEFAULTED flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsDaclPresent() const { return IsFlagSet(SE_DACL_PRESENT); } ///< Returns the SE_DACL_PRESENT flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsDaclProtected() const { return IsFlagSet(SE_DACL_PROTECTED); } ///< Returns the SE_DACL_PROTECTED flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsGroupDefaulted() const { return IsFlagSet(SE_GROUP_DEFAULTED); } ///< Returns the SE_GROUP_DEFAULTED flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsOwnerDefaulted() const { return IsFlagSet(SE_OWNER_DEFAULTED); } ///< Returns the SE_OWNER_DEFAULTED flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsRmControlValid() const { return IsFlagSet(SE_RM_CONTROL_VALID); } ///< Returns the SE_RM_CONTROL_VALID flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsSaclAutoInheritRequired() const { return IsFlagSet(SE_SACL_AUTO_INHERIT_REQ); } ///< Returns the SE_SACL_AUTO_INHERIT_REQ flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsSaclAutoInherited() const { return IsFlagSet(SE_SACL_AUTO_INHERITED); } ///< Returns the SE_SACL_AUTO_INHERITED flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsSaclDefaulted() const { return IsFlagSet(SE_SACL_DEFAULTED); } ///< Returns the SE_SACL_DEFAULTED flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsSaclPresent() const { return IsFlagSet(SE_SACL_PRESENT); } ///< Returns the SE_SACL_PRESENT flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsSaclProtected() const { return IsFlagSet(SE_SACL_PROTECTED); } ///< Returns the SE_SACL_PROTECTED flag in the SECURITY_DESCRIPTOR_CONTROL structure.
    auto IsSelfRelative() const { return IsFlagSet(SE_SELF_RELATIVE); } ///< Returns the SE_SELF_RELATIVE flag in the SECURITY_DESCRIPTOR_CONTROL structure.

    /// @}

  private:
    std::vector<std::byte> Data;
  };

  auto GetSecurity(SECURITY_INFORMATION infoRequested, PSECURITY_DESCRIPTOR secDesc, DWORD* secDescSize) const -> long;
  auto GetSecurity(SECURITY_INFORMATION infoRequested) const -> TSecurityDescriptor;
  auto SetSecurity(SECURITY_INFORMATION infoToSet, PSECURITY_DESCRIPTOR secDesc) const -> long;
  void SetSecurity(SECURITY_INFORMATION infoToSet, const TSecurityDescriptor& s) const;

  /// @}

  // Saving & loading of this key & subKeys
  //
  long Save(LPCTSTR fileName);
  long Save(const tstring& fileName)
  { return Save(fileName.c_str()); }

  long LoadKey(LPCTSTR subKeyName, LPCTSTR fileName);
  long LoadKey(const tstring& subKeyName, const tstring& fileName)
  { return LoadKey(subKeyName.c_str(), fileName.c_str()); }

  long ReplaceKey(LPCTSTR subKeyName, LPCTSTR newFileName, LPCTSTR oldFileName);
  long ReplaceKey(const tstring& subKeyName, const tstring& newFileName, const tstring& oldFileName)
  { return ReplaceKey(subKeyName.c_str(), newFileName.c_str(), oldFileName.c_str()); }

  long Restore(LPCTSTR fileName, uint32 options = 0);
  long Restore(const tstring& fileName, uint32 options = 0)
  { return Restore(fileName.c_str(), options); }

  long UnLoadKey(LPCTSTR subKeyName);
  long UnLoadKey(const tstring& subKeyName)
  { return UnLoadKey(subKeyName.c_str()); }

  long QueryInfo(LPTSTR class_, DWORD* classSize,
    DWORD* subkeyCount, DWORD* maxSubkeyNameSize, DWORD* maxSubkeyClassSize,
    DWORD* valueCount, DWORD* maxValueNameSize, DWORD* maxValueDataSize,
    DWORD* securityDescriptorSize, PFILETIME lastWriteTime) const;

  //
  /// Data structure returned by QueryInfo.
  //
  struct TInfo
  {
    tstring Class;
    DWORD SubkeyCount;
    DWORD MaxSubkeyNameSize;
    DWORD MaxSubkeyClassSize;
    DWORD ValueCount;
    DWORD MaxValueNameSize;
    DWORD MaxValueDataSize; ///< In bytes.
    DWORD SecurityDescriptorSize;
    FILETIME LastWriteTime;
  };

  auto QueryInfo() const -> TInfo;

  LPCTSTR GetName() const;
  auto HasSubkey(const tstring& keyName) const -> bool;
  auto GetSubkey(const tstring& keyName, REGSAM samDesired = KEY_READ) const->std::optional<TRegKey>;
  uint32 GetSubkeyCount() const;
  auto HasValue(const tstring& valueName) const -> bool;
  auto GetValue(const tstring& valueName) const->std::optional<TRegValue>;
  uint32 GetValueCount() const;

  //
  /// Returns the value of the given name, if it exists, or the given default value, otherwise.
  /// A conversion of the TRegValue to type T is attempted. The provided default value must be
  /// convertible to T. The types supported for T are owl::uint32, owl::uint64 and owl::tstring.
  ///
  /// \note T can not be LPCTSTR, because that would return a pointer to a dead object (the
  /// TRegValue object looked up is local to the function). Use owl::tstring instead.
  //
  template <typename T>
  auto GetValueOrDefault(const tstring& valueName, T defaultValue) const
  {
    static_assert(!std::is_pointer_v<T>, "Return type can not be pointer.");
    const auto v = GetValue(valueName);
    return v ? static_cast<T>(*v) : defaultValue;
  }

  // Friend iterators only...?
  // TODO: Provide string-returning overload.
  //
  long EnumKey(int index, LPTSTR subKeyName, int subKeyNameSize) const;

#if defined(OWL5_COMPAT)

  // Older, nameless subkey+(Default) value access
  //
  long SetDefValue(LPCTSTR subkeyName, uint32 type,
    LPCTSTR data, uint32 dataSize);
  long SetDefValue(const tstring& subkeyName, uint32 type, const tstring& data, uint32 dataSize)
  { return SetDefValue(subkeyName.c_str(), type, data.c_str(), dataSize); }

  long QueryDefValue(LPCTSTR subkeyName, LPTSTR data,
    uint32* dataSize) const;
  long QueryDefValue(const tstring& subkeyName, LPTSTR data, uint32* dataSize) const
  { return QueryDefValue(subkeyName.c_str(), data, dataSize); }

#endif

  // Newer, named value set & get functions. Subkeys must be opened
  //
  long SetValue(LPCTSTR valName, uint32 type, const uint8* data, uint32 dataSize) const;
  long SetValue(const tstring& valName, uint32 type, const uint8* data, uint32 dataSize) const
  { return SetValue(valName.c_str(), type, data, dataSize); }

  long SetValue(LPCTSTR valName, uint32 data) const;
  long SetValue(const tstring& valName, uint32 data) const
  { return SetValue(valName.c_str(), data); }

  long SetValue(LPCTSTR valName, const tstring& value) const;
  long SetValue(const tstring& valName, const tstring& value) const
  { return SetValue(valName.c_str(), value); }

  long QueryValue(LPCTSTR valName, uint32* type, uint8* data, uint32* dataSize) const;
  long QueryValue(const tstring& valName, uint32* type, uint8* data, uint32* dataSize) const
  { return QueryValue(valName.c_str(), type, data, dataSize); }

  long DeleteValue(LPCTSTR valName) const;
  long DeleteValue(const tstring& valName) const
  { return DeleteValue(valName.c_str()); }

  // Friend iterators only...?
  // TODO: Provide value-returning overload.
  //
  long EnumValue(int index, LPTSTR valueName, uint32& valueNameSize, uint32* type = nullptr, uint8* data = nullptr, uint32* dataSize = nullptr) const;

protected:
  THandle Key; ///< This Key's Handle.
  tstring Name; ///< This Key's Name.
  DWORD SubkeyCount; ///< Number of subkeys.
  DWORD ValueCount; ///< Number of value entries.
  bool ShouldClose; ///< Should this key be closed on destruction.
};

//
/// Iterator for walking thru the subkeys of a key.
//
class _OWLCLASS TRegKeyIterator
{
public:
  TRegKeyIterator(const TRegKey& key);

  operator bool();

  uint32 operator ++();
  uint32 operator ++(int);
  uint32 operator --();
  uint32 operator --(int);
  uint32 operator [](int index);

  int Current() const;
  const TRegKey& BaseKey() const;

  void Reset();

private:
  const TRegKey& Key;
  int Index;
};

//
/// Iterator for walking through the values of a key.
//
class _OWLCLASS TRegValueIterator
{
public:
  TRegValueIterator(const TRegKey& regKey);

  operator bool();

  uint32 operator ++();
  uint32 operator ++(int);
  uint32 operator --();
  uint32 operator --(int);
  uint32 operator [](int index);

  int Current() const;
  const TRegKey& BaseKey() const;

  void Reset();

private:
  const TRegKey& Key;
  int Index;
};

//----------------------------------------------------------------------------

//
/// Thrown for errors within the Registry classes.
//
class _OWLCLASS TXRegistry : public TXOwl
{
public:
  TXRegistry(const tstring& context, const TRegKey& key, long errorCode = 0)
    : TXOwl{context}, KeyName{key.GetName()}, ErrorCode{errorCode}
  {}

  TXRegistry(const tstring& context, tstring keyName, long errorCode = 0)
    : TXOwl{context}, KeyName{std::move(keyName)}, ErrorCode{errorCode}
  {}

  auto GetKeyName() const -> const tstring& { return KeyName; }
  auto GetErrorCode() const -> long { return ErrorCode; }

private:
  tstring KeyName;
  long ErrorCode;
};


//----------------------------------------------------------------------------
// Registration parameter structures and formatting functions
//

//
/// Used internally to provide buffers for formating registration strings.
//
class _OWLCLASS TRegFormatHeap
{
public:
  TRegFormatHeap();
  ~TRegFormatHeap();
  tchar* Alloc(int spaceNeeded);

private:
  struct TBlock
  {
    TBlock* Next; ///< Next heap in chain.
    tchar Data[1]; ///< Allocated memory starts here.
  };
  TBlock* Head;
};

//
/// A single registration list entry.
///
/// TRegItem defines localizable values for parameters or subkeys. These values can
/// be passed to TLocaleString, which defines a localizable substitute for char*.
/// TRegItem contains several undocumented functions that are used privately by the
/// registration macros REGFORMAT and REGSTATUS.
//
struct _OWLCLASS TRegItem
{
  const char* Key; ///< Non-localized parameter or registry subkey
  TLocaleString Value; ///< Localizable value for parameter or subkey

  // Used privately by REGFORMAT, REGSTATUS macros.
  //
  static tchar* RegFormat(int f, int a, int t, int d, TRegFormatHeap& heap);
  static tchar* RegFormat(LPCTSTR f, int a, int t, int d, TRegFormatHeap& heap);
  static tchar* RegFormat(const tstring& f, int a, int t, int d, TRegFormatHeap& heap) { return RegFormat(f.c_str(), a, t, d, heap); }
  static tchar* RegFlags(long flags, TRegFormatHeap& heap);
  static tchar* RegVerbOpt(int mf, int sf, TRegFormatHeap& heap);
  static void    OverflowCheck();
};

//
/// A registration parameter table, composed of a list of TRegItems.
//
/// Holds an array of items of type TRegItem. Provides functionality that lets you
/// access each item in the array and return the name of the item. Instead of
/// creating a TRegList directly, you can use ObjectWindows' registration macros to
/// build a TRegList for you.
//
class _OWLCLASS TRegList
{
public:
  TRegList(TRegItem* _list);

  LPCTSTR Lookup(LPCSTR key, TLangId lang = TLocaleString::UserDefaultLangId);
  LPCTSTR Lookup(const std::string& key, TLangId lang = TLocaleString::UserDefaultLangId)
  { return Lookup(key.c_str(), lang); }

  TLocaleString& LookupRef(LPCSTR key);
  TLocaleString& LookupRef(const std::string& key)
  { return LookupRef(key.c_str()); }

  LPCTSTR operator[](LPCSTR key);
  LPCTSTR operator[](const std::string& key)
  { return operator[](key.c_str()); }

  TRegItem* Items;
};

//
/// A linked structure in which each node points to a list of TRegList
/// objects (or TRegList-derived objects) or TDocTemplate objects.
///
/// Each object has an item name and a string value associated with the item name. The structure
/// forms a typical linked list as the following diagram illustrates:
/// \image html bm251.BMP
///
/// A TDocTemplate object uses the following variation of the TRegLink structure:
/// \image html bm252.BMP
//
class _OWLCLASS TRegLink
{
public:
  TRegLink(TRegList& regList, TRegLink*& head);
  virtual ~TRegLink();

  TRegLink* GetNext() const;
  void SetNext(TRegLink* link);
  TRegList& GetRegList() const;

  static void AddLink(TRegLink** head, TRegLink* newLink);
  static bool RemoveLink(TRegLink** head, TRegLink* remLink);

protected:
  TRegLink(); ///< Derived class must fill in ptrs
  TRegLink* Next; ///< Next RegLink
  TRegList* RegList; ///< Pointer to registration parameter table
};

//
// Registration parameter table macro definitions
//
#define BEGIN_REGISTRATION(regname) \
  extern TRegItem regname##_list[];\
  extern TRegFormatHeap __regHeap;\
  TRegList regname(regname##_list);\
  TRegItem regname##_list[] = {

#define END_REGISTRATION {0,{0}} };

#define REGDATA(var,val) {#var, {val}},
#define REGXLAT(var,val) {#var, {AUTOLANG_XLAT val}},
#define REGITEM(key,val) {" " key, {val}},
#define REGFORMAT(i,f,a,t,d) {"format" #i,{TRegItem::RegFormat(f,a,t,d,__regHeap)}},
#define REGSTATUS(a,f) {"aspect" #a, {TRegItem::RegFlags(f,__regHeap)}},
#define REGVERBOPT(v,mf,sf) {#v "opt",{TRegItem::RegVerbOpt(mf,sf,__regHeap)}},
#define REGICON(i) {"iconindex",{TRegItem::RegFlags(i,__regHeap)}},
#define REGDOCFLAGS(i) {"docflags",{TRegItem::RegFlags(i,__regHeap)}},

//
// Buffer size is no longer needed, use the macro below
//
#define REGISTRATION_FORMAT_BUFFER(n) \
  TRegFormatHeap __regHeap;

#define REGISTRATION_FORMAT_HEAP \
  TRegFormatHeap __regHeap;

//----------------------------------------------------------------------------
// Clipboard and registry data transfer format definitions
//

const int ocrVerbLimit = 8; ///< Maximum number of verbs registered per class.
const int ocrFormatLimit = 8; ///< Maximum number of data formats per class.

//
/// Format: standard clipboard numeric format, or name of custom format.
//
enum ocrClipFormat
{
  ocrText = 1,  ///< CF_TEXT
  ocrBitmap = 2,  ///< CF_BITMAP
  ocrMetafilePict = 3,  ///< CF_METAFILEPICT
  ocrSylk = 4,  ///< CF_SYLK
  ocrDif = 5,  ///< CF_DIF
  ocrTiff = 6,  ///< CF_TIFF
  ocrOemText = 7,  ///< CF_OEMTEXT
  ocrDib = 8,  ///< CF_DIB
  ocrPalette = 9,  ///< CF_PALETTE
  ocrPenData = 10,  ///< CF_PENDATA
  ocrRiff = 11,  ///< CF_RIFF
  ocrWave = 12,  ///< CF_WAVE
  ocrUnicodeText = 13,  ///< CF_UNICODETEXT  Win32 only
  ocrEnhMetafile = 14,  ///< CF_ENHMETAFILE  Win32 only
};

#define ocrRichText          _T("Rich Text Format")
#define ocrEmbedSource       _T("Embed Source")
#define ocrEmbeddedObject    _T("Embedded Object")
#define ocrLinkSource        _T("Link Source")
#define ocrObjectDescriptor  _T("Object Descriptor")
#define ocrLinkSrcDescriptor _T("Link Source Descriptor")

//
/// Aspect: view types supported by transfer.
//
enum ocrAspect
{
  ocrContent = 1, ///< DVASPECT_CONTENT - normal display representation.
  ocrThumbnail = 2, ///< DVASPECT_THUMBNAIL - picture appropriate for browser.
  ocrIcon = 4, ///< DVASPECT_ICON - iconized representation of object.
  ocrDocPrint = 8, ///< DVASPECT_DOCPRINT - print preview representation.
};

//
/// Medium: means of data transfer.
//
enum ocrMedium
{
  ocrNull = 0,
  ocrHGlobal = 1, ///< TYMED_HGLOBAL - global memory handle.
  ocrFile = 2, ///< TYMED_FILE - data as contents of file.
  ocrIStream = 4, ///< TYMED_ISTREAM - instance of an IStream object.
  ocrIStorage = 8, ///< TYMED_ISTORAGE - streams within an instance of IStorage.
  ocrGDI = 16, ///< TYMED_GDI - GDI object in global handle.
  ocrMfPict = 32, ///< TYMED_MFPICT - CF_METAFILEPICT containing global handle.
  ocrStaticMed = 1024 ///< OLE 2 static object.
};

//
/// Direction: transfer directions supported.
//
enum ocrDirection
{
  ocrGet = 1,
  ocrSet = 2,
  ocrGetSet = 3,
};

//----------------------------------------------------------------------------
// Miscellaneous registry definitions
//

//
/// IOleObject miscellaneous status flags, defined for each or all aspects.
//
enum ocrObjectStatus
{
  ocrRecomposeOnResize = 1, ///< Request redraw on container resize.
  ocrOnlyIconic = 2, ///< Only useful context view is Icon.
  ocrInsertNotReplace = 4, ///< Should not replace current select.
  ocrStatic = 8, ///< Object is an OLE static object.
  ocrCantLinkInside = 16, ///< Should not be the link source.
  ocrCanLinkByOle1 = 32, ///< Only used in OBJECTDESCRIPTOR.
  ocrIsLinkObject = 64, ///< Set by OLE2 link for OLE1 compat.
  ocrInsideOut = 128, ///< Can be activated concurrently.
  ocrActivateWhenVisible = 256, ///< Hint to cntnr when ocrInsideOut set.
  ocrRenderingIsDeviceIndependent = 512, ///< No decisions made based on target.
  ocrNoSpecialRendering = 512, ///< Older enum for previous entry.
};

//
/// IOleObject verb menu flags.
//
enum ocrVerbMenuFlags
{
  ocrGrayed = 1, ///< MF_GRAYED
  ocrDisabled = 2, ///< MF_DISABLED
  ocrChecked = 8, ///< MF_CHECKED
  ocrMenuBarBreak = 32, ///< MF_MENUBARBREAK
  ocrMenuBreak = 64, ///< MF_MENUBAR
};

//
/// IOleObject verb attribute flags.
//
enum ocrVerbAttributes
{
  ocrNeverDirties = 1, ///< Verb can never cause object to become dirty.
  ocrOnContainerMenu = 2, ///< Only useful context view is Icon.
};

//
// Entries for <usage> registration parameter, class factory registration mode
//
#define ocrSingleUse     _T("1") ///< Single client per instance.
#define ocrMultipleUse   _T("2") ///< Multiple clients per instance.
#define ocrMultipleLocal _T("3") ///< Multiple clients, separate inproc server.

//----------------------------------------------------------------------------
// High-level table based registration support
//

//
/// List of parameterized template strings that represent the actual entries to be registered.
///
/// List is indexed from 1 for used with param-list template activation strings. See TRegParamList below.
//
// TODO: Add string-class support in constructor.
//
class _OWLCLASS TRegTemplateList
{
public:
  TRegTemplateList(TRegKey& basekey, LPCTSTR _list[]);
  ~TRegTemplateList();

  int GetCount() const;
  TRegKey& GetBaseKey();

  LPCTSTR operator [](int i);

  // Enable/Disable and activate templates
  //
  void DisableAll();
  void EnableAll();
  void Enable(int i);
  void Enable(LPCTSTR set);
  void Enable(const tstring& set) { Enable(set.c_str()); }

  void Activate(int i);
  void Activate(LPCTSTR set);
  void Activate(const tstring& set) { Activate(set.c_str()); }

  bool IsActive(int i) const;

private:
  TRegKey& BaseKey; ///< Registry key on which these templates are based.
  const tchar** List; ///< List of templates.
  int Count; ///< Number of templates in list.
  int8* EnabledFlags; // Which templates are currently enabled/active.
};

//
/// A list of param entries for use as an intermediate between a TRegList and
/// the actual template list used to generate the registration.
///
/// Provides default values in 'Default', and tags required templates using octal char entries in
/// 'TemplatesNeeded'.
//
class _OWLCLASS TRegParamList
{
public:
  struct TEntry
  {
    LPCTSTR Param; ///< Substituted parameter name.
    LPCTSTR Default; ///< Default value, 0 if no default & param is required.
    LPCTSTR TemplatesNeeded; ///< Octal string list of template indices to activate.
  };

  TRegParamList(const TEntry*);
  ~TRegParamList();

  int Find(LPCTSTR param); 
  int Find(const tstring& param) { return Find(param.c_str()); }
  int GetCount() const;
  const tchar*& Value(int i);
  void ResetDefaultValues();

  const TEntry& operator [](int i) const;

private:
  const TEntry* List;
  int Count;
  const tchar** Values;
};

//
/// High level symbol-based registry entry manipulation.
///
//
// TODO: Add string-class support in constructor.
//
class _OWLCLASS TRegSymbolTable
{
public:
  TRegSymbolTable(TRegKey& basekey, LPCTSTR tplList[], const TRegParamList::TEntry* paramList);

  void Init(LPCTSTR filter);
  void UpdateParams(TLangId lang, TRegItem* item);
  void StreamOut(TRegItem* item, tostream& out);

  TRegTemplateList Templates;
  TRegParamList Params;
  int UserKeyCount;
};

//
/// TRegistry provides high level stream and list access to the registry.
//
class _OWLCLASS TRegistry
{
public:
  static int Validate(TRegKey& baseKey, tistream& in);  ///< Returns number of mismatched entries
  static void Update(TRegKey& baseKey, tistream& in);    ///< Writes lines to registry

#pragma pack(push,4)

  struct TUnregParams
  {
    tchar Prepend; ///< Optional tchar to prepend to key before removing.
    LPCTSTR Name; ///< Name of param.
    TRegKey* BaseKey; ///< Key that that the param is based upon.
  };

#pragma pack(pop)

  static int Unregister(TRegList& regInfo, const TUnregParams* params, const TRegItem* overrides = nullptr);
};

/// @}

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// TRegKey inlines
//

//
/// Returns the HANDLE identifying this registry key.
//
inline TRegKey::operator THandle() const
{
  return Key;
}

//
/// Enumerates the subkeys of this registry key.
//
inline long TRegKey::EnumKey(int index, LPTSTR subKeyName, int subKeyNameSize) const
{
  return ::RegEnumKey(Key, index, subKeyName, subKeyNameSize);
  //::RegEnumKeyEx(); ??
}

//
/// Deletes the specified subkey of this registry key.
//
inline long TRegKey::DeleteKey(LPCTSTR subKeyName)
{
  return ::RegDeleteKey(Key, subKeyName);
}

//
/// Writes the attribute of this key in the registry
//
inline long TRegKey::Flush() const
{
  return ::RegFlushKey(Key);
}

//
/// Retrieves a copy of the security descriptor protecting this registry key.
///
/// \sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetkeysecurity">
/// RegGetKeySecurity</a> in the Windows API.
//
inline auto TRegKey::GetSecurity(SECURITY_INFORMATION infoRequested, PSECURITY_DESCRIPTOR secDesc, DWORD* secDescSize) const -> long
{
  return ::RegGetKeySecurity(Key, infoRequested, secDesc, secDescSize);
}

//
/// Retrieves a copy of the security descriptor protecting this registry key.
/// \param infoRequested indicates the requested security information.
/// \returns If successful, a TRegKey::TSecurityDescriptor is returned.
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetkeysecurity">
/// RegGetKeySecurity</a> in the Windows API.
//
inline auto TRegKey::GetSecurity(SECURITY_INFORMATION infoRequested) const -> TSecurityDescriptor
{
  return TSecurityDescriptor{*this, infoRequested};
}

//
/// Sets the security descriptor of this key.
///
/// \sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetkeysecurity">
/// RegSetKeySecurity</a> in the Windows API.
//
inline auto TRegKey::SetSecurity(SECURITY_INFORMATION infoToSet, PSECURITY_DESCRIPTOR secDesc) const -> long
{
  return ::RegSetKeySecurity(Key, infoToSet, secDesc);
}

//
/// Sets the security descriptor of this key.
/// \param infoToSet indicates the security information to set.
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetkeysecurity">
/// RegSetKeySecurity</a> in the Windows API.
//
inline void TRegKey::SetSecurity(SECURITY_INFORMATION infoToSet, const TSecurityDescriptor& s) const
{
  const auto r = SetSecurity(infoToSet, const_cast<SECURITY_DESCRIPTOR*>(s.GetData()));
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegKey::SetSecurity failed"), *this, r};
}

//
/// Saves this key and all of its subkeys and values to the specified file.
//
inline long TRegKey::Save(LPCTSTR fileName)
{
  return ::RegSaveKey(Key, fileName, nullptr/*Security*/);
}

//
/// Creates a subkey under HKEY_USER or HKEY_LOCAL_MACHINE and stores registration
/// information from a specified file into that subkey. This registration
/// information is in the form of a hive. A hive is a discrete body of keys,
/// subkeys, and values that is rooted at the top of the registry hierarchy. A hive
/// is backed by a single file and a .LOG file.
//
inline long TRegKey::LoadKey(LPCTSTR subKeyName, LPCTSTR fileName)
{
  return ::RegLoadKey(Key, subKeyName, fileName);
}

//
/// Replaces the file backing this key and all of its subkeys with another file, so
/// that when the system is next started, the key and subkeys will have the values
/// stored in the new file.
//
inline long TRegKey::ReplaceKey(LPCTSTR subKeyName, LPCTSTR newFileName, LPCTSTR oldFileName)
{
  return ::RegReplaceKey(Key, subKeyName, newFileName, oldFileName);
}

//
/// Reads the registry information in a specified file and copies it over this key.
/// This registry information may be in the form of a key and multiple levels of
/// subkeys.
//
inline long TRegKey::Restore(LPCTSTR fileName, uint32 options)
{
  return ::RegRestoreKey(Key, fileName, options);
}

//
/// Unloads this key and its subkeys from the registry.
//
inline long TRegKey::UnLoadKey(LPCTSTR subKeyName)
{
  return ::RegUnLoadKey(Key, subKeyName);
}

//
/// Returns the number of subkeys attached to this key.
//
inline uint32 TRegKey::GetSubkeyCount() const
{
  return SubkeyCount;
}

//
/// Returns the number of values attached to this key.
//
inline uint32 TRegKey::GetValueCount() const
{
  return ValueCount;
}

//
/// Returns a string identifying this key.
//
inline LPCTSTR TRegKey::GetName() const
{
  return Name.c_str();
}

//
/// Associates a value with this key.
//
inline long TRegKey::SetValue(LPCTSTR valName, uint32 type, const uint8* data, uint32 dataSize) const
{
  return ::RegSetValueEx(Key, valName, 0, type, data, dataSize);
}

//
/// Associates a 4-byte value with this key.
//
inline long TRegKey::SetValue(LPCTSTR valName, uint32 data) const
{
  return SetValue(valName, REG_DWORD, (uint8*) &data, sizeof data);
}

//
/// Associates a string value with this key.
//
inline long TRegKey::SetValue(LPCTSTR valName, const tstring& value) const
{
  const auto data = reinterpret_cast<const uint8*>(value.data());
  const auto dataSize = static_cast<uint32>((value.size() + 1) * sizeof(tchar));
  return SetValue(valName, REG_SZ, data, dataSize);
}

//
/// Retrieves the value associated with the unnamed value for this key in the registry.
//
inline long TRegKey::QueryValue(LPCTSTR valName, uint32* type, uint8* data, uint32* dataSize) const
{
#if defined WINELIB   //it seems that Winlib SDK declares a 'unsigned int*' instead of DWORD* and GCC it's tooo strict
  return ::RegQueryValueEx(Key, valName, nullptr, (unsigned int*) type, data, (unsigned int*) dataSize);
#else
  return ::RegQueryValueEx(Key, valName, nullptr, type, data, dataSize);
#endif//WINELIB
}

//
/// Removes a named value from this registry key.
//
inline long TRegKey::DeleteValue(LPCTSTR valName) const
{
  return ::RegDeleteValue(Key, valName);
}

#if defined(OWL5_COMPAT)

//
/// Sets the default [unnamed] value associated with this key.
//
inline long TRegKey::SetDefValue(LPCTSTR subkeyName, uint32 type, LPCTSTR data, uint32 dataSize)
{
  return ::RegSetValue(Key, subkeyName, type, data, dataSize);
}

//
/// Retrieves the default [unnamed] value associated with this key.
//
inline long TRegKey::QueryDefValue(LPCTSTR subkeyName, LPTSTR data, uint32* dataSize) const
{
#if defined WINELIB // It seems that Winlib SDK declares a 'int*' instead of DWORD*, and GCC is too strict.
  return ::RegQueryValue(Key, subkeyName, data, (int*) dataSize);
#else
  return ::RegQueryValue(Key, subkeyName, data, (long*) dataSize);
#endif
}

#endif

//
/// Enumerates the values associated with this key.
/// Copy the value name and data block associated with the passed index.
//
inline long TRegKey::EnumValue(int index, LPTSTR valueName, uint32& valueNameSize, uint32* type, uint8* data, uint32* dataSize) const
{
#if defined WINELIB // It seems that Winlib SDK declares a 'int*' instead of DWORD*, and GCC is too strict.
  return ::RegEnumValue(Key, index, valueName, (unsigned int*) &valueNameSize, 0, (unsigned int*) type, data, (unsigned int*) dataSize);
#else
  return ::RegEnumValue(Key, index, valueName, &valueNameSize, 0, type, data, dataSize);
#endif
}

//----------------------------------------------------------------------------
// TRegValue inlines
//

//
/// Returns a string identifying this value.
//
inline LPCTSTR TRegValue::GetName() const
{
  return Name.c_str();
}

//
/// Returns the type code for the data associated with this value
//
inline uint32 TRegValue::GetDataType() const
{
  return DataType;
}

//
/// Returns the size in bytes of the data associated with this value.
//
inline uint32 TRegValue::GetDataSize() const
{
  return DataSize;
}

//----------------------------------------------------------------------------
// TRegKeyIterator inlines
//

//
/// Creates a subkey iterator for a registration key.
//
inline TRegKeyIterator::TRegKeyIterator(const TRegKey& key)
  :
  Key(key),
  Index(0)
{
}

//
/// Tests the validity of this iterator.
/// \return True if the iterator's index is greater than or equal to 0 and less than the number of subkeys.
//
inline TRegKeyIterator::operator bool()
{
  return Index >= 0 && Index < int(Key.GetSubkeyCount());
}

//
/// Preincrements to the next subkey.
//
inline uint32 TRegKeyIterator::operator ++()
{
  return ++Index;
}

//
/// Postincrements to the next subkey.
//
inline uint32 TRegKeyIterator::operator ++(int)
{
  return Index++;
}

//
/// Predecrements to the previous subkey.
//
inline uint32 TRegKeyIterator::operator --()
{
  return --Index;
}

//
/// Postdecrements to the previous subkey.
//
inline uint32 TRegKeyIterator::operator --(int)
{
  return Index--;
}

//
/// Sets the index of the iterator to the passed value.
/// \return The new index.
//
inline uint32 TRegKeyIterator::operator [](int index)
{
  PRECONDITION((index >= 0) && (index < int(Key.GetSubkeyCount())));
  return Index = index;
}

//
/// Returns the index to the current subkey.
//
inline int TRegKeyIterator::Current() const
{
  return Index;
}

//
/// Returns the registration key this iterator is bound to.
//
inline const TRegKey& TRegKeyIterator::BaseKey() const
{
  return Key;
}

//
/// Resets the subkey index to zero.
//
inline void TRegKeyIterator::Reset()
{
  Index = 0;
}

//----------------------------------------------------------------------------
// TRegValueIterator inlines
//

//
/// Creates a value iterator for a registration key.
//
inline TRegValueIterator::TRegValueIterator(const TRegKey& regKey)
  : Key(regKey), Index(0)
{}

//
/// Tests the validity of this iterator. 
/// \return True if the iterator's index is greater than or equal to 0 and less than the number of values.
//
inline TRegValueIterator::operator bool()
{
  return Index >= 0 && Index < int(Key.GetValueCount());
}

//
/// Preincrements to the next value.
//
inline uint32 TRegValueIterator::operator ++()
{
  return ++Index;
}

//
/// Postincrements to the next value.
//
inline uint32 TRegValueIterator::operator ++(int)
{
  return Index++;
}

//
/// Predecrements to the previous value.
//
inline uint32 TRegValueIterator::operator --()
{
  return --Index;
}

//
/// Postdecrements to the previous value.
//
inline uint32 TRegValueIterator::operator --(int)
{
  return Index--;
}

//
/// Sets the index of the iterator to the passed value.
/// \return The new index.
//
inline uint32 TRegValueIterator::operator [](int index)
{
  PRECONDITION((index >= 0) && (index < int(Key.GetValueCount())));
  return Index = index;
}

//
/// Returns the index to the current value.
//
inline int TRegValueIterator::Current() const
{
  return Index;
}

//
/// Returns the registration key that this iterator is bound to.
//
inline const TRegKey& TRegValueIterator::BaseKey() const
{
  return Key;
}

//
/// Resets the value index to zero
//
inline void TRegValueIterator::Reset()
{
  Index = 0;
}


//----------------------------------------------------------------------------
// TRegList inlines
//

//
/// Constructs a TRegList object from an array of TRegItems terminated by a NULL item name.
//
inline TRegList::TRegList(TRegItem* _list)
  : Items(_list)
{
  PRECONDITION(_list);
}

//
/// The array operator uses an item name (key) to locate an item in the array.
/// Returns the value of the passed key as a const tchar*.
//
inline LPCTSTR TRegList::operator[](LPCSTR key)
{
  PRECONDITION(key);
  return Lookup(key);
}


//----------------------------------------------------------------------------
// TRegLink inlines
//

//
/// Registration link node destructor.
//
inline TRegLink::~TRegLink()
{}

//
/// Returns a pointer to the next link.
//
inline TRegLink* TRegLink::GetNext() const
{
  return Next;
}

inline void TRegLink::SetNext(TRegLink* link)
{
  Next = link;
}

//
/// Returns a pointer to the registration parameter table (reglist).
//
inline TRegList& TRegLink::GetRegList() const
{
  return *RegList;
}

//
/// Protected constructor where the derived class must initialize all pointers.
//
inline TRegLink::TRegLink()
  : Next(nullptr), RegList(nullptr)
{}

//----------------------------------------------------------------------------
// TRegTemplateList
//

//
/// Returns the number of templates in this list.
//
inline int TRegTemplateList::GetCount() const
{
  return Count;
}

//
/// Returns the registry key upon which these templates are based.
//
inline TRegKey& TRegTemplateList::GetBaseKey()
{
  return BaseKey;
}

//
/// Returns the template string at the passed index.
/// \note The list is indexed beginning with 1 not 0.
//
inline LPCTSTR TRegTemplateList::operator [](int i)
{
  PRECONDITION(i > 0 && i <= Count);
  if (i <= 0 || i > Count)
    return nullptr;
  else
    return List[i - 1];
}

//
/// Disables all templates in this list.
//
inline void TRegTemplateList::DisableAll()
{
  memset(EnabledFlags, 0x80, Count);
}

//
/// Enables all templates in this list.
//
inline void TRegTemplateList::EnableAll()
{
  memset(EnabledFlags, 0x00, Count);
}

//
/// Enables the template at the passed index.
/// \note The list is indexed beginning with 1 not 0.
//
inline void TRegTemplateList::Enable(int i)
{
  PRECONDITION(i > 0 && i <= Count);
  if (i > 0 && i <= Count)
    EnabledFlags[i - 1] = 0;
}

//
/// Activates the template at the passed index.
/// \note The list is indexed beginning with 1 not 0.
//
inline void TRegTemplateList::Activate(int i)
{
  PRECONDITION(i > 0 && i <= Count);
  if (i > 0 && i <= Count)
    EnabledFlags[i - 1]++;
}

//
/// Returns true if the template at the passed index is active, false otherwise.
/// \note The list is indexed beginning with 1 not 0.
//
inline bool TRegTemplateList::IsActive(int i) const
{
  PRECONDITION(i > 0 && i <= Count);
  if (i > 0 && i <= Count)
    return EnabledFlags[i - 1] > 0;
  else
    return false;
}

//----------------------------------------------------------------------------
// TRegParamList
//

//
/// Return the number of param entries in this list.
//
inline int TRegParamList::GetCount() const
{
  return Count;
}

//
/// Return the template string at the passed index.
//
inline const TRegParamList::TEntry& TRegParamList::operator [](int i) const
{
  PRECONDITION(i >= 0 && i < Count);
  return List[i];
}

//
/// Return the value of the param entry at the passed index.
//
inline const tchar*& TRegParamList::Value(int i)
{
  PRECONDITION(i >= 0 && i < Count);
  return Values[i];
}

} // OWL namespace

#endif  // OWL_REGISTRY_H
