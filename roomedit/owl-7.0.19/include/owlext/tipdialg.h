//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
//  Copyright (c) 1998 by Yura Bidus. All Rights Reserved.
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TTipDialog (TDialog).
//
//------------------------------------------------------------------------------
#if !defined(OWLEXT_TIPDIALG_H) // Sentry, use file only if it's not already included.
#define OWLEXT_TIPDIALG_H


#include <owl/filename.h>

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

class OWLEXTCLASS TTipDialog : public owl::TDialog {
  public:
    TTipDialog(owl::TWindow *parent, LPCTSTR regkey, owl::TModule* module = 0);

    virtual ~TTipDialog();
    bool ShowOnStartup();

  protected:
    bool         ShowDlg;
    bool         TipError;
    owl::tstring  CurrentTip;
    owl::tstring  RegKey;
    owl::tifstream     TipFile;
    owl::TFileName    FileName;
    owl::TBitmap*     BulbBitmap;

  public:
    void SetupWindow() override;
    void CleanupWindow() override;

  protected:
    virtual void GetNextTipString();
    virtual void OpenTipFile(long pos);
    virtual long CloseTipFile();
    //Get User data from registry or from file, can be overriden
    // to get data from else place
    virtual long ReadSettings();
    //Write User data to registry or to file, can be overriden
    // to get data from else place
    virtual void WriteSettings(long pos);
    virtual owl::tstring GetTipName();
    virtual owl::TBitmap* GetBitmap();

  protected:
    void EvPaint ();
    void CmNextTip();
    HBRUSH EvCtlColor (HDC hDC, HWND hWndChild, owl::uint ctlType);

  DECLARE_RESPONSE_TABLE(TTipDialog);
  DECLARE_STREAMABLE(OWLEXTCLASS, TTipDialog, 1);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif     // OWLEXT_TIPDIALG_H sentry.
