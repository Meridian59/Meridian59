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
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/private/checks.h>
#include <owl/exbase.h>
#include <owl/wsysinc.h>
#include <owl/lclstrng.h>
#include <owl/private/memory.h>
#include <owl/private/strmdefs.h>
#include <vector>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup winsys
/// @{



//----------------------------------------------------------------------------
// Direct registry access classes. Can read & iterate as well as create and
// write keys and value-data pairs
//

//
/// \class TRegKey
// ~~~~~ ~~~~~~~
/// TRegKey is the encapsulation of a registration key.
//
class _OWLCLASS TRegKeyIterator;
class _OWLCLASS TRegKey {
  private:
    //BV, Sirma, Engview: Copy constructor is essential, it is implemented by default and default
    //implementation is not appropriate, we would better disable it until we write a good one:
    TRegKey(const TRegKey&){}
  public:
    typedef HKEY THandle;

    /// Enumeration used to specify whether a key should be created (or 
    /// simply opened).
    //
    enum TCreateOK {
      CreateOK,             ///< Create key if it does not exist
      NoCreate              ///< Don't create, simply open
    };

    /// Create or open a key given a base key and a subkeyname. Security
    /// information is ignored in 16bit (and under Win95)
    /// Can also provide an ok-to-create or open-only indicator.
    //
    TRegKey(THandle baseKey,
            LPCTSTR keyName,
            REGSAM samDesired = KEY_ALL_ACCESS,
            TCreateOK createOK = CreateOK);

    //
    /// String-aware overload
    //
    TRegKey(
      THandle baseKey, 
      const tstring& keyName, 
      REGSAM samDesired = KEY_ALL_ACCESS, 
      TCreateOK createOK = CreateOK);

    /// Construct a key give the current position of a regkey iterator
    //
    TRegKey(const TRegKeyIterator& iter, REGSAM samDesired = KEY_ALL_ACCESS);

    /// Contruct a key that is an alias to an existing HKEY
    //
    TRegKey(THandle aliasKey, bool shouldClose=false, LPCTSTR keyName = 0);

    //
    /// String aware overload
    //
    TRegKey(THandle aliasKey, bool shouldClose, const tstring& keyName);

   ~TRegKey();

    operator THandle() const;

    long DeleteKey(LPCTSTR subKeyName);
    long DeleteKey(const tstring& subKeyName) 
    {return DeleteKey(subKeyName.c_str());}

    long NukeKey(LPCTSTR subKeyName);
    long NukeKey(const tstring& subKeyName) 
    {return NukeKey(subKeyName.c_str());}

    long Flush() const;

    long GetSecurity(SECURITY_INFORMATION secInf, PSECURITY_DESCRIPTOR secDesc,
                     DWORD* secDescSize);
    long SetSecurity(SECURITY_INFORMATION secInf, PSECURITY_DESCRIPTOR secDesc);

    // Saving & loading of this key & subKeys
    //
    long Save(LPCTSTR fileName);
    long Save(const tstring& fileName) 
    {return Save(fileName.c_str());}

    long LoadKey(LPCTSTR subKeyName, LPCTSTR fileName);
    long LoadKey(const tstring& subKeyName, const tstring& fileName) 
    {return LoadKey(subKeyName.c_str(), fileName.c_str());}

    long ReplaceKey(LPCTSTR subKeyName, LPCTSTR newFileName, LPCTSTR oldFileName);
    long ReplaceKey(const tstring& subKeyName, const tstring& newFileName, const tstring& oldFileName)
    {return ReplaceKey(subKeyName.c_str(), newFileName.c_str(), oldFileName.c_str());}

    long Restore(LPCTSTR fileName, uint32 options=0);
    long Restore(const tstring& fileName, uint32 options = 0)
    {return Restore(fileName.c_str(), options);}

    long UnLoadKey(LPCTSTR subKeyName);
    long UnLoadKey(const tstring& subKeyName)
    {return UnLoadKey(subKeyName.c_str());}

    //long NotifyChangeKeyValue();

    // Some of these are initialized at ctor
    // TODO: Provide string-returning overload.
    //
    long QueryInfo(LPTSTR class_, DWORD* classSize, DWORD* subkeyCount,
                   DWORD* maxSubkeySize, DWORD* maxClassSize,
                   DWORD* valueCount, DWORD* maxValueName,
                   DWORD* maxValueData, DWORD* secDescSize,
                   PFILETIME lastWriteTime);

    LPCTSTR GetName() const;
    uint32 GetSubkeyCount() const;
    uint32 GetValueCount() const;

    // Special predefined root keys
    //

/// Special predefined root key used by shell and OLE applications.
    static TRegKey& ClassesRoot()      { return GetClassesRoot();      }
    
/// A subkey commonly used by shell and OLE applications.
    static TRegKey& ClassesRootClsid() { return GetClassesRootClsid(); }
    
/// Special predefined root key defining the preferences of the current user.
    static TRegKey& CurrentUser()      { return GetCurrentUser();      }
    
/// Special predefined root key defining the physical state of the computer.
    static TRegKey& LocalMachine()     { return GetLocalMachine();     }
    
/// Special predefined root key defining the default user configuration.
    static TRegKey& Users()            { return GetUsers();            }
    
    static TRegKey& CurrentConfig()    { return GetCurrentConfig();    }
    
/// Special predefined root key.
    static TRegKey& DynData()          { return GetDynData();          }
    
/// Special predefined root key used to obtain performance data.
    static TRegKey& PerformanceData()  { return GetPerformanceData();  }

    // Accessors common/predefined registry keys
    //
    static TRegKey& GetClassesRoot();
    static TRegKey& GetClassesRootClsid();
    static TRegKey& GetCurrentUser();
    static TRegKey& GetLocalMachine();
    static TRegKey& GetUsers();
    static TRegKey& GetCurrentConfig();
    static TRegKey& GetDynData();
    static TRegKey& GetPerformanceData();

    // Friend iterators only...?
    // TODO: Provide string-returning overload.
    //
    long EnumKey(int index, LPTSTR subKeyName, int subKeyNameSize) const;

    // Older, nameless subkey+(Default) value access
    //
    long SetDefValue(LPCTSTR subkeyName, uint32 type,
                     LPCTSTR data, uint32 dataSize);
    long SetDefValue(const tstring& subkeyName, uint32 type, const tstring& data, uint32 dataSize)
    {return SetDefValue(subkeyName.c_str(), type, data.c_str(), dataSize);}

    long QueryDefValue(LPCTSTR subkeyName, LPTSTR data,
                       uint32* dataSize) const;
    long QueryDefValue(const tstring& subkeyName, LPTSTR data, uint32* dataSize) const
    {return QueryDefValue(subkeyName.c_str(), data, dataSize);}

    // Newer, named value set & get functions. Subkeys must be opened
    //
    long SetValue(LPCTSTR valName, uint32 type, const uint8* data,
                  uint32 dataSize) const;
    long SetValue(const tstring& valName, uint32 type, const uint8* data, uint32 dataSize) const
    {return SetValue(valName.c_str(), type, data, dataSize);}

    long SetValue(LPCTSTR valName, uint32 data) const;
    long SetValue(const tstring& valName, uint32 data) const
    {return SetValue(valName.c_str(), data);}

    long QueryValue(LPCTSTR valName, uint32* type, uint8* data,
                    uint32* dataSize) const;
    long QueryValue(const tstring& valName, uint32* type, uint8* data, uint32* dataSize) const
    {return QueryValue(valName.c_str(), type, data, dataSize);}

    long DeleteValue(LPCTSTR valName) const;
    long DeleteValue(const tstring& valName) const
    {return DeleteValue(valName.c_str());}

    // Friend iterators only...?
    // TODO: Provide value-returning overload.
    //
    long EnumValue(int index, LPTSTR valueName, uint32& valueNameSize,
                   uint32* type = 0, uint8* data = 0, uint32* dataSize = 0) const;

  protected:
    THandle   Key;          ///< This Key's Handle
    LPTSTR    Name;         ///< This Key's Name
    DWORD    SubkeyCount;  ///< Number of subkeys
    DWORD    ValueCount;   ///< Number of value entries
    bool      ShouldClose;  ///< Should this key be closed on destruction

    void Init(THandle baseKey, REGSAM samDesired, TCreateOK createOK);
    void Init();
};


class _OWLCLASS TRegValueIterator;

//
/// \class TRegValue
// ~~~~~ ~~~~~~~~~
/// TRegValue encapsulates a value-data entry within one registration key.
//
class _OWLCLASS TRegValue {
  public:
    TRegValue(const TRegKey& key, LPCTSTR name);
    TRegValue(const TRegKey& key, const tstring& name);
    explicit TRegValue(const TRegValueIterator& iter);

    /// \name Accessors
    /// @{

    LPCTSTR GetName() const;
    const uint32 GetDataType() const;
    const uint32 GetDataSize() const;
    const uint8* GetData() const;

    operator uint8*() const;
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

    /// @{

    void RetrieveOnDemand() const;

  private:
    const TRegKey& Key; ///< Key that this value lives in
    tstring Name; ///< Value name; if empty, this object represents the default value for the associated key.

    uint32 DataType; ///< Type code for value data
    mutable std::vector<uint8> Data; ///< Value data; mutable to support lazy retrieval (see RetrieveOnDemand).
    uint32 DataSize; ///< Size in bytes of Data

    long QueryTypeAndSize();
};

//
/// \class TRegKeyIterator
// ~~~~~ ~~~~~~~~~~~~~~~
/// Iterator for walking thru the subkeys of a key
//
class _OWLCLASS TRegKeyIterator {
  public:
    TRegKeyIterator(const TRegKey& key);

    operator bool();

    uint32 operator ++();
    uint32 operator ++(int);
    uint32 operator --();
    uint32 operator --(int);
    uint32 operator [](int index);

    int             Current() const;
    const TRegKey&  BaseKey() const;

    void Reset();

  private:
    const TRegKey& Key;
    int            Index;
};

//
/// \class TRegValueIterator
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// TRegValueIterator is an iterator for walking through the values of a key.
//
class _OWLCLASS TRegValueIterator {
  public:
    TRegValueIterator(const TRegKey& regKey);

    operator bool();

    uint32 operator ++();
    uint32 operator ++(int);
    uint32 operator --();
    uint32 operator --(int);
    uint32 operator [](int index);

    int            Current() const;
    const TRegKey& BaseKey() const;

    void Reset();

  private:
    const TRegKey& Key;
    int            Index;
};

//----------------------------------------------------------------------------

//
/// \class TXRegistry
// ~~~~~ ~~~~~~~~~~
/// TXRegistry is the object thrown when exceptions are encountered within the
/// WinSys Registry classes.
///
/// TODO: Make the Key member hold a copy of the key name rather than a weak pointer, and add string-class support.
//
class _OWLCLASS TXRegistry : public TXBase {
  public:
    TXRegistry(const tstring& msg, LPCTSTR key);
    TXRegistry(const TXRegistry& copy);

    static void Check(long stat, LPCTSTR key);

    const tchar* Key;
};


//----------------------------------------------------------------------------
// Registration parameter structures and formatting functions
//

//
/// \class TRegFormatHeap
// ~~~~~ ~~~~~~~~~~~~~~
/// Used internally to provide buffers for formating registration strings
//
class _OWLCLASS TRegFormatHeap {
  public:
    TRegFormatHeap();
   ~TRegFormatHeap();
    tchar*  Alloc(int spaceNeeded);
  private:
    struct TBlock {
      TBlock*  Next;     // Next heap in chain
      tchar   Data[1];  // Allocated memory starts here
    };
    TBlock* Head;
};

//
/// \struct TRegItem
// ~~~~~~ ~~~~~~~~
/// A single registration list entry
///
/// TRegItem defines localizable values for parameters or subkeys. These values can
/// be passed to TLocaleString, which defines a localizable substitute for char*.
/// TRegItem contains several undocumented functions that are used privately by the
/// registration macros REGFORMAT and REGSTATUS.
//
struct _OWLCLASS TRegItem {
  const char*   Key;           ///< Non-localized parameter or registry subkey
  TLocaleString Value;         ///< Localizable value for parameter or subkey

  // Used privately by REGFORMAT, REGSTATUS macros
  //
  static tchar* RegFormat(int f, int a, int t, int d, TRegFormatHeap& heap);
  static tchar* RegFormat(LPCTSTR f, int a, int t, int d, TRegFormatHeap& heap);
  static tchar* RegFormat(const tstring& f, int a, int t, int d, TRegFormatHeap& heap) {return RegFormat(f.c_str(), a, t, d, heap);}
  static tchar* RegFlags(long flags, TRegFormatHeap& heap);
  static tchar* RegVerbOpt(int mf, int sf, TRegFormatHeap& heap);
  static void    OverflowCheck();
};

//
/// \class TRegList
// ~~~~~ ~~~~~~~~
/// A registration parameter table, composed of a list of TRegItems
//
/// Holds an array of items of type TRegItem. Provides functionality that lets you
/// access each item in the array and return the name of the item. Instead of
/// creating a TRegList directly, you can use ObjectWindows' registration macros to
/// build a TRegList for you.
//
class _OWLCLASS TRegList {
  public:
    TRegList(TRegItem* _list);
    LPCTSTR Lookup(LPCSTR key,
                       TLangId lang = TLocaleString::UserDefaultLangId);
    LPCTSTR Lookup(const std::string& key, TLangId lang = TLocaleString::UserDefaultLangId)
    {return Lookup(key.c_str(), lang);}

    TLocaleString& LookupRef(LPCSTR key);
    TLocaleString& LookupRef(const std::string& key)
    {return LookupRef(key.c_str());}

    LPCTSTR operator[](LPCSTR key);
    LPCTSTR operator[](const std::string& key)
    {return operator[](key.c_str());}

    TRegItem* Items;
};

//
/// \class TRegLink
// ~~~~~ ~~~~~~~~
/// TRegLink is a linked structure in which each node points to a list of TRegList
/// objects (or TRegList-derived objects) or TDocTemplate objects. Each object has
/// an item name and a string value associated with the item name. The structure
/// forms a typical linked list as the following diagram illustrates:
/// \image html bm251.BMP
/// 
/// A TDocTemplate object uses the following variation of the TRegLink structure:
/// \image html bm252.BMP
//
class _OWLCLASS TRegLink {
  public:
    TRegLink(TRegList& regList, TRegLink*& head);
    virtual ~TRegLink();
    TRegLink* GetNext() const;
    void      SetNext(TRegLink* link);
    TRegList& GetRegList() const ;

    static void AddLink(TRegLink** head, TRegLink* newLink);
    static bool RemoveLink(TRegLink** head, TRegLink* remLink);

  protected:
    TRegLink();           ///< Derived class must fill in ptrs
    TRegLink*  Next;      ///< Next RegLink
    TRegList*  RegList;   ///< Pointer to registration parameter table
};

//
// Registration parameter table macro definitions
//
#define BEGIN_REGISTRATION(regname) extern TRegItem regname##_list[]; \
                                    extern TRegFormatHeap __regHeap; \
                                    TRegList regname(regname##_list); \
                                    static TRegItem regname##_list[] = {
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
// !CQ from OCF. Really put all these here?

const int ocrVerbLimit   = 8;  ///< maximum number of verbs registered per class
const int ocrFormatLimit = 8;  ///< maximum number of data formats per class

//
/// Format: standard clipboard numeric format, or name of custom format
//
enum ocrClipFormat {
  ocrText           =  1,  ///< CF_TEXT
  ocrBitmap         =  2,  ///< CF_BITMAP
  ocrMetafilePict   =  3,  ///< CF_METAFILEPICT
  ocrSylk           =  4,  ///< CF_SYLK
  ocrDif            =  5,  ///< CF_DIF
  ocrTiff           =  6,  ///< CF_TIFF
  ocrOemText        =  7,  ///< CF_OEMTEXT
  ocrDib            =  8,  ///< CF_DIB
  ocrPalette        =  9,  ///< CF_PALETTE
  ocrPenData        = 10,  ///< CF_PENDATA
  ocrRiff           = 11,  ///< CF_RIFF
  ocrWave           = 12,  ///< CF_WAVE
  ocrUnicodeText    = 13,  ///< CF_UNICODETEXT  Win32 only
  ocrEnhMetafile    = 14,  ///< CF_ENHMETAFILE  Win32 only
};
#define ocrRichText          _T("Rich Text Format")
#define ocrEmbedSource       _T("Embed Source")
#define ocrEmbeddedObject    _T("Embedded Object")
#define ocrLinkSource        _T("Link Source")
#define ocrObjectDescriptor  _T("Object Descriptor")
#define ocrLinkSrcDescriptor _T("Link Source Descriptor")

//
/// Aspect: view types supported by transfer
//
enum ocrAspect {
  ocrContent   = 1,  ///< DVASPECT_CONTENT    normal display representation
  ocrThumbnail = 2,  ///< DVASPECT_THUMBNAIL  picture appropriate for browser
  ocrIcon      = 4,  ///< DVASPECT_ICON       iconized representation of object
  ocrDocPrint  = 8,  ///< DVASPECT_DOCPRINT   print preview representation
};

//
/// Medium: means of data transfer
//
enum ocrMedium {
  ocrNull       = 0,
  ocrHGlobal    = 1,  ///< TYMED_HGLOBAL  global memory handle
  ocrFile       = 2,  ///< TYMED_FILE     data as contents of file
  ocrIStream    = 4,  ///< TYMED_ISTREAM  instance of an IStream object
  ocrIStorage   = 8,  ///< TYMED_ISTORAGE streams within an instance of IStorage
  ocrGDI        = 16, ///< TYMED_GDI      GDI object in global handle
  ocrMfPict     = 32, ///< TYMED_MFPICT   CF_METAFILEPICT containing global handle
  ocrStaticMed  = 1024 ///< OLE 2 static object
};

//
/// Direction: transfer directions supported
//
enum ocrDirection {
  ocrGet    = 1,
  ocrSet    = 2,
  ocrGetSet = 3,
};

//----------------------------------------------------------------------------
// Miscellaneous registry definitions
//

//
/// IOleObject miscellaneous status flags, defined for each or all aspects
//
enum ocrObjectStatus {
  ocrRecomposeOnResize           = 1,   ///< request redraw on container resize
  ocrOnlyIconic                  = 2,   ///< only useful context view is Icon
  ocrInsertNotReplace            = 4,   ///< should not replace current select.
  ocrStatic                      = 8,   ///< object is an OLE static object
  ocrCantLinkInside              = 16,  ///< should not be the link source
  ocrCanLinkByOle1               = 32,  ///< only used in OBJECTDESCRIPTOR
  ocrIsLinkObject                = 64,  ///< set by OLE2 link for OLE1 compat.
  ocrInsideOut                   = 128, ///< can be activated concurrently
  ocrActivateWhenVisible         = 256, ///< hint to cntnr when ocrInsideOut set
  ocrRenderingIsDeviceIndependent= 512, ///< no decisions made based on target
  ocrNoSpecialRendering          = 512, ///< older enum for previous entry
};

//
/// IOleObject verb menu flags
//
enum ocrVerbMenuFlags {
  ocrGrayed      = 1,   ///< MF_GRAYED
  ocrDisabled    = 2,   ///< MF_DISABLED
  ocrChecked     = 8,   ///< MF_CHECKED
  ocrMenuBarBreak= 32,  ///< MF_MENUBARBREAK
  ocrMenuBreak   = 64,  ///< MF_MENUBAR
};

//
/// IOleObject verb attribute flags
//
enum ocrVerbAttributes {
  ocrNeverDirties      = 1,   ///< verb can never cause object to become dirty
  ocrOnContainerMenu   = 2,   ///< only useful context view is Icon
};

//
// Entries for <usage> registration parameter, class factory registration mode
//
#define ocrSingleUse     _T("1")  ///< single client per instance
#define ocrMultipleUse   _T("2")  ///< multiple clients per instance
#define ocrMultipleLocal _T("3")  ///< multiple clients, separate inproc server

//----------------------------------------------------------------------------
// High-level table based registration support
//

//
/// \class TRegTemplateList
// ~~~~~ ~~~~~~~~~~~~~~~~
/// List of parameterized template strings that represent the actual entries to
/// be registered. List is indexed from 1 for used with param-list template
/// activation strings. See TRegParamList below.
/// TODO: Add string-class support in constructor.
//
class _OWLCLASS TRegTemplateList {
  public:
    TRegTemplateList(TRegKey& basekey, LPCTSTR _list[]);
   ~TRegTemplateList();

    int      GetCount() const;
    TRegKey& GetBaseKey();

    LPCTSTR operator [](int i);

    // Enable/Disable and activate templates
    //
    void  DisableAll();
    void  EnableAll();
    void  Enable(int i);
    void  Enable(LPCTSTR set);
    void  Enable(const tstring& set) {Enable(set.c_str());}

    void  Activate(int i);
    void  Activate(LPCTSTR set);
    void  Activate(const tstring& set) {Activate(set.c_str());}

    bool  IsActive(int i) const;

  private:
    TRegKey&        BaseKey;       ///< Registry key that these templates are based
    const tchar**  List;          ///< List of templates
    int             Count;         ///< # of templates in list
    
    //TODO Jogy - Change with enumeration
    int8*         EnabledFlags;  // Which templates are currently enabled/active
};

//
/// \class TRegParamList
// ~~~~~ ~~~~~~~~~~~~~
/// A list of param entries for use as an intermediate between a TRegList and
/// the actual template list used to generate the registration. Provides default
/// values in 'Default', and tags required templates using octal char entries in
/// 'TemplatesNeeded'
//
class _OWLCLASS TRegParamList {
  public:
    struct TEntry {
      tchar* Param;            ///< Substituted parameter name
      tchar* Default;          ///< Default value, 0 if no default & param is required
      tchar* TemplatesNeeded;  ///< Octal string list of template indices to activate
    };

    TRegParamList(TEntry* _list);
   ~TRegParamList();

    int Find(LPCTSTR  param);   ///< Associative lookup of value by param
    int Find(const tstring& param) {return Find(param.c_str());}
    int GetCount() const;

    TEntry& operator [](int i);

    const tchar*& Value(int i);
    void   ResetDefaultValues();

  private:
    TEntry*        List;
    int            Count;
    const tchar** Values;
};

//
/// \class TRegSymbolTable
// ~~~~~ ~~~~~~~~~~~~~~~
/// High level symbol-based registry entry manipulation
/// TODO: Add string-class support in constructor.
//
class _OWLCLASS TRegSymbolTable {
  public:
    TRegSymbolTable(TRegKey& basekey, LPCTSTR tplList[], TRegParamList::TEntry* paramList);

    void Init(tchar* filter);
    void UpdateParams(TLangId lang, TRegItem* item);
    void StreamOut(TRegItem* item, tostream& out);
//    void StreamIn(TRegItem* item, tistream& in);

    TRegTemplateList Templates;
    TRegParamList    Params;
    int              UserKeyCount;
};

//
/// \class TRegistry
// ~~~~~ ~~~~~~~~~
/// TRegistry provides high level stream and list access to the registry.
//
class _OWLCLASS TRegistry {
  public:
    static int  Validate(TRegKey& baseKey, tistream& in);  ///< Returns number of mismatched entries
    static void Update(TRegKey& baseKey, tistream& in);    ///< Writes lines to registry
#pragma pack(push,4)
    struct TUnregParams {
      tchar    Prepend;    ///< Optional tchar to prepend to key before removing
      tchar*   Name;       ///< Name of param
      TRegKey*  BaseKey;    ///< Key that that the param is based upon
    };
#pragma pack(pop)
    static int  Unregister(TRegList& regInfo, TUnregParams* params, TRegItem* overrides = 0);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// TRegKey inlines
//

//
/// Returns the HANDLE identifying this registry key.
//
inline
TRegKey::operator TRegKey::THandle() const
{
  return Key;
}

//
/// Enumerates the subkeys of this registry key.
//
inline long
TRegKey::EnumKey(int index, tchar * subKeyName, int subKeyNameSize) const
{
  return ::RegEnumKey(Key, index, subKeyName, subKeyNameSize);
  //::RegEnumKeyEx(); ??
}

//
/// Deletes the specified subkey of this registry key.
//
inline long
TRegKey::DeleteKey(LPCTSTR subKeyName)
{
  return ::RegDeleteKey(Key, subKeyName);
}

//
/// Writes the attribute of this key in the registry
//
inline long
TRegKey::Flush() const
{
  return ::RegFlushKey(Key);
}

//
/// Retrieves a copy of the security descriptor protecting this registry key.
//
inline long
TRegKey::GetSecurity(SECURITY_INFORMATION secInf, PSECURITY_DESCRIPTOR secDesc,
                     DWORD* secDescSize)
{
  return ::RegGetKeySecurity(Key, secInf, secDesc, secDescSize);
}

//
/// Sets the security descriptor of this key.
//
inline long
TRegKey::SetSecurity(SECURITY_INFORMATION secInf, PSECURITY_DESCRIPTOR secDesc)
{
  return ::RegSetKeySecurity(Key, secInf, secDesc);
}

//
/// Saves this key and all of its subkeys and values to the specified file.
//
inline long
TRegKey::Save(LPCTSTR fileName)
{
  return ::RegSaveKey(Key, fileName, 0/*Security*/);
}

//
/// Creates a subkey under HKEY_USER or HKEY_LOCAL_MACHINE and stores registration
/// information from a specified file into that subkey. This registration
/// information is in the form of a hive. A hive is a discrete body of keys,
/// subkeys, and values that is rooted at the top of the registry hierarchy. A hive
/// is backed by a single file and a .LOG file.
//
inline long
TRegKey::LoadKey(LPCTSTR subKeyName, LPCTSTR fileName)
{
  return ::RegLoadKey(Key, subKeyName, fileName);
}

//
/// Replaces the file backing this key and all of its subkeys with another file, so
/// that when the system is next started, the key and subkeys will have the values
/// stored in the new file.
//
inline long
TRegKey::ReplaceKey(LPCTSTR subKeyName, LPCTSTR newFileName,
                    LPCTSTR oldFileName)
{
  return ::RegReplaceKey(Key, subKeyName, newFileName, oldFileName);
}

//
/// Reads the registry information in a specified file and copies it over this key.
/// This registry information may be in the form of a key and multiple levels of
/// subkeys.
//
inline long
TRegKey::Restore(LPCTSTR fileName, uint32 options)
{
  return ::RegRestoreKey(Key, fileName, options);
}

//
/// Unloads this key and its subkeys from the registry.
//
inline long
TRegKey::UnLoadKey(LPCTSTR subKeyName)
{
  return ::RegUnLoadKey(Key, subKeyName);
}

//inline long TRegKey::NotifyChangeKeyValue() {}

//
/// Returns the number of subkeys attached to this key.
//
inline uint32
TRegKey::GetSubkeyCount() const
{
  return SubkeyCount;
}

//
/// Returns the number of values attached to this key.
//
inline uint32
TRegKey::GetValueCount() const
{
  return ValueCount;
}

//
/// Returns a string identifying this key.
//
inline LPCTSTR
TRegKey::GetName() const
{
  return Name;
}

//
/// Associates a value with this key.
//
inline long
TRegKey::SetValue(LPCTSTR valName, uint32 type, const uint8* data,
                  uint32 dataSize) const
{
  return ::RegSetValueEx(Key, valName, 0, type, data, dataSize);
}

//
/// Associates a 4-byte value with this key.
//
inline long
TRegKey::SetValue(LPCTSTR valName, uint32 data) const
{
  return SetValue(valName, REG_DWORD, (uint8*)&data, sizeof data);
}

//
/// Retrieves the value associated with the unnamed value for this key in the registry.
//
inline long
TRegKey::QueryValue(LPCTSTR valName, uint32* type, uint8* data,
                    uint32* dataSize) const
{
#if defined WINELIB   //it seems that Winlib SDK declares a 'unsigned int*' instead of DWORD* and GCC it's tooo strict
  return ::RegQueryValueEx(Key, valName, 0, (unsigned int*)type, data, (unsigned int*)dataSize);
#else
  return ::RegQueryValueEx(Key, valName, 0, type, data, dataSize);
#endif//WINELIB
}

//
/// Removes a named value from this registry key.
//
inline long
TRegKey::DeleteValue(LPCTSTR valName) const
{
  return ::RegDeleteValue(Key, valName);
}

//
/// Sets the default [unnamed] value associated with this key.
//
inline long
TRegKey::SetDefValue(LPCTSTR subkeyName, uint32 type,
                     LPCTSTR data, uint32 dataSize)
{
  return ::RegSetValue(Key, subkeyName, type, data, dataSize);
}

//
/// Retrieves the default [unnamed] value associated with this key.
//
inline long
TRegKey::QueryDefValue(LPCTSTR subkeyName, tchar * data,
                       uint32* dataSize) const
{
#if defined WINELIB   //it seems that Winlib SDK declares a 'int*' instead of DWORD* and GCC it's tooo strict
  return ::RegQueryValue(Key, subkeyName, data, (int*)dataSize);
#else
  return ::RegQueryValue(Key, subkeyName, data, (long*)dataSize);
#endif
}

//
/// Enumerates the values associated with this key.  Copy the value name and
/// data block associated with the passed index.
//
inline long
TRegKey::EnumValue(int index, tchar * valueName, uint32& valueNameSize,
                   uint32* type, uint8* data, uint32* dataSize) const
{
#if defined WINELIB   //it seems that Winlib SDK declares a 'int*' instead of DWORD* and GCC it's tooo strict
//  unsigned int auxV=valueNameSize;
  return ::RegEnumValue(Key, index, valueName, (unsigned int*)&valueNameSize, 0, (unsigned int*)type, data, (unsigned int*)dataSize);
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
inline LPCTSTR
TRegValue::GetName() const
{
  return Name.c_str();
}

//
/// Returns the type code for the data associated with this value
//
inline const uint32
TRegValue::GetDataType() const
{
  return DataType;
}

//
/// Returns the size in bytes of the data associated with this value.
//
inline const uint32
TRegValue::GetDataSize() const
{
  return DataSize;
}

//----------------------------------------------------------------------------
// TRegKeyIterator inlines
//

//
/// Creates a subkey iterator for a registration key
//
inline
TRegKeyIterator::TRegKeyIterator(const TRegKey& key)
:
  Key(key),
  Index(0)
{
}

//
/// Tests the validity of this iterator.  True if the iterator's
/// index is greater than or equal to 0 and less than the number
/// of subkeys.
//
inline
TRegKeyIterator::operator bool()
{
  return Index >= 0 && Index < int(Key.GetSubkeyCount());
}

//
/// Preincrements to the next subkey
//
inline uint32
TRegKeyIterator::operator ++()
{
  return ++Index;
}

//
/// Postincrements to the next subkey
//
inline uint32
TRegKeyIterator::operator ++(int)
{
  return Index++;
}

//
/// Predecrements to the previous subkey
//
inline uint32
TRegKeyIterator::operator --()
{
  return --Index;
}

//
/// Postdecrements to the previous subkey
//
inline uint32
TRegKeyIterator::operator --(int)
{
  return Index--;
}

//
/// Sets the index of the iterator to the passed value.  Return the new index.
//
inline uint32
TRegKeyIterator::operator [](int index)
{
  PRECONDITION((index >= 0) && (index < int(Key.GetSubkeyCount())));
  return Index = index;
}

//
/// Returns the index to the current subkey
//
inline int
TRegKeyIterator::Current() const
{
  return Index;
}

//
/// Returns the registration key this iterator is bound to
//
inline const TRegKey&
TRegKeyIterator::BaseKey() const
{
  return Key;
}

//
/// Resets the subkey index to zero
//
inline void
TRegKeyIterator::Reset()
{
  Index = 0;
}

//----------------------------------------------------------------------------
// TRegValueIterator inlines
//

//
/// Creates a value iterator for a registration key
//
inline
TRegValueIterator::TRegValueIterator(const TRegKey& regKey)
:
  Key(regKey),
  Index(0)
{
}

//
/// Tests the validity of this iterator.  True if the iterator's
/// index is greater than or equal to 0 and less than the number
/// of values.
//
inline
TRegValueIterator::operator bool()
{
  return Index >= 0 && Index < int(Key.GetValueCount());
}

//
/// Preincrements to the next value
//
inline uint32
TRegValueIterator::operator ++()
{
  return ++Index;
}

//
/// Postincrements to the next value
//
inline uint32
TRegValueIterator::operator ++(int)
{
  return Index++;
}

//
/// Predecrements to the previous value
//
inline uint32
TRegValueIterator::operator --()
{
  return --Index;
}

//
/// Postdecrements to the previous value
//
inline uint32
TRegValueIterator::operator --(int)
{
  return Index--;
}

//
/// Sets the index of the iterator to the passed value.  Return the new index.
//
inline uint32
TRegValueIterator::operator [](int index)
{
  PRECONDITION((index >= 0) && (index < int(Key.GetValueCount())));
  return Index = index;
}

//
/// Returns the index to the current value
//
inline int
TRegValueIterator::Current() const
{
  return Index;
}

//
/// Returns the registration key that this iterator is bound to.
//
inline const TRegKey&
TRegValueIterator::BaseKey() const
{
  return Key;
}

//
/// Resets the value index to zero
//
inline void
TRegValueIterator::Reset()
{
  Index = 0;
}


//----------------------------------------------------------------------------
// TXRegistry inlines
//

//
/// Creates a registry exception object.  msg points to an error message and
/// key points to the name of the registry key that ObjectComponents was
/// processing when the exception occurred.
//
inline
TXRegistry::TXRegistry(const tstring& msg, LPCTSTR key)
:
  TXBase(msg),
  Key(key)
{
}

//
/// The copy constructor constructs a new registry exception object by copying
/// the one passed as copy. 
//
inline
TXRegistry::TXRegistry(const TXRegistry& src)
:
  TXBase(src),
  Key(src.Key)
{
}



//----------------------------------------------------------------------------
// TRegList inlines
//

//
/// Constructs a TRegList object from an array of TRegItems terminated by a NULL
/// item name.
//
inline
TRegList::TRegList(TRegItem* _list)
:
  Items(_list)
{
  PRECONDITION(_list);
}

//
/// The array operator uses an item name (key) to locate an item in the array.
/// Returns the value of the passed key as a const tchar*
//
inline LPCTSTR
TRegList::operator[](LPCSTR key)
{
  PRECONDITION(key);
  return Lookup(key);
}


//----------------------------------------------------------------------------
// TRegLink inlines
//

//
/// Registration link node destructor
//
inline
TRegLink::~TRegLink()
{
}

//
/// Returns a pointer to the next link.
//
inline TRegLink*
TRegLink::GetNext() const {
  return Next;
}
inline void 
TRegLink::SetNext(TRegLink* link){ 
  Next = link;
}

//
/// Returns a pointer to the registration parameter table (reglist).
//
inline TRegList&
TRegLink::GetRegList() const
{
  return *RegList;
}

//
/// Protected constructor where the derived class must initialize all pointers
//
inline
TRegLink::TRegLink()
:
  Next(0),
  RegList(0)
{

}


//----------------------------------------------------------------------------
// TRegTemplateList
//

//
/// Returns the number of templates in this list
//
inline int
TRegTemplateList::GetCount() const
{
  return Count;
}

//
/// Returns the registry key upon which these templates are based
//
inline TRegKey&
TRegTemplateList::GetBaseKey()
{
  return BaseKey;
}

//
/// Returns the template string at the passed index.
/// \note The list is indexed beginning with 1 not 0.
//
inline LPCTSTR
TRegTemplateList::operator [](int i)
{
  PRECONDITION(i > 0 && i <= Count); ///CM added range check
  if (i <= 0 || i > Count)
    return 0;
  else
    return List[i-1];
}

//
/// Disables all templates in this list
//
inline void
TRegTemplateList::DisableAll()
{
  memset(EnabledFlags, 0x80, Count);
}

//
/// Enables all templates in this list
//
inline void
TRegTemplateList::EnableAll()
{
  memset(EnabledFlags, 0x00, Count);
}

//
/// Enables the template at the passed index
/// \note The list is indexed beginning with 1 not 0.
//
inline void
TRegTemplateList::Enable(int i)
{
  PRECONDITION(i > 0 && i <= Count); ///CM added range check
  if (i > 0 && i <= Count)
    EnabledFlags[i-1] = 0;
}

//
/// Activates the template at the passed index
/// \note The list is indexed beginning with 1 not 0.
//
inline void
TRegTemplateList::Activate(int i)
{
  PRECONDITION(i > 0 && i <= Count); ///CM added range check
  if (i > 0 && i <= Count)
    EnabledFlags[i-1]++;
}

//
/// Returns true if the template at the passed index is active, false otherwise.
/// \note The list is indexed beginning with 1 not 0.
//
inline bool
TRegTemplateList::IsActive(int i) const
{
  PRECONDITION(i > 0 && i <= Count); ///CM added range check
  if (i > 0 && i <= Count)
    return EnabledFlags[i-1] > 0;
  else
    return false;
}

//----------------------------------------------------------------------------
// TRegParamList
//

//
/// Return the number of param entries in this list
//
inline int
TRegParamList::GetCount() const
{
  return Count;
}

//
/// Return the template string at the passed index.
//
inline TRegParamList::TEntry&
TRegParamList::operator [](int i)
{
  PRECONDITION(i >= 0 && i < Count); ///CM added range check
  if (i < 0 || i >= Count)
    TXRegistry::Check(1,0); ///CM Throw exception on bad index
  return List[i];
}


//
/// Return the value of the param entry at the passed index
//
inline const tchar*&
TRegParamList::Value(int i)
{
  PRECONDITION(i >= 0 && i < Count); ///CM added range check
  return Values[i];
}


} // OWL namespace


#endif  // OWL_REGISTRY_H
