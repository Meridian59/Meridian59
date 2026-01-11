//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Definition of class TDateTimePicker.
//----------------------------------------------------------------------------

#if !defined(OWL_DATETIME_H)
#define OWL_DATETIME_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/control.h>
#include <owl/commctrl.h>

namespace owl {

#include <owl/preclass.h>

class _OWLCLASS TDateTimePickerData
{
  public:
    TDateTimePickerData(
      const TSystemTime& dt = TSystemTime::LocalTime(),
      const TSystemTime& mind = TSystemTime(1980,0,0),
      const TSystemTime& maxd = TSystemTime(2100,0,0)
      )
      : DateTime(dt), MaxDate(maxd), MinDate(mind)
      {}

    TSystemTime DateTime;
    TSystemTime MaxDate; ///< default TSystemTime(0) not set
    TSystemTime MinDate; ///< default TSystemTime(0) not set
};

/// \addtogroup newctrl
/// @{

//
/// \class TDateTimePicker
//
class _OWLCLASS TDateTimePicker
  : public TControl
{
  public:
    enum TDateTimeColors
    {
      dtBackColor=MCSC_BACKGROUND,
      dtTextColor=MCSC_TEXT,
      dtTitleBackColor=MCSC_TITLEBK,
      dtTitleTextColor=MCSC_TITLETEXT,
      dtMonthBackColor=MCSC_MONTHBK,
      dtTrailingTextColor=MCSC_TRAILINGTEXT,
    };

    TDateTimePicker(TWindow* parent, int id, int x, int y, int w, int h, TModule* module = 0);
    TDateTimePicker(TWindow* parent, int resourceId, TModule* module = 0);
    TDateTimePicker(THandle hWnd, TModule* module = 0);
    ~TDateTimePicker();

    void SetColor(TDateTimeColors index, const TColor& clr);
    TColor GetColor(TDateTimeColors index) const;

    bool SetFormat(LPCTSTR format);
    bool SetFormat(const tstring& format) {return SetFormat(format.c_str());}
    HFONT GetFont() const;
    void SetFont(HFONT font, bool redraw = true);

    bool SetTime(const TSystemTime& dt);
    bool SetNoTime();
    int GetTime(TSystemTime& dt) const;
    TSystemTime GetTime() const {TSystemTime t; GetTime(t); return t;} // TODO: Add error handling.

    bool SetRange(const TSystemTime& mint, const TSystemTime& maxt);
    void GetRange(TSystemTime& mint, TSystemTime& maxt) const;

    HWND GetMonthCalCtrl();

    //
    /// Safe overload
    //
    uint Transfer(TDateTimePickerData& data, TTransferDirection direction) {return Transfer(&data, direction);}

  protected:
    //
    /// Override TWindow virtual member functions
    //
    uint Transfer(void* buffer, TTransferDirection direction);
    virtual auto GetWindowClassName() -> TWindowClassName;

  private:
    //
    // Hidden to prevent accidental copying or assignment
    //
    TDateTimePicker(const TDateTimePicker&);
    TDateTimePicker& operator =(const TDateTimePicker&);
};

/// @}

#include <owl/posclass.h>

//
// inlines
//

//
inline void TDateTimePicker::SetColor(TDateTimeColors index, const TColor& clr)
{
  PRECONDITION(GetHandle());
  SendMessage(DTM_SETMCCOLOR, TParam1(index), TParam2(clr.GetValue()));
}

//
inline TColor TDateTimePicker::GetColor(TDateTimeColors index) const
{
  PRECONDITION(GetHandle());
   return static_cast<COLORREF>(::SendMessage(GetHandle(), DTM_GETMCCOLOR, TParam1(index),0));
}

//
inline bool TDateTimePicker::SetFormat(LPCTSTR format)
{
  PRECONDITION(GetHandle());
  return static_cast<bool>(SendMessage(DTM_SETFORMAT, 0, reinterpret_cast<TParam2>(format)));
}

//
inline HFONT TDateTimePicker::GetFont() const
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HFONT>(::SendMessage(GetHandle(), DTM_GETMCFONT,0,0));  //Jogy: Why not call the member SendMessage??
}

//
inline void TDateTimePicker::SetFont(HFONT font, bool redraw)
{
  PRECONDITION(GetHandle());
  SendMessage(DTM_SETMCFONT, reinterpret_cast<TParam1>(font), MkUint32(static_cast<uint16>(redraw), 0));
}

//
inline HWND TDateTimePicker::GetMonthCalCtrl()
{
  PRECONDITION(GetHandle());
  return reinterpret_cast<HWND>(SendMessage(DTM_GETMONTHCAL, 0, 0));
}

//
inline bool TDateTimePicker::SetTime(const TSystemTime& dt)
{
  PRECONDITION(GetHandle());
  return static_cast<bool>(SendMessage(DTM_SETSYSTEMTIME, static_cast<TParam1>(GDT_VALID), reinterpret_cast<TParam2>(&dt)));
}

//
inline bool TDateTimePicker::SetNoTime()
{
  PRECONDITION(GetHandle());
  return static_cast<bool>(SendMessage(DTM_SETSYSTEMTIME, static_cast<TParam1>(GDT_NONE), 0));
}

// return GDT_VALID,GDT_NONE,GDT_ERROR
inline int TDateTimePicker::GetTime(TSystemTime& dt) const
{
  PRECONDITION(GetHandle());
  return static_cast<int>(::SendMessage(GetHandle(), DTM_GETSYSTEMTIME, 0, reinterpret_cast<TParam2>(&dt)));  //Jogy: Why not call the member SendMessage??
}

} // OWL namespace

#endif  // OWL_DATETIME_H
