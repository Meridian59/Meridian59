//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Various general OC enums and structs
//----------------------------------------------------------------------------

#if !defined(OCF_OCOBJECT_H)
#define OCF_OCOBJECT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/defs.h>
#include <ocf/oleutil.h>
#include <ocf/ocreg.h>



namespace ocf {

class _ICLASS IBContainer; // forward references
class TOcVerb;

enum TOcDialogHelp {
  dhBrowse,
  dhBrowseClipboard,
  dhConvert,
  dhBrowseLinks,
  dhChangeIcon,
  dhFileOpen,
  dhSourceSet,
  dhIconFileOpen,
  dhBrowseControl
};

enum TOcDropAction {
  daDropNone = 0,
  daDropCopy = 1,
  daDropMove = 2,
  daDropLink = 4
};

enum TOcMouseAction {
  maEnter     = 1,
  maContained = 2,
  maExit      = 3
};

enum TOcScrollDir {
  sdScrollUp    = 1,
  sdScrollDown  = 2,
  sdScrollLeft  = 4,
  sdScrollRight = 8
};

enum TOcInvalidate {
  invData = 1,
  invView = 2
};

enum TOcAspect {
  asContent   = 1,
  asThumbnail = 2,
  asIcon      = 4,
  asDocPrint  = 8,
  asDefault   = 16,
  asMaintain  = 32,
  asClip      = 64
};

struct  TOcScaleInfo {
  owl::ulong  xN;      ///< numerator in x direction
  owl::ulong  xD;      ///< denominator in x direction
  owl::ulong  yN;      ///< numerator in y direction
  owl::ulong  yD;      ///< denominator in y direction
};

enum TOcDraw {
  drNone        = 0,
  drShadingOnly = 1,
  drShowObjects = 2
};

enum TOcPartName {
  pnLong  = 1,
  pnShort = 2,
  pnApp   = 3
};

enum TOcInitHow {
  ihLink,
  ihEmbed,
  ihMetafile,
  ihBitmap,
};

enum TOcInitWhere {
  iwFile,
  iwStorage,
  iwDataObject,
  iwNew,
  iwHandle,
  iwNewOcx
};

class _OCFCLASS TOcInitInfo {
  public:
    TOcInitInfo(IBContainer * container);
    TOcInitInfo(TOcInitHow how, TOcInitWhere where, IBContainer * container);

    owl::uint32 ReleaseDataObject() {
      return ((Where==iwDataObject) && Data) ? ((IUnknown*)Data)->Release() : 0;
    }

  private:
  public:
    TOcInitHow       How;
    TOcInitWhere     Where;
    IBContainer * Container;
    HICON            HIcon;
    IStorage *    Storage;
    union {
      LPCOLESTR    Path;      ///< where == iwFile
      IDataObject* Data;      ///< where == iwDataObject
      owl::uint32       CId;       ///< where == iwNew, iwNewOcx
      struct {                ///< where == iwHandle
        HANDLE       Data;
        owl::uint         DataFormat;
      } Handle;
    };
};

struct TOcMenuWidths {
  long   Width[6];
};

struct TOcMenuDescr {
  HMENU  HMenu;
  int    Width[6];
};

enum TOcMenuEnable {
  meEnablePaste           = 1,
  meEnablePasteLink       = 2,
  meEnableBrowseClipboard = 4,
  meEnableBrowseLinks     = 8,
};

enum TOcHelp {
  hlpExit,
  hlpEnter,
  hlpGet
};

enum TOcConvertAction {
  caConvertToClsid  = 1,
  caConvertToFormat = 2,
  caActivateAsClsid = 4,
  caAspectToIcon    = 8,
  caAspectToContent = 16
};

struct  TOcConvertInfo {
  TOcConvertAction Action;
  union {
    struct {
      CLSID Current;
      CLSID New;
    } Clsid;
    struct {
      owl::uint ClipFormat;
    } Format;
    struct {
      HICON hIcon;
    } SwitchToIcon;
  };
};

enum TOcLinkUpdate {
  luUpdateAuto   = 1,
  luUpdateManual = 3
};

struct TOcFormatInfo {
  owl::uint      Id;
  OLECHAR   Name[32];
  OLECHAR   ResultName[32];
  owl::ocrMedium Medium;              // BOleMedium
  BOOL      IsLinkable;
};

} // OCF namespace

#endif  // OCF_OLEOBJECT_H
