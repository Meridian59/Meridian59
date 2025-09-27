//
/// \file progressbar.h
/// Encapsulation for Windows ProgressBar common control
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 2010-2011 Jogy
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#if !defined(OWL_PROGRESSBAR_H)
#define OWL_PROGRESSBAR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/control.h>
#include <owl/color.h>
#include <owl/commctrl.h>
#include <utility>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
/// \class TProgressBar
// ~~~~~ ~~~~~~
//
class _OWLCLASS TProgressBar : public TControl {
  public:
    TProgressBar(TWindow*        parent,
           int             id,
           int x, int y, int w, int h = 0,
           TModule*        module = 0);

    TProgressBar(TWindow*        parent,
           int             resId,
           TModule*        module = 0);

    // Getting & setting gauge properties
    //
    void          GetRange(int& minValue, int& maxValue) const;
    auto GetRange() const -> std::pair<int, int> {return {Min, Max};}
    int           GetStep() const;
    int           GetValue() const;
    auto GetState() const -> uint;

    void          SetRange(int minValue, int maxValue);
    void SetRange(const std::pair<int, int>& r) {SetRange(r.first, r.second);}
    void          SetStep(int step);
    void          SetValue(int value);  // !CQ SetPos/GetPos alias?
    void          DeltaValue(int delta);
    void          StepIt();
    void operator ++(int);
    auto SetState(uint) -> uint;
    void          SetMarquee(bool enable, uint timeBetweenUpdates = 0);
    
    // Set the LED style & sizing as well as the indicator color
    // Ignored by CommonControl impl.
    //
    // NOTE: Invoking these methods may *NOT* be effective if OWL
    //       uses the Common Control support and instead of
    //       emulating the control.
    //
    void          SetColor(const TColor& color);
    void          SetBkgndColor(const TColor& color); // new


  protected:

    // Override TWindow virtual member functions
    //
    virtual auto GetWindowClassName() -> TWindowClassName;
    void          SetupWindow();


  protected_data:
    int           Min;        ///< Holds the minimum value (in gauge units) displayed on the gauge.
    int           Max;        ///< Holds the maximum value (in gauge units) displayed on the gauge.
    int           Value;      ///< Holds the current value of the gauge.
    int           Step;       ///< Holds the step factor to be used by StepIt operations.
    int           Margin;     ///< Contains the border width and height of the gauge.
    bool          Marquee;    ///< Enable marquee mode (display indeterminate progress bar).
    uint          MarqueeUpdateTime; ///< Time, in milliseconds, between marquee animation updates. If this parameter is zero, the marquee animation is updated every 30 milliseconds.

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TProgressBar(const TProgressBar&);
    TProgressBar& operator=(const TProgressBar&);
};

/// @}

#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations

//
/// This inline implementation gets the minimum and maximum values for the gauge.
//
inline void TProgressBar::GetRange(int& minValue, int& maxValue) const
{
  minValue = Min; maxValue = Max;
}

//
/// Returns the step factor.
//
inline int TProgressBar::GetStep() const
{
  return Step;
}

//
/// Returns the current value of the gauge.
//
inline int TProgressBar::GetValue() const
{
  return Value;
  //  return CONST_CAST(TProgressBar*, this)->SendMessage(PBM_GETPOS);
}

//
/// Returns the state of the progress bar.
/// \note Wrapper for PBM_GETSTATE.
/// \see https://msdn.microsoft.com/en-us/library/windows/desktop/bb760834.aspx
//
inline auto TProgressBar::GetState() const -> uint
{
  return static_cast<uint>(SendMessage(PBM_GETSTATE));
}

//
/// Another way of stepping (calls StepIt)
//
inline void TProgressBar::operator ++(int)
{
  StepIt();
}

} // OWL namespace


#endif  // OWL_PROGRESSBAR_H
