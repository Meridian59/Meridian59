//------------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus
//
/// \file
/// This header file declares the class: TConfigFile,TIniConfigFile,
/// TRegConfigFile and TConfigFileSection.
//
// Log History:
// 03/27/99 - Added Read/WriteData sugested by Jogy
// 06/17/2002 - Added variations of functions with tstring as parameter
//------------------------------------------------------------------------------

#if !defined(OWL_CONFIGFL_H)
#define OWL_CONFIGFL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>

namespace owl {

class _OWLCLASS TDate;
class _OWLCLASS TTime;
class _OWLCLASS TSystemTime;
class _OWLCLASS TPoint;
class _OWLCLASS TRect;
class _OWLCLASS TSize;
class _OWLCLASS TFont;
class _OWLCLASS TColor;


// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup winsys
/// @{


/// \class TConfigFile
// ~~~~~ ~~~~~~~~~~~
/// Save/Load configuration parameters 
/// Base abstract class
//
class _OWLCLASS TConfigFile {
  public:

    //
    /// Constructs instance and sets FileName to name.
    //
    TConfigFile(const tstring& name) : FileName(name) {}

/// Destroys the instance.
    virtual ~TConfigFile(){}

        // -1 = pring default(%1.20Lg), == 0 not use precision(%Lg), else use specified
        static void SetPrecision(int newprec = -1){ DefaultPrecision = newprec; }
        static int  GetPrecision(){ return DefaultPrecision; }

    const tstring& GetFileName() {return FileName;}
    virtual void    LoadValues(const TConfigFile& file);
    
    virtual bool    SectionExists(LPCTSTR section) const;
    virtual int     ReadSection(LPCTSTR section, LPTSTR buffer, uint bufSize) const = 0;
    virtual int     ReadSections(LPTSTR sections, uint bufSize) const = 0;
    virtual bool    EraseSection(LPCTSTR section) = 0;
    virtual bool    EraseEntry(LPCTSTR section, LPCTSTR entry) = 0;
    virtual void    UpdateFile() = 0;

    virtual int     ReadString(LPCTSTR section, LPCTSTR entry, LPTSTR buffer, uint bufSize, LPCTSTR defstr= 0) const = 0;
    virtual bool    WriteString(LPCTSTR section, LPCTSTR entry, LPCTSTR value) = 0;

    virtual bool    ReadData(LPCTSTR section, LPCTSTR entry, void* buffer, uint size) const;
    virtual bool    WriteData(LPCTSTR section, LPCTSTR entry, void* buffer, uint size);
    
    virtual int     ReadInteger(LPCTSTR section, LPCTSTR entry,int defint=0) const;
    virtual bool    WriteInteger(LPCTSTR section, LPCTSTR entry, int value);

            bool    SectionExists(const tstring& section) const;
            int     ReadSection(const tstring& section, tstring& buffer) const;
            int     ReadSections(tstring& sections) const;
            bool    EraseSection(const tstring& section);
            bool    EraseEntry(const tstring& section, const tstring& entry);

            int     ReadString(const tstring& section, const tstring& entry, tstring& buffer, LPCTSTR defstr=0) const;
            int     ReadString(const tstring& section, const tstring& entry, tstring& buffer, const tstring& defstr) const
            {return ReadString(section, entry, buffer, defstr.c_str());}
            bool    WriteString(const tstring& section, const tstring& entry, const tstring& value);

            bool    ReadData(const tstring& section, const tstring& entry, void* buffer, uint size) const;
            bool    WriteData(const tstring& section, const tstring& entry, void* buffer, uint size);
    
            int     ReadInteger(const tstring& section, const tstring& entry,int defint=0) const;
            bool    WriteInteger(const tstring& section, const tstring& entry, int value);

            double  ReadDouble(LPCTSTR section, LPCTSTR entry, double defval=0.0) const;
            bool    WriteDouble(LPCTSTR section, LPCTSTR entry, double value);
            double  ReadDouble(const tstring& section, const tstring& entry, double defval=0.0) const;
            bool    WriteDouble(const tstring& section, const tstring& entry, double value);

            bool    ReadBool(LPCTSTR section, LPCTSTR entry, bool defval=false) const;
            bool    WriteBool(LPCTSTR section, LPCTSTR entry, bool val);
            bool    ReadBool(const tstring& section, const tstring& entry, bool defval=false) const;
            bool    WriteBool(const tstring& section, const tstring& entry, bool val);
            
            bool    ReadDate(LPCTSTR section, LPCTSTR entry, TDate& value) const;
            bool    WriteDate(LPCTSTR section, LPCTSTR entry, const TDate& val);
            bool    ReadDate(const tstring& section, const tstring& entry, TDate& value) const;
            bool    WriteDate(const tstring& section, const tstring& entry, const TDate& val);

            bool    ReadSystemTime(LPCTSTR section, LPCTSTR entry, TSystemTime& val) const;
            bool    WriteSystemTime(LPCTSTR section, LPCTSTR entry, const TSystemTime& val); 
            bool    ReadSystemTime(const tstring& section, const tstring& entry, TSystemTime& val) const;
            bool    WriteSystemTime(const tstring& section, const tstring& entry, const TSystemTime& val); 
    
            bool    ReadTime(LPCTSTR section, LPCTSTR entry, TTime& val) const;
            bool    WriteTime(LPCTSTR section, LPCTSTR entry, TTime& val);
            bool    ReadTime(const tstring& section, const tstring& entry, TTime& val) const;
            bool    WriteTime(const tstring& section, const tstring& entry, TTime& val);

            bool    ReadFont(LPCTSTR section, LPCTSTR entry, LOGFONT& font) const;
            bool    WriteFont(LPCTSTR section, LPCTSTR entry, const LOGFONT& font);
            bool    ReadFont(const tstring& section, const tstring& entry, LOGFONT& font) const;
            bool    WriteFont(const tstring& section, const tstring& entry, const LOGFONT& font);

            bool    ReadColor(LPCTSTR section, LPCTSTR entry, TColor& color) const;
            bool    WriteColor(LPCTSTR section, LPCTSTR entry, const TColor& clr);
            bool    ReadColor(const tstring& section, const tstring& entry, TColor& color) const;
            bool    WriteColor(const tstring& section, const tstring& entry, const TColor& clr);

            bool    ReadRect(LPCTSTR section, LPCTSTR entry, TRect& val) const;
            bool    WriteRect(LPCTSTR section, LPCTSTR entry, const TRect& rc);
            bool    ReadRect(const tstring& section, const tstring& entry, TRect& val) const;
            bool    WriteRect(const tstring& section, const tstring& entry, const TRect& rc);

            bool    ReadPoint(LPCTSTR section, LPCTSTR entry, TPoint& val)const;
            bool    WritePoint(LPCTSTR section, LPCTSTR entry, const TPoint& point);
            bool    ReadPoint(const tstring& section, const tstring& entry, TPoint& val)const;
            bool    WritePoint(const tstring& section, const tstring& entry, const TPoint& point);
            
            bool    ReadSize(LPCTSTR section, LPCTSTR entry, TSize& sz) const;
            bool    WriteSize(LPCTSTR section, LPCTSTR entry, const TSize& size);
            bool    ReadSize(const tstring& section, const tstring& entry, TSize& sz) const;
            bool    WriteSize(const tstring& section, const tstring& entry, const TSize& size);
  protected:
            static bool       StringToBool(LPCTSTR str, bool* val);
            static tstring BoolToString(bool val);
  
  protected:
    tstring  FileName;
    static int  DefaultPrecision;
};

//
/// \class TConfigFileSection
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// Small inline wrapper around Section.
/// ConfigFile must exist all the time while the section exist!
//
class _OWLCLASS TConfigFileSection {
  public:

    TConfigFileSection(TConfigFile& file, const tstring& section)
      : Section(section), File(file) {}

    const tstring& GetSection() {return Section;}

    bool    Erase()   { return File.EraseSection(Section.c_str());  }
    bool    EraseEntry(LPCTSTR entry);

    int     ReadString(LPCTSTR entry, LPTSTR buffer, uint bufSize, LPCTSTR defstr=0) const;
    bool    WriteString(LPCTSTR entry, LPCTSTR value);
    int     ReadString(const tstring& entry, tstring& buffer, LPCTSTR defstr=0) const;
    int     ReadString(const tstring& entry, tstring& buffer, const tstring& defstr) const
    {return ReadString(entry, buffer, defstr.c_str());}
    bool    WriteString(const tstring& entry, const tstring& value);
    
    bool    ReadData(LPCTSTR entry, void* buffer, uint size) const;
    bool    WriteData(LPCTSTR entry, void* buffer, uint size);
    bool    ReadData(const tstring& entry, void* buffer, uint size) const;
    bool    WriteData(const tstring& entry, void* buffer, uint size);

    int     ReadInteger(LPCTSTR entry,int defint=0) const ;
    bool    WriteInteger(LPCTSTR entry, int value);
    int     ReadInteger(const tstring& entry,int defint=0) const ;
    bool    WriteInteger(const tstring& entry, int value);

    double  ReadDouble(LPCTSTR entry, double defval) const;
    bool    WriteDouble(LPCTSTR entry, double defval);
    double  ReadDouble(const tstring& entry, double defval) const;
    bool    WriteDouble(const tstring& entry, double defval);
    
    bool    ReadBool(LPCTSTR entry, bool defval) const;
    bool    WriteBool(LPCTSTR entry, bool val);
    bool    ReadBool(const tstring& entry, bool defval) const;
    bool    WriteBool(const tstring& entry, bool val);
    
    bool    ReadDate(LPCTSTR entry, TDate& value) const;
    bool    WriteDate(LPCTSTR entry, const TDate& val);
    bool    ReadDate(const tstring& entry, TDate& value) const;
    bool    WriteDate(const tstring& entry, const TDate& val);

    bool    ReadSystemTime(LPCTSTR entry, TSystemTime& val) const;
    bool    WriteSystemTime(LPCTSTR entry, const TSystemTime& val); 
    bool    ReadSystemTime(const tstring& entry, TSystemTime& val) const;
    bool    WriteSystemTime(const tstring& entry, const TSystemTime& val); 
    
    bool    ReadTime(LPCTSTR entry, TTime& val) const;
    bool    WriteTime(LPCTSTR entry, TTime& val);
    bool    ReadTime(const tstring& entry, TTime& val) const;
    bool    WriteTime(const tstring& entry, TTime& val);

    bool    ReadFont(LPCTSTR entry, LOGFONT& font) const;
    bool    WriteFont(LPCTSTR entry, const LOGFONT& font);
    bool    ReadFont(const tstring& entry, LOGFONT& font) const;
    bool    WriteFont(const tstring& entry, const LOGFONT& font);

    bool    ReadColor(LPCTSTR entry, TColor& color) const;
    bool    WriteColor(LPCTSTR entry, const TColor& clr);
    bool    ReadColor(const tstring& entry, TColor& color) const;
    bool    WriteColor(const tstring& entry, const TColor& clr);

    bool    ReadRect(LPCTSTR entry, TRect& val) const;
    bool    WriteRect(LPCTSTR entry, const TRect& rc);
    bool    ReadRect(const tstring& entry, TRect& val) const;
    bool    WriteRect(const tstring& entry, const TRect& rc);

    bool    ReadPoint(LPCTSTR entry, TPoint& val)const;
    bool    WritePoint(LPCTSTR entry, const TPoint& point);
    bool    ReadPoint(const tstring& entry, TPoint& val)const;
    bool    WritePoint(const tstring& entry, const TPoint& point);
    
    bool    ReadSize(LPCTSTR entry, TSize& sz) const;
    bool    WriteSize(LPCTSTR entry, const TSize& size);
    bool    ReadSize(const tstring& entry, TSize& sz) const;
    bool    WriteSize(const tstring& entry, const TSize& size);

  protected:
    tstring    Section;
    TConfigFile&  File;
};

/// \class TIniConfigFile
///  Encapsulates the Windows INI file interface
///  Get/SetPrivateProfileXXX functions
class _OWLCLASS TIniConfigFile: public TConfigFile {
  public:
    
    TIniConfigFile(const tstring& name) 
      : TConfigFile (name) {}

    virtual ~TIniConfigFile(){}

    virtual int     ReadSection(LPCTSTR section, LPTSTR buffer, uint bufSize) const;
    virtual int     ReadSections(LPTSTR buffer, uint bufSize) const;
    virtual bool    EraseSection(LPCTSTR section);
    virtual bool    EraseEntry(LPCTSTR section, LPCTSTR entry);
    virtual void    UpdateFile();

    virtual int     ReadString(LPCTSTR section, LPCTSTR entry, LPTSTR buffer, uint bufSize, LPCTSTR defstr=0) const;
    virtual bool    WriteString(LPCTSTR section, LPCTSTR entry, LPCTSTR value);
    virtual bool    ReadData(LPCTSTR section, LPCTSTR entry, void* buffer, uint size) const;
    virtual bool    WriteData(LPCTSTR section, LPCTSTR entry, void* buffer, uint size);

    // Overloads from base
    
    using TConfigFile::ReadSection;
    using TConfigFile::ReadSections;
    using TConfigFile::EraseSection;
    using TConfigFile::EraseEntry;

    using TConfigFile::ReadString;
    using TConfigFile::WriteString;
    using TConfigFile::ReadData;
    using TConfigFile::WriteData;
};

////////////////////////////////////////////////////////////////////////////
//
/// \class  TMemConfigFile
// ~~~~~  ~~~~~~~~~~~~~~
/// Derived from TConfigFile, TMemConfigFile is used to maintain configuration
/// settings in memory.
//
class TMemConfigFileSections; 
class _OWLCLASS TMemConfigFile: public TConfigFile {
  private:
    TMemConfigFileSections* Sections;

    int AddSection(const tstring& section);

  public:
    TMemConfigFile(const tstring& name);
    virtual ~TMemConfigFile();
    virtual void    LoadValues(const TConfigFile& file);

    virtual int     ReadSection(LPCTSTR section, LPTSTR buffer, uint bufSize) const;
    virtual int     ReadSections(LPTSTR buffer, uint bufSize) const;
    virtual bool    EraseSection(LPCTSTR section);
    virtual bool    EraseEntry(LPCTSTR section, LPCTSTR entry);
    virtual void    UpdateFile();

    virtual int     ReadString(LPCTSTR section, LPCTSTR entry, LPTSTR buffer, uint bufSize, LPCTSTR defstr=0) const;
    virtual bool    WriteString(LPCTSTR section, LPCTSTR entry, LPCTSTR value);

    // Overloads from base
    
    using TConfigFile::ReadSection;
    using TConfigFile::ReadSections;
    using TConfigFile::EraseSection;
    using TConfigFile::EraseEntry;

    using TConfigFile::ReadString;
    using TConfigFile::WriteString;

            void    Flush();
            void    LoadFromFile();
};

//
/// \class  TRegConfigFile
// ~~~~~  ~~~~~~~~~~~~~~
/// Derived from TConfigFile, TRegConfigFile is used to maintain configuration
/// settings in the registry.
//
class _OWLCLASS TRegConfigFile : public TConfigFile {
  public:
    TRegConfigFile(const tstring& name, HKEY hRoot = HKEY_CURRENT_USER);
    virtual ~TRegConfigFile();

    //  Override these to change storage technique:
    //
    virtual bool    SectionExists(LPCTSTR section) const;
    virtual int     ReadSection(LPCTSTR section, LPTSTR buffer, uint bufSize) const;
    virtual int     ReadSections(LPTSTR buffer, uint bufSize) const;
    virtual bool    EraseSection(LPCTSTR section);
    virtual bool    EraseEntry(LPCTSTR section, LPCTSTR entry);
    virtual void    UpdateFile();

    virtual int     ReadInteger(LPCTSTR section, LPCTSTR entry,int defint=0) const;
    virtual bool    WriteInteger(LPCTSTR section, LPCTSTR entry, int value);
    virtual int     ReadString(LPCTSTR section, LPCTSTR entry, LPTSTR buffer, uint bufSize, LPCTSTR defstr=0) const;
    virtual bool    WriteString(LPCTSTR section, LPCTSTR entry, LPCTSTR value);
    virtual bool    ReadData(LPCTSTR section, LPCTSTR entry, void* buffer, uint size) const;
    virtual bool    WriteData(LPCTSTR section, LPCTSTR entry, void* buffer, uint size);

    // Overloads from base
    
    using TConfigFile::SectionExists;
    using TConfigFile::ReadSection;
    using TConfigFile::ReadSections;
    using TConfigFile::EraseSection;
    using TConfigFile::EraseEntry;

    using TConfigFile::ReadInteger;
    using TConfigFile::WriteInteger;
    using TConfigFile::ReadString;
    using TConfigFile::WriteString;
    using TConfigFile::ReadData;
    using TConfigFile::WriteData;

    protected:
        HKEY        hRoot;
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// Inlines
//
inline bool 
TConfigFile::SectionExists(const tstring& section) const
{
  return SectionExists(section.c_str());
}
//
inline bool 
TConfigFile::EraseSection(const tstring& section)
{
  return EraseSection(section.c_str());
}
//
inline bool
TConfigFile::EraseEntry(const tstring& section, const tstring& entry)
{
  return EraseEntry(section.c_str(), entry.c_str());
}

//
inline bool
TConfigFile::ReadData(const tstring& section, const tstring& entry, 
                      void* buffer, uint size) const
{
  return ReadData(section.c_str(), entry.c_str(),buffer,size);
}
//
inline bool
TConfigFile::WriteData(const tstring& section, const tstring& entry, 
                       void* buffer, uint size)
{
  return WriteData(section.c_str(), entry.c_str(), buffer, size);
}
    
//
inline int
TConfigFile::ReadInteger(const tstring& section, const tstring& entry,
                         int defint) const
{
  return ReadInteger(section.c_str(), entry.c_str(), defint);
}
//
inline bool
TConfigFile::WriteInteger(const tstring& section, const tstring& entry,
                          int value)
{
  return WriteInteger(section.c_str(), entry.c_str(), value);
}
//
inline double
TConfigFile::ReadDouble(const tstring& section, const tstring& entry,
                        double defval) const
{
  return ReadDouble(section.c_str(), entry.c_str(), defval);
}
//
inline bool
TConfigFile::WriteDouble(const tstring& section, const tstring& entry,
                         double value)
{
  return WriteDouble(section.c_str(), entry.c_str(), value);
}
//
inline bool
TConfigFile::ReadBool(const tstring& section, const tstring& entry, 
                      bool defval) const
{
  return ReadBool(section.c_str(), entry.c_str(), defval);
}
//
inline bool
TConfigFile::WriteBool(const tstring& section, const tstring& entry, 
                       bool val)
{
  return WriteBool(section.c_str(), entry.c_str(), val);
}
//
inline bool
TConfigFile::ReadDate(const tstring& section, const tstring& entry, 
                      TDate& value) const
{
  return ReadDate(section.c_str(), entry.c_str(), value);
}
//
inline bool
TConfigFile::WriteDate(const tstring& section, const tstring& entry, 
                       const TDate& val)
{
  return WriteDate(section.c_str(), entry.c_str(), val);
}
//
inline bool
TConfigFile::ReadSystemTime(const tstring& section, 
                            const tstring& entry, TSystemTime& val) const
{
  return ReadSystemTime(section.c_str(), entry.c_str(), val);
}
//
inline bool
TConfigFile::WriteSystemTime(const tstring& section, 
                             const tstring& entry, const TSystemTime& val)
{
  return WriteSystemTime(section.c_str(), entry.c_str(), val);
}
//
inline bool
TConfigFile::ReadTime(const tstring& section, const tstring& entry, TTime& val) const
{
  return ReadTime(section.c_str(), entry.c_str(), val);
}
//
inline bool
TConfigFile::WriteTime(const tstring& section, const tstring& entry, TTime& val)
{
  return WriteTime(section.c_str(), entry.c_str(), val);
}
//
inline bool
TConfigFile::ReadFont(const tstring& section, const tstring& entry, 
                      LOGFONT& font) const
{
  return ReadFont(section.c_str(), entry.c_str(), font);
}
//
inline bool
TConfigFile::WriteFont(const tstring& section, const tstring& entry, 
                       const LOGFONT& font)
{
  return WriteFont(section.c_str(), entry.c_str(), font);
}
//
inline bool
TConfigFile::ReadColor(const tstring& section, const tstring& entry, 
                       TColor& color) const
{
  return ReadColor(section.c_str(), entry.c_str(), color);
}
//
inline bool
TConfigFile::WriteColor(const tstring& section, const tstring& entry, 
                        const TColor& color)
{
  return WriteColor(section.c_str(), entry.c_str(), color);
}
//
inline bool
TConfigFile::ReadRect(const tstring& section, const tstring& entry, 
                      TRect& val) const
{
  return ReadRect(section.c_str(), entry.c_str(), val);
}
//
inline bool
TConfigFile::WriteRect(const tstring& section, const tstring& entry, 
                       const TRect& rc)
{
  return WriteRect(section.c_str(), entry.c_str(), rc);
}
//
inline bool
TConfigFile::ReadPoint(const tstring& section, const tstring& entry, 
                       TPoint& val) const
{
  return ReadPoint(section.c_str(), entry.c_str(), val);
}
//
inline bool
TConfigFile::WritePoint(const tstring& section, const tstring& entry, 
                        const TPoint& point)
{
  return WritePoint(section.c_str(), entry.c_str(), point);
}
//
inline bool
TConfigFile::ReadSize(const tstring& section, const tstring& entry, 
                      TSize& sz) const
{
  return ReadSize(section.c_str(), entry.c_str(), sz);
}
//
inline bool
TConfigFile::WriteSize(const tstring& section, const tstring& entry, 
                       const TSize& size)
{
  return WriteSize(section.c_str(), entry.c_str(), size);
}
//
inline bool 
TConfigFile::ReadSize(LPCTSTR section, LPCTSTR entry, TSize& size) const{
  return ReadPoint(section, entry, (TPoint&)size); 
}
//
inline bool 
TConfigFile::WriteString(const tstring& section, const tstring& entry, 
                         const tstring& value)
{
  return WriteString(section.c_str(), entry.c_str(), value.c_str());
}

//
//
//
inline bool 
TConfigFileSection::EraseEntry(LPCTSTR entry) 
{ 
  return File.EraseEntry(Section.c_str(), entry); 
} 

//
//
//
inline int 
TConfigFileSection::ReadString(LPCTSTR entry, LPTSTR buffer, uint bufSize, 
                               LPCTSTR defstr) const
{
  return File.ReadString(Section.c_str(), entry, buffer, bufSize, defstr);
}
//
//
//
inline bool   
TConfigFileSection::WriteString(LPCTSTR entry, LPCTSTR value)
{
  return File.WriteString(Section.c_str(), entry, value);
}
//
//
//
inline int 
TConfigFileSection::ReadString(const tstring& entry, tstring& buffer, 
                               LPCTSTR defstr) const
{
  return File.ReadString(Section, entry, buffer, defstr);
}
//
//
//
inline bool   
TConfigFileSection::WriteString(const tstring& entry, const tstring& value)
{
  return File.WriteString(Section, entry, value);
}
//
//
//
inline bool
TConfigFileSection::ReadData(LPCTSTR entry, void* buffer, uint size) const
{
  return File.ReadData(Section.c_str(), entry, buffer, size);
}
//
//
//
inline bool   
TConfigFileSection::WriteData(LPCTSTR entry, void* buffer, uint size)
{
  return File.WriteData(Section.c_str(), entry, buffer, size);
}
//
//
//
inline bool   
TConfigFileSection::ReadData(const tstring& entry, void* buffer, uint size) const
{
  return File.ReadData(Section, entry, buffer, size);
}
//
//
//
inline bool   
TConfigFileSection::WriteData(const tstring& entry, void* buffer, uint size)
{
  return File.WriteData(Section, entry, buffer, size);
}
//
//
//
inline int      
TConfigFileSection::ReadInteger(LPCTSTR entry,int defint) const
{
  return File.ReadInteger(Section.c_str(), entry, defint);
}
//
//
//
inline bool   
TConfigFileSection::WriteInteger(LPCTSTR entry, int value)
{
  return File.WriteInteger(Section.c_str(), entry, value);
}
//
//
//
inline int     
TConfigFileSection::ReadInteger(const tstring& entry,int defint) const 
{
  return File.ReadInteger(Section.c_str(), entry.c_str(), defint);
}
//
//
//
inline bool    
TConfigFileSection::WriteInteger(const tstring& entry, int value)
{
  return File.WriteInteger(Section.c_str(), entry.c_str(), value);
}
//
//
//
inline double  
TConfigFileSection::ReadDouble(LPCTSTR entry, double defval) const
{
  return File.ReadDouble(Section.c_str(), entry, defval);
}
//
//
//
inline bool   
TConfigFileSection::WriteDouble(LPCTSTR entry, double defval)
{
  return File.WriteDouble(Section.c_str(), entry, defval);
}
//
//
//
inline double  
TConfigFileSection::ReadDouble(const tstring& entry, double defval) const
{
  return File.ReadDouble(Section.c_str(), entry.c_str(), defval);
}
//
//
//
inline bool    
TConfigFileSection::WriteDouble(const tstring& entry, double defval)
{
  return File.WriteDouble(Section.c_str(), entry.c_str(), defval);
}
//
//
//
inline bool   
TConfigFileSection::ReadBool(LPCTSTR entry, bool defval) const
{
  return File.ReadBool(Section.c_str(), entry, defval);
}
//
//
//
inline bool   
TConfigFileSection::WriteBool(LPCTSTR entry, bool val)
{
  return File.WriteBool(Section.c_str(), entry, val);
}
//
//
//
inline bool   
TConfigFileSection::ReadBool(const tstring& entry, bool defval) const
{
  return File.ReadBool(Section, entry, defval);
}
//
//
//
inline bool   
TConfigFileSection::WriteBool(const tstring& entry, bool val)
{
  return File.WriteBool(Section, entry, val);
}
//
//
//
inline bool   
TConfigFileSection::ReadDate(LPCTSTR entry, TDate& value) const
{
  return File.ReadDate(Section.c_str(), entry, value);
}
//
//
//
inline bool   
TConfigFileSection::WriteDate(LPCTSTR entry, const TDate& val)
{
  return File.WriteDate(Section.c_str(), entry, val);
}
//
//
//
inline bool   
TConfigFileSection::ReadDate(const tstring& entry, TDate& value) const
{
  return File.ReadDate(Section, entry, value);
}
//
//
//
inline bool   
TConfigFileSection::WriteDate(const tstring& entry, const TDate& val)
{
  return File.WriteDate(Section, entry, val);
}
//
//
//
inline bool   
TConfigFileSection::ReadSystemTime(LPCTSTR entry, TSystemTime& val) const
{
  return File.ReadSystemTime(Section.c_str(), entry, val);
}
//
//
//
inline bool   
TConfigFileSection::WriteSystemTime(LPCTSTR entry, const TSystemTime& val)
{
  return File.WriteSystemTime(Section.c_str(), entry, val);
}
//
//
//
inline bool   
TConfigFileSection::ReadSystemTime(const tstring& entry, TSystemTime& val) const
{
  return File.ReadSystemTime(Section, entry, val);
}
//
//
//
inline bool   
TConfigFileSection::WriteSystemTime(const tstring& entry, const TSystemTime& val)
{
  return File.WriteSystemTime(Section, entry, val);
}
//
//
//
inline bool   
TConfigFileSection::ReadTime(LPCTSTR entry, TTime& val) const
{
  return File.ReadTime(Section.c_str(), entry, val);
}
//
//
//
inline bool   
TConfigFileSection::WriteTime(LPCTSTR entry, TTime& val)
{
  return File.WriteTime(Section.c_str(), entry, val);
}
//
//
//
inline bool   
TConfigFileSection::ReadTime(const tstring& entry, TTime& val) const
{
  return File.ReadTime(Section, entry, val);
}
//
//
//
inline bool   
TConfigFileSection::WriteTime(const tstring& entry, TTime& val)
{
  return File.WriteTime(Section, entry, val);
}
//
//
//
inline bool    
TConfigFileSection::ReadFont(LPCTSTR entry, LOGFONT& font) const
{
  return File.ReadFont(Section.c_str(), entry, font);
}
//
//
//
inline bool    
TConfigFileSection::WriteFont(LPCTSTR entry, const LOGFONT& font)
{
  return File.WriteFont(Section.c_str(), entry, font);
}
//
//
//
inline bool    
TConfigFileSection::ReadFont(const tstring& entry, LOGFONT& font) const
{
  return File.ReadFont(Section, entry, font);
}
//
//
//
inline bool    
TConfigFileSection::WriteFont(const tstring& entry, const LOGFONT& font)
{
  return File.WriteFont(Section, entry, font);
}
//
//
//
inline bool    
TConfigFileSection::ReadColor(LPCTSTR entry, TColor& color) const
{
  return File.ReadColor(Section.c_str(), entry, color);
}
//
//
//
inline bool    
TConfigFileSection::WriteColor(LPCTSTR entry, const TColor& color)
{
  return File.WriteColor(Section.c_str(), entry, color);
}
//
//
//
inline bool    
TConfigFileSection::ReadColor(const tstring& entry, TColor& color) const
{
  return File.ReadColor(Section, entry, color);
}
//
//
//
inline bool    
TConfigFileSection::WriteColor(const tstring& entry, const TColor& color)
{
  return File.WriteColor(Section, entry, color);
}
//
//
//
inline bool    
TConfigFileSection::ReadRect(LPCTSTR entry, TRect& rect) const
{
  return File.ReadRect(Section.c_str(), entry, rect);
}
//
//
//
inline bool    
TConfigFileSection::WriteRect(LPCTSTR entry, const TRect& rect)
{
  return File.WriteRect(Section.c_str(), entry, rect);
}
//
//
//
inline bool    
TConfigFileSection::ReadRect(const tstring& entry, TRect& rect) const
{
  return File.ReadRect(Section, entry, rect);
}
//
//
//
inline bool    
TConfigFileSection::WriteRect(const tstring& entry, const TRect& rect)
{
  return File.WriteRect(Section, entry, rect);
}
//
//
//
inline bool    
TConfigFileSection::ReadPoint(LPCTSTR entry, TPoint& point)const
{
  return File.ReadPoint(Section.c_str(), entry, point);
}
//
//
//
inline bool    
TConfigFileSection::WritePoint(LPCTSTR entry, const TPoint& point)
{
  return File.WritePoint(Section.c_str(), entry, point);
}
//
//
//
inline bool    
TConfigFileSection::ReadPoint(const tstring& entry, TPoint& point)const
{
  return File.ReadPoint(Section, entry, point);
}
//
//
//
inline bool    
TConfigFileSection::WritePoint(const tstring& entry, const TPoint& point)
{
  return File.WritePoint(Section, entry, point);
}
//
//
//
inline bool    
TConfigFileSection::ReadSize(LPCTSTR entry, TSize& size) const
{
  return File.ReadSize(Section.c_str(), entry, size);
}
//
//
//
inline bool    
TConfigFileSection::WriteSize(LPCTSTR entry, const TSize& size)
{
  return File.WriteSize(Section.c_str(), entry, size);
}
//
//
//
inline bool    
TConfigFileSection::ReadSize(const tstring& entry, TSize& size) const
{
  return File.ReadSize(Section, entry, size);
}
//
//
//
inline bool    
TConfigFileSection::WriteSize(const tstring& entry, const TSize& size)
{
  return File.WriteSize(Section, entry, size);
}
//
//
//

//
//
//
} // OWL namespace


#endif   // OWL_CONFIGFL_H

