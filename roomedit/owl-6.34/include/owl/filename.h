//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#if !defined(OWL_FILENAME_H)
#define OWL_FILENAME_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/file.h>

#include <owl/private/dir.h>  // struct ffblk

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

// Copied from old owl (04-16-01 JAM)
#ifdef UNIX
#include <owl/system.h> 
#endif

//#include <string>
//using std::string;

struct TFileStatus;

//
/// \class TFileName
// ~~~~~ ~~~~~~~~~
/// The TFileName class constructs filenames.
//
class _OWLCLASS TFileName {
  protected:
    void   Parse(LPCTSTR freeform, bool pathOnly = false);

/// Holds the server filename part.
    tstring ServerStr;
    
/// Holds the device filename part.
    tstring DeviceStr;
    
/// Holds the path filename part.
    mutable tstring PathStr; //had to do this to make Squeezed function const
    
/// Holds the file filename part (no extension).
    tstring FileStr;
    
/// Holds the extension filename part.
    tstring ExtStr;

    tstring FullCanonicalName;
    mutable tchar NameBuffer[_MAX_PATH];
    
/// true if UNC naming convention in use; false otherwise.
    bool   Unc;

  public:
    // Construct an empty filename
    //
    TFileName();

    // Construct a filename from its parts. OK to pass 0 to skip part
    //
    TFileName(LPCTSTR serverName, LPCTSTR shareOrDeviceName, LPCTSTR path,
              LPCTSTR file, LPCTSTR ext);

    // Construct a filename from a freeform string
    //
    TFileName(LPCTSTR  freeform, bool pathOnly = false);
    TFileName(const tstring& freeform, bool pathOnly = false);

    // Construct a filename from another filename
    //
    TFileName(const TFileName& src);

    //virtual ~TFileName(){} ??

    // Assign new filename or freeform to this name
    //
    TFileName& operator =(const TFileName& src);
    TFileName& operator =(const tstring& src);
    TFileName& operator =(LPCTSTR src);

    /// Construct a filename representing a special file or directory
    //
    enum TSpecialType {
      TempFile,     ///< A temporary filename
      TempDir,      ///< Location of temporary files
      CurrentDir,   ///< Current working directory if any
      BootDir,      ///< Root dir of boot device (LDID_BOOT)
      MachineDir,   ///< (LDID_MACHINE)   currently not implemented?
      HomeDir,      ///< Home directory for OS (LDID_WIN)
      SharedDir,    ///< Shared home directory for OS (LDID_SHARED)  currently not implemented?
      SysDir,       ///< Location of system files (LDID_SYS)
      ComputerName,
    };
    TFileName(TSpecialType type);

    // Normal fully qualified path string, & short filename version of it
    // Convert a possibly logical drive based name to a UNC name if indicated
    //
        //PERL API USE ONLY
    tchar* GetNameNoExt();
    tchar* GetNameAndExt() const;
    tchar* GetFullFileName();
    tchar* GetFullFileNameNoExt();

    void FormCanonicalName();
    const tstring& Canonical(bool forceUNC = false) const;
    const tchar* CanonicalStr(bool forceUNC = false) const;
    const tchar* ShortName(bool forceUNC = false) const;

    const tchar* Title() const;

    const tchar* Squeezed(int maxLen, bool keepName = true) const;

    /// Obtain any combination of various filename parts. Seperators inserted
    /// only as appropriate
    //
    enum TPart {
      Server = 1,  ///< Server name
      Device = 2,  ///< Logical device or sharename
      Path   = 4,  ///< Directory path to the file
      File   = 8,  ///< Filename part without the extension
      Ext    =16,  ///< Extension
    };
    const tchar* GetParts(uint p) const;      // Return assembled string of parts
    bool   HasParts(uint p) const;      // Does name have an given part
    TFileName& SetPart(uint p, const tstring& partStr);    // Modify sub part
    TFileName& MergeParts(uint p, const TFileName& source); // Modify sub parts
    TFileName& StripParts(uint p);         // Remove indicated parts
    TFileName& AddSubDir(const tstring& subdir);

    // Information about the filename
    //
    bool IsValid() const;       // Is a valid name of any kind
    bool IsUNC() const;         // Is UNC vs logical drive based name
    bool Exists() const;        // Does device/dir/file exist?

    // Is another filename equivalent to this filename?
    //
    bool operator ==(const TFileName& other) const;

    // Remove the file or dir associated with this filename.
    //
    bool Remove() const;

    enum { ReplaceExisting=1, CopyAllowed=2, DelayUntilReboot=4 };
    // Move (rename) the file associated with this filename, and change this
    // name to the new name
    //
    bool Move(const TFileName& newName, uint32 how = CopyAllowed);

    // Copy the file associated with this filename to a new file
    //
    bool Copy(const TFileName& newName, bool failIfExists) const;

    int ChangeDir() const;
    int CreateDir() const;

    // Get and Set the file status struct for the item associated with this
    // filename
    //
    int GetStatus(TFileStatus& status) const;
    int SetStatus(const TFileStatus& status);

    // Other ideas...
    //
    static LPCTSTR WildName() {return _T("*");}       ///< General wildstring
    static LPCTSTR WildPart(uint /*p*/) {return _T("*");} ///< Part specific?

    // ***********************************************************************************    
    // (JAM 03-12-01) Added GetNameAndExt for compliance with old code

};

//
/// \class TFileNameIterator
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// TFileNameIterator is used to iterate through a list of file names.
//
class _OWLCLASS TFileNameIterator {
  public:
    TFileNameIterator(const tstring& wildName);
    virtual ~TFileNameIterator();

    operator const TFileStatus&() const;
    const TFileStatus& operator *() const;
    operator const tchar*() const;
    const TFileStatus& operator ++();
    const TFileStatus& Current() const;

    bool IsDir() const;
    bool IsSysDir() const;///< true if it is the "." or ".." system directory; 
    unsigned long  GetAttributes() const { return Status.attribute; }
  protected:
    TFileNameIterator(){}
    /// all work do here -> must fill TFileStatus structure return true on success
    virtual bool FindNext(TFileStatus& status, void* findData);

  protected:
/// Set false in the constructor. Set true if the last attempt to find a file
/// failed.
    bool            Done;
    
/// Under Win32 a pointer to a WIN32_FIND_DATA structure. 
    void*           Data;
    
/// Status of last attempt to find a file. See TFileStatus structure
    TFileStatus     Status;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inlines
//

/// Assigns new filename or freeform to this name.
inline TFileName& TFileName::operator =(const tstring& src)
{
  Parse(src.c_str());
  return *this;
}

/// Assigns new filename or freeform to this name.
inline TFileName& TFileName::operator =(LPCTSTR src)
{
  Parse(src);
  return *this;
}

/// Returns true if this filename is canonically equal to other
inline bool TFileName::operator ==(const TFileName& other) const
{
  return _tcsicmp(CanonicalStr(), other.CanonicalStr()) == 0;
}

/// Determines whether the name is UNC format as opposed to a logical-drive based
/// machine.
inline bool TFileName::IsUNC() const
{
  return Unc;
}

inline const tchar* TFileName::CanonicalStr(bool forceUNC) const
{
  return Canonical(forceUNC).c_str();
}

//
/// Returns a reference to the Status member indicating the results of the last
/// operation. See TFileStatus structure.
/// For example:
/// \code
/// TFileNameIterator iterator("file?.exe");
/// 
/// while(iterator)
///  { 
///   TFileStatus& status = iterator;
///   //do something
///   ++iterator;
///  }
/// \endcode
//
inline TFileNameIterator::operator const TFileStatus&() const 
{
  return Status;
}
//
/// Returns the full name if the last operator was sucessful; otherwise it returns 0.
//
inline TFileNameIterator::operator const tchar*() const 
{
  return Done ? 0 : Status.fullName;
}
//
/// Dereference operator returns a reference to the Status member indicating the
/// results of the last operation. Similar to STL form. See TFileStatus structure.
/// For example:
/// \code
/// TFileNameIterator iterator("file?.exe");
/// 
/// while(iterator)
///  { 
///   TFileStatus& status = *iterator;
///   //do something
///   ++iterator;
///  }
/// \endcode
//
inline const TFileStatus& TFileNameIterator::operator *()const
{
  return Status;
}
//
/// Returns a reference to the Status member indicating the results of the last
/// operation. See TFileStatus structure
//
inline const TFileStatus& TFileNameIterator::Current() const
{
  return Status;
}
//
/// Attempts to find the next file. Returns a reference to the Status member. See
/// TFileStatus structure.
/// For example:
/// \code
/// TFileNameIterator iterator("file?.exe");
/// 
/// while(iterator)
///  { 
///   TFileStatus& status = *iterator;
///   //do something
///   ++iterator;
///  }
/// \endcode
//
inline const TFileStatus& TFileNameIterator::operator ++() 
{
  if(!FindNext(Status, Data))
    Done = true;
  return Status;
}

inline bool TFileNameIterator::IsDir() const
{ 
  return Status.attribute & FILE_ATTRIBUTE_DIRECTORY; 
}

//Returns true if it is a directory & the directory is "." or ".."; 
inline bool TFileNameIterator::IsSysDir() const
{
  LPCTSTR p = Status.fullName;
  return IsDir() && p[0]==_T('.') &&
          (p[1]==_T('\0') || p[1]==_T('.') && p[2]==_T('\0'));
}    

} // OWL namespace


#endif  // OWL_FILENAME_H
