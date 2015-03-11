//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TGauge, a gauge control encapsulation & implementation.
//----------------------------------------------------------------------------

#if !defined(OWL_GAUGE_H)
#define OWL_GAUGE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>
#include <owl/color.h>
#include <owl/commctrl.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
/// \class TGauge
// ~~~~~ ~~~~~~
/// A streamable class derived from TControl, TGauge defines the basic behavior of
/// gauge controls. Gauges are display-only horizontal or vertical controls that
/// provide duration or analog information about a particular process. A typical use
/// of a gauge occurs in installation programs where a control provides a graphical
/// display indicating the percentage of files copied. In general, horizontal gauges
/// with a broken (dashed-line) bar are used to display short-duration, process
/// information, and horizontal gauges with a solid bar are used to illustrate
/// long-duration, process information. Usually, vertical gauges are preferred for
/// displaying analog information.
//
class _OWLCLASS TGauge : public TControl {
  public:
    TGauge(TWindow*        parent,
           LPCTSTR         title,
           int             id,
           int x, int y, int w, int h = 0,
           bool            isHorizontal = true,
           int             margin = 1,
           TModule*        module = 0);

    TGauge(
      TWindow* parent,
      const tstring& title,
      int id,
      int x, int y, int w, int h = 0,
      bool isHorizontal = true,
      int margin = 1,
      TModule* module = 0);

    TGauge(TWindow*        parent,
           int             id,
           int x, int y, int w, int h = 0,
           TModule*        module = 0);

    TGauge(TWindow*        parent,
           int             resId,
           TModule*        module = 0);

    // Getting & setting gauge properties
    //
    void          GetRange(int& minValue, int& maxValue) const;
    int           GetStep() const;
    int           GetValue() const;

    void          SetRange(int minValue, int maxValue);
    void          SetStep(int step);
    void          SetValue(int value);  // !CQ SetPos/GetPos alias?
    void          DeltaValue(int delta);
    void          StepIt();
    void operator ++(int);

    // Set whether the control is horizontal. This method is useful
    // for an object coming from a dialog resource. [For objects created
    // by OWL, use the constructor which accepts a boolean 'isHorizontal'
    // parameter.
    //
    void          SetHorizontal(bool horizontal);

    // Set the LED style & sizing as well as the indicator color
    //
    void          SetLed(int spacing, int thickPercent = 90);
    void          SetColor(const TColor& color);
    void          SetBkgndColor(const TColor& color); // new

  protected:

    void Init(bool isHorizontal, int margin);

    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();
    void          Paint(TDC& dc, bool erase, TRect& rect);
    void          SetupWindow();

    // Self sent by method Paint(). override this is if you want to
    // implement a border style that isn't supported
    //
    virtual void  PaintBorder(TDC& dc);

    // Message response functions
    //
    bool          EvEraseBkgnd(HDC);

  protected_data:
    int           Min;        ///< Holds the minimum value (in gauge units) displayed on the gauge.
    int           Max;        ///< Holds the maximum value (in gauge units) displayed on the gauge.
    int           Value;      ///< Holds the current value of the gauge.
    int           Step;       ///< Holds the step factor to be used by StepIt operations.
    int           Margin;     ///< Contains the border width and height of the gauge.
    
/// Set to the isHorizontal argument of the constructor. IsHorizontal is true if the
/// gauge is horizontal and false if it is vertical.
    int           IsHorizontal;
    
/// Holds the integer value (in gauge units) of the spacing between the broken bars
/// of the gauge. Note that TGauge does not paint the title while using LED spacing.
    int           LedSpacing; 
    
    int           LedThick;   ///< Holds the thickness of the broken bar in percent of spacing
    
    TColor        BarColor;   ///< Holds the bar or LED color, which defaults to blue.
    
  private:
    // Hidden to prevent accidental copying or assignment
    //
    TGauge(const TGauge&);
    TGauge& operator=(const TGauge&);

  DECLARE_RESPONSE_TABLE(TGauge);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations

//
/// This inline implementation gets the minimum and maximum values for the gauge.
//
inline void TGauge::GetRange(int& minValue, int& maxValue) const
{
  minValue = Min; maxValue = Max;
}

//
/// Returns the step factor.
//
inline int TGauge::GetStep() const
{
  return Step;
}

//
/// Returns the current value of the gauge.
//
inline int TGauge::GetValue() const
{
  return Value;
}

//
/// Another way of stepping (calls StepIt)
//
inline void TGauge::operator ++(int)
{
  StepIt();
}

/// Specifies whether the control is a horizontal (or vertical) gauge control
//
inline void TGauge::SetHorizontal(bool isHorizontal)
{
  IsHorizontal = isHorizontal;
}

} // OWL namespace


#endif  // OWL_GAUGE_H
