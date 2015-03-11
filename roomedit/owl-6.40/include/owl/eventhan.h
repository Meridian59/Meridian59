//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TEventHandler and related classes & macros
//----------------------------------------------------------------------------

#if !defined(OWL_EVENTHAN_H)
#define OWL_EVENTHAN_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/signatur.h>
#include <owl/dispatch.h>

#if (defined(__TRACE) || defined(__WARN))
#include <owl/private/strmdefs.h>
#endif

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
// Portable types for message & event components
//
typedef LRESULT TResult;  ///< Returned result of a message
typedef uint    TMsgId;   ///< Message ID type
typedef WPARAM  TParam1;  ///< Type of first message parameter (uint)
typedef LPARAM  TParam2;  ///< Type of second message parameter (uint32)

/// \addtogroup base
/// @{

#if defined(__TRACE) || defined(__WARN)

//
// Class/operator which converts a Windows message to its string equivalent for
// use with the diagnostic library
//
class _OWLCLASS TMsgName
{
public:
  TMsgName(TMsgId msg) : Message(msg) {}

private:
  TMsgId Message;

  friend _OWLCFUNC(tostream &) operator <<(tostream& os, const TMsgName& msg);
};

_OWLCFUNC(tostream &) operator <<(tostream& os, const TMsgName& msg);

#endif

//
// Messages defined for OWL use - the top of the user range of ids is reserved
//
#define WM_OWLLAST          0x7FFF
#define WM_OWLFIRST        (WM_OWLLAST - 0x03FF)

#define WM_COMMAND_ENABLE  (WM_OWLLAST - 0)
#define WM_CHILDINVALID    (WM_OWLLAST - 1)
#define WM_OWLDOCUMENT     (WM_OWLLAST - 2)
#define WM_OWLVIEW         (WM_OWLLAST - 3)
#define WM_OWLNOTIFY       (WM_OWLLAST - 4)
#define WM_OWLPREPROCMENU  (WM_OWLLAST - 5)
#define WM_OWLCANCLOSE     (WM_OWLLAST - 6)
#define WM_VBXINITFORM     (WM_OWLLAST - 7)
#define WM_VBXNAME         (WM_OWLLAST - 8)
#define WM_VBXBASE         (WM_OWLLAST - 8 - 256)
#define WM_OWLWAKEUP       (WM_VBXBASE - 1)
#define WM_OWLFRAMESIZE    (WM_VBXBASE - 2) ///< Notify children of frame resizing
#define WM_OWLSLIPDBLCLK   (WM_VBXBASE - 3) ///< Notify parent of user dblclick of edge slip
#define WM_OWLWINDOWDOCKED (WM_VBXBASE - 4) ///< Notify window it was [un]docked/reparented
#define WM_OWLCREATETTIP   (WM_VBXBASE - 5) ///< Notify gadget window to create tooltips
#define WM_OWLHELPHIT      (WM_VBXBASE - 6) ///< Notify Help manager with hit point

template <class T1, class T2>
inline TParam2 MkParam2(const T1& lo, const T2& hi) {
  return (uint32(hi) << 16) | uint16(lo);
}
inline TParam1 MkParam1(uint lo, uint hi) {
  return (uint32(hi) << 16) | uint16(lo);
}

//
// Forward declarations
//
template <class T> class _OWLCLASS TResponseTableEntry;
typedef TResponseTableEntry<TGeneric>  TGenericTableEntry;

//
/// \class TEventHandler
// ~~~~~ ~~~~~~~~~~~~~
/// TEventHandler is a base class from which you can derive classes that handle
/// messages. Specifically, TEventHandler performs the following event-handling
/// tasks:
/// - 1.	Analyzes a window message.
/// - 2.	Searches the class's response table entries for an appropriate
/// event-handling function.
/// - 3.	Dispatches the message to the designated event-handling function.
/// 
/// Most of ObjectWindows' classes are derived from TEventHandler and, therefore,
/// inherit this event-handling behavior. In addition, any user-defined class
/// derived from TEventHandler can handle message response functions that are
/// associated with a particular window message.
//
class _OWLCLASS _RTTI TEventHandler {
  public:
  	
		/// A nested class, TEventInfo provides specific information about the type of
		/// message sent, the class that contains the function to be handled, the
		/// corresponding response table entry, and the dispatch function that processes the
		/// message.
    class TEventInfo 
    {
      public:
				/// Contains the type of message sent. These can be command messages, child ID
				/// messages, notify-based messages such as LBN_SELCHANGE, or windows messages such
				/// as LBUTTONDOWN.
        const TMsgId Msg;
        
				/// Contains the menu or accelerator resource ID (CM_xxxx) for the message response
				/// member function.
        const uint                  Id;
        
				/// Points to the object that contains the function to be handled.
        TGeneric*                    Object;
        
				/// Points to the response table entry (for example, EvActivate).
        TGenericTableEntry * Entry;
        
				/// Constructs a TEventInfo object with the specified ID and message type.
        TEventInfo(TMsgId msg, uint id=0) : Msg(msg), Id(id) {Entry = 0;}
    };
    
    typedef bool(*TEqualOperator)(TGenericTableEntry &, TEventInfo&);

    /// Searches the list of response table entries looking for a match
    //
    virtual bool     Find(TEventInfo& info, TEqualOperator op = 0);

    TResult          Dispatch(TEventInfo& info, TParam1, TParam2 = 0);
    TResult          DispatchMsg(TMsgId, uint id, TParam1, TParam2);

  protected:
    bool             SearchEntries(TGenericTableEntry * entries,
                                   TEventInfo& info,
                                   TEqualOperator op);
};

//
/// \class TResponseTableEntry<>
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~
/// A template class, TResponseTableEntry lets you define a pattern for entries into
/// a response table. Entries consist of a message, a notification code, a resource
/// ID, a dispatcher type, and a pointer to a member function.
/// See DECLARE_RESPONSE_TABLE and DEFINE_RESPONSE_TABLE for additional information
/// about the macros in the response tables.
//
template <class T> class _OWLCLASS TResponseTableEntry {
  public:
  	
		/// Type for a generic member function that responds to notification messages. T is
		/// the template for the response table.
    typedef void (T::*PMF)();

    union {
			/// Contains the ID of the message sent. These can be command messages, child id
			/// messages, notify-based messages such as LBN_SELCHANGE, or messages such as
			/// LBUTTONDOWN.
      TMsgId Msg;
      
			/// Stores the control notification code (for example, ID_LISTBOX) for the response
			/// table entry. These can be button, combo box, edit control, or list box
			/// notification codes.
      uint          NotifyCode;
    };
    
		/// Contains the menu or accelerator resource ID (CM_xxxx) for the message response
		/// member function.
    uint            Id;
    
		/// An abstract dispatcher type that points to one of the dispatcher functions.
    TAnyDispatcher  Dispatcher;
    
		/// Points to the actual handler or member function.
    PMF             Pmf;
};

/// @}


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

//
// Macros to declare a response table
//
#define DECLARE_RESPONSE_TABLE(cls)\
  private:\
    static ::owl::TResponseTableEntry< cls > __entries[];\
    typedef ::owl::TResponseTableEntry< cls >::PMF TMyPMF;\
    typedef cls                               TMyClass;\
  public:\
    bool  Find(TEventInfo&, TEqualOperator = 0)

#define DECLARE_EXPORT_RESPONSE_TABLE(cls)\
  private:\
    static ::owl::TResponseTableEntry< cls > __entries[];\
    typedef ::owl::TResponseTableEntry< cls >::PMF TMyPMF;\
    typedef cls                               TMyClass;\
  public:\
    bool __declspec(dllexport) Find(TEventInfo&, TEqualOperator = 0)

#define DECLARE_IMPORT_RESPONSE_TABLE(cls)\
  private:\
    static ::owl::TResponseTableEntry< cls > __entries[];\
    typedef ::owl::TResponseTableEntry< cls >::PMF TMyPMF;\
    typedef cls                               TMyClass;\
  public:\
    bool __declspec(dllimport) Find(TEventInfo&, TEqualOperator = 0)

#define END_RESPONSE_TABLE\
  {{0}, 0, 0, 0}}

#define DEFINE_RESPONSE_TABLE_ENTRIES(cls)\
  ::owl::TResponseTableEntry< cls > cls::__entries[] = {

//
/// Macro to define a response table for a class with no base response tables
//
/// Use it like this:
/// \code 
///    DEFINE_RESPONSE_TABLE(cls)
///      EV_WM_PAINT,
///     EV_WM_LBUTTONDOWN,
///    END_RESPONSE_TABLE;
/// \endcode
#define DEFINE_RESPONSE_TABLE(cls)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
      return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with one base. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE1(cls, base)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
          base::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with two bases. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE2(cls, base1, base2)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
          base2::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with three bases. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE3(cls, base1, base2, base3)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal) ||\
          base3::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with three bases. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE4(cls, base1, base2, base3, base4)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal) ||\
              base3::Find(eventInfo, equal) ||\
          base4::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with three bases. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE5(cls, base1, base2, base3, base4, base5)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal) ||\
              base3::Find(eventInfo, equal) ||\
              base4::Find(eventInfo, equal) ||\
          base5::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with three bases. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE6(cls, base1, base2, base3, base4, base5, base6)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal) ||\
              base3::Find(eventInfo, equal) ||\
              base4::Find(eventInfo, equal) ||\
              base5::Find(eventInfo, equal) ||\
          base6::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with three bases. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE7(cls, base1, base2, base3, base4, base5, base6, base7)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal) ||\
              base3::Find(eventInfo, equal) ||\
              base4::Find(eventInfo, equal) ||\
              base5::Find(eventInfo, equal) ||\
              base6::Find(eventInfo, equal) ||\
          base7::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with three bases. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE8(cls, base1, base2, base3, base4, base5, base6, base7, base8)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal) ||\
              base3::Find(eventInfo, equal) ||\
              base4::Find(eventInfo, equal) ||\
              base5::Find(eventInfo, equal) ||\
              base6::Find(eventInfo, equal) ||\
              base7::Find(eventInfo, equal) ||\
          base8::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

//
/// Macro to define a response table for a class with three bases. Use this macro
/// exactly like macro DEFINE_RESPONSE_TABLE
//
#define DEFINE_RESPONSE_TABLE9(cls, base1, base2, base3, base4, base5, base6, base7, base8, base9)\
  bool  cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
      {\
      using namespace ::owl;\
      eventInfo.Object = (TGeneric*)this;\
       return SearchEntries((TGenericTableEntry *)__entries, eventInfo, equal) ||\
              base1::Find(eventInfo, equal) ||\
              base2::Find(eventInfo, equal) ||\
              base3::Find(eventInfo, equal) ||\
              base4::Find(eventInfo, equal) ||\
              base5::Find(eventInfo, equal) ||\
              base6::Find(eventInfo, equal) ||\
              base7::Find(eventInfo, equal) ||\
              base8::Find(eventInfo, equal) ||\
          base9::Find(eventInfo, equal);\
      }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)


#endif  // OWL_EVENTHAN_H
