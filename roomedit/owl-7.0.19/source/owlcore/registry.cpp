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
#include <utility>
#include <type_traits>
#include <vector>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

//
/// Special predefined root key used by shell and OLE applications (`HKEY_CLASSES_ROOT`).
/// Registry entries subordinate to this key define types (or classes) of documents and the
/// properties associated with those types. Shell and COM applications use the information
/// stored under this key.
/// \sa https://docs.microsoft.com/en-us/windows/win32/sysinfo/hkey-classes-root-key
//
auto TRegKey::GetClassesRoot() -> TRegKey&
{
  static TRegKey ClassesRoot(HKEY_CLASSES_ROOT, true, _T("HKEY_CLASSES_ROOT"));
  return ClassesRoot;
}

//
/// Commonly used subkey by shell and OLE applications (`HKEY_CLASSES_ROOT\CLSID`).
//
auto TRegKey::GetClassesRootClsid() -> TRegKey&
{
  static TRegKey ClassesRootClsid(GetClassesRoot(), _T("CLSID"), KEY_ALL_ACCESS, NoCreate);
  return ClassesRootClsid;
}

//
/// Special predefined root key containing information about the current hardware profile of the
/// local computer system (`HKEY_CURRENT_CONFIG`).
/// The information under this key describes only the differences between the current hardware
/// configuration and the standard configuration. Information about the standard hardware
/// configuration is stored under the Software and System keys of `HKEY_LOCAL_MACHINE`.
/// \sa https://technet.microsoft.com/library/286f12b7-265b-4632-a4e1-987d025023e6
//
auto TRegKey::GetCurrentConfig() -> TRegKey&
{
  static TRegKey CurrentConfig(HKEY_CURRENT_CONFIG, true, _T("HKEY_CURRENT_CONFIG"));
  return CurrentConfig;
}

//
/// Special predefined root key defining the preferences of the current user (`HKEY_CURRENT_USER`).
/// These preferences include the settings of environment variables, data about program groups,
/// colors, printers, network connections, and application preferences. This key makes it easier to
/// establish the current user's settings; the key maps to the current user's branch in HKEY_USERS.
/// Under this key, software vendors store the current user-specific preferences to be used within
/// their applications. Microsoft, for example, creates the `HKEY_CURRENT_USER\Software\Microsoft`
/// key for its applications to use, with each application creating its own subkey under the
/// Microsoft key.
/// \sa https://technet.microsoft.com/library/6b6d2dcc-a083-4c49-9000-6f1324b20877
//
auto TRegKey::GetCurrentUser() -> TRegKey&
{
  static TRegKey CurrentUser(HKEY_CURRENT_USER, true, _T("HKEY_CURRENT_USER"));
  return CurrentUser;
}

//
/// Special predefined root key defining the physical state of the computer (`HKEY_LOCAL_MACHINE`).
/// Includes data about the bus type, system memory, and installed hardware and software. It
/// contains subkeys that hold current configuration data, including Plug and Play information
/// (the Enum branch, which includes a complete list of all hardware that has ever been on the
/// system), network logon preferences, network security information, software-related information
/// (such as server names and the location of the server), and other system information.
/// \sa https://technet.microsoft.com/library/f4704e81-0b33-4ecd-b2e4-e41b50bb758c
//
auto TRegKey::GetLocalMachine() -> TRegKey&
{
  static TRegKey LocalMachine(HKEY_LOCAL_MACHINE, true, _T("HKEY_LOCAL_MACHINE"));
  return LocalMachine;
}

//
/// Special predefined root key used to obtain performance data (`HKEY_PERFORMANCE_DATA`).
/// The data is not actually stored in the registry; the registry functions cause the system to
/// collect the data from its source.
//
auto TRegKey::GetPerformanceData() -> TRegKey&
{
  static TRegKey PerformanceData(HKEY_PERFORMANCE_DATA, true, _T("HKEY_PERFORMANCE_DATA"));
  return PerformanceData;
}

//
/// Special predefined root key defining the default user configuration (`HKEY_USERS`).
/// Registry entries subordinate to this key define the default user configuration for new users
/// on the local computer and the user configuration for the current user.
/// \sa TRegKey::GetCurrentUser
//
auto TRegKey::GetUsers() -> TRegKey&
{
  static TRegKey Users(HKEY_USERS, true, _T("HKEY_USERS"));
  return Users;
}

//
/// Creates or opens a key given a base key and a subkey name.
/// The user can specify the desired access rights to the key and provide an ok-to-create or
/// open-only indicator. To check whether a subkey exists before creating a TRegKey, you can call
/// TRegKey::HasSubkey on the base key.
///
/// \returns Throws TXRegistry on failure.
///
/// \note Passing an empty string for the key name is allowed but not very useful. It will just
/// give you a new handle to the base key. In most cases, you should use the aliasing constructor
/// instead; TRegKey::TRegKey(THandle aliasKey, bool shouldClose, tstring keyName).
//
TRegKey::TRegKey(THandle baseKey, tstring keyName, REGSAM samDesired, TCreateOK createOK)
  : Key{}, Name{move(keyName)}, SubkeyCount{}, ValueCount{}, ShouldClose{true}
{
  // Use a local lambda to handle SEH system exceptions (not compatible with C++ exception unwinding).
  // A system SEH exception with code EXCEPTION_ACCESS_VIOLATION is translated to ERROR_ACCESS_DENIED.
  // Any other SEH exception is not handled.
  //
  const auto init = [&]() noexcept -> long // Error code.
  {
    __try
    {
      if (createOK == CreateOK)
      {
        auto disposition = DWORD{};
        const auto r = ::RegCreateKeyEx(baseKey, Name.c_str(), 0 /* Reserved */, nullptr /* Class */,
          REG_OPTION_NON_VOLATILE, samDesired, nullptr /* SecurityAttributes */, &Key, &disposition);
        if (r != ERROR_SUCCESS) return r;
      }
      else
      {
        const auto r = ::RegOpenKeyEx(baseKey, Name.c_str(), 0 /* Options */, samDesired, &Key);
        if (r != ERROR_SUCCESS) return r;
      }
      const auto r = QueryInfo(nullptr, nullptr, &SubkeyCount, nullptr, nullptr, &ValueCount, nullptr, nullptr, nullptr, nullptr);
      if (r != ERROR_SUCCESS && r != ERROR_INSUFFICIENT_BUFFER) return r;
      return ERROR_SUCCESS;
    }
    __except (true)
    {
      const auto x = GetExceptionCode();
      if (x == EXCEPTION_ACCESS_VIOLATION) return ERROR_ACCESS_DENIED;
      RaiseException(x, 0, 0, nullptr);
      return -1; // Never executed.
    }
  };
  const auto r = init();
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegKey::TRegKey: Init failed"), Name, r};
  CHECK(Key); // Postcondition.
}

//
/// LPCTSTR overload.
/// Forwards to TRegKey::TRegKey(THandle baseKey, tstring keyName, REGSAM, TCreateOK).
/// \note If you pass `nullptr` for the key name, it will be translated to an empty string.
//
TRegKey::TRegKey(THandle baseKey, LPCTSTR keyName, REGSAM samDesired, TCreateOK createOK)
  : TRegKey{baseKey, keyName ? keyName : tstring{}, samDesired, createOK}
{
  CHECK(Key); // Postcondition.
}

//
/// Constructs a key given the current position of a TRegKeyIterator.
/// \returns Throws TXRegistry on failure.
//
TRegKey::TRegKey(const TRegKeyIterator& iter, REGSAM samDesired)
  : Key{}, Name{}, SubkeyCount{}, ValueCount{}, ShouldClose{true}
{
  // Use a local lambda to handle SEH system exceptions (not compatible with C++ exception unwinding).
  // A system SEH exception with code EXCEPTION_ACCESS_VIOLATION is translated to ERROR_ACCESS_DENIED.
  // Any other SEH exception is not handled.
  //
  const auto init = [&]() noexcept -> long // Error code.
  {
    __try
    {
      tchar name[_MAX_PATH + 1];
      const auto re = iter.BaseKey().EnumKey(iter.Current(), &name[0], COUNTOF(name));
      if (re != ERROR_SUCCESS) return re;
      Name = name;
      const auto ro = ::RegOpenKeyEx(iter.BaseKey(), Name.c_str(), 0, samDesired, &Key);
      if (ro != ERROR_SUCCESS) return ro;
      const auto rq = QueryInfo(nullptr, nullptr, &SubkeyCount, nullptr, nullptr, &ValueCount, nullptr, nullptr, nullptr, nullptr);
      if (rq != ERROR_SUCCESS && rq != ERROR_INSUFFICIENT_BUFFER) return rq;
      return ERROR_SUCCESS;
    }
    __except (true)
    {
      const auto x = GetExceptionCode();
      if (x == EXCEPTION_ACCESS_VIOLATION) return ERROR_ACCESS_DENIED;
      RaiseException(x, 0, 0, nullptr);
      return -1; // Never executed.
    }
  };
  const auto r = init();
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegKey::TRegKey: Init failed"), Name, r};
  CHECK(Key); // Postcondition.
}

//
//
/// Constructs a TRegKey object around an existing key.
/// Commonly used for wrapping the preexisting pseudo-keys, such as HKEY_CURRENT_USER.
/// If `true` is passed for parameter `shouldClose`, then the object takes ownership of the handle,
/// and the handle will be closed when the object is destructed.
///
/// \returns Throws TXRegistry on failure.
///
/// \sa The preexisting pseudo-keys can be accessed by the static member functions
/// TRegKey::GetClassesRoot, TRegKey::GetClassesRootClsid, TRegKey::GetCurrentConfig,
/// TRegKey::GetCurrentUser, TRegKey::GetLocalMachine, TRegKey::GetPerformanceData and
/// TRegKey::GetUsers.
//
TRegKey::TRegKey(THandle aliasKey, bool shouldClose, tstring keyName)
  : Key{aliasKey}, Name{move(keyName)}, SubkeyCount{}, ValueCount{}, ShouldClose{shouldClose}
{
  // Use a local lambda to handle SEH system exceptions (not compatible with C++ exception unwinding).
  // A system SEH exception with code EXCEPTION_ACCESS_VIOLATION is translated to ERROR_ACCESS_DENIED.
  // Any other SEH exception is not handled.
  //
  const auto init = [&]() noexcept -> long // Error code.
  {
    __try
    {
      const auto r = QueryInfo(nullptr, nullptr, &SubkeyCount, nullptr, nullptr, &ValueCount, nullptr, nullptr, nullptr, nullptr);
      if (r != ERROR_SUCCESS && r != ERROR_INSUFFICIENT_BUFFER) return r;
      return ERROR_SUCCESS;
    }
    __except (true)
    {
      const auto x = GetExceptionCode();
      if (x == EXCEPTION_ACCESS_VIOLATION) return ERROR_ACCESS_DENIED;
      RaiseException(x, 0, 0, nullptr);
      return -1; // Never executed.
    }
  };
  const auto r = init();
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegKey::TRegKey: Init failed"), Name, r};
  CHECK(Key); // Postcondition.
}

//
/// LPCTSTR overload.
/// Forwards to TRegKey::TRegKey(THandle, bool shouldClose, tstring keyName).
/// \note If you pass `nullptr` for the key name, it will be translated to an empty string.
//
TRegKey::TRegKey(THandle aliasKey, bool shouldClose, LPCTSTR keyName)
  : TRegKey{aliasKey, shouldClose, keyName ? keyName : tstring{}}
{
  CHECK(Key); // Postcondition.
}

//
/// Move constructor; steals the innards of the given object.
//
TRegKey::TRegKey(TRegKey&& v) noexcept
  : Key{}, Name{}, SubkeyCount{}, ValueCount{}, ShouldClose{}
{
  using std::swap;
  swap(Name, v.Name);
  swap(Key, v.Key);
  swap(SubkeyCount, v.SubkeyCount);
  swap(ValueCount, v.ValueCount);
  swap(ShouldClose, v.ShouldClose);
  WARN(!Key, _T("TRegKey::TRegKey: Moved from invalid key"));
}

//
/// Closes the underlying key handle, if owned.
//
TRegKey::~TRegKey()
{
  if (Key && ShouldClose)
  {
    [[maybe_unused]] const auto r = ::RegCloseKey(Key);
    WARN(r != ERROR_SUCCESS, _T("TRegKey::~TRegKey: RegCloseKey failed, error: ") << r);
  }
}

//
/// Retrieves information about this registry key.
/// Wrapper for RegQueryInfoKey in the Windows API.
/// \sa https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryinfokeya
//
long TRegKey::QueryInfo(LPTSTR class_, DWORD* classSize,
  DWORD* subkeyCount,  DWORD* maxSubkeyNameSize, DWORD* maxSubkeyClassSize, 
  DWORD* valueCount, DWORD* maxValueNameSize, DWORD* maxValueDataSize,
  DWORD* securityDescriptorSize, PFILETIME lastWriteTime) const
{
  return ::RegQueryInfoKey(Key,
    class_,
    classSize,
    nullptr, // Reserved, and must be NULL.
    subkeyCount,
    maxSubkeyNameSize,
    maxSubkeyClassSize,
    valueCount,
    maxValueNameSize,
    maxValueDataSize,
    securityDescriptorSize,
    lastWriteTime);
}

//
/// Functional-style overload.
/// \returns A data structure containing information about the key.
/// If an error occurs, TXRegistry is thrown.
//
auto TRegKey::QueryInfo() const -> TInfo
{
  auto i = TInfo{};
  auto classSize = DWORD{MAX_PATH};
  i.Class.resize(classSize); // Should include space for the null-terminator, according to the Windows API documentation.
  const auto p = const_cast<LPTSTR>(i.Class.data()); // Workaround for pre-C++17 compilers.
  const auto r = QueryInfo(
    p,
    &classSize,
    &i.SubkeyCount,
    &i.MaxSubkeyNameSize,
    &i.MaxSubkeyClassSize,
    &i.ValueCount,
    &i.MaxValueNameSize,
    &i.MaxValueDataSize,
    &i.SecurityDescriptorSize,
    &i.LastWriteTime);
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegKey::QueryInfo failed"), *this, r};
  i.Class.resize(classSize); // Returned classSize does not include the null-terminator, according to the Windows API documentation.
  i.Class.shrink_to_fit();
  return i;
}

//
/// Returns true if this key has a subkey with the given name.
/// If an error occurs, e.g. access is denied, it is assumed the subkey exists.
/// The key name may include a path with subkey names separated by backslash, e.g. "Software\\Program".
//
auto TRegKey::HasSubkey(const tstring& keyName) const -> bool
{
  PRECONDITION(Key);
  auto subkey = HKEY{};
  const auto r = ::RegOpenKeyEx(Key, keyName.c_str(), 0, KEY_READ, &subkey);
  if (subkey)
    ::RegCloseKey(subkey);
  return r != ERROR_FILE_NOT_FOUND;
}

//
/// Returns the subkey with the given name, if it exists.
/// If the key does not exist, std::nullopt is returned.
/// If access is denied, or any other error occurs, a TXRegistry exception is thrown.
/// The key name may include a path with subkey names separated by backslash, e.g. "Software\\Program".
///
/// \param samDesired A mask that specifies the desired access rights to the key to be opened. 
/// For possible values of this parameter, see the Windows API documentation.
/// 
/// \sa https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regopenkeyexa
/// \sa https://docs.microsoft.com/en-gb/windows/win32/sysinfo/registry-key-security-and-access-rights
//
auto TRegKey::GetSubkey(const tstring& keyName, REGSAM samDesired) const -> optional<TRegKey>
{
  PRECONDITION(Key);
  auto subkey = HKEY{};
  const auto r = ::RegOpenKeyEx(Key, keyName.c_str(), 0, samDesired, &subkey);
  if (r != ERROR_SUCCESS && r != ERROR_FILE_NOT_FOUND) throw TXRegistry{_T("TRegKey::GetSubKey: RegOpenKeyEx failed"), keyName, r};
  return subkey ? make_optional<TRegKey>(subkey, true, keyName) : nullopt; // TRegKey takes ownership.
}

//
/// Return true if this key has a value with the given name.
/// If the given name is empty, the default value is assumed, which always exists.
/// If an error occurs, e.g. access is denied, it is assumed the value exists.
//
auto TRegKey::HasValue(const tstring& valueName) const -> bool
{
  PRECONDITION(Key);
  return QueryValue(valueName, nullptr, nullptr, nullptr) != ERROR_FILE_NOT_FOUND;
}

//
/// Returns the value with the given name, if it exists within this key.
/// If the value does not exist, std::nullopt is returned.
/// If the given name is empty, the default value is assumed, which always exists.
/// If access is denied, or any other error occurs, a TXRegistry exception is thrown.
//
auto TRegKey::GetValue(const tstring& valueName) const -> optional<TRegValue>
{
  PRECONDITION(Key);
  return HasValue(valueName) ? make_optional<TRegValue>(*this, valueName) : nullopt;
}

//
/// Completely eliminates a child key, including any of its subkeys. 
//
long TRegKey::NukeKey(LPCTSTR subKeyName)
{
  // DeleteKey fails if a key has subkeys, so we must nuke them first.
  //
  const auto nukeSubkeys = [&](LPCTSTR keyName)
  {
    auto key = GetSubkey(keyName, KEY_ALL_ACCESS);
    if (!key) return ERROR_FILE_NOT_FOUND;
    auto subkeys = vector<tstring>{};
    for (auto i = TRegKeyIterator{*key}; i; ++i)
      subkeys.push_back(TRegKey{i}.GetName());
    for (const auto& s : subkeys)
    {
      const auto r = key->NukeKey(s);
      if (r != ERROR_SUCCESS) return r;
    }
    return ERROR_SUCCESS;
  };
  const auto r = nukeSubkeys(subKeyName);
  if (r != ERROR_SUCCESS) return r;
  return DeleteKey(subKeyName);
}

//
/// Retrieves a copy of the security descriptor protecting the specified open registry key.
///
/// To read the owner, group, or discretionary access control list (DACL) from the key's security
/// descriptor, the calling process must have been granted READ_CONTROL access when the handle was
/// opened. To get READ_CONTROL access, the caller must be the owner of the key or the key's DACL
/// must grant the access.
///
/// To read the system access control list (SACL) from the security descriptor, the calling process
/// must have been granted ACCESS_SYSTEM_SECURITY access when the key was opened. The correct way
/// to get this access is to enable the SE_SECURITY_NAME privilege in the caller's current token,
/// open the handle for ACCESS_SYSTEM_SECURITY access, and then disable the privilege.
///
/// \param infoRequested indicates the requested security information.
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetkeysecurity">
/// RegGetKeySecurity</a> in the Windows API.
//
TRegKey::TSecurityDescriptor::TSecurityDescriptor(const TRegKey& key, SECURITY_INFORMATION infoRequested)
  : Data(key.QueryInfo().SecurityDescriptorSize)
{
  auto n = static_cast<DWORD>(Data.size());
  const auto r = key.GetSecurity(infoRequested, Data.data(), &n);
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TSecurityDescriptor::TSecurityDescriptor: GetSecurity failed"), key, r};
  CHECK(n == static_cast<DWORD>(Data.size()));
}

//
/// Determines whether the components of the security descriptor are valid.
/// \returns `true` if the security descriptor is valid.
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-isvalidsecuritydescriptor">
/// IsValidSecurityDescriptor</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::IsValid() const noexcept -> bool
{ 
  return ::IsValidSecurityDescriptor(const_cast<SECURITY_DESCRIPTOR*>(GetData())) != FALSE; 
}

//
/// Retrieves the control information from the security descriptor.
/// Call TSecurityDescriptor::GetRevision to get the revision number of the structure.
///
/// \returns If successful, the control information is returned.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorcontrol">
/// GetSecurityDescriptorControl</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::GetControl() const -> SECURITY_DESCRIPTOR_CONTROL
{
  auto c = SECURITY_DESCRIPTOR_CONTROL{};
  auto revision = DWORD{};
  const auto ok = ::GetSecurityDescriptorControl(const_cast<SECURITY_DESCRIPTOR*>(GetData()), &c, &revision);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::GetControl: GetSecurityDescriptorControl failed"), tstring{}, static_cast<long>(::GetLastError())};
  return c;
}

//
/// Returns the length, in bytes, of the security descriptor.
/// The length includes the length of all associated structures.
/// 
/// \returns If successful, the length is returned.
/// \exception TXRegistry is thrown if the security descriptor is invalid.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorlength">
/// GetSecurityDescriptorLength</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::GetLength() const -> DWORD
{
  if (!IsValid()) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::GetLength failed"), tstring{}};
  return ::GetSecurityDescriptorLength(const_cast<SECURITY_DESCRIPTOR*>(GetData()));
}

//
/// Retrieves the discretionary access control list (DACL) from the security descriptor.
/// Call TSecurityDescriptor::IsDaclDefaulted to determine whether the DACL was set by default
/// means or user specified.
///
/// \returns If this security descriptor contains a DACL, a `PACL` is returned pointing to the
/// DACL. Note that `nullptr` is a valid value for the `PACL`. A `nullopt` is returned, if a DACL
/// is not present.
///
/// \exception TXRegistry is thrown on failure.
///
/// \note To simply determine whether a DACL is present in the security descriptor, you can call
/// TSecurityDescriptor::IsDaclPresent.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptordacl">
/// GetSecurityDescriptorDacl</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::GetDacl() const -> optional<PACL>
{
  auto daclPresent = BOOL{};
  auto dacl = PACL{};
  auto daclDefaulted = BOOL{};
  const auto ok = ::GetSecurityDescriptorDacl(const_cast<SECURITY_DESCRIPTOR*>(GetData()), &daclPresent, &dacl, &daclDefaulted);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::GetDacl: GetSecurityDescriptorDacl failed"), tstring{}, static_cast<long>(::GetLastError())};
  return daclPresent ? make_optional(dacl) : nullopt;
}

//
/// Retrieves the primary group information from the security descriptor.
/// Call TSecurityDescriptor::IsGroupDefaulted to determine whether the group was set by default
/// means or user specified.
///
/// \returns If the security descriptor contains a primary group, a `PSID` pointing to the security
/// identifier that identifies the primary group is returned. Otherwise, `nullptr` is returned.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorgroup">
/// GetSecurityDescriptorGroup</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::GetGroup() const -> PSID
{
  auto group = PSID{};
  auto groupDefaulted = BOOL{};
  const auto ok = ::GetSecurityDescriptorGroup(const_cast<SECURITY_DESCRIPTOR*>(GetData()), &group, &groupDefaulted);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::GetGroup: GetSecurityDescriptorGroup failed"), tstring{}, static_cast<long>(::GetLastError())};
  return group;
}

//
/// Retrieves the owner information from the security descriptor.
/// Call TSecurityDescriptor::IsOwnerDefaulted to determine whether the owner was set by default
/// means or user specified.
///
/// \returns If the security descriptor contains an owner, a `PSID` pointing to the security
/// identifier that identifies the owner is returned. Otherwise, `nullptr` is returned.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorowner">
/// GetSecurityDescriptorOwner</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::GetOwner() const -> PSID
{
  auto owner = PSID{};
  auto ownerDefaulted = BOOL{};
  const auto ok = ::GetSecurityDescriptorOwner(const_cast<SECURITY_DESCRIPTOR*>(GetData()), &owner, &ownerDefaulted);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::GetOwner: GetSecurityDescriptorOwner failed"), tstring{}, static_cast<long>(::GetLastError())};
  return owner;
}

//
/// Retrieves the revision number of the security descriptor structure.
/// Call TSecurityDescriptor::GetControl to get the control information of the structure.
///
/// \returns If successful, the revision number is returned.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorcontrol">
/// GetSecurityDescriptorControl</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::GetRevision() const -> DWORD
{
  auto c = SECURITY_DESCRIPTOR_CONTROL{};
  auto revision = DWORD{};
  const auto ok = ::GetSecurityDescriptorControl(const_cast<SECURITY_DESCRIPTOR*>(GetData()), &c, &revision);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::GetRevision: GetSecurityDescriptorControl failed"), tstring{}, static_cast<long>(::GetLastError())};
  return revision;
}

//
/// Retrieves the resource manager control bits from the security descriptor.
/// \returns If the SE_RM_CONTROL_VALID bit flag is set in the security descriptor, a `UCHAR`
/// containing the control bits is returned. Otherwise, a `nullopt` is returned.
///
/// \exception TXRegistry is thrown on failure.
///
/// \note To simply determine whether resource manager control bits are present in the security
/// descriptor, you can call TSecurityDescriptor::IsRmControlValid.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorrmcontrol">
/// GetSecurityDescriptorRMControl</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::GetRmControl() const -> optional<UCHAR>
{
  auto rmControl = UCHAR{};
  const auto r = ::GetSecurityDescriptorRMControl(const_cast<SECURITY_DESCRIPTOR*>(GetData()), &rmControl);
  if (r == ERROR_INVALID_DATA) return nullopt;
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::GetRmControl: GetSecurityDescriptorRMControl failed"), tstring{}, static_cast<long>(r)};
  return make_optional(rmControl);
}

//
/// Retrieves the system access control list (SACL) from the security descriptor.
/// Call TSecurityDescriptor::IsSaclDefaulted to determine whether the SACL was set by default
/// means or user specified.
///
/// \returns If this security descriptor contains a SACL, a `PACL` is returned pointing to the
/// SACL. Note that `nullptr` is a valid value for the `PACL`. A `nullopt` is returned, if a SACL
/// is not present.
///
/// \exception TXRegistry is thrown on failure.
///
/// \note To simply determine whether a SACL is present in the security descriptor, you can call
/// TSecurityDescriptor::IsSaclPresent.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsecuritydescriptorsacl">
/// GetSecurityDescriptorSacl</a> in the Windows API.
//
auto TRegKey::TSecurityDescriptor::GetSacl() const -> optional<PACL>
{
  auto saclPresent = BOOL{};
  auto sacl = PACL{};
  auto saclDefaulted = BOOL{};
  const auto ok = ::GetSecurityDescriptorSacl(const_cast<SECURITY_DESCRIPTOR*>(GetData()), &saclPresent, &sacl, &saclDefaulted);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::GetSacl: GetSecurityDescriptorSacl failed"), tstring{}, static_cast<long>(::GetLastError())};
  return saclPresent ? make_optional(sacl) : nullopt;
}

//
/// Initializes a new security descriptor.
///
/// The security descriptor is initialized to have no system access control list (SACL), no
/// discretionary access control list (DACL), no owner, no primary group, and all control flags set
/// to `FALSE` (`NULL`). Thus, except for its revision level, it is empty.
///
/// \param revision must be set to SECURITY_DESCRIPTOR_REVISION (the default).
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-initializesecuritydescriptor">
/// InitializeSecurityDescriptor</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::Initialize(DWORD revision)
{
  const auto ok = ::InitializeSecurityDescriptor(GetData(), revision);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::Initialize: InitializeSecurityDescriptor failed"), tstring{}, static_cast<long>(::GetLastError())};
}

//
/// Removes the discretionary access control list (DACL) from the security descriptor.
/// The flags SE_DACL_PRESENT and SE_DACL_DEFAULTED are cleared in the SECURITY_DESCRIPTOR_CONTROL
/// structure, and the DACL `PACL` is set to `nullptr`.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptordacl">
/// SetSecurityDescriptorDacl</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::RemoveDacl()
{
  const auto ok = ::SetSecurityDescriptorDacl(GetData(), FALSE, nullptr, FALSE);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::RemoveDacl: SetSecurityDescriptorDacl failed"), tstring{}, static_cast<long>(::GetLastError())};
}

//
/// Removes the primary group information from the security descriptor.
/// Sets the group `PSID` to `nullptr`. This marks the security descriptor as having no primary
/// group. Also, the flag SE_GROUP_DEFAULTED is cleared in the SECURITY_DESCRIPTOR_CONTROL
/// structure.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptorgroup">
/// SetSecurityDescriptorGroup</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::RemoveGroup()
{
  const auto ok = ::SetSecurityDescriptorGroup(GetData(), nullptr, FALSE);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::RemoveGroup: SetSecurityDescriptorGroup failed"), tstring{}, static_cast<long>(::GetLastError())};
}

//
/// Removes the owner information from the security descriptor.
/// Sets the owner `PSID` to `nullptr`. This marks the security descriptor as having no owner.
/// Also, the flag SE_OWNER_DEFAULTED is cleared in the SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptorowner">
/// SetSecurityDescriptorOwner</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::RemoveOwner()
{
  const auto ok = ::SetSecurityDescriptorOwner(GetData(), nullptr, FALSE);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::RemoveOwner: SetSecurityDescriptorOwner failed"), tstring{}, static_cast<long>(::GetLastError())};
}

//
/// Removes the resource manager control bits from the security descriptor.
/// The flag SE_RM_CONTROL_VALID in cleared in the SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptorrmcontrol">
/// SetSecurityDescriptorRMControl</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::RemoveRmControl()
{
  const auto r = ::SetSecurityDescriptorRMControl(GetData(), nullptr);
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::RemoveRmControl: SetSecurityDescriptorRMControl failed"), tstring{}, static_cast<long>(r)};
}

//
/// Removes the system access control list (SACL) from the security descriptor.
/// The flags SE_SACL_PRESENT and SE_SACL_DEFAULTED are cleared in the SECURITY_DESCRIPTOR_CONTROL
/// structure, and the SACL `PACL` is set to `nullptr`.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptorsacl">
/// SetSecurityDescriptorSacl</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::RemoveSacl()
{
  const auto ok = ::SetSecurityDescriptorSacl(GetData(), FALSE, nullptr, FALSE);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::RemoveSacl: SetSecurityDescriptorSacl failed"), tstring{}, static_cast<long>(::GetLastError())};
}

//
/// Sets the discretionary access control list (DACL) of the security descriptor.
/// If a DACL is already present in the security descriptor, the DACL is replaced.
/// On success, the flag SE_DACL_PRESENT is set in the SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \param dacl points to an ACL structure that specifies the DACL for the security descriptor.
/// The DACL is referenced by, not copied into, the security descriptor. If this parameter is a
/// `nullptr`, a **NULL** DACL is assigned to the security descriptor, which allows all access to
/// the object.
///
/// \param daclDefaulted indicates whether the DACL information was derived from a default
/// mechanism or explicitly specified by a user. The function stores this value in the flag
/// SE_DACL_DEFAULTED within the SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \exception TXRegistry is thrown on failure.
///
/// \note Passing a `nullptr` to this function does not remove the DACL; rather it sets a **NULL**
/// DACL. To remove the DACL from the security descriptor, call TSecurityDescriptor::RemoveDacl.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptordacl">
/// SetSecurityDescriptorDacl</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::SetDacl(PACL dacl, bool daclDefaulted)
{
  const auto ok = ::SetSecurityDescriptorDacl(GetData(), TRUE, dacl, daclDefaulted ? TRUE : FALSE);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::SetDacl: SetSecurityDescriptorDacl failed"), tstring{}, static_cast<long>(::GetLastError())};
}

//
/// Sets the primary group information of the security descriptor.
/// Any primary group information already present in the security descriptor is replaced.
///
/// \param group points to a SID structure for the security descriptor's new primary group. The SID
/// structure is referenced by, not copied into, the security descriptor. If this parameter is
/// `nullptr`, the security descriptor's primary group information is cleared. This marks the
/// security descriptor as having no primary group.
///
/// \param groupDefaulted indicates whether the primary group information was derived from a
/// default mechanism or explicitly specified by a user. The function stores this value in the flag
/// SE_GROUP_DEFAULTED within the SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \exception TXRegistry is thrown on failure.
///
/// \note Rather than call this function with a `nullptr`, instead you should prefer to call
/// TSecurityDescriptor::RemoveGroup.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptorgroup">
/// SetSecurityDescriptorGroup</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::SetGroup(PSID group, bool groupDefaulted)
{
  const auto ok = ::SetSecurityDescriptorGroup(GetData(), group, groupDefaulted ? TRUE : FALSE);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::SetGroup: SetSecurityDescriptorGroup failed"), tstring{}, static_cast<long>(::GetLastError())};
}

//
/// Sets the owner information of the security descriptor.
/// Any owner information already present in the security descriptor is replaced.
///
/// \param owner points to a SID structure for the security descriptor's new owner. The SID
/// structure is referenced by, not copied into, the security descriptor. If this parameter is
/// `nullptr`, the security descriptor's primary owner information is cleared. This marks the
/// security descriptor as having no owner.
///
/// \param ownerDefaulted indicates whether the owner information was derived from a default
/// mechanism or explicitly specified by a user. The function stores this value in the flag
/// SE_OWNER_DEFAULTED within the SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \exception TXRegistry is thrown on failure.
///
/// \note Rather than call this function with a `nullptr`, instead you should prefer to call
/// TSecurityDescriptor::RemoveOwner.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptorowner">
/// SetSecurityDescriptorOwner</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::SetOwner(PSID owner, bool ownerDefaulted)
{
  const auto ok = ::SetSecurityDescriptorOwner(GetData(), owner, ownerDefaulted ? TRUE : FALSE);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::SetOwner: SetSecurityDescriptorOwner failed"), tstring{}, static_cast<long>(::GetLastError())};
}

//
/// Sets the resource manager control bits of the security descriptor.
/// On success, the function also sets the SE_RM_CONTROL_VALID flag in the
/// SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \param rmControl contains the control bits.
///
/// \exception TXRegistry is thrown on failure.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptorrmcontrol">
/// SetSecurityDescriptorRMControl</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::SetRmControl(UCHAR rmControl)
{
  const auto r = ::SetSecurityDescriptorRMControl(GetData(), &rmControl);
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::SetRmControl: SetSecurityDescriptorRMControl failed"), tstring{}, static_cast<long>(r)};
}

//
/// Sets the system access control list (SACL) of the security descriptor.
/// If a SACL is already present in the security descriptor, the SACL is replaced.
/// On success, the flag SE_SACL_PRESENT is set in the SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \param sacl points to an ACL structure that specifies the SACL for the security descriptor.
/// The SACL is referenced by, not copied into, the security descriptor. If this parameter is a
/// `nullptr`, a **NULL** SACL is assigned to the security descriptor.
///
/// \param saclDefaulted indicates whether the SACL information was derived from a default
/// mechanism or explicitly specified by a user. The function stores this value in the flag
/// SE_SACL_DEFAULTED within the SECURITY_DESCRIPTOR_CONTROL structure.
///
/// \exception TXRegistry is thrown on failure.
///
/// \note Passing a `nullptr` to this function does not remove the SACL; rather it sets a **NULL**
/// SACL. To remove the SACL from the security descriptor, call TSecurityDescriptor::RemoveSacl.
///
/// \sa <a href="https://docs.microsoft.com/en-gb/windows/win32/api/securitybaseapi/nf-securitybaseapi-setsecuritydescriptorsacl">
/// SetSecurityDescriptorSacl</a> in the Windows API.
//
void TRegKey::TSecurityDescriptor::SetSacl(PACL sacl, bool saclDefaulted)
{
  const auto ok = ::SetSecurityDescriptorSacl(GetData(), TRUE, sacl, saclDefaulted ? TRUE : FALSE);
  if (!ok) throw TXRegistry{_T("TRegKey::TSecurityDescriptor::SetSacl: SetSecurityDescriptorSacl failed"), tstring{}, static_cast<long>(::GetLastError())};
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

#if defined(BI_COMP_BORLANDC) 

// Circumvent bug in std::optional (requires default constructor for the contained object type).
// TODO: Remove this when the library is standard compliant.
//
TRegValue::TRegValue() : Key{TRegKey::GetCurrentUser()} {}

#endif

//
/// Query the registry for the value type and data size, though not the actual data.
/// The value data itself is retrieved lazily; see RetrieveOnDemand.
/// Returns the error code returned by the query.
//
long TRegValue::QueryTypeAndSize()
{
  long r = Key.QueryValue(Name, &DataType, nullptr, &DataSize);
  WARN(r != ERROR_SUCCESS, _T("TRegValue::QueryTypeAndSize: Query failed: ") << Key.GetName() << _T('\\') << Name << _T(", error: ") << r);
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
  long r = Key.EnumValue(iter.Current(), &nameBuffer[0], nameSize, &DataType, nullptr, &DataSize);
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegValue::TRegValue(const TRegValueIterator&): EnumValue failed"), Key, r};
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
  if (DataType != REG_DWORD && DataType != REG_DWORD_BIG_ENDIAN)
    throw TXRegistry{_T("TRegValue::operator uint32: Incompatible data type"), Key};

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
    throw TXRegistry{_T("TRegValue::operator uint64: Incompatible data type"), Key};

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
    throw TXRegistry{_T("TRegValue::operator LPCTSTR: Incompatible data type"), Key};

  // Ensure null-termination.
  // The data may have been stored without proper null-termination.
  // Also, check that the string has room for null-termination.
  //
  CHECK(!Data.empty());
  LPTSTR s = reinterpret_cast<LPTSTR>(&Data[0]);
  const size_t n = Data.size() / sizeof(tchar);
  if (n == 0) throw TXRegistry{_T("TRegValue::operator LPCTSTR: Data size is zero"), Key};
  WARN((Data.size() % sizeof(tchar)) != 0, _T("TRegValue::operator LPCTSTR: Oddly sized string"));
  WARN(s[n - 1] != _T('\0'), _T("TRegValue::operator LPCTSTR: String lacks null-termination"));
  s[n - 1] = _T('\0');
  if (DataType == REG_MULTI_SZ && n > 1)
  {
    WARN(s[n - 2] != _T('\0'), _T("TRegValue::operator LPCTSTR: Multi-string lacks double null-termination"));
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
  return Data.empty() ? nullptr : &Data[0];
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
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegValue::operator =(uint32): Set failed"), Key, r};
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
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegValue::operator =(uint64): Set failed"), Key, r};
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
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegValue::operator =(LPCTSTR): Set failed"), Key, r};
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
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegValue::operator =(const tstring&): Set failed"), Key, r};
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
  if (DataType == REG_NONE || DataSize == 0) throw TXRegistry{_T("TRegValue::RetrieveOnDemand: Value is void"), Key};

  uint32 dataType = DataType;
  Data.resize(DataSize);
  uint32 dataSize = DataSize;
  long r = Key.QueryValue(Name, &dataType, &Data[0], &dataSize);
  if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegValue::RetrieveOnDemand: QueryValue failed"), Key, r};
  if (dataType != DataType) throw TXRegistry{_T("TRegValue::RetrieveOnDemand: Type changed"), Key};
  if (dataSize != DataSize) throw TXRegistry{_T("TRegValue::RetrieveOnDemand: Size changed"), Key};
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
TRegParamList::TRegParamList(const TEntry* list)
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
/// Looks for a given param in this param list.
/// Associative lookup of value by param.
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
                                 const TRegParamList::TEntry* paramList)
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
TRegSymbolTable::Init(LPCTSTR filter)
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
      const auto k = to_tstring(item->Key);
      int i = Params.Find(k);
      if (i < 0) throw TXRegistry{_T("TRegSymbolTable::UpdateParams failed"), k};
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
          if (i < 0) throw TXRegistry{_T("TRegSymbolTable::StreamOut failed"), tstring{pc}};

          // Now get the value obtained above
          //
          pb = (tchar*)pc; // Move buffer ptr back
          pc = Params.Value(i);
          if (!pc) throw TXRegistry{_T("TRegSymbolTable::StreamOut failed"), tstring{Params[i].Param}};

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
          userval = nullptr;
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
  while (in.good()) {
    // Get line into entry buffer, skip over base key if it exists, ignore line
    // if it doesn't
    //
    tchar entry[512];
    in.get(entry, ::_tcslen(baseKey.GetName())+1);
    if (_tcscmp(entry, baseKey.GetName()) != 0)
      continue;
    if (in.peek() == _T('\\'))
      in.ignore();  // Eat the '\' separator.
    in.getline(entry, 512);

    // Split entry into keys-value and data strings
    //
    tchar* data = _tcschr(entry, _T('='));
    if (data) {
      tchar* pc = data;
      while (*(pc-1) == _T(' '))
        pc--;
      *pc = _T('\0');
      while (*(++data) == _T(' '))
        ;
    }
    tchar* valName = _tcschr(entry, _T('|'));
    if (valName)
      *valName++ = _T('\0');  // Terminate key at value name separator

    TRegKey subKey(baseKey, entry);
    LPCTSTR s = data ? data : _T("");
    const uint8* buf = reinterpret_cast<const uint8*>(s);
    const uint32 bufSize = static_cast<uint32>((_tcslen(s) + 1) * sizeof(*s));
    const auto r = subKey.SetValue(valName, REG_SZ, buf, bufSize);
    if (r != ERROR_SUCCESS) throw TXRegistry{_T("TRegistry::Update: SetValue failed"), subKey};
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
  while (in.good()) {
    // Get line into entry buffer, skip over base key if it exists, ignore line
    // if it doesn't
    //
    tchar entry[512];
    in.get(entry, ::_tcslen(baseKey.GetName())+1);
    if (_tcscmp(entry, baseKey.GetName()) != 0)
      continue;
    if (in.peek() == _T('\\'))
      in.ignore();  // Eat the '\' separator.
    in.getline(entry, 512);

    // Split entry into keys-value and data strings
    //
    tchar* data = _tcschr(entry, _T('='));
    if (data) {
      tchar* pc = data;
      while (*(pc-1) == _T(' '))
        pc--;
      *pc = _T('\0');
      while (*(++data) == _T(' '))
        ;
    }
    tchar* valName = _tcschr(entry, _T('|'));
    if (valName)
      *valName++ = _T('\0');  // Terminate key at value name separator

    // Now lookup keys-value part to get its data and see if it matches the
    // data value from the instream
    //
    TRegKey subKey(baseKey, entry);
    uint32 dataType = REG_SZ;
    tchar buf[300];
    uint32 bufSize = 300;
    if (subKey.QueryValue(valName, &dataType, reinterpret_cast<uint8*>(buf), &bufSize) != ERROR_SUCCESS
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
TRegistry::Unregister(TRegList& regInfo, const TUnregParams* params, const TRegItem* overrides)
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
