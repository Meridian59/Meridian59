//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// TDirDialog v. 1.0 - (C) Copyright 1996 by Kent Reisdorph, All Rights Reserved
// Copyright (c) 1998 by Yura Bidus. All Rights Reserved.
//
// Original code by Kent Reisdorph, 75522,1174
//
//  OVERVIEW
//  ~~~~~~~~
// Header for class TDirDialog - A dialog class that allows
// directory selection similar to Win95's Explorer
//
//----------------------------------------------------------------------------
#if !defined(OWLEXT_DIRDIALG_H) // Sentry, use file only if it's not already included.
#define OWLEXT_DIRDIALG_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
# include <owlext/core.h>
#endif

#include <owl/dialog.h>

namespace owl {class _OWLCLASS TTreeViewCtrl;};
namespace owl {class _OWLCLASS TImageList;};
namespace owl {class _OWLCLASS TStatic;};
namespace owl {class _OWLCLASS TTvNotify;};
namespace owl {class _OWLCLASS TTreeNode;};

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

class OWLEXTCLASS TDirDialog : public owl::TDialog {
  public:
    class OWLEXTCLASS TData {
      public:
          TData();
          ~TData();

        LPTSTR DirPath;
    };

    TDirDialog(owl::TWindow* parent, TData& data, owl::TModule* module = 0);
    ~TDirDialog();

    void SetupWindow() override;

    void CmOk();

    void TvnSelChanged(owl::TTvNotify &);
    void TvnItemExpanded(owl::TTvNotify &);
    void BuildPath(owl::tstring&,owl::TTreeNode&);
    void LoadTree();
    void AddNode(LPCTSTR, const owl::TTreeNode&, int);

  protected:
    owl::TTreeViewCtrl*   DirTree;
    owl::TImageList*   Images;
    owl::TStatic*       CurrentDir;
    TData&               Data;
    int                 level;

  DECLARE_RESPONSE_TABLE(TDirDialog);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif     // OWLEXT_DIRDIALG_H sentry.

