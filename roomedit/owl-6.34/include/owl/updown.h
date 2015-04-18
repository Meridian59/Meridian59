//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TUpDown.
//----------------------------------------------------------------------------

#if !defined(OWL_UPDOWN_H)
#define OWL_UPDOWN_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>
#include <owl/commctrl.h>
#include <owl/bitset.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TUDAccel
// ~~~~~ ~~~~~~~~
/// TUDAccel is a very thin wrapper for the UDACCEL structure which contains
/// information about updown accelarators. The main purpose of this class is to have
/// a place-holder for future abstraction/encapsulation.
//
class TUDAccel : public UDACCEL {
};

//
/// \class TUpDown
// ~~~~~ ~~~~~~~
/// TUpDown encapsulates an up-down control, which is a window with a pair of arrow
/// buttons that the user can click to increment or decrement a value.
//
class _OWLCLASS TUpDown : public TControl, public TBitFlags<uint> {
  public:
    TUpDown(TWindow* parent,
            int      id,
            int x, int y, int w, int h,
            TWindow* buddy = 0,
            TModule* module = 0);

    TUpDown(TWindow* parent, int resourceId, TWindow* buddy = 0, TModule* module = 0);

    int           GetAccel(int count, TUDAccel * accels) const;
    int           GetBase() const;
    HWND          GetBuddy() const;
    int32         GetPos() const;
    uint32        GetRange() const;
    void          GetRange(int& lower, int& upper) const;

    bool          SetAccel(int count, const TUDAccel * accels);
    int           SetBase(int base);
    HWND          SetBuddy(HWND hBuddy);
    TResult       SetPos(int pos);
    void          SetRange(int lower, int upper);

  protected:
    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();
    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);
    void          EvVScroll(uint, uint, HWND);
    void          EvHScroll(uint, uint, HWND);


  private:
    // Hidden to prevent accidental copying or assignment
    //
    TUpDown(const TUpDown&);
    TUpDown& operator =(const TUpDown&);

    // Data members used to hold creation attributes of control
    // NOTE: These variables are not kept in sync with the actual
    //       state of the control when the class makes use of the
    //       CommonControl implementation of 'UPDOWN'
    //
    TWindow*      Buddy;                    ///< Pointer to buddy window
    int           Lower;                    ///< Low end of range
    int           Upper;                    ///< High end of range
    int           Pos;                      ///< Current/Staring posotion

  DECLARE_RESPONSE_TABLE(TUpDown);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inlines
//

/// Retrieves acceleration information for the underlying up-down control
//
inline int TUpDown::GetAccel(int count, TUDAccel * accels) const {
  return (int)CONST_CAST(TUpDown*,this)->SendMessage(UDM_GETACCEL, count, TParam2(accels));
}

/// Retrieves the current radix base of the underlying up-down control.
/// Return value is either 10 or 16.
//
inline int TUpDown::GetBase() const {
  return (int)CONST_CAST(TUpDown*,this)->SendMessage(UDM_GETBASE);
}

/// Retrieves handle of buddy window of underlying up-down control
//
inline HWND TUpDown::GetBuddy() const {
  return (HWND)CONST_CAST(TUpDown*,this)->SendMessage(UDM_GETBUDDY);
}

/// Returns current position of underlying up-down control. The high-order
/// word in non-zero in case of an error. The current position is in the
/// low-order word.
//
inline int32 TUpDown::GetPos() const {
  return (int32)LoUint16(CONST_CAST(TUpDown*,this)->SendMessage(UDM_GETPOS));
}

/// Retrieves the minimum and maximum range of the underlying up-down control.
/// The low-order word contains the maximum position while the high-order
/// word contains the minimum position.
//
inline uint32 TUpDown::GetRange() const {
  return (uint32)CONST_CAST(TUpDown*,this)->SendMessage(UDM_GETRANGE);
}

/// Retrieves the minimum and maximum range of the underlying up-down control
/// into the specified 'lower' and 'upper' variables respectively.
//
inline void TUpDown::GetRange(int& lower, int& upper) const {
  TResult ret = CONST_CAST(TUpDown*,this)->SendMessage(UDM_GETRANGE);
  lower = HiUint16(ret);
  upper = LoUint16(ret);
}

/// Set the acceleration of the underlying up-down control. 'count' 
/// specifies the number of structures specified in 'accels' while the
/// latter is the address of an array of TUDAccel structures.
//
inline bool TUpDown::SetAccel(int count, const TUDAccel * accels) {
  return ToBool(SendMessage(UDM_SETACCEL, count, TParam2(accels)));
}

/// Sets the radix of the underlying up-down control. The 'base' parameter
/// should be either '10' or '16' for decimal and hexadecimal respectively.
//
inline int TUpDown::SetBase(int base) {
  return int(SendMessage(UDM_SETBASE, base));
}

/// Sets the buddy window of the underlying up-down control.
//
inline TWindow::THandle TUpDown::SetBuddy(HWND hBuddy) {
  return TWindow::THandle(SendMessage(UDM_SETBUDDY, TParam1(hBuddy)));
}

/// Sets the current position of the underlying up-down control. The
/// return value is the previous position.
//
inline TResult TUpDown::SetPos(int pos) {
  return SendMessage(UDM_SETPOS, 0, MkParam2(pos, 0));
}

/// Sets the minimum and maximum positions of the up-down control. 
/// \note Neither 'lower' nor 'upper' can be greater than UD_MAXVAL or
/// less than UD_MINVAL. Futhermore, the difference between the two
/// positions must not exceed UD_MAXVAL;
//
inline void TUpDown::SetRange(int lower, int upper) {
  PRECONDITION(lower >= UD_MINVAL);
  PRECONDITION(lower <= UD_MAXVAL);
  PRECONDITION(upper >= UD_MINVAL);
  PRECONDITION(upper <= UD_MAXVAL);
  PRECONDITION(abs(upper-lower) <= UD_MAXVAL);
  SendMessage(UDM_SETRANGE, 0, MkParam2(upper, lower));
}



} // OWL namespace


#endif  // OWL_UPDOWN_H
