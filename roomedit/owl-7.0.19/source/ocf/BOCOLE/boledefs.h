//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Contains the main constants and structures used by Bolero to encapsulate
//  OLE2 compound document functionality.
//----------------------------------------------------------------------------
#ifndef _BOLEDEFS_H
#define _BOLEDEFS_H

#ifndef _BOLEVER_H
#include <bolever.h>
#endif

#ifndef _BCOM_H
#include <bcom.h>
#endif

#include "pshpack8.h"

#define BOLEBIND TEXT("CREATECLASSMGR")


/*
 *        Useful OLE2.H  declarations
 */


  typedef struct tPOINTF
  {
  float x;
  float y;
  } TPOINTF;


#if !defined (_OLE2_H_)

  #define LPRECTL PRECTL

  #ifndef LPCRECT
  typedef const RECT FAR* LPCRECT;
  #endif

  #ifndef LPCRECTL
  typedef const RECTL FAR* LPCRECTL;
  #endif

  //  opaque pass throughs

  struct _ICLASS IMoniker;
  struct _ICLASS IStorage;
  struct _ICLASS IDataObject;

#endif

typedef const SIZE FAR* LPCSIZE;

class _ICLASS IBContainer; // forward reference

/*
 * BOLERO linking and embedding parameter typedefs
 */

typedef long Coord;
#define HIMETRIC_PER_INCH   2540      // number HIMETRIC units per inch

// MAP_PIX_TO_LOGHIM / MAP_LOGHIM_TO_PIX --
//
// The OLE2-defined macros for these guys don't round. Our versions
// add half the denominator so that the truncated result of the integer
// division is rounded to the nearest integer. e.g. 9/10 truncates to 0,
// instead of rounding to 1, so add 5/10 which makes 14/10 which truncates
// to 1 which simulates rounding.
//
#ifdef MAP_PIX_TO_LOGHIM
#undef MAP_PIX_TO_LOGHIM
#endif
#define MAP_PIX_TO_LOGHIM(pixel, pixelsPerInch) \
  ((ULONG) ((((ULONG)HIMETRIC_PER_INCH) * pixel + (pixelsPerInch >> 1)) / pixelsPerInch))

#ifdef MAP_LOGHIM_TO_PIX
#undef MAP_LOGHIM_TO_PIX
#endif
#define MAP_LOGHIM_TO_PIX(hiMetric, pixelsPerInch) \
  (UINT) ((((double) hiMetric) * pixelsPerInch + (HIMETRIC_PER_INCH >> 1)) / HIMETRIC_PER_INCH)

#if defined(INIT_BGUID)
  #define DEFINE_BOLEGUID(name, l) \
   EXTERN_C __declspec(dllexport) const GUID CDECL FAR name = { l, 0, 0, {0xC0,0,0,0,0,0,0,0x46} }
#else
  #define DEFINE_BOLEGUID(name, l) \
   EXTERN_C const GUID CDECL FAR name
#endif

DEFINE_BOLEGUID(IID_IBPart,          0x02A104L);
DEFINE_BOLEGUID(IID_IBSite,          0x02A105L);
DEFINE_BOLEGUID(IID_IBContainer,     0x02A106L);
DEFINE_BOLEGUID(IID_IBDocument,      0x02A107L);
DEFINE_BOLEGUID(IID_IBService,       0x02A108L);
DEFINE_BOLEGUID(IID_IBApplication,   0x02A109L);
DEFINE_BOLEGUID(IID_IBClassMgr,      0x02A10AL);
DEFINE_BOLEGUID(IID_IBLinkInfo,      0x02A10BL);
DEFINE_BOLEGUID(IID_IBLinkable,      0x02A10CL);
DEFINE_BOLEGUID(IID_IBRootLinkable,  0x02A10DL);
DEFINE_BOLEGUID(IID_IBContains,      0x02A10EL);
DEFINE_BOLEGUID(IID_BOleDocument,    0x02A10FL);
DEFINE_BOLEGUID(IID_BOlePart,        0x02A110L);
DEFINE_BOLEGUID(IID_IBDataConsumer,  0x02A111L);
DEFINE_BOLEGUID(IID_IBDataProvider,  0x02A112L);
DEFINE_BOLEGUID(IID_IBWindow,        0x02A113L);
DEFINE_BOLEGUID(IID_IBDataNegotiator,0x02A114L);
DEFINE_BOLEGUID(IID_IBDropDest,      0x02A115L);
DEFINE_BOLEGUID(IID_BOleShadowData,  0x02A116L);
DEFINE_BOLEGUID(IID_IBClass,         0x02A117L);
DEFINE_BOLEGUID(IID_IBOverrideBrowseLinks,   0x02A118L);
DEFINE_BOLEGUID(IID_BOleSite,        0x02A119L);
DEFINE_BOLEGUID(IID_IBSite2,         0x02A11AL);
DEFINE_BOLEGUID(IID_IBOverrideHelpButton,  0x02A11BL);
DEFINE_BOLEGUID(IID_IBDataProvider2, 0x02A11CL);
DEFINE_BOLEGUID(IID_IBPart2,         0x02A11DL);
DEFINE_BOLEGUID(IID_IBService2,      0x02A11EL);
DEFINE_BOLEGUID(IID_BOleControl,     0x02A11FL);
DEFINE_BOLEGUID(IID_IBControl,       0x02A120L);
DEFINE_BOLEGUID(IID_IBControlSite,   0x02A121L);
DEFINE_BOLEGUID(IID_BOleFont,        0x02A122L);
DEFINE_BOLEGUID(IID_BOleControlSite, 0x02A123L);
DEFINE_BOLEGUID(IID_IBEventsHandler, 0x02A124L);
DEFINE_BOLEGUID(IID_IBEventClass,    0x02A125L);
DEFINE_BOLEGUID(IID_IBSinkList,      0x02A126L);
DEFINE_BOLEGUID(IID_BOlePicture,     0x02A127L);
DEFINE_BOLEGUID(IID_IBPicture,       0x02A128L);
DEFINE_BOLEGUID(IID_IBDataState,     0x02A129L);


// Bolero Class ids  for instantiating bolero objects (helpers)

typedef ULONG BCID;

const BCID      cidBOleService          = 1L;   //
const BCID      cidBOleDocument         = 2L;   // helper for containers
const BCID      cidBOlePart             = 3L;   // helper for sites
const BCID      cidBOleSite             = 4L;   // helper for parts
const BCID      cidBOleInProcSite       = 5L;   // helper for parts
const BCID      cidBOleContainer        = 6L;   // helper for part documents
const BCID      cidBOleData             = 7L;   // helper for data transfer
const BCID      cidBOleFactory          = 8L;   // helper for class factory
const BCID      cidBOleShadowData       = 9L;   // helper for non delayed rendering
const BCID      cidBOleInProcHandler    = 10L;  // helper for dll handler parts
const BCID      cidBOleControl          = 11L;  // helper for controls
const BCID      cidBOleFont             = 12L;  // IBFont implementation
const BCID      cidBOleControlSite      = 13L;  // helper for controls
const BCID      cidBOleControlFactory   = 14L;  // helper for control class factory
const BCID      cidBOlePicture          = 15L;  // IBPicture implementation


typedef enum {
  BOLE_MED_NULL    = 0,
  BOLE_MED_HGLOBAL = 1,
  BOLE_MED_FILE    = 2,
  BOLE_MED_STREAM  = 4,
  BOLE_MED_STORAGE = 8,
  BOLE_MED_GDI     = 16,
  BOLE_MED_MFPICT  = 32, // leave room for any new OLE mediums
  BOLE_MED_STATIC  = 1024
} BOleMedium;

typedef enum {
  BOLE_HELP_EXIT,
  BOLE_HELP_ENTER,
  BOLE_HELP_GET
} BOleHelp;

typedef enum {
  BOLE_MOUSEENTER = 1,
  BOLE_MOUSECONTAINED = 2,
  BOLE_MOUSEEXIT = 3
} BOleMouseAction;

typedef enum {
  BOLE_DROP_NONE = 0,
  BOLE_DROP_COPY = 1,
  BOLE_DROP_MOVE = 2,
  BOLE_DROP_LINK = 4
} BOleDropAction;

typedef enum {
  BOLE_CONTENT         = 1,
  BOLE_THUMBNAIL       = 2,
  BOLE_ICON            = 4,
  BOLE_DOCPRINT        = 8,
  BOLE_DEFAULT         = 16,
  BOLE_ASPECT_MAINTAIN = 32,
  BOLE_CLIP            = 64
} BOleAspect;

typedef enum {
  BOLE_DRAW_NONE        = 0,
  BOLE_DRAW_SHADINGONLY = 1,
  BOLE_DRAW_SHOWOBJECTS = 2
} BOleDraw;

typedef enum {
  BOLE_SCROLL_UP    = 1,
  BOLE_SCROLL_DOWN  = 2,
  BOLE_SCROLL_LEFT  = 4,
  BOLE_SCROLL_RIGHT = 8
} BOleScroll;

typedef enum {
  BOLE_NAME_LONG  = 1,
  BOLE_NAME_SHORT = 2,
  BOLE_NAME_APP   = 3
} BOlePartName;

typedef enum {
  BOLE_HELP_BROWSE,
  BOLE_HELP_BROWSECLIPBOARD,
  BOLE_HELP_CONVERT,
  BOLE_HELP_BROWSELINKS,
  BOLE_HELP_CHANGEICON,
  BOLE_HELP_FILEOPEN,
  BOLE_HELP_SOURCESET,
  BOLE_HELP_ICONFILEOPEN,
  BOLE_HELP_BROWSECONTROL
} BOleDialogHelp;

typedef enum {
  BOLE_ENABLE_PASTE           = 1,
  BOLE_ENABLE_PASTELINK       = 2,
  BOLE_ENABLE_BROWSECLIPBOARD = 4,
  BOLE_ENABLE_BROWSELINKS     = 8
} BOleMenuEnable;

typedef enum {
  BOLE_INVAL_DATA       = 1,
  BOLE_INVAL_VIEW       = 2,
  BOLE_INVAL_PERSISTENT = 4
} BOleInvalidate;

typedef struct FAR {
  LPCOLESTR       typeName;
  LPCOLESTR       verbName;
  UINT            verbIndex;
  BOOL            fCanDirty;
} BOleVerb;

typedef struct FAR {
  LONG    width[6];
} BOleMenuWidths;

typedef struct FAR {
  UINT fmtId;
  OLECHAR fmtName[32];
  OLECHAR fmtResultName[32];
  BOleMedium fmtMedium;
  BOOL fmtIsLinkable;
} BOleFormat;

typedef enum {
  BOLE_CONVERT_TO_CLSID   = 1,
  BOLE_ACTIVATE_AS_CLSID  = 2,
  BOLE_ASPECT_TO_ICON     = 4,
  BOLE_ASPECT_TO_CONTENT  = 8
} BOleConvertAction;

typedef enum {
  BOLE_LINK,
  BOLE_EMBED,
  BOLE_EMBED_ASMETAFILE,
  BOLE_EMBED_ASBITMAP
} InitHow;

typedef enum {
  BOLE_FILE,
  BOLE_STORAGE,
  BOLE_DATAOBJECT,
  BOLE_NEW,
  BOLE_HANDLE,
  BOLE_NEW_OCX,
  BOLE_STREAM
} InitWhere;

typedef struct FAR {
  InitHow How;
  InitWhere Where;
  IBContainer FAR* pContainer;

  HICON hIcon;

  IStorage *pStorage;
  union {
    struct {    // where == BOLE_FILE
      LPCOLESTR pPath;
    } whereFile;
    struct {    // where == BOLE_DATAOBJECT
      IDataObject *pData;
    } whereData;
    struct {    // where == BOLE_NEW
      BCID cid;
    } whereNew;
    struct {    // where == BOLE_HANDLE
      HANDLE data;
      UINT dataFormat;
    } whereHandle;
    struct {    // where == BOLE_STREAM
      LPSTREAM pStream;
    } whereStream;
  };
  inline LONG ReleaseDataObject()
  { return ((Where == BOLE_DATAOBJECT) && whereData.pData) ?
    ((IUnknown *)whereData.pData)->Release() :
    0;
  }

} BOleInitInfo;

typedef enum {
  BOLE_UPDATE_AUTO   = 1,
  BOLE_UPDATE_MANUAL = 3
} BOleLinkUpdate;

typedef struct FAR {
  ULONG       xN;      // numerator in x direction
  ULONG       xD;      // denominator in x direction
  ULONG       yN;      // numerator in y direction
  ULONG       yD;      // denominator in y direction
} BOleScaleFactor;

// This tagged union allows macro record/playback on the Convert dialog
//
typedef struct FAR {
  BOleConvertAction action;
  CLSID clsidNew;
  HGLOBAL hIcon;
} BOleConvertInfo;

// Entry Points
//
extern "C" {
  extern HRESULT PASCAL FAR _export CreateClassMgr (PIUnknown FAR*pCM, PIUnknown pOut, IMalloc FAR*);
}
inline PIUnknown CreateClassMgr( PIUnknown pOut = NULLP, IMalloc FAR* pMalloc = NULLP)
{
  PIUnknown tmpCM;
  return CreateClassMgr(&tmpCM, pOut, pMalloc) == NOERROR ? tmpCM : NULLP;
}

#endif
