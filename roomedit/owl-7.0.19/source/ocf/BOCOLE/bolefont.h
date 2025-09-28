//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
// Implements the Bolero versoin of the OLE2 font object.
//----------------------------------------------------------------------------
#ifndef _BOLEFONT_H
#define _BOLEFONT_H

#ifndef _BOLECOMP_H
#include "bolecomp.h"
#endif

#define SETFONTSIZE(size, n)                { size.s.Lo = n##0000; size.s.Hi = 0; }
#define XFERFONTSIZE(sizeout, sizein)       { sizeout.s.Lo = sizein.s.Lo; sizeout.s.Hi = sizein.s.Hi; }
#define ISEQUALFONTSIZE(sizeout, sizein)    ((sizeout.s.Hi == sizein.s.Hi) && (sizein.s.Hi == sizein.s.Lo))


class _ICLASS BOleFontObj;
class _ICLASS BEventList;

enum TFontStyle { fsBold=0x01, fsItalic=0x02, fsUnderline=0x04,
                  fsStrikeThrough=0x08 };

class _ICLASS BOleFont : public BOleComponent,   public IFont,
                         public IFontDisp,
                         public IConnectionPointContainer
{
protected:
  // next 5 members equivalent to FONTDESC struct
  BSTR       bstrName;
  CY         height;
  short      weight;
  short      charSet;
  int        style;

  long       cyLogical;
  long       cyHimetric;
  BOleFontObj *pF;
  BOOL       modified;
  ITypeLib   *pTL;
  ITypeInfo  *pTI;
  BEventList *pEventList; // ConnectionPoints list

public:
   BOleFont (BOleClassManager *pF, IBUnknownMain * pO);
   ~BOleFont ();

  // IUnknown Methods
  //
  DEFINE_IUNKNOWN(pObjOuter);
  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IFont methods
  //
  // property support
  HRESULT _IFUNC get_Name (BSTR FAR* pname);
  HRESULT _IFUNC put_Name (BSTR name);
  HRESULT _IFUNC get_Size (CY FAR* psize);
  HRESULT _IFUNC put_Size (CY size);
  HRESULT _IFUNC get_Bold (BOOL FAR* pbold);
  HRESULT _IFUNC put_Bold (BOOL bold);
  HRESULT _IFUNC get_Italic (BOOL FAR* pitalic);
  HRESULT _IFUNC put_Italic (BOOL italic);
  HRESULT _IFUNC get_Underline (BOOL FAR* punderline);
  HRESULT _IFUNC put_Underline (BOOL underline);
  HRESULT _IFUNC get_Strikethrough (BOOL FAR* pstrikethrough);
  HRESULT _IFUNC put_Strikethrough (BOOL strikethrough);
  HRESULT _IFUNC get_Weight (short FAR* pweight);
  HRESULT _IFUNC put_Weight (short weight);
  HRESULT _IFUNC get_Charset (short FAR* pcharset);
  HRESULT _IFUNC put_Charset (short charset);

  // main functionality
  //
  HRESULT _IFUNC get_hFont (HFONT FAR* phfont);
  HRESULT _IFUNC Clone (IFont FAR* FAR* lplpfont);
  HRESULT _IFUNC IsEqual (IFont FAR * lpFontOther);
  HRESULT _IFUNC SetRatio (long cyLogical, long cyHimetric);
  HRESULT _IFUNC QueryTextMetrics (LPTEXTMETRICOLE lptm);
  HRESULT _IFUNC AddRefHfont (HFONT hfont);
  HRESULT _IFUNC ReleaseHfont (HFONT hfont);
  HRESULT _IFUNC SetHdc (HDC hdc) { return ResultFromScode (E_NOTIMPL); }


  // IDispatch methods
  //
  HRESULT _IFUNC GetTypeInfoCount (UINT FAR* pctinfo);
  HRESULT _IFUNC GetTypeInfo (UINT itinfo, LCID lcid,
     ITypeInfo FAR* FAR* pptinfo);
  HRESULT _IFUNC GetIDsOfNames (REFIID riid, LPOLESTR FAR* rgszNames,
     UINT cNames, LCID lcid, DISPID FAR* rgdispid);
  HRESULT _IFUNC Invoke (DISPID dispidMember, REFIID riid, LCID lcid,
     WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,
     EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr);

  // IConnectionPointContainer methods
  //
  HRESULT _IFUNC EnumConnectionPoints (LPENUMCONNECTIONPOINTS FAR*);
  HRESULT _IFUNC FindConnectionPoint (REFIID, LPCONNECTIONPOINT FAR*);

  // Other accessors
  //
  long getCyHimetric ()           { return cyHimetric; };
  long getCyLogical ()            { return cyLogical; };
  int  getStyle ()                { return style; };
  BSTR getName ()                 { return bstrName; };

protected:
  BOleFontObj *FindFontObj ();
  BOleFontObj *FindFontObj (HFONT hFont);
  BOOL         IsEqual (BOleFontObj FAR * pObj);
  HRESULT      PropChanged (DISPID dispid);
  HRESULT      PropRequestEdit (DISPID dispid);
};


class _ICLASS BOleFontObj : public BOleComponent
{
protected:
  HFONT     hFont;
  CY        height;
  short     weight;
  short     charSet;
  int       style;
  BSTR      bstrName;
  long      cyLogical;
  long      cyHimetric;
  BOleFontObj *pNext;
  BOleFontObj *pPrev;
  int       nRefs;

public:
   BOleFontObj (BOleClassManager *pF, IBUnknownMain * pO);
   ~BOleFontObj ();

  BOOL SingleRef ()     { return (nRefs <= 1); };

  // IUnknown Methods
  //
  DEFINE_IUNKNOWN(pObjOuter);
  ULONG _IFUNC AddRefMain();
  ULONG _IFUNC ReleaseMain();

  HRESULT CreateHFont (BOleFont *pBF);

  void   SetPrev (BOleFontObj *pF)    { pPrev = pF; };
  void   SetNext (BOleFontObj *pF)    { pNext = pF; };
  BOleFontObj *GetPrev ()             { return pPrev; };
  BOleFontObj *GetNext ()             { return pNext; };
  friend BOleFont;  // access to object members
};


#endif
