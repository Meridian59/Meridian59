//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TDocTemplate, and class template TDocTemplateT<D,V>
//----------------------------------------------------------------------------
#if !defined(OWL_DOCTPL_H)
#define OWL_DOCTPL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/registry.h>
#include <owl/objstrm.h>
#include <owl/private/gmodule.h>


# define _DOCVIEWENTRY


namespace owl {class _OWLCLASS TDocTemplate;};


//
/// Templates constructed before app get linked onto this head pointer
//
extern owl::TDocTemplate* DocTemplateStaticHead;

namespace owl {

/// \addtogroup docview
/// @{

class _OWLCLASS TDocManager;
class _OWLCLASS TModule;
class _OWLCLASS TDocument;
class _OWLCLASS TView;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TDocTemplate
// ~~~~~ ~~~~~~~~~~~~
/// TDocTemplate is an abstract base class that contains document template
/// functionality. This document template class defines several functions that make
/// it easier for you to use documents and their corresponding views. TDocTemplate
/// classes create documents and views from resources and handle document naming and
/// browsing. The document manager maintains a list of the current template objects.
/// Each document type requires a separate document template.
//
class _OWLCLASS TDocTemplate : public TRegLink, public TStreamableBase {
  public:
  	
/// A pure virtual function that must be defined in a derived class, ConstructDoc
/// creates a document specified by the document template class. Use this function
/// in place of CreateDoc.
    virtual TDocument*      ConstructDoc(TDocument* parent = 0) = 0;
    
/// A pure virtual function that must be defined in a derived class, ConstructView
/// creates the view specified by the document template class.
    virtual TView*          ConstructView(TDocument& doc) = 0;
    
/// A pure virtual function that must be defined in a derived class, IsMyKindOfDoc
/// tests if the template belongs to the same class as the document or to a derived
/// class.
    virtual TDocument*      IsMyKindOfDoc(TDocument& doc) = 0;
    
/// A pure virtual function that must be defined in a derived class, IsMyKindofView
/// tests if the template belongs to the same class as the view or to a derived
/// class.
    virtual TView*          IsMyKindOfView(TView& view) = 0;
    
/// A pure virtual function that must be defined in a derived class, GetViewName
/// gets the name of the view associated with the template.
    virtual LPCTSTR         GetViewName() = 0;

    bool                    IsVisible(); ///< displayable in file select dialogs
    
/// Makes a copy of a document template.
#if !defined (BI_GLOBALNM_BUG)
    virtual TDocTemplate*   Clone(TModule* module,
                                  TDocTemplate*& phead=::DocTemplateStaticHead)=0;
#else
    virtual TDocTemplate*   Clone(TModule* module,
                                  TDocTemplate*& phead=DocTemplateStaticHead)=0;
#endif

    TDocManager*            GetDocManager() const;
    void                    SetDocManager(TDocManager* dm);
    LPCTSTR                 GetDirectory() const;
    void                    SetDirectory(LPCTSTR);
    void                    SetDirectory(LPCTSTR, int len);
    TDocTemplate*           GetNextTemplate() const;
    bool                    IsFlagSet(long flag) const;
    long                    GetFlags() const;
    void                    SetFlag(long flag);
    void                    ClearFlag(long flag);
    bool                    IsStatic() const;
    TModule*&               GetModule();
    void                    SetModule(TModule* module);
    int                     GetRefCount() const;

    LPCTSTR                 GetFileFilter() const;
    LPCTSTR                 GetDescription() const;
    LPCTSTR                 GetDefaultExt() const;
    void                    SetFileFilter(LPCTSTR);
    void                    SetDescription(LPCTSTR);
    void                    SetDefaultExt(LPCTSTR);


  protected:                // Called only from parameterized derived class
    TDocTemplate(TRegList& regList, TModule*& module, TDocTemplate*& phead);
   ~TDocTemplate();

  private:
    tchar *    Directory;     ///< Current directory path, 0 until changed
    long          Flags;         ///< dtXxx bits for dialogs and creation modes
    TDocManager*  DocManager;    ///< Pointer back to document manager
    int            RefCnt;        ///< Documents attached + 1 if attached to docmgr
    TModule**      ModulePtr;     ///< Would be TModule*&, except for streaming
    TModule*      ModuleDeref;   ///< Used only when setting a new module

  protected:                // Called only from parameterized derived class
    TDocTemplate(LPCTSTR desc, LPCTSTR filt,
                 LPCTSTR dir,  LPCTSTR ext, long flags,
                 TModule*& module, TDocTemplate*& phead);

  friend class TDocument;   // access to RefCnt
  friend class TDocManager;
  //DECLARE_ABSTRACT_STREAMABLE(_OWLCLASS, TDocTemplate, 1);
  DECLARE_ABSTRACT_STREAMABLE_OWL(TDocTemplate, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TDocTemplate );

#define DECLARE_STREAMABLE_OPSDV( cls )                             \
static ipstream& readRef( ipstream& is, cls& cl );                  \
friend inline ipstream& operator >> ( ipstream& is, cls& cl )       \
    { return cls::readRef( is, cl ); }                              \
static ipstream& readPtr( ipstream& is, cls*& cl );                 \
friend inline ipstream& operator >> ( ipstream& is, cls*& cl )      \
    { return cls::readPtr( is, cl ); }                              \
static opstream& writeRef( opstream& is, const cls& cl );           \
friend inline opstream& operator << ( opstream& os, const cls& cl ) \
    { return cls::writeRef( os, cl ); }                             \
static opstream& writePtr( opstream& is, const cls* cl );           \
friend inline opstream& operator << ( opstream& os, const cls* cl ) \
    { return cls::writePtr( os, cl ); }

//
// class TDocTemplateT<D,V>
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// To register the associated document and view classes, a parameterized subclass,
/// TDocTemplateT<D,V>, is used to construct a particular document and view, where D
/// represents the document class and V represents the view class. The parameterized
/// template classes are created using a macro, which also generates the associated
/// streamable support. The document and view classes are provided through the use
/// of a parameterized subclass. The template class name is used as a typedef for
/// the parameterized class. For example,
/// \code
/// DEFINE_DOC_TEMPLATE_CLASS(TFileDocument, TEditView, MyEditFile)
/// \endcode
/// You can instantiate a document template using either a static member or an
/// explicit construction. For example,
/// \code
/// MyEditFile et1(_T("Edit text files"),
///       T("*.txt"),T("D:\\doc"),_T(".TXT"),dtNoAutoView);
/// new MyEditFile(.....)
/// \endcode
/// When a document template is created, the document manager (TDocManager)
/// registers the template. When the document template's delete function is called
/// to delete the template, it is no longer visible to the user. However, it remains
/// in memory as long as any documents still use it.
//
template<class D, class V>
class _DOCVIEWENTRY TDocTemplateT : public TDocTemplate {
  public:
    TDocTemplateT(TRegList& regList, TModule*& module = owl::Module,
                  TDocTemplate*& phead = DocTemplateStaticHead);
    TDocTemplateT(LPCTSTR desc, LPCTSTR filt,
                  LPCTSTR dir, LPCTSTR ext, long flags = 0,
                  TModule*& module = owl::Module,
                  TDocTemplate*& phead = DocTemplateStaticHead);
#if !defined (BI_GLOBALNM_BUG)
    TDocTemplateT* Clone(TModule* module,
                         TDocTemplate*& phead=::DocTemplateStaticHead);
#else
    TDocTemplateT* Clone(TModule* module,
                         TDocTemplate*& phead=DocTemplateStaticHead);
#endif

    D* ConstructDoc(TDocument* parent = 0);
    V* ConstructView(TDocument& doc);
    D* IsMyKindOfDoc(TDocument& doc);  ///< Returns 0 if template can't support
    V* IsMyKindOfView(TView& view);    ///< Returns 0 if template incompatible

    virtual LPCTSTR GetViewName();

#if !defined(BI_NO_OBJ_STREAMING)
  // Explicit expansion of DECLARE_STREAMABLE for use with parameterized class
  // DECLARE_STREAMABLE_FROM_BASE(TDocTemplateT<D,V>);
  //
  typedef TDocTemplateT<D,V> TEMPLATECLASS;
  DECLARE_CASTABLE;
  DECLARE_STREAMABLE_CTOR(TDocTemplateT);
  DECLARE_STREAMER_FROM_BASES(TEMPLATECLASS, TDocTemplate);
  DECLARE_STREAMABLE_OPSDV(TEMPLATECLASS);
#endif
};

// define streameable inlines (VC)
//DECLARE_STREAMABLE_INLINESDV( D,V );
//DECLARE_STREAMABLE_INLINEST( TDocTemplateT );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Template flags used in reg parameter "docflags"
//

//
// Definitions of dtXxx document/view flags for templates and CreateDoc()
//
const long dtNewDoc     = 0x80000000L; ///< create new document, no path used
const long dtAutoDelete = 0x40000000L; ///< delete doc when last view is deleted
const long dtNoAutoView = 0x20000000L; ///< no automatic create of default view
const long dtSingleView = 0x10000000L; ///< only a single view per document
const long dtAutoOpen   = 0x08000000L; ///< open document upon creation
const long dtUpdateDir  = 0x04000000L; ///< syncronize directory with dialog dir
const long dtHidden     = 0x02000000L; ///< hide template from user selection
const long dtSelected   = 0x01000000L; ///< indicates template last selected
const long dtDynRegInfo = 0x00800000L; ///< reginfo table is dynamic, not static
const long dtSingleUse  = 0x00400000L; ///< to be registered as single use
const long dtRegisterExt= 0x00200000L; ///< register extension with this app
const long dtForbidNew  = 0x00100000L; ///< forbids creating new document

//
// Flags defined by Windows in <commdlg.h> - use the low order 17 bits
//
const long dtReadOnly       = 0x0001; //OFN_READONLY;         ///< initial check readonly box
const long dtOverwritePrompt= 0x0002; //OFN_OVERWRITEPROMPT;  ///< prompt before overwrite
const long dtHideReadOnly   = 0x0004; //OFN_HIDEREADONLY;     ///< no show readonly box
const long dtPathMustExist  = 0x0800; //OFN_PATHMUSTEXIST;    ///< path must be valid
const long dtFileMustExist  = 0x1000; //OFN_FILEMUSTEXIST;    ///< file must exist
const long dtCreatePrompt   = 0x2000; //OFN_CREATEPROMPT;     ///< prompt before create
const long dtNoReadOnly     = 0x8000; //OFN_NOREADONLYRETURN; ///< return writable file
const long dtNoTestCreate   = 0x10000L;//OFN_NOTESTFILECREATE; ///< skip create tests;

/// @}



//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Points to the document manager.
//
inline TDocManager* TDocTemplate::GetDocManager() const
{
  return DocManager;
}

//
/// Sets the current document manager to the argument dm.
//
inline void TDocTemplate::SetDocManager(TDocManager* dm)
{
  DocManager = dm;
}

//
/// Gets the next template in the list of templates.
//
inline TDocTemplate* TDocTemplate::GetNextTemplate() const 
{ 
  return (TDocTemplate*)Next;
}

//
/// Returns nonzero if the document template flag is set.
//
inline bool TDocTemplate::IsFlagSet(long flag) const
{
  return (GetFlags() & flag) != 0;
}

//
/// Gets the document template constants, which indicate how the document is created
/// and opened.
//
inline long TDocTemplate::GetFlags() const
{
  return Flags;
}

//
/// Returns true if the template is statically constructed.
//
inline bool TDocTemplate::IsStatic() const
{
  return (RefCnt & 0x8000) != 0;
}

//
/// Returns a module pointer.
//
inline TModule*& TDocTemplate::GetModule()
{
  return *ModulePtr;
}

//
/// Return the number of reference count of the template.
/// \note The reference count of static templates has the high bit set.
//
inline int TDocTemplate::GetRefCount() const
{
  return RefCnt;
}

//
/// Sets a module pointer.
//
inline void TDocTemplate::SetModule(TModule* module)
{
  ModuleDeref = module;
  ModulePtr = &ModuleDeref;
}

//
/// Indicates whether the document can be displayed in the file selection dialog
/// box. A document is visible if dtHidden isn't set and Description isn't 0.
//
inline bool
TDocTemplate::IsVisible()
{
  return (GetFlags() & dtHidden) == 0;
}

//
/// Constructs a TDocTemplateT  using the registration table to determine the file
/// filter pattern, search path for viewing the directory, default file extension,
/// and flag values. See the entry in this manual for registration macros for more
/// information about how the registration tables are created. module, which is
/// instantiated and exported directly from every executable module, can be used to
/// access the current instance.
//
template<class D, class V> inline
TDocTemplateT<D,V>::TDocTemplateT(TRegList& regList, TModule*& module,
                                  TDocTemplate*& phead)
:
  TDocTemplate(regList, module, phead)
{
}

//
/// Constructs a TDocTemplateT with the specified file description (desc), file
/// filter pattern (filt), search path for viewing the directory (dir), default file
/// extension (ext), and flags representing the view and creation options (flags).
/// module, which is instantiated and exported directly from every executable
/// module, can be used to access the current instance.
//
template<class D, class V> inline
TDocTemplateT<D,V>::TDocTemplateT(LPCTSTR desc, LPCTSTR filt,
                        LPCTSTR dir,  LPCTSTR ext, long flags,
                        TModule*& module, TDocTemplate*& phead)
:
  TDocTemplate(desc, filt, dir, ext, flags, module, phead)
{
}

//
/// Makes a copy of the TDocTemplateT object.
//
template<class D, class V> inline TDocTemplateT<D,V>*
TDocTemplateT<D,V>::Clone(TModule* module, TDocTemplate*& phead)
{
  TDocTemplateT<D,V>* tpl = new TDocTemplateT<D,V>(GetRegList(),
                                                   GetModule(), phead);
  tpl->SetModule(module);
  return tpl;
}

//
/// 'Factory' method to create a new document of type 'D' using the specified
/// parameter as the parent document.
//
template<class D, class V> inline D*
TDocTemplateT<D,V>::ConstructDoc(TDocument* parent/* = 0*/)
{
  return new D(parent);
}

//
/// 'Factory' method to create a new view of type 'V' from the specified
/// document parameter.
//
template<class D, class V> inline V*
TDocTemplateT<D,V>::ConstructView(TDocument& doc)
{
  return new V((D&)doc);
}

//
/// Gets the name of the view associated with the template.
//
template<class D, class V> inline LPCTSTR
TDocTemplateT<D,V>::GetViewName()
{
  return V::StaticName();
}

//----------------------------------------------------------------------------

/// Tests to see if the document (doc) is either the same class as the template's
/// document class or a derived class. If the template can't use the document,
/// IsMyKIndOfDoc returns 0.
//
template<class D, class V> D*
TDocTemplateT<D,V>::IsMyKindOfDoc(TDocument& doc)
{
  return dynamic_cast<D*>(&doc);
}

/// Tests to see if the view (view) is either the same class as the template's view
/// class or a derived class. If the template can't use the view, IsMyKIndOfView
/// returns 0.
//
template<class D, class V> V*
TDocTemplateT<D,V>::IsMyKindOfView(TView& view)
{
  return dynamic_cast<V*>(&view);
}

#define DEFINE_DOC_TEMPLATE_CLASS(docClass, viewClass, cls)\
  typedef ::owl::TDocTemplateT<docClass, viewClass> cls;\
  IMPLEMENT_STREAMABLE_CLASS(cls);\
  IMPLEMENT_STREAMABLE_CTOR1_IMPL(template<>, cls, TDocTemplateT, ::owl::TDocTemplate)\
  IMPLEMENT_STREAMABLE_POINTER_IMPL(template<>, cls)

} // OWL namespace

#endif  // OWL_DOCTPL_H
