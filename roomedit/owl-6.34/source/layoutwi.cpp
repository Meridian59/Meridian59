//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TLayoutWindow.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/layoutwi.h>
#include <owl/uimetric.h>
#include <owl/fixedpnt.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlLayout, 1, 0);  // diag. group for windows
//----------------------------------------------------------------------------

// Small wrapper around the 'DeferWindowPos' APIs
//
class TDeferWinPos {
  public:
    TDeferWinPos(int numWindows);
   ~TDeferWinPos();
    bool  DeferWindowPos(HWND hwnd, HWND after, int x, int y, int cx, int cy,
                         uint flags);
    bool  EndDeferWindowPos();
  protected:
    HDWP  Hdwp;
};

//} // OWL namespace

//
//
//
TDeferWinPos::TDeferWinPos(int numWindows)
{
  Hdwp = numWindows ? ::BeginDeferWindowPos(numWindows) : 0;
}

//
//
//
bool
TDeferWinPos::DeferWindowPos(HWND hwnd, HWND after, int x, int y,
                             int cx, int cy, uint flags)
{
  PRECONDITION(Hdwp);
  TRACEX(OwlLayout, 0, "DeferWindowPos( x=> " << x << ", y => " << y << ", cx => " << cx << ", cy => " << cy);
  Hdwp = ::DeferWindowPos(Hdwp, hwnd, after, x, y, cx, cy, flags);
  CHECK(Hdwp);
  return Hdwp != 0;
}

//
//
//
bool
TDeferWinPos::EndDeferWindowPos()
{
  PRECONDITION(Hdwp);
  if (::EndDeferWindowPos(Hdwp)) {
    Hdwp = 0;
    return true;
  }
  return false;
}

//
//
//
TDeferWinPos::~TDeferWinPos()
{
  if (Hdwp)
    EndDeferWindowPos();
}


DEFINE_RESPONSE_TABLE1(TLayoutWindow, TWindow)
  EV_WM_SIZE,
END_RESPONSE_TABLE;

//namespace owl {

/// \addtogroup internal
/// @{



/// \class TConstraint
/// Constraints can have up to three input variables
///
/// the method for solving the constraint is represented as an ordered linear
/// combination of the inputs and the constant with the constant expressed last
//
struct TVariable;
struct TConstraint {
  TVariable*    Inputs[3];
  TVariable*    Output;
  TFixedPoint   OrderedCombination[4];
  TConstraint*  Next;

  TConstraint();

  bool  IsResolved();  // iff its inputs have been resolved
  int   Evaluate();
  int   NumActualInputs();
};

inline tostream& operator <<(tostream& os, const TFixedPoint& p){
  return os <<  (int)(TFixedPoint&)p;
}
inline tostream& operator <<(tostream& os, const TLayoutConstraint& c){
  return os <<  _T('(') << hex << (long)c.RelWin << dec
  << _T(',') << c.MyEdge << _T(',') << (int)c.Relationship << _T(',') << (int)c.Units
  << _T(',') << c.OtherEdge << _T(',') << c.Value <<  _T(')');
}
inline tostream& operator <<(tostream& os, const TLayoutMetrics& m){
  return os <<  _T('(') << m.X << _T(',') << m.Y << _T('x') << m.Width << 
                _T(',') << m.Height <<  _T(')');
}
inline tostream& operator <<(tostream& os, const TConstraint& c){
  return os << _T('(') << hex << (long)c.Inputs
            << _T(',') << (long)c.Output << dec << _T(',') << c.OrderedCombination[0]
            << _T(',') << c.OrderedCombination[1] << _T(',') << c.OrderedCombination[2]
            << c.OrderedCombination[3] << _T(',') <<  hex << (long)c.Next << dec <<  _T(')');
}

struct TVariable {
  int           Value;
  TConstraint*  DeterminedBy;  // 0 if variable is constant
  bool          Resolved;

  TVariable() {Value = 0; DeterminedBy = 0; Resolved = false;}
};
inline tostream& operator <<(tostream& os, const TVariable& v){
  return os << _T('(') << v.Value << _T(',') << hex << (long)v.DeterminedBy 
            << dec << _T("->") << (v.Resolved? _T("Resolved"):_T("No Resolved")) << _T(')');
}
//
/// The layout metrics represent four equations. For equations that are
/// "absolute" or "as is" we don't add a constraint and just set the variable
/// value directly(and mark the variable as constant); otherwise we produce an
/// ordered linear combination from the equation and add a constraint
//
struct TChildMetrics {
  public:
    bool            GeneratedConstraints;
    TWindow*        Child;
    TLayoutMetrics  Metrics;
    TVariable       Variables[4];  // x => 0, y => 1, right => 2, bottom => 3
    TChildMetrics*  Next;

    TChildMetrics(TWindow& child, TLayoutMetrics& metrics);
};
inline tostream& operator <<(tostream& os, const TChildMetrics& m){
  return os << _T('(') << (m.GeneratedConstraints ? _T("true") : _T("false"))
  << _T(',') << hex << (long)m.Child << dec << _T(',') <<
  m.Metrics << _T(',') << m.Variables[0] << _T(',') << m.Variables[1] << _T(',')
  << m.Variables[2] << _T(',') << m.Variables[3] << _T(',') << hex << (long)m.Next << dec
  << _T(')');
}

/// @}

//} // OWL namespace

//----------------------------------------------------------------------------


/// \class TLayoutWindow
/// The following examples show how to set up various metrics using edge
/// constraints. For purposes of illustration, these examples use a parent-child
/// relationship, but you can also use a child-to-child (sibling) relationship. Keep
/// in mind that moving the parent's origin (the left and top edges) also moves the
/// child window.
/// Example 1
/// To create windows that can grow, set the top and left edges of the child
/// window's boundaries in a fixed relationship to the top and left edges of the
/// parent's window. In this example, if you expand the bottom and right edges of
/// the parent, the child's bottom and right edges grow the same amount. Both the X
/// and Y constraints are 10 units from the parent window's edges. Both the Width
/// and Height constraints are 40 layout units from the parent window's edges.
/// Specifically, Width (lmWidth) is 40 units to the left of the parent's right edge
/// (lmLeftOf = lmSameAs + offset or sameas - 40).
/// \image html bm211.BMP
/// Use the following layout constraints:
/// \code
/// layoutmetrics.X.Set(lmLeft, lmRightOf, lmParent, lmLeft, 10);
/// layoutmetrics.Y.Set(lmTop, lmBelow, lmParent, lmTop, 10);
/// layoutmetrics.Width.Set(lmRight, lmLeftOf, lmParent, lmRight, 40);
/// layoutmetrics.Height.Set(lmBottom, lmAbove, lmParent, lmBottom, 40);
/// SetChildLayoutMetrics(*MyChildWindow, layoutMetrics);
/// \endcode
/// Example 2
/// To create fixed-size and fixed-position windows, set the child's right edge a
/// fixed distance from the parent's left edge, and the child's bottom edge a fixed
/// distance from the parent's top edge. In this example, both the X and Y edge
/// constraints are set to 10 and both the Width and Height edge constraints are set
/// to 100.
/// \image html bm212.BMP
/// Use the following layout constraints:
/// \code
/// layoutmetrics.X.Set(lmLeft, lmRightOf, lmParent, lmLeft, 10);
/// layoutmetrics.Y.Set(lmTop, lmBelow, lmParent, lmTop, 10);
/// layoutmetrics.Width.Absolute(100);
/// layoutmetrics.Height.Absolute(100);
/// SetChildLayoutMetrics(*MyChildWindow, layoutMetrics);
/// \endcode
/// Example 3
/// To create a fixed-size window that remains a constant distance from the parent's
/// right corner, set the child's top and bottom edges a fixed distance (lmLayout
/// unit or pixels) from the parent window's bottom. Also, set the child's left and
/// right edges a fixed distance from the parent's right edge. In this example, both
/// the Width and the Height edge constraints are set to 100 and the X and Y edge
/// constraints are set to 10. In this case, the child window, which stays the same
/// size, moves with the lower right corner of the parent.
/// \image html bm213.BMP
/// Use the following layout constraints:
/// \code
/// layoutmetrics.X.Set(lmRight, lmLeftOf, lmParent, lmRight, 10);
/// layoutmetrics.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, 10);
/// layoutmetrics.Width.Absolute(100);
/// layoutmetrics.Height.Absolute(100);
/// SetChildLayoutMetrics(*MyChildWindow, layoutMetrics);
/// \endcode
/// Example 4
/// To create a window in which the child's edges are a percentage of the parent's
/// window, set the child's edges a percentage of the distance from the parent's
/// edges. Specifically, the child's top and bottom edges are a percentage of the
/// parent's bottom edge. The child's left and right edges are a percentage of the
/// parent's right edge.
/// If you resize the parent window, the child window will change size and origin
/// (that is, the top and left edges will also change).
/// \image html bm214.BMP
/// Use the following layout constraints:
/// \code
/// layoutmetrics.X.Set(lmLeft, lmPercentOf, lmParent, lmRight, 33);
/// layoutmetrics.Y.Set(lmTop, lmPercentOf, lmParent, lmBottom, 33);
/// layoutmetrics.Width.Set(lmRight, lmPercentOf, lmParent, lmRight, 66);
/// layoutmetrics.Height.Set(lmBottom, lmPercentOf, lmParent, lmBottom, 66);
/// SetChildLayoutMetrics(*MyChildWindow, layoutMetrics);
/// \endcode
/// 


//
/// Creates a TLayoutWindow object with specified parent, window caption, and
/// library ID.
//
TLayoutWindow::TLayoutWindow(TWindow*  parent,
                             LPCTSTR   title,
                             TModule* module)
  : TWindow(parent, title, module)
{
  Init(parent, title, module);
}

//
/// String-aware overload
//
TLayoutWindow::TLayoutWindow(TWindow* parent, const tstring& title, TModule* module)
  : TWindow(parent, title, module)
{
  Init(parent, title.c_str(), module);
}

void TLayoutWindow::Init(TWindow* parent, LPCTSTR title, TModule* module)
{
  // Initialize virtual bases, in case the derived-most used default ctor
  //
  TWindow::Init(parent, title, module);

  NumChildMetrics = 0;
  ChildMetrics = 0;
  Constraints = 0;
  Plan = 0;
  PlanIsDirty = false;
  ClientSize.cx = ClientSize.cy = 0;

  // Allocate variables for the parent's left, top, right, and bottom and
  // mark them as resolved
  //
  Variables = new TVariable[4];
  Variables[0].Resolved = true;
  Variables[1].Resolved = true;
  Variables[2].Resolved = true;
  Variables[3].Resolved = true;
}

//
/// Deletes variables and frees the child metrics and constraints.
//
TLayoutWindow::~TLayoutWindow()
{
  delete[] Variables;

  // Free the child metrics
  //
  for (TChildMetrics* childMetrics = ChildMetrics; childMetrics;) {
    TChildMetrics*  tmp = childMetrics;
    childMetrics = childMetrics->Next;
    delete tmp;
  }

	ChildMetrics = 0;  //Bug fix: https://sourceforge.net/forum/message.php?msg_id=4186646
	
	
  // Free the constraints
  //
  ClearPlan();
  for (TConstraint* c = Constraints; c;) {
    TConstraint*  tmp = c;
    c = c->Next;
    delete tmp;
  }
  
  Constraints = 0;  //Bug fix: https://sourceforge.net/forum/message.php?msg_id=4186646
}

static bool hasBorder(TWindow* win)
{
  // We consider it to have a border unless it is a pop-up or child window
  // without WS_BORDER set
  //
  if ((win->GetWindowAttr().Style & (WS_CHILD|WS_POPUP)) && !(win->GetWindowAttr().Style & WS_BORDER))
    return false;

  else
    return true;
}

//
/// Causes the window to resize and position its children according to the specified
/// metrics. You can call Layout to implement changes that occur in the layout
/// metrics.
//
void
TLayoutWindow::Layout()
{
  if (ChildMetrics) {
    TChildMetrics*  childMetrics;

    GetFontHeight();

    // Initialize the parent's variables
    //
    Variables[2].Value = ClientSize.cx - 1;
    Variables[3].Value = ClientSize.cy - 1;
    TRACEX(OwlLayout, 0, _T("Layout() ClientSize: ") << ClientSize);

    if (hasBorder(this)) {
      int  cxBorder = TUIMetric::CxBorder;
      int  cyBorder = TUIMetric::CyBorder;

      Variables[0].Value = -cxBorder;
      Variables[1].Value = -cyBorder;
      Variables[2].Value += cxBorder;
      Variables[3].Value += cyBorder;
    }
    else {
      Variables[0].Value = 0;
      Variables[1].Value = 0;
    }
    TRACEX(OwlLayout, 1, _T("Layout() Variables: 0=>") << Variables[0] << _T(", 1=>" << Variables[1]<< ", 2=>" << Variables[2]<< ", 3=>") << Variables[3]);

    // Rebuild layout plan if necessary
    //
    if (PlanIsDirty) {
      PlanIsDirty = false;

      for (childMetrics = ChildMetrics; childMetrics;
           childMetrics = childMetrics->Next)
        BuildConstraints(*childMetrics);
      BuildPlan();
    }

    // Use the plan to calculate actual child window position values
    //
    ExecutePlan();

    // Find out how many windows we're dealing with
    //
    int numWindows = 0;
    for (childMetrics = ChildMetrics; childMetrics; childMetrics = childMetrics->Next) {
      TWindow*    win = childMetrics->Child;
      if (win->GetHandle())
        numWindows++;
    }

#if !defined(OWL_NO_DEFERWINDOWPOS_LAYOUT)
    // Helper object to use 'DefWindowPos' API
    //
    TDeferWinPos dwp(numWindows);
#endif

    // Do actual resizing
    //
    for (childMetrics = ChildMetrics; childMetrics; childMetrics = childMetrics->Next) {
      TWindow*    win = childMetrics->Child;
      TVariable*  variables = childMetrics->Variables;
      TRACEX(OwlLayout, 0, _T("Layout() variables: ") << variables[0] << _T(',') << 
             variables[1] << _T(',') << variables[2] << _T(',') << variables[3]);

      if (win->GetHandle()) {

#if defined(OWL_NO_DEFERWINDOWPOS_LAYOUT)
        win->SetWindowPos(
          0,
          variables[0].Value,
          variables[1].Value,
          variables[2].Value - variables[0].Value + 1,
          variables[3].Value - variables[1].Value + 1,
          SWP_NOACTIVATE | SWP_NOZORDER
        );
#else
        dwp.DeferWindowPos(*win, 0, 
                           variables[0].Value,
                           variables[1].Value,
                           variables[2].Value - variables[0].Value + 1,
                           variables[3].Value - variables[1].Value + 1,
                           SWP_NOACTIVATE | SWP_NOZORDER);
#endif
      }
      else {
        win->GetWindowAttr().X = variables[0].Value;
        win->GetWindowAttr().Y = variables[1].Value;
        win->GetWindowAttr().W = variables[2].Value - variables[0].Value + 1;
        win->GetWindowAttr().H = variables[3].Value - variables[1].Value + 1;
      }
    }
  }
}
//
/// Sets the metrics for the window and removes any existing ones. Set the metrics
/// as shown:
/// \code
/// layoutMetrics->X.Absolute(lmLeft, 10);
/// layoutMetrics->Y.Absolute(lmTop, 10);
/// layoutMetrics->Width.Set(lmWidth, lmRightOf, GetClientWindow(), lmWidth, -40);
/// layoutMetrics->Height.Set(lmHeight, lmRightOf, GetClientWindow(), lmHeight,
/// -40);
/// \endcode
/// Then call SetChildLayoutMetrics to associate them with the position of the child
/// window:
/// \code
/// SetChildLayoutMetrics(* MyChildWindow, * layoutMetrics);
/// \endcode
//
void
TLayoutWindow::SetChildLayoutMetrics(TWindow& child, TLayoutMetrics& metrics)
{
  PRECONDITION(&child);
  PRECONDITION(&metrics);

  PlanIsDirty = true;

  if (ChildMetrics) {

    // See if we already have metrics for the child
    //
    for (TChildMetrics* childMetrics = ChildMetrics; childMetrics;
         childMetrics = childMetrics->Next) {
      if (childMetrics->Child == &child) {
        childMetrics->Child = &child;
        childMetrics->Metrics = metrics;

        // Get rid of the old constraints
        //
        RemoveConstraints(*childMetrics);
        return;
      }
    }
  }

  TChildMetrics* childMetrics = new TChildMetrics(child, metrics);
  childMetrics->Next = ChildMetrics;
  ChildMetrics = childMetrics;
  NumChildMetrics++;
}

//
/// Gets the layout metrics of the child window.
//
bool
TLayoutWindow::GetChildLayoutMetrics(TWindow& child, TLayoutMetrics& metrics)
{
  PRECONDITION(&child);

  TChildMetrics* childMetrics = GetChildMetrics(child);
  if (childMetrics) {
    metrics = childMetrics->Metrics;
    return true;
  }
  return false;
}

//
/// Removes child (layout) metrics for a given child (if found) and updates
/// other children as necessary
//
bool
TLayoutWindow::RemoveChildLayoutMetrics(TWindow& child)
{
  PRECONDITION(&child);

  for (TChildMetrics** childMetrics = &ChildMetrics; *childMetrics;
       childMetrics = &(*childMetrics)->Next) {
    if ((*childMetrics)->Child == &child) {

      // Unlink target metrics from list & clean up a bit
      //
      TChildMetrics* tmp = *childMetrics;
      *childMetrics = tmp->Next;
      RemoveConstraints(*tmp);
      NumChildMetrics--;

      // Update other child metrics now that removed metric is gone
      // Check for case where new relWin is lmParent and adjust other edge
      // to be what removed window was using. If an 'edge' is really a size,
      // then give up & just leave it asis. If the removed window had an edge
      // that was really a size, then use the other constraint in that
      // dimension (X or Y)
      //
      for (TChildMetrics* cm = ChildMetrics; cm; cm = cm->Next) {
        if (cm->Metrics.X.RelWin == &child) {
          RemoveConstraints(*cm);
          cm->Metrics.X.RelWin = tmp->Metrics.X.RelWin;
          if (cm->Metrics.X.RelWin == lmParent)
            cm->Metrics.X.OtherEdge = tmp->Metrics.X.OtherEdge;
        }
        if (cm->Metrics.Y.RelWin == &child) {
          RemoveConstraints(*cm);
          cm->Metrics.Y.RelWin = tmp->Metrics.Y.RelWin;
          if (cm->Metrics.Y.RelWin == lmParent)
            cm->Metrics.Y.OtherEdge = tmp->Metrics.Y.OtherEdge;
        }
        if (cm->Metrics.Width.RelWin == &child) {
          RemoveConstraints(*cm);
          if (cm->Metrics.Width.MyEdge == lmWidth)
            cm->Metrics.Width.Relationship = lmAsIs;
          else {
            if (tmp->Metrics.Width.MyEdge == lmWidth) {
              cm->Metrics.Width.RelWin = tmp->Metrics.X.RelWin;
              if (cm->Metrics.Width.RelWin == lmParent)
                cm->Metrics.Width.OtherEdge = tmp->Metrics.X.OtherEdge;
            }
            else {
              cm->Metrics.Width.RelWin = tmp->Metrics.Width.RelWin;
              if (cm->Metrics.Width.RelWin == lmParent)
                cm->Metrics.Width.OtherEdge = tmp->Metrics.Width.OtherEdge;
            }
          }
        }
        if (cm->Metrics.Height.RelWin == &child) {
          RemoveConstraints(*cm);
          if (cm->Metrics.Height.MyEdge == lmHeight)
            cm->Metrics.Height.Relationship = lmAsIs;
          else {
            if (tmp->Metrics.Height.MyEdge == lmHeight) {
              cm->Metrics.Height.RelWin = tmp->Metrics.Y.RelWin;
              if (cm->Metrics.Height.RelWin == lmParent)
                cm->Metrics.Height.OtherEdge = tmp->Metrics.Y.OtherEdge;
            }
            else {
              cm->Metrics.Height.RelWin = tmp->Metrics.Height.RelWin;
              if (cm->Metrics.Height.RelWin == lmParent)
                cm->Metrics.Height.OtherEdge = tmp->Metrics.Height.OtherEdge;
            }
          }
        }
      }

      // Finaly, delete target metrics
      //
      delete tmp;
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------

//
/// Responds to a change in size by calling Layout() to resize the window.
//
void
TLayoutWindow::EvSize(uint sizeType, const TSize& size)
{
  TWindow::EvSize(sizeType, size);

  if (sizeType != SIZE_MINIMIZED &&
      //sizeType != SIZE_MAXIMIZED && // fix-bug was bug or not ????
      size != ClientSize) {
    ClientSize = size;
    Layout();
  }
}

//
/// Overrides TWindow virtual to allow cleanup of child metrics.
//
void
TLayoutWindow::RemoveChild(TWindow* child)
{
  TWindow::RemoveChild(child);
  RemoveChildLayoutMetrics(*child);
}

//----------------------------------------------------------------------------

//
//
//
TChildMetrics*
TLayoutWindow::GetChildMetrics(TWindow& child)
{
  PRECONDITION(&child);

  for (TChildMetrics* childMetrics = ChildMetrics; childMetrics;
       childMetrics = childMetrics->Next)
    if (childMetrics->Child == &child)
      return childMetrics;

  return 0;
}

//
//
//
void
TLayoutWindow::ExecutePlan()
{
  for (TConstraint* c = Plan; c; c = c->Next){
    c->Output->Value = c->Evaluate();
    TRACEX(OwlLayout, 1, _T("ExecutePlan() c->Output => ") << *c->Output);
  }
}

//
//
//
void
TLayoutWindow::ClearPlan()
{
  if (Plan) {
    // Move all constraints that were in the plan back to the list of
    // constraints
    //
    if (!Constraints)
      Constraints = Plan;

    else {
      TConstraint* c;
      for (c = Constraints; c->Next; c = c->Next)
        ;

      c->Next = Plan;
    }

    Plan = 0;
  }
}

//
//
//
void
TLayoutWindow::BuildPlan()
{
  TChildMetrics*  childMetrics;
  TConstraint*    lastInPlan = 0;

  ClearPlan();

  // Mark all variables that aren't determined by a constraint as resolved
  //
  for (childMetrics = ChildMetrics; childMetrics; childMetrics = childMetrics->Next) {
    TVariable*  variable = childMetrics->Variables;
    variable->Resolved = variable->DeterminedBy ? false : true;
    variable++;
    variable->Resolved = variable->DeterminedBy ? false : true;
    variable++;
    variable->Resolved = variable->DeterminedBy ? false : true;
    variable++;
    variable->Resolved = variable->DeterminedBy ? false : true;
    TRACEX(OwlLayout, 1, _T("BuildPlan() variables: ") << childMetrics->Variables[0]
      << _T(',') << childMetrics->Variables[1] << _T(',') << childMetrics->Variables[2]
      << _T(',') << childMetrics->Variables[3]);
  }

  // Uses local propagation as much as possible (because it's fast)
  //
  // If cycles exist then we will end up with constraints that haven't been
  // added to the plan. we convert the remaining constraints into simultaneous
  // linear equations which we solve using Gaussian elimination
  //
  // Look for constraints that have all their input variables resolved and
  // append them to the plan
  //
  for (bool foundOne = true; foundOne;) {
    TConstraint* c = Constraints;
    TConstraint* previous = 0;

    foundOne = false;

    while (c) {
      TRACEX(OwlLayout, 1, _T("BuildPlan() *c ") << *c);
      if (c->IsResolved()) {
        TConstraint*  resolved = c;

        c->Output->Resolved = true;
        foundOne = true;

        // Extract the constraint from the list of constraints
        //
        if (previous)
          previous->Next = c->Next;

        else
          Constraints = c->Next;

        c = c->Next;

        // Append the constraint to the plan
        //
        if (lastInPlan)
          lastInPlan->Next = resolved;

        else
          Plan = resolved;

        lastInPlan = resolved;
      }
      else {
        previous = c;
        c = c->Next;
      }
    }
  }

  // Gaussian elimination not currently supported--give up
  //
  if (Constraints)
    TXWindow::Raise(this, IDS_LAYOUTINCOMPLETE);
}

//
//
//
static int
findInput(TConstraint* simplify, TVariable* input)
{
  for (int i = 0; i < 3; i++)
    if (simplify->Inputs[i] == input)
      return i;

  return -1;
}

//
// simplify constraint "simplify" by substituting constraint "_using"
//
// we do this when the two constraints are defined in terms of each other
//   1. the output of "simplify" is an input of "_using"
//   2. the output of "_using" is an input of "simplify"
//
// we do this to avoid a layout cycle
//
// "output" is the output variable for constraint "_using"
//
static
void
Simplify(TConstraint* simplify, TVariable* output, TConstraint* _using)
{
  if (!simplify)
    return;

  int  outputOfSimplify = findInput(_using, simplify->Output);  // check #1
  int  target = findInput(simplify, output);                    // check #2

  if (outputOfSimplify != -1 && target != -1) {
    int  commonInputs[3];
    int  numInputsOfUsing = _using->NumActualInputs();
    int  i;

    // Count how many inputs are common between "simplify" and "_using"
    //
    for (i = 0; i < numInputsOfUsing; i++)
      commonInputs[i] = findInput(simplify, _using->Inputs[i]);

    // Since constraints only have room for 3 inputs we can not simplify if the
    // total number of the existing inputs minus the input we are going to back
    // substitute for plus the number of inputs added by "_using" (i.e. inputs
    // not common between the two constraints) exceeds 3
    //
    int  numInputsOfSimplify = simplify->NumActualInputs() - 1;
    int  newInputs = 0;

    // Compute the number of additional inputs contributed by "_using"
    //
    for (i = 0; i < numInputsOfUsing; i++)
      if (commonInputs[i] == -1 && i != outputOfSimplify)
        newInputs++;

    if (numInputsOfSimplify + newInputs > 3)
      return;

    TFixedPoint  m = simplify->OrderedCombination[target];

    // Adjust the constant part
    //
    simplify->OrderedCombination[3] += m * _using->OrderedCombination[3];

    // Merge the common inputs
    //
    for (i = 0; i < numInputsOfUsing; i++)
      if (commonInputs[i] != -1)
        simplify->OrderedCombination[commonInputs[i]] +=
          m * _using->OrderedCombination[i];

    simplify->Inputs[target] = 0;  // input has been back substituted out
    TRACEX(OwlLayout, 1, _T("Simplify() simplify->Inputs[") << target << _T("]=> 0"));

    // If necessary shift the inputs following "output" (and their associated
    // mutiplier) left by one...
    //
    for (i = target + 1; i < 3; i++)
      if (simplify->Inputs[i]) {
        simplify->Inputs[i - 1] = simplify->Inputs[i];
        simplify->Inputs[i] = 0;
        TRACEX(OwlLayout, 1, _T("Simplify() simplify->Inputs[") << i << _T(" - 1]=> ") << 
               *simplify->Inputs[i - 1] << _T(", simplify->Inputs[") << i << _T("]=> 0"));
        simplify->OrderedCombination[i - 1] = simplify->OrderedCombination[i];
      }

    // Add the new inputs
    //
    for (i = 0; i < numInputsOfUsing; i++)
      if (commonInputs[i] == -1 && i != outputOfSimplify) {
        simplify->Inputs[numInputsOfSimplify] = _using->Inputs[i];
        TRACEX(OwlLayout, 1, _T("Simplify() simplify->Inputs[") << numInputsOfSimplify 
          << _T("]=> ") << *simplify->Inputs[numInputsOfSimplify]);
        simplify->OrderedCombination[numInputsOfSimplify] =
          m * _using->OrderedCombination[i];
        numInputsOfSimplify++;
      }

    // Now scale things back so that the output of "simplify" is 1
    //
    TFixedPoint  f = 1 - m;

    simplify->OrderedCombination[3] /= f;
    for (i = 0; i < numInputsOfSimplify; i++)
      simplify->OrderedCombination[i] /= f;
  }
}

//
//
//
void
TLayoutWindow::AddConstraint(TChildMetrics&     metrics,
                             TLayoutConstraint* c,
                             TWhichConstraint   whichConstraint)
{
  int           index;
  TVariable*    output;
  TConstraint*  result = new TConstraint;

  // Set the output variable
  //
  if (whichConstraint == XConstraint && metrics.Metrics.X.MyEdge == lmRight){
    output = &metrics.Variables[2];
    TRACEX(OwlLayout, 1, _T("AddConstraint() output =>") << *output);
  }

  else if (whichConstraint == YConstraint && metrics.Metrics.Y.MyEdge == lmBottom){
    output = &metrics.Variables[3];
    TRACEX(OwlLayout, 1, _T("AddConstraint() output =>") << *output);
  }

  else{
    output = &metrics.Variables[whichConstraint];
    TRACEX(OwlLayout, 1, _T("AddConstraint() output =>") << *output);
  }

  // Set the inputs based on the edge
  //
  if (((int)c->Relationship != lmAbsolute) && ((int)c->Relationship != lmAsIs)) {
    TVariable*  variables;

    if (c->RelWin == lmParent){
      variables = Variables;
      TRACEX(OwlLayout, 1, _T("AddConstraint() variables =>") << *variables);
    }

    else {
      TChildMetrics*  relWinMetrics = GetChildMetrics(*c->RelWin);
      if (!relWinMetrics) {
        delete result;
        TXWindow::Raise(this, IDS_LAYOUTBADRELWIN);
      }
      variables = relWinMetrics->Variables;
      TRACEX(OwlLayout, 1, _T("AddConstraint() variables =>") << *variables);
    }

    switch (c->OtherEdge) {
      case lmLeft:
      case lmTop:
      case lmRight:
      case lmBottom:
        result->Inputs[0] = &variables[c->OtherEdge];
        TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[0]=> ") << 
               *result->Inputs[0]);
        break;

      case lmWidth:
      case lmHeight:
        // width => right - left + 1
        // height => bottom - top + 1
        //
        result->Inputs[0] = &variables[c->OtherEdge - lmWidth+lmRight];
        result->Inputs[1] = &variables[c->OtherEdge - lmWidth+lmLeft];
        TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[0]=> ") << 
               *result->Inputs[0] << _T(", result->Inputs[1]=> ") << *result->Inputs[1]);
        result->OrderedCombination[1] = -1;
        result->OrderedCombination[3] = 1;
        break;

      case lmCenter:
        switch (whichConstraint) {
          case XConstraint:
          case WidthConstraint:
            // Center => (left + right) / 2
            //
            result->Inputs[0] = &variables[0];
            result->Inputs[1] = &variables[2];
            TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[0]=> ") << 
                   *result->Inputs[0] << _T(", result->Inputs[1]=> ") << *result->Inputs[1]);
            break;

          case YConstraint:
          case HeightConstraint:
            // Center => (top + bottom) / 2
            //
            result->Inputs[0] = &variables[1];
            result->Inputs[1] = &variables[3];
            TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[0]=> ") << 
                  *result->Inputs[0] << _T(", result->Inputs[1]=> ") << *result->Inputs[1]);
            break;
        }
        result->OrderedCombination[0] = result->OrderedCombination[1] = TFixedPoint(1,2);
        break;
    }
  }

  // Now store the constant term as the last of the ordered linear combination
  //
  // We must do this after setting the inputs
  //
  // NOTE: we cannot assume that the constant part is 0, because it might have
  //       been set above
  //
  switch (c->Relationship) {
    case lmAsIs:
      result->OrderedCombination[3] += whichConstraint == WidthConstraint ?
                                                          metrics.Child->GetWindowAttr().W :
                                                          metrics.Child->GetWindowAttr().H;
      break;

    case lmAbsolute:
    case lmSameAs:
    case lmBelow:
    case lmAbove: {
      int  value = c->Units == lmPixels ? c->Value : LayoutUnitsToPixels(c->Value);

      if (c->Relationship == lmAbove)
        value = -value - 1;

      else if (c->Relationship == lmBelow)
        value++;

      result->OrderedCombination[3] += value;
      break;
    }

    case lmPercentOf:
      TFixedPoint  percent = c->Percent;

      percent /= 100;
      result->OrderedCombination[0] *= percent;
      result->OrderedCombination[3] *= percent;

      switch (c->OtherEdge) {
        case lmWidth:
        case lmHeight:
        case lmCenter:
          result->OrderedCombination[1] *= percent;
          break;
      }
      break;
  }

  // Now handle cases where the left hand side is width, height, or center
  //
  // This must be done last...
  //
  if (result->Inputs[0])
    index = result->Inputs[1] ? 2 : 1;

  else
    index = 0;

  switch (c->MyEdge) {
    case lmWidth:
      if (whichConstraint == XConstraint || metrics.Metrics.X.MyEdge == lmRight) {
        // Rewrite "right - left + 1 = " as "left = right - (...) + 1"
        //
        for (int i = 0; i < index; i++)
          result->OrderedCombination[i] = -result->OrderedCombination[i];

        result->OrderedCombination[3] = -result->OrderedCombination[3];
        (result->OrderedCombination[3])++;
        result->Inputs[index] = &(metrics.Variables[2]);
        TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[") << 
               index << _T("]=> ") << *result->Inputs[index]);

        if (whichConstraint == WidthConstraint){
          output = &(metrics.Variables[XConstraint]);
          TRACEX(OwlLayout, 1, _T("AddConstraint() output=> ") << *output);
        }
      }
      else {
        // Rewrite "right - left + 1 = " as "right = left + ... - 1"
        //
        result->Inputs[index] = &(metrics.Variables[0]);
        TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[") << 
               index << _T("]=> ") << *result->Inputs[index]);
        (result->OrderedCombination[3])--;

        Simplify(metrics.Variables[0].DeterminedBy, output, result);
      }
      break;

    case lmHeight:
      if (whichConstraint == YConstraint || metrics.Metrics.Y.MyEdge == lmBottom) {
        // Rewrite "bottom - top + 1 = " as "top = bottom - (...) + 1"
        //
        for (int i = 0; i < index; i++)
          result->OrderedCombination[i] = -(result->OrderedCombination[i]);

        result->OrderedCombination[3] = -(result->OrderedCombination[3]);
        (result->OrderedCombination[3])++;
        result->Inputs[index] = &(metrics.Variables[3]);
        TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[") << 
               index << _T("]=> ") << *result->Inputs[index]);

        if (whichConstraint == HeightConstraint){
          output = &metrics.Variables[YConstraint];
          TRACEX(OwlLayout, 1, _T("AddConstraint() lmHeight: output =>") << *output);
        }
      }
      else {
        // Rewrite "bottom - top + 1 = " as "bottom = top + ... - 1"
        //
        result->Inputs[index] = &metrics.Variables[1];
        TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[") << 
              index << _T("]=> ") << *result->Inputs[index]);
        result->OrderedCombination[3]--;
        TRACEX(OwlLayout, 1, _T("AddConstraint() lmHeight: result->Inputs[") << 
               index << _T("] =>") << *result->Inputs[index]);

        Simplify(metrics.Variables[1].DeterminedBy, output, result);
      }
      break;

    case lmCenter:
      TVariable*  input = &metrics.Variables[0];  // left

      switch (whichConstraint) {
        case XConstraint:
          // Rewrite "(left + right) / 2 = " as "left = -right + 2 * (...)"
          //
          input += 2;  // right
          TRACEX(OwlLayout, 1, _T("AddConstraint() XConstraint: input =>") << *input);
          break;

        case YConstraint:
          // Rewrite "(top + bottom) / 2 = " as "top = -bottom + 2 * (...)"
          //
          input += 3;  // bottom
          TRACEX(OwlLayout, 1, _T("AddConstraint() YConstraint: input =>") << *input);
          break;

        case WidthConstraint:
          // Rewrite "(left + right) / 2 = " as "right = -left + 2 * (...)" or
          // "left = -right + 2 * (...)" depending on whether the "x" constraint
          // is left or right
          //
          if (metrics.Metrics.X.MyEdge == lmRight) {
            input += 2;  // right
            output = &metrics.Variables[XConstraint];
            TRACEX(OwlLayout, 1, _T("AddConstraint() WidthConstraint: input =>") << 
                   *input << _T(",output => ") << *output);
          }
          break;

        case HeightConstraint:
          // Rewrite "(top + bottom) / 2 = " as "bottom = -top + 2 * (...)" or
          // "top = -bottom + 2 * (...)" depending on whether the "y" constraint
          // is top or bottom
          //
          if (metrics.Metrics.Y.MyEdge != lmBottom)
            input++;  // top

          else {
            input += 3;  // bottom
            output = &metrics.Variables[XConstraint];
          }
          TRACEX(OwlLayout, 1, _T("AddConstraint() HeightConstraint: input =>") << 
                 *input << _T(",output => ") << *output);
          break;
      }
      result->Inputs[index] = input;
      TRACEX(OwlLayout, 1, _T("AddConstraint() result->Inputs[") << index << 
             _T("]=> ") << *result->Inputs[index]);
      for (int i = 0; i < index; i++)
        result->OrderedCombination[i] <<= 1;

      result->OrderedCombination[3] <<= 1;
      result->OrderedCombination[index] = -1;
      break;
  }

  // Now set the constraint output
  //
  output->DeterminedBy = result;
  result->Output = output;

  // Add the constraint to the list of constraints
  //
  result->Next = Constraints;
  Constraints = result;
}

//
//
//
void
TLayoutWindow::RemoveConstraints(TChildMetrics& childMetrics)
{
  TVariable*  variable = childMetrics.Variables;

  PlanIsDirty = true;
  ClearPlan();
  childMetrics.GeneratedConstraints = false;

  for (int i = 0; i < 4; i++) {
    TConstraint*  constraint = variable->DeterminedBy;

    variable->Value = 0;

    if (constraint) {

      // Remove the constraint from the list of constraints
      //
      if (Constraints == constraint)
        Constraints = constraint->Next;

      else
        for (TConstraint*  c = Constraints; c->Next; c = c->Next)
          if (c->Next == constraint) {
            c->Next = constraint->Next;
            break;
          }

      delete constraint;
      variable->DeterminedBy = 0;
    }

    variable++;
  }
}

//
//
//
void
TLayoutWindow::BuildConstraints(TChildMetrics& childMetrics)
{
  // NOTE: to get uniformity we consider the window edges to sit on pixels
  //       and not between pixels. so our idea of right is left + width - 1
  //       and not left + width
  //
  if (!childMetrics.GeneratedConstraints) {
    TLayoutConstraint*  c = &childMetrics.Metrics.X;
    TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Metrics.X =>") << *c);

    childMetrics.GeneratedConstraints = true;

    // "x" can be one of: left, right, center
    //
    if (c->Relationship == lmAsIs){
      if (c->MyEdge == lmLeft){
        childMetrics.Variables[0].Value = childMetrics.Child->GetWindowAttr().X;
        TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Variables[0] =>") << 
               childMetrics.Variables[0]);
      }
      else{
        childMetrics.Variables[2].Value = childMetrics.Child->GetWindowAttr().X +
                                          childMetrics.Child->GetWindowAttr().W - 1;
        TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Variables[2] =>") << 
               childMetrics.Variables[2]);
      }
    }
    else if (c->Relationship == lmAbsolute && c->MyEdge != lmCenter) {
      int  value = c->Units == lmPixels ? c->Value : LayoutUnitsToPixels(c->Value);

      childMetrics.Variables[c->MyEdge == lmLeft ? 0 : 2].Value = value;
      TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Variables[") << 
             (c->MyEdge == lmLeft ? 0 : 2)<< _T("] =>") << childMetrics.Variables[c->MyEdge == lmLeft ? 0 : 2]);
    }
    else {
      AddConstraint(childMetrics, c, XConstraint);
      TRACEX(OwlLayout, 1, _T("BuildConstraints() after Add constraints childMetrics.Metrics.X =>") << *c);
    }

    // "y" can be one of: top, bottom, center
    //
    c = &childMetrics.Metrics.Y;
    TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Metrics.Y =>") << *c);

    if (c->Relationship == lmAsIs){
      if (c->MyEdge == lmTop){
        childMetrics.Variables[1].Value = childMetrics.Child->GetWindowAttr().Y;
        TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Variables[1] =>") << 
               childMetrics.Variables[1]);
      }
      else{
        childMetrics.Variables[3].Value = childMetrics.Child->GetWindowAttr().Y +
                                          childMetrics.Child->GetWindowAttr().H - 1;
        TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Variables[3] =>") << 
               childMetrics.Variables[3]);
       }
    }
    else if (c->Relationship == lmAbsolute && c->MyEdge != lmCenter) {
      int  value = c->Units == lmPixels ? c->Value : LayoutUnitsToPixels(c->Value);

      childMetrics.Variables[c->MyEdge == lmTop ? 1 : 3].Value = value;
      TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Variables[") << 
              (c->MyEdge == lmTop ? 1 : 3) << _T("] =>") << 
              childMetrics.Variables[c->MyEdge == lmTop ? 1 : 3]);
    }
    else {
      AddConstraint(childMetrics, c, YConstraint);
      TRACEX(OwlLayout, 1, _T("BuildConstraints() after AddConstraint childMetrics.Metrics.Y =>") << *c);
    }

    // "width" can be one of: width, right, center
    //
    c = &childMetrics.Metrics.Width;
    TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Metrics.Width =>") << *c);

    if (c->MyEdge == lmRight && (c->Relationship == lmAsIs || c->Relationship == lmAbsolute)){
      childMetrics.Variables[2].Value = c->Relationship == lmAsIs ?
                                        childMetrics.Child->GetWindowAttr().X +
                                        childMetrics.Child->GetWindowAttr().W - 1 :
                                        c->Units == lmPixels ? c->Value : LayoutUnitsToPixels(c->Value);
    TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Variables[2] =>") << 
             childMetrics.Variables[2]);
    }
    else{
      AddConstraint(childMetrics, c, WidthConstraint);
       TRACEX(OwlLayout, 1, _T("BuildConstraints() after AddConstraint childMetrics.Metrics.Width =>") << *c);
     }

    // "height" can be one of: height, bottom, center
    //
    c = &childMetrics.Metrics.Height;
     TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Metrics.Height =>") << *c);

    if (c->MyEdge == lmBottom && (c->Relationship == lmAsIs || c->Relationship == lmAbsolute)){
      childMetrics.Variables[3].Value = c->Relationship == lmAsIs ?
                                        childMetrics.Child->GetWindowAttr().Y +
                                        childMetrics.Child->GetWindowAttr().H - 1 :
                                        c->Units == lmPixels ? c->Value : LayoutUnitsToPixels(c->Value);
       TRACEX(OwlLayout, 1, _T("BuildConstraints() childMetrics.Variables[3] =>") << childMetrics.Variables[3]);
    }
    else{
      AddConstraint(childMetrics, c, HeightConstraint);
       TRACEX(OwlLayout, 1, _T("BuildConstraints() after AddConstraint childMetrics.Metrics.Height =>") << *c);
     }
  }
}

//
//
//
int
TLayoutWindow::LayoutUnitsToPixels(int value)
{
  const long  UnitsPerEM = 8;

  return int((long(value) * FontHeight + UnitsPerEM / 2) / UnitsPerEM);
}

//
//
//
void
TLayoutWindow::GetFontHeight()
{
  HFONT hFont = 0;
  if (GetHandle())
    hFont = HFONT(HandleMessage(WM_GETFONT));

  // NOTE: It's fairly customary to return NULL to the WM_GETFONT
  //       request - specially when the window is using the system
  //       font - Hence, we'll default to the system font too...
  //
  if (!hFont)
    hFont = HFONT(GetStockObject(SYSTEM_FONT));

  CHECK(hFont);
  TFont font(hFont);
  FontHeight = font.GetHeight();
}


IMPLEMENT_STREAMABLE1(TLayoutWindow, TWindow);

#if !defined(BI_NO_OBJ_STREAMING)

//
// Read the object from the persistent stream.
//
void*
TLayoutWindow::Streamer::Read(ipstream&, uint32) const
{
  return GetObject();
}

//
// Write the object into a persistent stream.
//
void
TLayoutWindow::Streamer::Write(opstream&) const
{
}
#endif  // if !defined(BI_NO_OBJ_STREAMING)


//----------------------------------------------------------------------------

//
//
//
TChildMetrics::TChildMetrics(TWindow&        child,
                             TLayoutMetrics& metrics)
:
  Child(&child),
  Metrics(metrics)
{
  GeneratedConstraints = false;
  Next = 0;
}

//
//
//
TConstraint::TConstraint()
{
  Inputs[0] = Inputs[1] = Inputs[2] = 0;
  OrderedCombination[0] = OrderedCombination[1] = OrderedCombination[2] = 1;

  // NOTE: OrderedCombination[3] was initialized to 0 by the TFixedPoint ctor
  //
  Output = 0;
}

//
//
//
bool
TConstraint::IsResolved()
{
  return (!Inputs[0] || Inputs[0]->Resolved) &&
         (!Inputs[1] || Inputs[1]->Resolved) &&
         (!Inputs[2] || Inputs[2]->Resolved);
}

//
//
//
int
TConstraint::Evaluate()
{
  TFixedPoint value = OrderedCombination[3];  // initialize to constant part
  TRACEX(OwlLayout, 1, _T("Evaluate() value ") << value);

  if (Inputs[0]) {
    value += OrderedCombination[0] * Inputs[0]->Value;
    TRACEX(OwlLayout, 1, _T("Evaluate() value ") << value << 
           _T(", OrderedCombination[0] => ") << OrderedCombination[0] << 
           _T(", Inputs[0]->Value => ") << Inputs[0]->Value);
  }

  if (Inputs[1]) {
    value += OrderedCombination[1] * Inputs[1]->Value;
    TRACEX(OwlLayout, 1, _T("Evaluate() value ") << value << 
      _T(", OrderedCombination[1] => ") << OrderedCombination[1] << 
      _T(", Inputs[1]->Value => ") << Inputs[1]->Value);
  }

  if (Inputs[2]) {
    value += OrderedCombination[2] * Inputs[2]->Value;
    TRACEX(OwlLayout, 1, _T("Evaluate() value ") << value << 
      _T(", OrderedCombination[2] => ") << OrderedCombination[1] << 
      _T(", Inputs[2]->Value => ") << Inputs[2]->Value);
  }
  return value;
}

//
//
//
int
TConstraint::NumActualInputs()
{
  int i;
  for (i = 0; i < 3; i++)
    if (!Inputs[i])
      break;

  return i;
}

//----------------------------------------------------------------------------

//
/// Creates a TLayoutMetrics object and initializes the object. It sets the units
/// for the child and parent window to the specified layout units, and the
/// relationship between the two windows to what is defined in ImAsIs (of
/// TRelationship). TLayoutMetrics sets the following default values:
/// \code
/// X.RelWin = 0;
/// X.MyEdge = lmLeft;
/// X.Relationship = lmAsIs;
/// X.Units = lmLayoutUnits;
/// X.Value = 0;
/// Y.RelWin = 0;
/// Y.MyEdge = lmTop;
/// Y.Relationship = lmAsIs;
/// Y.Units = lmLayoutUnits;
/// Y.Value = 0;
/// Width.RelWin = 0;
/// Width.MyEdge = lmWidth;
/// Width.Relationship = lmAsIs;
/// Width.Units = lmLayoutUnits;
/// Width.Value = 0;
/// Height.RelWin = 0;
/// Height.MyEdge = lmHeight;
/// Height.Relationship = lmAsIs;
/// Height.Units = lmLayoutUnits;
/// Height.Value = 0;
/// \endcode
/// The following program creates two child windows and a frame into which you can
/// add layout constraints.
/// \code
/// #include <owl/owl.h>
/// #include <owl/framewin.h>
/// #include <owl/applicat.h>
/// #include <owl/layoutwi.h>
/// #include <owl/decorate.h>
/// #include <owl/decmdifr.h>
/// #include <owl/layoutco.h>
/// #pragma hdrstop
/// 
/// // Create a derived class. //
/// 
/// class TMyDecoratedFrame : public TDecoratedFrame { 
/// public:
///   TMyDecoratedFrame(TWindow* parent, const char far* title,
///                     TWindow& clientWnd, TWindow* MyChildWindow);
///   void SetupWindow();
///   { 
///     TDecoratedFrame::SetupWindow();
///     MyChildWindow->ShowWindow(SW_NORMAL);
///     MyChildWindow->BringWindowToTop();
///   }
/// };
/// 
/// // Setup a frame window. //
/// 
/// TMyDecoratedFrame::TMyDecoratedFrame(TWindow * parent,
///                                      const char far * title,
///                                      TWindow& clientWnd)
///   : TDecoratedFrame(parent, title, clientWnd),
///     TFrameWindow(parent, title, &clientWnd),
///     TWindow(parent, title)
/// { 
/// // Create a new TMyChildWindow. //
/// 
///   MyChildWindow = new TWindow(this, "");
///   MyChildWindow->Attr.Style |=  WS_BORDER |WS_VISIBLE |WS_CHILD;
///   MyChildwindow->SetBkgndColor(RGB(0,100,0));
/// 
/// // Establish metrics for the child window. //
/// 
///   TLayoutMetrics  layoutMetrics;
/// 
///   layoutMetrics.X.Absolute(lmLeft, 10);
///   layoutMetrics.Y.Absolute(lmTop, 10);
///   layoutMetrics.Width.Absolute( 80 );
///   layoutMetrics.Height.Absolute( 80 );
/// }
///   SetChildLayoutMetrics(*MyChildWindow, layoutMetrics);
/// class TMyApp : public TApplication { 
/// public:
/// 
///   virtual void InitMainWindow()
///   { 
///     TWindow* client = new TWindow(0, "title");
///     MainWindow = new TMyDecoratedFrame(0,
///                                        "Layout Window Ex",
///                                        *client);
///   }
/// };
/// int OwlMain(int, char**) { 
///   return TMyApp.Run();
/// }
/// \endcode
//
TLayoutMetrics::TLayoutMetrics()
{
  X.RelWin = 0;
  X.MyEdge = X.OtherEdge = lmLeft;
  X.Relationship = lmAsIs;
  X.Units = lmLayoutUnits;
  X.Value = 0;

  Y.RelWin = 0;
  Y.MyEdge = Y.OtherEdge = lmTop;
  Y.Relationship = lmAsIs;
  Y.Units = lmLayoutUnits;
  Y.Value = 0;

  Width.RelWin = 0;
  Width.MyEdge = Width.OtherEdge = lmWidth;
  Width.Relationship = lmAsIs;
  Width.Units = lmLayoutUnits;
  Width.Value = 0;

  Height.RelWin = 0;
  Height.MyEdge = Height.OtherEdge = lmHeight;
  Height.Relationship = lmAsIs;
  Height.Units = lmLayoutUnits;
  Height.Value = 0;
}

//
//
//
void
TLayoutMetrics::SetMeasurementUnits(TMeasurementUnits units)
{
  X.Units = Y.Units = Width.Units = Height.Units = units;
}

} // OWL namespace

/* ========================================================================== */

