//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TMultiFrame: TFrameWindow that manages "owned" TFrameWindows
//-------------------------------------------------------------------
#ifndef __OWLEXT_MULTIWIN_H
#define __OWLEXT_MULTIWIN_H

#ifndef __OWLEXT_CORE_H
#  include <owlext/core.h>              // required for all OWLEXT headers
#endif

#include <owl/template.h> // or use STL -> better

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                   TMultiFrame
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class TMultiFrameDatumArray;
class OWLEXTCLASS TMultiFrame : public owl::TFrameWindow
{
  // Object lifetime methods
  //
public:
  TMultiFrame(owl::TWindow* parent, LPCTSTR title = 0, owl::TWindow* client = 0,
      bool shrinkToClient = false, owl::TModule* module = 0);
  TMultiFrame(HWND hWnd, owl::TModule* module = 0);
  virtual ~TMultiFrame();

  // OWL overrides
  //
public:
  virtual bool Create();
  virtual void Destroy(int retVal = 0);
  virtual bool CanClose();

  // Accessors
  //
public:
  unsigned int GetWindowCount();
  owl::TFrameWindow* operator[](unsigned int index);

  // Mutators
  //
public:
  virtual bool AddFrame(owl::TFrameWindow* frame, bool multiframeOwnsIt = true);
  virtual bool RemoveFrame(owl::TFrameWindow* frame);
  virtual bool RemoveAllFrames();

  // Internal data
  //
private:
  // Datum is a simple structure (with corresponding requisite methods) to help
  // contain TFrameWindow*'s, as well as a bool to indicate if TMultiFrame is
  // responsible for calling delete on the pointer at destruction. 
  //
  struct OWLEXTCLASS Datum  {
    owl::TFrameWindow* m_frame;
    bool m_ownsIt;

    Datum()
      : m_frame(0), m_ownsIt(false)
    { }
    Datum(owl::TFrameWindow* f, bool b)
      : m_frame(f), m_ownsIt(b)
    { }
    Datum(const Datum& d)
      : m_frame(d.m_frame), m_ownsIt(d.m_ownsIt)
    { }
    ~Datum()
    { }
    Datum& operator =(const Datum& d)
    { m_frame = d.m_frame; m_ownsIt = d.m_ownsIt; return *this; }

    bool operator==(const Datum& d)
    { return (m_frame==d.m_frame); }
  };
  owl::TObjectArray<Datum>*     m_frameArray;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace


#endif

/* If you don't want to use TMultiFrame, you can implement the same functionality
 * yourself by using code similar to the following:

 #include <owl/owlcore.h>

class TMyFrameWindow : public TFrameWindow
{
public:
  TMyFrameWindow()
    : TFrameWindow(0, "Main")
  {
    m_subMain1 = new TFrameWindow(0, "SubMain1");
    m_subMain2 = new TFrameWindow(0, "SubMain2");
  }
  ~TMyFrameWindow()
  {
    delete m_subMain1; delete m_subMain2;
  }

protected:
  virtual void SetupWindow()
  {
    m_subMain1->Create(); m_subMain1->Show(SW_SHOW);
    m_subMain2->Create(); m_subMain2->Show(SW_SHOW);
  }
  virtual void CleanupWindow()
  {
    m_subMain1->Destroy(); m_subMain2->Destroy();
  }

private:
  TFrameWindow* m_subMain1;  TFrameWindow* m_subMain2;
};

class TMyApplication : public TApplication
{
public:
  TMyApplication()
    : TApplication("Multiple Main Window Test")
  { }

protected:
  virtual void InitMainWindow()
  { SetMainWindow(new TMyFrameWindow); }
};

int OwlMain(int, char**)
{ return TMyApplication().Run(); }

 * The advantage, of course, to using TMultiFrame, is that you need not manage
 * all of this code yourself.
 */

