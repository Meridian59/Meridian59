//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// General Registry access & registration implementation
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/registry.h>
#include <owl/system.h>
#include <owl/pointer.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

/////////////////////////////////////////////////////////////////////////////////
//
// Predefined constant registry keys
//
#if 0
TRegKey TRegKey::ClassesRoot(HKEY_CLASSES_ROOT, true, _T("HKEY_CLASSES_ROOT"));
#endif
#if 0
TRegKey TRegKey::ClassesRootClsid(ClassesRoot, _T("CLSID"), KEY_ALL_ACCESS, NoCreate);
#endif

//
//
//
TRegKey&
TRegKey::GetClassesRoot()
{
  static TRegKey ClassesRoot(HKEY_CLASSES_ROOT, true, _T("HKEY_CLASSES_ROOT"));
  return ClassesRoot;
}

//
//
//
TRegKey&
TRegKey::GetClassesRootClsid()
{
  static TRegKey ClassesRootClsid(GetClassesRoot(), _T("CLSID"), KEY_ALL_ACCESS, NoCreate);
  return ClassesRootClsid;
}

//
//
//
TRegKey&
TRegKey::GetCurrentUser()
{
  static TRegKey CurrentUser(HKEY_CURRENT_USER, true, _T("HKEY_CURRENT_USER"));
  return CurrentUser;
}

//
//
//
TRegKey&
TRegKey::GetLocalMachine()
{
  static TRegKey LocalMachine(HKEY_LOCAL_MACHINE, true, _T("HKEY_LOCAL_MACHINE"));
  return LocalMachine;
}

//
//
//
TRegKey&
TRegKey::GetUsers()
{
  static TRegKey Users(HKEY_USERS, true, _T("HKEY_USERS"));
  return Users;
}

//
//
//
TRegKey&
TRegKey::GetCurrentConfig()
{
  static TRegKey CurrentConfig(HKEY_CURRENT_CONFIG, true, _T("HKEY_CURRENT_CONFIG"));
  return CurrentConfig;
}

//
//
//
TRegKey&
TRegKey::GetDynData()
{
  static TRegKey DynData(HKEY_DYN_DATA, true, _T("HKEY_DYN_DATA"));
  return DynData;
}

//
//
//
TRegKey&
TRegKey::GetPerformanceData()
{
  static TRegKey PerformanceData(HKEY_PERFORMANCE_DATA, true, _T("HKEY_PERFORMANCE_DATA"));
  return PerformanceData;
}

//
/// Initialize the object with the passed-in data.
//
/// Creates or opens a key given a base key and a subkeyname. Security information
/// is ignored in 16bit (and under Win95).  This can also provide an ok-to-create or
/// open-only indicator.
//
TRegKey::TRegKey(THandle baseKey, LPCTSTR keyName, REGSAM samDesired, TCreateOK createOK)
:
  Key(0),
  Name(strnewdup(keyName))
{
  Init(baseKey, samDesired, createOK);
}

//
/// String-aware overload
//
TRegKey::TRegKey(THandle baseKey, const tstring& keyName, REGSAM samDesired, TCreateOK createOK)
:
  Key(0),
  Name(strnewdup(keyName.c_str()))
{
  Init(baseKey, samDesired, createOK);
}

//
/// Initialization shared by the constructors
//
void TRegKey::Init(THandle baseKey, REGSAM samDesired, TCreateOK createOK)
{
  if (createOK == CreateOK) {
    DWORD disposition;
    ::RegCreateKeyEx(baseKey, Name, 0, _T("")/*class*/, 
      REG_OPTION_NON_VOLATILE, samDesired, 
      0/*SecurityAttributes*/, &Key, &disposition);
  }
  else
    ::RegOpenKeyEx(baseKey, Name, 0, samDesired, &Key);

  if (Key)
    QueryInfo(0, 0, &SubkeyCount, 0, 0, &ValueCount, 0, 0, 0, 0);

  ShouldClose = true;

//  TXRegistry::Check(long(Key), Name);
}

//
/// Constructs a key given the current position of a regkey iterator.
//
TRegKey::TRegKey(const TRegKeyIterator& iter, REGSAM samDesired)
:
  Key(0),
  Name(new tchar[_MAX_PATH+1])
{
  iter.BaseKey().EnumKey(iter.Current(), Name, _MAX_PATH+1);
  ::RegOpenKeyEx(iter.BaseKey(), Name, 0, samDesired, &Key);

  ShouldClose = true;
  QueryInfo(0, 0, &SubkeyCount, 0, 0, &ValueCount, 0, 0, 0, 0);
}

//Changes by Peter Sliepenbeek
#if (((__GNUC__ != 3) || (__GNUC_MINOR__ != 3)) && \
  ((__GNUC__ != 3) || (__GNUC_MINOR__ != 2)))

//
//
/// Construct a TRegKey object around an existing key. Commonly used for
/// wrapping the preexisting pseudo-keys.
TRegKey::TRegKey(THandle aliasKey, bool shouldClose, LPCTSTR keyName)
:
  Key(aliasKey),
  Name(strnewdup(keyName)),
  ShouldClose(shouldClose)
{
  Init();
}

//
/// String-aware overload
//
TRegKey::TRegKey(THandle aliasKey, bool shouldClose, const tstring& keyName)
:
  Key(aliasKey),
  Name(strnewdup(keyName.c_str())),
  ShouldClose(shouldClose)
{
  Init();
}

//
/// Initialization shared by the constructors
//
void TRegKey::Init()
{
// copied from old unix owl (JAM 4-16-01)
// Added by Val Ovechkin 10:50 AM 5/22/98
// CC 5.1 filtered out Sun.  dont know if HP still needs this
//#if defined( __unix ) && !defined(__SUNPRO_CC)
//#define __try try
//#define __except catch
//#endif


  //JJH - WINELIB stuff
#  if !defined(WINELIB)
#    if !defined(BI_COMP_BORLANDC)
  __try 
#    else  //#if !defined(BI_COMP_BORLANDC)
  try
#    endif //#if !defined(BI_COMP_BORLANDC)
#  endif //#  if !defined(WINELIB)
  {
    long err = QueryInfo(0, 0, &SubkeyCount, 0, 0, &ValueCount, 0, 0, 0, 0);
    if (err != S_OK && err != ERROR_INSUFFICIENT_BUFFER)
      Key = 0;
  }//__try

  // Some key handles are unsupported & sometimes the OS crashes, & doesn't
  // just return an error. Catch it here & zero out this key.
  //
//  __except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ?
//              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
//    Key = 0;
//  }
//  __endexcept
// JRS fix to get it to compile in OCC, this should be the same as above :)
  //JJH - WINELIB stuff
#  if !defined(WINELIB)
  __except(true)
  {
    if (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
      Key = 0;
    else
      RaiseException(GetExceptionCode(), 0, 0, 0);
  }
  __endexcept
#  endif // !defined(WINELIB)
}

//Changes by Peter Sliepenbeek
#endif  // (((__GNUC__ != 3) || (__GNUC_MINOR__ != 3)) &&
        // ((__GNUC__ != 3) || (__GNUC_MINOR__ != 2)))

//
/// Destructor closes the key if necessary and deletes any previously
/// allocated memory by the object.
//
TRegKey::~TRegKey()
{
  if (Key && ShouldClose)
    ::RegCloseKey(Key);
  delete[] Name;
}

//
/// Retrieves information about this registry key.
// Wrapper for RegQueryInfoKey API.
//
long
TRegKey::QueryInfo(LPTSTR class_, DWORD* classSize,
                   DWORD* subkeyCount,  DWORD* maxSubkeySize,
                   DWORD* maxClassSize, DWORD* valueCount,
                   DWORD* maxValueName, DWORD* maxValueData,
                   DWORD* secDescSize, PFILETIME lastWriteTime)
{

#if 0
//Jogy - Querying the class of a predefined root key can return ERROR_INVALID_PARAMETER.
  tchar    db[256];
  DWORD    d1 = 256;
#endif

  DWORD    d2 = 0;
  DWORD    d3 = 0;
  DWORD    d4 = 0;
  DWORD    d5 = 0;
  DWORD    d6 = 0;
  DWORD    d7 = 0;
  DWORD    d8 = 0;
  FILETIME  ft;

  // NT 3.50 has trouble with 0 ptrs for some of these
  //
  return ::RegQueryInfoKey(
#if 0
                           Key, class_ ? class_ : db,
                           classSize ? classSize : &d1, 0,
#else
                           Key, class_, classSize, 0,
#endif
                           subkeyCount ? subkeyCount : &d2,
                           maxSubkeySize ? maxSubkeySize : &d3,
                           maxClassSize ? maxClassSize : &d4,
                           valueCount ? valueCount : &d5,
                           maxValueName ? maxValueName : &d6,
                           maxValueData ? maxValueData : &d7,
                           secDescSize ? secDescSize : &d8,
                           lastWriteTime ? lastWriteTime : &ft);
}

//
/// Completely eliminates a child key, including any of its subkeys. RegDeleteKey
/// fails if a key has subkeys, so must tail-recurse to clean them up first.
//
long
TRegKey::NukeKey(LPCTSTR subKeyName)
{
  {
    TRegKey subKey(*this, subKeyName, KEY_ALL_ACCESS, TRegKey::NoCreate);

    // Don't increment since deleting & all subkeys will slide down to 0
    //
    for (TRegKeyIterator i(subKey); i; ) {
      TRegKey subSubKey(i);
      if (!subSubKey)
        break;     // All done, ran out before iterator knew, since deleting
      long err = subKey.NukeKey(subSubKey.GetName());
      if (err)
        return err;
    }
  }
  return DeleteKey(subKeyName);
}

//
/// Creates a registry value object from the specified registry key and name.
/// If the name is null or empty, the object will represent the key's default value.
//
TRegValue::TRegValue(const TRegKey& key, LPCTSTR name)
:
  Key(key), Name(name ? name : _T("")), DataType(REG_NONE), Data(0), DataSize(0)
{
  QueryTypeAndSize();
}

//
/// String-aware overload
/// If the name is empty, the object will represent the key's default value.
//
TRegValue::TRegValue(const TRegKey& key, const tstring& name)
:
  Key(key), Name(name), DataType(REG_NONE), Data(0), DataSize(0)
{
  QueryTypeAndSize();
}

//
/// Query the registry for the value type and data size, though not the actual data.
/// The value data itself is retrieved lazily; see RetrieveOnDemand.
/// Returns the error code returned by the query.
//
long TRegValue::QueryTypeAndSize()
{
  long r = Key.QueryValue(Name, &DataType, 0, &DataSize);
  WARN(r != ERROR_SUCCESS, "TRegValue::QueryTypeAndSize: Query failed: " << Key.GetName() << '\\' << Name);
  return r;
}

//
/// Creates a registry object from the current location of the specified iterator.
//
TRegValue::TRegValue(const TRegValueIterator& iter)
:
  Key(iter.BaseKey()), Name(), DataType(REG_NONE), Data(0), DataSize(0)
{
  uint32 nameSize = 16383; // See Windows API documentation; http://msdn.microsoft.com/en-us/library/windows/desktop/ms724872.aspx
  std::vector<tchar> nameBuffer(nameSize);
  long r = Key.EnumValue(iter.Current(), &nameBuffer[0], nameSize, &DataType, 0, &DataSize);
  if (r != ERROR_SUCCESS) throw TXRegistry(_T("Registry value enumeration failed"), Key.GetName());
  nameBuffer.resize(nameSize); // EnumValue sets nameSize to the number of characters stored, excluding null-termination.
  Name.assign(nameBuffer.begin(), nameBuffer.end());
}

//
/// Returns the data associated with this value as a 32-bit unsigned integer.
/// Throws an exception if the value is of incompatible type.
//
TRegValue::operator uint32() const
{
  RetrieveOnDemand();
  if (DataType != REG_DWORD && 
    DataType != REG_DWORD_BIG_ENDIAN
    )
    throw TXRegistry(_T("Incompatible registry value type conversion (uint32)"), Key.GetName());

  CHECK(DataSize == sizeof(uint32));
  return *reinterpret_cast<uint32*>(&Data[0]);
}

//
/// Returns the data associated with this value as a 64-bit unsigned integer.
/// Throws an exception if the value is of incompatible type.
//
TRegValue::operator uint64() const
{
  RetrieveOnDemand();
  if (DataType != REG_QWORD)
    throw TXRegistry(_T("Incompatible registry value type conversion (uint64)"), Key.GetName());

  CHECK(DataSize == sizeof(uint64));
  return *reinterpret_cast<uint64*>(&Data[0]);
}

//
/// Returns the data associated with this value as a null-terminated string.
/// Throws an exception if the value is of incompatible type or is otherwise invalid.
//
TRegValue::operator LPCTSTR() const
{
  RetrieveOnDemand();
  if (DataType != REG_SZ && 
    DataType != REG_MULTI_SZ && 
    DataType != REG_EXPAND_SZ &&
    !(DataType == REG_LINK && sizeof(tchar) == 2) // REG_LINK is always a Unicode UTF-16 string.
    )
    throw TXRegistry(_T("Incompatible registry value type conversion (LPCTSTR)"), Key.GetName());

  // Ensure null-termination.
  // The data may have been stored without proper null-termination.
  // Also, check that the string has room for null-termination.
  //
  CHECK(!Data.empty());
  LPTSTR s = reinterpret_cast<LPTSTR>(&Data[0]);
  const size_t n = Data.size() / sizeof(tchar);
  if (n == 0) throw TXRegistry(_T("Invalid registry value (string)"), Key.GetName());
  WARN((Data.size() % sizeof(tchar)) != 0, "TRegValue::operator LPCTSTR: Oddly sized string");
  WARN(s[n - 1] != _T('\0'), "TRegValue::operator LPCTSTR: String lacks null-termination");
  s[n - 1] = _T('\0');
  if (DataType == REG_MULTI_SZ && n > 1)
  {
    WARN(s[n - 2] != _T('\0'), "TRegValue::operator LPCTSTR: Multi-string lacks double null-termination");
    s[n - 2] = _T('\0');
  }
  return s;
}

//
/// Returns the data associated with this value.
//
const uint8*
TRegValue::GetData() const 
{
  RetrieveOnDemand();
  return Data.empty() ? 0 : &Data[0];
}

//
/// Sets the data associated with this value.  'type' describes the type of the
/// value. 'data' is the address of the data. 'size' specifies the length in
/// characters.
//
long
TRegValue::Set(uint32 type, const uint8* data, uint32 dataSize)
{
  long r = Key.SetValue(Name, type, data, dataSize);
  if (r != ERROR_SUCCESS) return r;

  // If same size, copy new data, otherwise retrieve the data on demand later.
  //
  DataType = type;
  if (dataSize == Data.size() && !Data.empty())
  {
    memcpy(&Data[0], data, Data.size());
  }
  else 
  {
    DataSize = dataSize;
    Data.clear();
  }
  return ERROR_SUCCESS;
}

//
/// Sets the data associated with this value.
/// The type of the value is set to REG_DWORD.
//
long
TRegValue::Set(uint32 v)
{
  const uint8* data = reinterpret_cast<const uint8*>(&v);
  uint32 dataSize = sizeof(uint32);
  return Set(REG_DWORD, data, dataSize);
}

//
/// Sets the data associated with this value.
/// The type of the value is set to REG_QWORD.
//
long
TRegValue::Set(uint64 v)
{
  const uint8* data = reinterpret_cast<const uint8*>(&v);
  uint32 dataSize = sizeof(uint64);
  return Set(REG_QWORD, data, dataSize);
}

//
/// Sets the data associated with this value.
/// The type of the value is set to REG_SZ.
//
long
TRegValue::Set(LPCTSTR v)
{
  const uint8* data = reinterpret_cast<const uint8*>(v);
  uint32 dataSize = static_cast<uint32>((::_tcslen(v) + 1) * sizeof(tchar));
  return Set(REG_SZ, data, dataSize);
}

//
/// Sets the data associated with this value.
/// If the string contains a terminating null, then it is stored in the registry as a multi-string,
/// i.e. as a REG_MULTI_SZ value type, e.g. "multi\0string\0\0". Else, it is stored as a REG_SZ.
//
long 
TRegValue::Set(const tstring& v) 
{
  bool isMultiString = (!v.empty() && *v.rbegin() == _T('\0'));
  const uint8* data = reinterpret_cast<const uint8*>(v.c_str());
  uint32 dataSize = static_cast<uint32>((v.size() + 1) * sizeof(tstring::value_type));
  uint32 dataType = isMultiString ? REG_MULTI_SZ : REG_SZ;
  return Set(dataType, data, dataSize);
}

//
/// Forwards to Set (uint32). 
/// Throws TXRegistry if data assignment fails.
//
TRegValue&
TRegValue::operator =(uint32 v)
{
  long r = Set(v);
  if (r != ERROR_SUCCESS) throw TXRegistry(_T("Registry value assignment failed (uint32)"), Key.GetName());
  return *this;
}

//
/// Forwards to Set (uint64). 
/// Throws TXRegistry if data assignment fails.
//
TRegValue&
TRegValue::operator =(uint64 v)
{
  long r = Set(v);
  if (r != ERROR_SUCCESS) throw TXRegistry(_T("Registry value assignment failed (uint64)"), Key.GetName());
  return *this;
}

//
/// Forwards to Set (LPCTSTR). 
/// Throws TXRegistry if data assignment fails.
//
TRegValue&
TRegValue::operator =(LPCTSTR v)
{
  long r = Set(v);
  if (r != ERROR_SUCCESS) throw TXRegistry(_T("Registry value assignment failed (LPCTSTR)"), Key.GetName());
  return *this;
}

//
/// Forwards to Set (const tstring&). 
/// Throws TXRegistry if data assignment fails.
//
TRegValue&
TRegValue::operator =(const tstring& v)
{
  long r = Set(v);
  if (r != ERROR_SUCCESS) throw TXRegistry(_T("Registry value assignment failed (tstring)"), Key.GetName());
  return *this;
}

//
/// Removes the value from its associated key, and clears the state of this object.
//
long
TRegValue::Delete()
{
  long r = Key.DeleteValue(Name);
  if (r != ERROR_SUCCESS) return r;
  DataType = REG_NONE;
  try {std::vector<uint8>().swap(Data);} // This deallocation idiom may throw, so guard.
  catch (...) {Data.clear();} // Simply clear instead, if things go awry.
  DataSize = 0;
  return ERROR_SUCCESS;
}

//
/// Retrieves and stores the data for the value when requested for the first time (i.e. lazily).
/// Note that this function may fail due to synchronisation issues; the registry value
/// may have been changed, e.g. by another process, since the object was constructed.
//
void
TRegValue::RetrieveOnDemand() const
{
  if (!Data.empty()) return;
  if (DataType == REG_NONE || DataSize == 0) throw TXRegistry(_T("Registry value is void"), Key.GetName());

  uint32 dataType = DataType;
  Data.resize(DataSize);
  uint32 dataSize = DataSize;
  long r = Key.QueryValue(Name, &dataType, &Data[0], &dataSize);
  if (r != ERROR_SUCCESS) throw TXRegistry(_T("Registry value query failed"), Key.GetName());
  if (dataType != DataType) throw TXRegistry(_T("Unexpected registry value type change"), Key.GetName());
  if (dataSize != DataSize) throw TXRegistry(_T("Unexpected registry value size change"), Key.GetName());
}

//----------------------------------------------------------------------------

//
/// Registry exception checking. Throw a TXRegistry if arg is non-zero.
//
void
TXRegistry::Check(long stat, LPCTSTR key)
{
  if (!stat) return;

  // Convert 1 (true) to E_FAIL.
  //
  if (stat == 1) 
    stat = E_FAIL;

  LPCTSTR k = key ? key : _T("(NULL)");
  tostringstream os;
  os << _T("Registry failure on key: ") << k << _T(", ErrorCode = ") << stat << _T("\n");
  tstring msg = os.str();

  WARN(true, msg);
  throw TXRegistry(msg, key);
}

//----------------------------------------------------------------------------

//
// Initialize object with the passed-in data.
// Counts the number of items in the list.
//
TRegTemplateList::TRegTemplateList(TRegKey& basekey, LPCTSTR list[])
:
  BaseKey(basekey),
  List(list)
{
  PRECONDITION(List);

  for (Count = 0; List[Count]; Count++)
    ;
  EnabledFlags = new int8[Count];
}

//
// Destructor deletes any previously allocated memory.
//
TRegTemplateList::~TRegTemplateList()
{
  delete[] EnabledFlags;
}

//
// Enable items from the set.
//
void
TRegTemplateList::Enable(LPCTSTR set)
{
  for (LPCTSTR pc = set; *pc != 0; pc++)
    EnabledFlags[*pc - 1] = 0;                 // Selectively allow enable
}

//
// Activate the items in the set.
//
void
TRegTemplateList::Activate(LPCTSTR set)
{
  for (LPCTSTR pc = set; *pc != 0; pc++)
    EnabledFlags[*pc - 1]++;
}

//----------------------------------------------------------------------------

//
// Initialize the object with the passed-in data.
// Counts the number of items in the list.
//
TRegParamList::TRegParamList(TEntry* list)
:
  List(list)
{
  PRECONDITION(List);

  for (Count = 0; List[Count].Param; Count++)
    ;
  typedef LPCTSTR cpchar;
  Values = new cpchar[Count];
}

//
// Destructor deletes any previously allocated memory.
//
TRegParamList::~TRegParamList()
{
  delete[] Values;
}

//
// Reset all values to their default values.
//
void
TRegParamList::ResetDefaultValues()
{
  for (int i = Count; --i >= 0; )
    Values[i] = List[i].Default;
}

//
// Look for a given param in this param list
//
int TRegParamList::Find(LPCTSTR param)
{
  PRECONDITION(param);
  if (!param)
    return -1;

  int i = Count;
  while (--i >= 0) {
    if (_tcscmp(List[i].Param, param) == 0)
      break;
  }
  return i;
}

//----------------------------------------------------------------------------

//
// Initialize the symbol table with the data.
//
TRegSymbolTable::TRegSymbolTable(TRegKey& basekey, LPCTSTR tplList[],
                                 TRegParamList::TEntry* paramList)
:
  Templates(basekey, tplList),
  Params(paramList)
{
  UserKeyCount = 0;
}

//
// Enable all templates, or if filter is given enable only those
// Initialize paramater values with defaults
//
void
TRegSymbolTable::Init(tchar* filter)
{
  if (filter) {
    Templates.DisableAll();
    Templates.Enable(filter);
  }
  else {
    Templates.EnableAll();
  }

  Params.ResetDefaultValues();
  UserKeyCount = 0;
}

//
// Scan through provided items assinging values from the item to the matching
// param. A langId is needed for locale translation on assignment.
//
void
TRegSymbolTable::UpdateParams(TLangId lang, TRegItem* item)
{
  for (; item && item->Key; item++) {
    // Note presence of user-specified key and value, process when streaming
    //
    if (*item->Key == ' ') {
      UserKeyCount++;
    }
    else {
      // Replace default with user-specified parameter value
      //
      _USES_CONVERSION;
      int i = Params.Find(_A2W(item->Key));
      TXRegistry::Check(i < 0, _A2W(item->Key));
      Params.Value(i) = item->Value.Translate(lang);

      // Activate all templates invoked by parameter
      //
      Templates.Activate(Params[i].TemplatesNeeded);
    }
  }
}

//
// Scan thru and process enabled templates, substituting filled in parameter
// values and streaming the resulting strings into the out stream provided.
//
void
TRegSymbolTable::StreamOut(TRegItem* item, tostream& out)
{
#if defined(UNICODE)
  _USES_CONVERSION;
   TAPointer<tchar> conv_buf(new tchar[MAX_PATH]);
#  define _A2WB_(val) _A2WB(val,(tchar*)conv_buf)
#else
#  define _A2WB_(val) val
#endif
  for (int itpl = 1; itpl <= Templates.GetCount() || UserKeyCount--; itpl++) {

    // Setup pt from standard templates, or from userKeys in item prepended
    // by spaces.
    //
    LPCTSTR userval = 0;
    LPCTSTR pt;
    if (itpl <= Templates.GetCount()) {  // Processing standard template array
      if (!Templates.IsActive(itpl))
        continue;
      pt = Templates[itpl];
    }
    else {     // Now processing user-defined templates
      while (*(pt = _A2WB_(item->Key)) != _T(' '))
        item++;
      pt++;
      userval = item->Value;
    }

    // Walk thru template (pt) copying it into buf, replacing <params> on the
    // way.
    //
    const int bufSize  = 512;
    TAPointer<tchar> buf(new tchar[bufSize]);

    tchar*    data = 0; // Optional data for keys-value=data
    LPCTSTR   pc;       // Point where param token began
    tchar*   pb = buf; // Working buffer write pointer
    for (;;) {
      tchar  c = *pt++;  // Current character being copied
      switch (c) {
        case _T('<'):
          pc = pb;      // Remember where param token starts
          continue;
        case _T('>'): {
          *pb = 0;        // Terminate param token

          // Lookup param in param list
          //
          int i = Params.Find(pc);
          TXRegistry::Check(i < 0, pc); // internal err

          // Now get the value obtained above
          //
          pb = (tchar*)pc; // Move buffer ptr back
          TXRegistry::Check((pc = Params.Value(i)) == 0, Params[i].Param);

          if (*pc == 0 && *pt == _T(' '))
            pt++;

          // Copy value to buf
          //
          while (*pc != 0)
            *pb++ = *pc++;
          continue;
        }
        case _T('='):
          while (*(pb-1) == _T(' '))  // Remove trailing space before =
            pb--;
          *pb++ = 0;              // Terminate keys-value
          data = pb;              // Rest of pt will be data written from here
          while (*pt == _T(' '))
            pt++;
          continue;
        default:
          *pb++ = c;  // Copy character to buf. Param name will be overwriten
          continue;
        case 0:
          *pb++ = 0;
          if (!userval)
            break;
          pt = userval;
          userval = 0;
          data = pb;
          continue;
      }
      if (!c)
        break;
    }
    // Now write out a key with an optional assignment value to output stream
    //
    out
      << Templates.GetBaseKey().GetName()
      << _T('\\') << (LPCTSTR)buf;
    if (data)
      out << _T(" = ") << data;
    out << _T('\n');
  }
}

//----------------------------------------------------------------------------

//
/// Walks through an input stream and uses  basekey\\key\\key=data  lines to set
/// registry entries.
///
/// Has named value support in the form:    basekey\\key\\key|valuename=data
//
void TRegistry::Update(TRegKey& baseKey, tistream& in)
{
  // Loop thru all lines in input stream
  //
  while (!in.eof()) {
    // Get line into entry buffer, skip over base key if it exists, ignore line
    // if it doesn't
    //
    tchar entry[512];
    in.get(entry, ::_tcslen(baseKey.GetName())+1);
    if (_tcscmp(entry, baseKey.GetName()) != 0)
      continue;
    in.get(entry, 1+1);  // Eat the '\' separator
    in.getline(entry, 512);

    // Split entry into keys-value and data strings
    //
    tchar* data = _tcschr(entry, _T('='));
    if (data) {
      tchar* pc = data;
      while (*(pc-1) == _T(' '))
        pc--;
      *pc = 0;
      while (*(++data) == _T(' '))
        ;
    }
    else
      data = _T("");
    tchar* valName = _tcschr(entry, _T('|'));
    if (valName)
      *valName++ = 0;  // Terminate key at value name separator

    // Set default value
    //
    if (valName) {
      TRegKey subKey(baseKey, entry);
      TXRegistry::Check(subKey.SetValue(valName, REG_SZ, (const uint8*)data, 0), entry);
    }
    else {
      TXRegistry::Check(baseKey.SetDefValue(entry, REG_SZ, data, 0), entry);
    }
  }
}

//
/// Walks through an input stream and uses  basekey\\key\\key=data  lines to check
/// registry entries.
/// Returns the number of differences. Zero means a complete match.
///
/// Has named value support in the following form:  basekey\\key\\key|valuename=data
//
int TRegistry::Validate(TRegKey& baseKey, tistream& in)
{
  int diffCount = 0;
  while (in) {
    // Get line into entry buffer, skip over hive key if it exists, ignore line
    // if it doesn't
    //
    tchar entry[512];
    //Staso : Call get instead of getline - getline eats too much
    in.get(entry, ::_tcslen(baseKey.GetName())+1);  // 1 for '\', 1 for 0
    if (_tcscmp(entry, baseKey.GetName()) != 0)
      continue;
    in.get(entry, 1+1);  // Eat the '\' separator
    in.getline(entry, 512);

    // Split entry into keys-value and data strings
    //
    tchar* data = _tcschr(entry, _T('='));
    if (data) {
      tchar* pc = data;
      while (*(pc-1) == _T(' '))
        pc--;
      *pc = 0;
      while (*(++data) == _T(' '))
        ;
    }
    tchar* valName = _tcschr(entry, _T('|'));
    if (valName)
      *valName = _T('\\');  // Replace value name separator with \ for query

    // Now lookup keys-value part to get its data and see if it matches the
    // data value from the instream
    //
    tchar buf[300];
    uint32 bufSize = 300;
    if (baseKey.QueryDefValue(entry, buf, &bufSize) != S_OK
        || (data && _tcscmp(data, buf) != 0))
      diffCount++;
  }
  return diffCount;
}

//
/// Unregisters entries given a reglist.  An optional overrides regItem.  Returns
/// the number of errors from deleting keys.
//
int
TRegistry::Unregister(TRegList& regInfo, TUnregParams* params, TRegItem* overrides)
{
#if defined(UNICODE)
  _USES_CONVERSION;
   TAPointer<tchar> conv_buf(new tchar[MAX_PATH]);
#  undef _A2WB_
#  undef _W2AB_
#  define _A2WB_(val) _A2WB(val,(tchar*)conv_buf)
#  define _W2AB_(val) _W2AB(val,(tchar*)conv_buf)
#else
#  undef _A2WB_
#  undef _W2AB_
#  define _A2WB_(val) val
#  define _W2AB_(val) val
#endif
  int  errorCount = 0;

  // Loop thru unregister params to nuke each root level key, thus cleaning
  // up all nested keys too
  //
  for (int i = 0; params[i].Name && params[i].BaseKey; i++) {
    LPCTSTR regKey = regInfo[_W2AB_(params[i].Name)];

    // If the param not found & it matches the overrides item, then use that
    // value
    //
    if (!regKey && overrides && _tcscmp(_A2WB_(overrides->Key), params[i].Name) == 0)
      regKey = overrides->Value;

    // If key-value was found, unregister it
    //
    if (regKey) {
      tchar buff[16];
      if (params[i].Prepend) { // Special case prepending char to key
        buff[0] = _T('.');
        ::_tcscpy(buff+1, regKey);
        regKey = buff;
      }

      // Nuke the key, using the basekey as a reference point
      //
      if (params[i].BaseKey->NukeKey(regKey))
        errorCount++;         // Should throw exception if certain errors?
    }
  }
  return errorCount;
}

} // OWL namespace
/* ========================================================================== */
