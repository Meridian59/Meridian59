//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of classes TLayoutMetrics & TLayoutWindow
//----------------------------------------------------------------------------

#if !defined(OWL_LAYOUTWI_H)
#define OWL_LAYOUTWI_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>
#include <owl/layoutco.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup Layout
/// @{
/// \class TLayoutMetrics
// ~~~~~ ~~~~~~~~~~~~~~
/// When specifying the layout metrics for a window, four layout constraints
/// are needed.
///
/// TLayoutMetrics contains the four layout constraints used to define the layout metrics for a
/// window. The following table lists the constraints you can use for the X, Y,
/// Height, and Width fields.
/// 
/// - \c \b  X	lmLeft, lmCenter, lmRight
/// - \c \b  Y	lmTop, lmCenter, lmBottom
/// - \c \b  Height	lmCenter, lmRight, lmWidth
/// - \c \b  Width	lmCenter, lmBottom, lmHeight
/// 
/// If the metrics for the child window are relative to the parent window, the
/// relation window pointer needs to be lmParent (not the actual parent window
/// pointer). For example,
/// \code
/// TWindow* child = new TWindow(this, "");
/// TLayoutMetrics metrics;
/// metrics.X.Set(lmCenter, lmSameAs, lm()t, lmCenter);
/// metrics.Y.Set(lmCenter, lmSameAs, lm()t, lmCenter);
/// SetChildLayoutMetrics(*child, metrics);
/// \endcode
/// The parent window pointer (this) should not be used as the relation window
/// pointer of the child window.
//
class _OWLCLASS TLayoutMetrics {
  public:
/// X contains the X (left, center, right) edge constraint of the window. 
    TEdgeConstraint         X;      ///< Horz1 can be lmLeft, lmCenter, lmRight
    
/// Y contains the Y (top, center, bottom) edge constraint of the window.
    TEdgeConstraint         Y;      ///< Vert1 can be lmTop, lmCenter, lmBottom
    
/// Contains the width size constraint, center edge constraint, or right edge
/// (lmRight) constraint of the window.
    TEdgeOrWidthConstraint  Width;  ///< Horz2 can be lmWidth, lmCenter, lmRight
    
/// Contains the height size constraint, center edge constraint, or bottom edge
/// constraint of the window.
    TEdgeOrHeightConstraint Height; ///< Vert2 can be lmHeight, lmCenter, lmBottom

    /// Defaults each co: RelWin=0, MyEdge=(1st from above), Relationship=AsIs
    //
    TLayoutMetrics();
  
    void SetMeasurementUnits(TMeasurementUnits units);
};

//
// Private structs used by TLayoutWindow
//
struct TChildMetrics;
struct TConstraint;
struct TVariable;

//
/// \class TLayoutWindow
// ~~~~~ ~~~~~~~~~~~~~
/// Derived from TWindow, TLayoutWindow provides functionality for defining the
/// layout metrics for a window. By using layout constraints, you can create windows
/// whose position and size are proportional to another window's dimensions, so that
/// one window constrains the size of the other window. Toolbars and status bars are
/// examples of constrained windows.
///
/// When specifying the layout metrics for a window, there are several options:
/// e.g. in the horizontal direction,
///
///  Two Edge Constraints in X and Width
/// - 1. left edge and right edge
/// - 2. center edge and right edge
/// - 3. left edge and center edge
///
///  Edge Constraint and Size constraint in X and Width
/// - 4. left edge and size
/// - 5. right edge and size
/// - 6. center edge and size
///
/// The same holds true in the vertical direction for Y and Height
///
/// It is also possible to specify "lmAsIs" in which case we use the windows
/// current value
///
/// Specifying "lmAbsolute" means that we will use whatever is in data member
/// "Value"
///
/// We just name the fields "X" and "Width" and "Y" and "Height",
/// although its okay to place a right or center edge constraint in the
/// "Width" field and its also okay to place a right edge constraint in
/// the "X" field (i.e. option #3)
///
/// However, it's NOT okay to place a width constraint in the "X" or
/// "Height" fields or a height constraint in the "Y" or "Width" fields.
//
class _OWLCLASS TLayoutWindow : virtual public TWindow {
  public:
    TLayoutWindow(TWindow* parent,
                  LPCTSTR  title = 0,
                  TModule* module = 0);
    TLayoutWindow(TWindow* parent, const tstring& title, TModule* = 0);
   ~TLayoutWindow();

    /// Causes the receiver to size/position its children according to the
    /// specified layout metrics
    ///
    /// If you change the layout metrics for a child window call Layout()
    /// to have the changes take effect
    ///
    // !BB
    // !BB Consider making Layout virtual. Beta users have requested so that
    // !BB they can enhance it: For example, one user wants Layout to use
    // !BB 'DeferWindowPos' API to minimize flicker and avoid dirty repaints
    // !BB in cases where the child windows overlap. Sounds like a fair
    // !BB request to me
    // !BB
    virtual void    Layout();

    void            SetChildLayoutMetrics(TWindow& child, TLayoutMetrics& metrics);
    bool            GetChildLayoutMetrics(TWindow& child, TLayoutMetrics& metrics);
    bool            RemoveChildLayoutMetrics(TWindow& child);

  protected:

    void Init(TWindow* parent, LPCTSTR title, TModule* module);

    void            EvSize(uint sizeType, const TSize& size);

    void            RemoveChild(TWindow* child);

  protected_data:
		/// Contains the size of the client area.
    TSize           ClientSize;

  private:
    enum TWhichConstraint {
      XConstraint,
      YConstraint,
      WidthConstraint,
      HeightConstraint
    };

    TChildMetrics*  ChildMetrics;
    TConstraint*    Constraints;
    TConstraint*    Plan;
    TVariable*      Variables;
    bool            PlanIsDirty;
    int             NumChildMetrics;
    int             FontHeight;

    TChildMetrics*  GetChildMetrics(TWindow& child);

    void            AddConstraint(TChildMetrics&     metrics,
                                  TLayoutConstraint* c,
                                  TWhichConstraint   whichContraint);
    void            BuildConstraints(TChildMetrics& childMetrics);
    void            RemoveConstraints(TChildMetrics& childMetrics);

    void            BuildPlan();
    void            ExecutePlan();
    void            ClearPlan();

    int             LayoutUnitsToPixels(int);
    void            GetFontHeight();

    // Hidden to prevent accidental copying or assignment
    //
    TLayoutWindow(const TLayoutWindow&);
    TLayoutWindow& operator =(const TLayoutWindow&);

  DECLARE_RESPONSE_TABLE(TLayoutWindow);
  //DECLARE_CASTABLE;
  //DECLARE_STREAMABLE(_OWLCLASS,TLayoutWindow,1); // Because VC++
  DECLARE_STREAMABLE_OWL(TLayoutWindow,1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TLayoutWindow );

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_LAYOUTWIN_H
