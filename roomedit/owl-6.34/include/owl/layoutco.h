//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TLayoutConstraint.
//----------------------------------------------------------------------------

#if !defined(OWL_LAYOUTCO_H)
#define OWL_LAYOUTCO_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TWindow;

/// \addtogroup Layout
/// @{
//
/// Use to represent the parent in layout metrics
//
#define lmParent    0

/// The TEdge enum describes the following constants that define the boundaries of a
/// window:
enum TEdge 
{
	lmLeft, 	///< The left edge of the window
	lmTop, 		///< The top edge of the window
	lmRight, 	///< The right edge of the window
	lmBottom, ///< The bottom edge of the window
	lmCenter	///< The center of the window. The object that owns the constraint, such as
						///< TLayoutMetrics, determines whether the center of the window is the vertical
						///< center or the horizontal center.
};

/// Used by the TLayoutConstraint struct, TWidthHeight enumerates the values that
/// control the width (lmWidth) and height (lmHeight) of the window.
enum TWidthHeight {lmWidth = lmCenter + 1, lmHeight};

/// Used by the TLayoutConstraint struct, TMeasurementUnits enumerates the
/// measurement units (lmPixels or lmLayoutUnits) that control the dimension of the
/// window. These can be either pixels or layout units that are obtained by dividing
/// the font height into eight vertical and eight horizontal segments.
enum TMeasurementUnits {lmPixels, lmLayoutUnits};

/// Used by the TLayoutConstraint struct, TRelationship specifies the relationship
/// between the edges and sizes of one window and the edges and sizes of another
/// window (which can be a parent or sibling). These relationships can be specified
/// as either the same value as the sibling or parent window (lmAsIs), an absolute
/// value (lmAbsolute), a percent of one of the windows (lmPercentOf), a value that
/// is either added above (lmAbove) or left (lmLeftOf) of one of the windows, or a
/// value that is subtracted from below (lmBelow) or right (lmRightOf) of one of the
/// windows.
enum TRelationship {
  lmAsIs      = 0,
  lmPercentOf = 1,
  lmAbove     = 2, lmLeftOf = lmAbove,
  lmBelow     = 3, lmRightOf = lmBelow,
  lmSameAs    = 4,
  lmAbsolute  = 5
};

//
/// \struct TLayoutConstraint
// ~~~~~~ ~~~~~~~~~~~~~~~~~
/// TLayoutConstraint is a structure that defines a relationship (a layout
/// constraint) between an edge or size of one window and an edge or size of one of
/// the window's siblings or its parent. If a parent-child relationship is
/// established between windows, the dimensions of the child windows are dependent
/// on the parent window. A window can have one of its sizes depend on the size of
/// the opposite dimension.
/// 
/// For example, the width can be twice the height. TLayoutMetrics lists the
/// relationships you can have among size and edge constraints
///
/// The following window is displayed by the sample program LAYOUT.CPP, which
/// demonstrates layout constraints:
/// \image html bm210.BMP
///
/// Layout constraints are specified as a relationship between an edge/size
/// of one window and an edge/size of one of the window's siblings or parent
///
/// It is acceptable for a window to have one of its sizes depend on the
/// size of the opposite dimension (e.g. width is twice height)
///
/// Distances can be specified in either pixels or layout units
///
/// A layout unit is defined by dividing the font "em" quad into 8 vertical
/// and 8 horizontal segments. we get the font by self-sending WM_GETFONT
/// (we use the system font if WM_GETFONT returns 0)
///
/// "lmAbove", "lmBelow", "lmLeftOf", and "lmRightOf" are only used with edge
/// constraints. They place the window 1 pixel to the indicated side (i.e.
/// adjacent to the other window) and then adjust it by "Margin" (e.g. above
/// window "A" by 6)
///
/// \note "Margin" is either added to ("lmAbove" and "lmLeftOf") or subtracted
///       from("lmBelow" and "lmRightOf") depending on the relationship
///
/// "lmSameAs" can be used with either edges or sizes, and it doesn't offset
/// by 1 pixel like the above four relationships did. it also uses "Value"
/// (e.g. same width as window "A" plus 10)
///
/// \note "Value" is always *added*. use a negative number if you want the
///       effect to be subtractive
//
struct TLayoutConstraint {
/// RelWin is a pointer to the sibling windows or lmParent if the child is a
/// proportion of the parent's dimensions. RelWin points to the window itself (this)
/// if a child window's dimension is a proportion of one of its other dimensions
/// (for example, its height is a proportion of its width).
  TWindow*           RelWin;            // relative window, lmParent for parent

/// MyEdge contains the name of the edge or size constraint (lmTop, lmBottom,
/// lmLeft, lmRight, lmCenter, lmWidth, or lmHeight) for your window.
  uint               MyEdge       : 4;  // edge or size (width/height)
  	
/// Relationship specifies the type of relationship that exists between the two
/// windows (that is, lmRightOf, lmLeftOf, lmAbove, lmBelow, lmSameAs, or
/// lmPercentOf). A value of lmAbsolute actually indicates that no relationship
/// exists.
  TRelationship      Relationship : 4;
  	
/// Units enumerates the units of measurement (either pixels or layout units) used
/// to measure the height and width of the windows. Unlike pixels, layout units are
/// based on system font size and will be consistent in their perceived size even if
/// the screen resolution changes.
  TMeasurementUnits  Units        : 4;
  	
/// OtherEdge contains the name of the edge or size constraint (lmTop, lmBottom,
/// lmLeft, lmRight, lmCenter, lmWidth, or lmHeight) for the other window.
  uint               OtherEdge    : 4;  // edge or size (width/height)

/// This union is included for the convenience of naming the layout constraints.
/// Margin is used for the ImAbove, ImLeftOf, ImLeftOf, or ImRightOf enumerated
/// values in TRelationship. Value is used for the ImSameAs or ImAbsolute enumerated
/// values in TRelationship. Percent is used for the ImPercentOf enumerated value in
/// TRelationship.
  union {
    int  Margin;   ///< used for "lmAbove", "lmBelow", "lmLeftOf", "lmRightOf"
    int  Value;    ///< used for "lmSameAs" and "lmAbsolute"
    int  Percent;  ///< used for "lmPercentOf"
  };
};

//
/// \struct TEdgeConstraint
// ~~~~~~ ~~~~~~~~~~~~~~~
/// TEdgeConstraint adds member functions that set edge (but not size) constraints.
/// TEdgeConstraint always places your window one pixel above the other window and
/// then adds margins.
/// 
/// For example, if the margin is 4, TEdgeConstraint places your window 5 pixels
/// above the other window. The margin, which does not need to be measured in
/// pixels, is defined using the units specified by the constraint. Therefore, if
/// the margin is specified as 8 layout units (which are then converted to 12
/// pixels), your window would be placed 13 pixels above the other window.
//
struct TEdgeConstraint : public TLayoutConstraint {

  /// For setting arbitrary edge constraints. use it like this:
  /// \code
  ///   metrics->X.Set(lmLeft, lmRightOf, lmParent, lmLeft, 10);
  /// \endcode
  void    Set(TEdge edge,      TRelationship rel, TWindow* otherWin,
              TEdge otherEdge, int value = 0);

  // These four member functions can be used to position your window with
  // respective to a sibling window. you specify the sibling window and an
  // optional margin between the two windows
  //
  void    LeftOf(TWindow* sibling, int margin = 0);
  void    RightOf(TWindow* sibling, int margin = 0);
  void    Above(TWindow* sibling, int margin = 0);
  void    Below(TWindow* sibling, int margin = 0);

  // These two work on the same edge, e.g. "SameAs(win, lmLeft)" means
  // that your left edge should be the same as the left edge of "otherWin"
  //
  void    SameAs(TWindow* otherWin, TEdge edge);
  void    PercentOf(TWindow* otherWin, TEdge edge, int percent);

  // Setting an edge to a fixed value
  //
  void    Absolute(TEdge edge, int value);

  // Letting an edge remain as-is
  //
  void    AsIs(TEdge edge);
};

//
/// \struct TEdgeOrSizeConstraint
// ~~~~~~ ~~~~~~~~~~~~~~~~~~~~~
struct TEdgeOrSizeConstraint : public TEdgeConstraint {

  // Redefine member functions defined by TEdgeConstraint that are hidden by
  // TEdgeOrSizeConstraint because of extra/different params
  //
  void    Absolute(TEdge edge, int value);
  void    PercentOf(TWindow* otherWin, TEdge edge, int percent);
  void    SameAs(TWindow* otherWin, TEdge edge);
  void    AsIs(TEdge edge);
  void    AsIs(TWidthHeight edge);
};

struct TEdgeOrWidthConstraint : public TEdgeOrSizeConstraint {

  // Redefine member functions defined by TEdgeOrSizeConstraint that are hidden by
  // TEdgeOrWidthConstraint because of extra/different params
  //
  void    Absolute(TEdge edge, int value);
  void    PercentOf(TWindow* otherWin, TEdge edge, int percent);
  void    SameAs(TWindow* otherWin, TEdge edge);

  /// Setting a width/height to a fixed value
  //
  void    Absolute(int value);

  /// Percent of another window's width/height (defaults to being the same
  /// dimension but could be the opposite dimension, e.g. make my width 50%
  /// of my parent's height)
  //
  void    PercentOf(TWindow*     otherWin,
                    int          percent,
                    TWidthHeight otherWidthHeight = lmWidth);

  /// Same as another window's width/height (defaults to being the same
  /// dimension but could be the opposite dimension, e.g. make my width
  /// the same as my height)
  //
  void    SameAs(TWindow*     otherWin,
                 TWidthHeight otherWidthHeight = lmWidth,
                 int          value = 0);

};

struct TEdgeOrHeightConstraint : public TEdgeOrSizeConstraint {

  // Redefine member functions defined by TEdgeOrSizeConstraint that are hidden by
  // TEdgeOrHeightConstraint because of extra/different params
  //
  void    Absolute(TEdge edge, int value);
  void    PercentOf(TWindow* otherWin, TEdge edge, int percent);
  void    SameAs(TWindow* otherWin, TEdge edge);

  /// Setting a width/height to a fixed value
  //
  void    Absolute(int value);

  /// Percent of another window's width/height (defaults to being the same
  /// dimension but could be the opposite dimension, e.g. make my width 50%
  /// of my parent's height)
  //
  void    PercentOf(TWindow*     otherWin,
                    int          percent,
                    TWidthHeight otherWidthHeight = lmHeight);

  /// Same as another window's width/height (defaults to being the same
  /// dimension but could be the opposite dimension, e.g. make my width
  /// the same as my height)
  //
  void    SameAs(TWindow*     otherWin,
                 TWidthHeight otherWidthHeight = lmHeight,
                 int          value = 0);

};

/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations

//
/// Used for setting arbitrary edge constraints, Set specifies that your window's
/// edge should be of a specified relationship to otherWin's specified edge.
inline void TEdgeConstraint::Set(TEdge edge, TRelationship rel, TWindow* otherWin,
                                 TEdge otherEdge, int value)
{
  RelWin = otherWin;
  MyEdge = edge;
  Relationship = rel;
  OtherEdge = otherEdge;
  Value = value;
}

//
/// Positions one window with respect to a sibling window. You can specify the
/// sibling window and an optional margin between the two windows.
inline void TEdgeConstraint::LeftOf(TWindow* sibling, int margin)
{
  Set(lmRight, lmLeftOf, sibling, lmLeft, margin);
}

//
/// Positions one window with respect to a sibling window. You can specify the
/// sibling window and an optional margin between the two windows.
inline void TEdgeConstraint::RightOf(TWindow* sibling, int margin)
{
  Set(lmLeft, lmRightOf, sibling, lmRight, margin);
}

//
/// Positions your window above a sibling window. You must specify the sibling
/// window and an optional margin between the two windows. If no margin is
/// specified, Above sets the bottom of one window one pixel above the top of the
/// other window.
inline void TEdgeConstraint::Above(TWindow* sibling, int margin)
{
  Set(lmBottom, lmAbove, sibling, lmTop, margin);
}

//
/// Positions your window with respect to a sibling window. You must specify the
/// sibling window and an optional margin between the two windows. If no margin is
/// specified, Below sets the top of one window one pixel below the bottom of the
/// other window.
inline void TEdgeConstraint::Below(TWindow* sibling, int margin)
{
  Set(lmTop, lmBelow, sibling, lmBottom, margin);
}

//
/// Sets the edge of your window indicated by edge equivalent to the corresponding
/// edge of the window in otherWin.
inline void TEdgeConstraint::SameAs(TWindow* otherWin, TEdge edge)
{
  Set(edge, lmSameAs, otherWin, edge, 0);
}

//
/// Specifies that the edge of one window indicated in edge should be a percentage
/// of the corresponding edge of another window  (otherWin.
inline void TEdgeConstraint::PercentOf(TWindow* otherWin, TEdge edge, int percent)
{
  Set(edge, lmPercentOf, otherWin, edge, percent);
}

//
/// Sets an edge of your window to a fixed value.
inline void TEdgeConstraint::Absolute(TEdge edge, int value)
{
  MyEdge = edge;
  Value = value;
  Relationship = lmAbsolute;
}

//
inline void TEdgeConstraint::AsIs(TEdge edge)
{
  MyEdge = edge;
  Relationship = lmAsIs;
}

//---

//
inline void TEdgeOrSizeConstraint::AsIs(TEdge edge)
{
  TEdgeConstraint::AsIs(edge);
}

//
inline void TEdgeOrSizeConstraint::AsIs(TWidthHeight edge)
{
  TEdgeConstraint::AsIs(TEdge(edge));
}

//---

//
inline void TEdgeOrWidthConstraint::Absolute(TEdge edge, int value)
{
  TEdgeConstraint::Absolute(edge, value);
}

//
inline void TEdgeOrWidthConstraint::PercentOf(TWindow* otherWin, TEdge edge, int percent)
{
  TEdgeConstraint::PercentOf(otherWin, edge, percent);
}

//
inline void TEdgeOrWidthConstraint::SameAs(TWindow* otherWin, TEdge edge)
{
  TEdgeConstraint::SameAs(otherWin, edge);
}

//
inline void TEdgeOrWidthConstraint::Absolute(int value)
{
  MyEdge = lmWidth;
  Value = value;
  Relationship = lmAbsolute;
}

//
inline void TEdgeOrWidthConstraint::PercentOf(TWindow* otherWin,
  int percent, TWidthHeight otherWidthHeight)
{
  RelWin = otherWin;
  MyEdge = lmWidth;
  Relationship = lmPercentOf;
  OtherEdge = otherWidthHeight;
  Percent = percent;
}

//
inline void TEdgeOrWidthConstraint::SameAs(TWindow* otherWin,
  TWidthHeight otherWidthHeight, int value)
{
  RelWin = otherWin;
  MyEdge = lmWidth;
  Relationship = lmSameAs;
  OtherEdge = otherWidthHeight;
  Value = value;
}

//---

//
inline void TEdgeOrHeightConstraint::Absolute(TEdge edge, int value)
{
  TEdgeConstraint::Absolute(edge, value);
}

//
inline void TEdgeOrHeightConstraint::PercentOf(TWindow* otherWin, TEdge edge, int percent)
{
  TEdgeConstraint::PercentOf(otherWin, edge, percent);
}

//
inline void TEdgeOrHeightConstraint::SameAs(TWindow* otherWin, TEdge edge)
{
  TEdgeConstraint::SameAs(otherWin, edge);
}

//
inline void TEdgeOrHeightConstraint::Absolute(int value)
{
  MyEdge = lmHeight;
  Value = value;
  Relationship = lmAbsolute;
}

//
inline void TEdgeOrHeightConstraint::PercentOf(TWindow* otherWin,
  int percent, TWidthHeight otherWidthHeight)
{
  RelWin = otherWin;
  MyEdge = lmHeight;
  Relationship = lmPercentOf;
  OtherEdge = otherWidthHeight;
  Percent = percent;
}

//
inline void TEdgeOrHeightConstraint::SameAs(TWindow* otherWin,
  TWidthHeight otherWidthHeight, int value)
{
  RelWin = otherWin;
  MyEdge = lmHeight;
  Relationship = lmSameAs;
  OtherEdge = otherWidthHeight;
  Value = value;
}



} // OWL namespace


#endif  // OWL_LAYOUTCO_H
