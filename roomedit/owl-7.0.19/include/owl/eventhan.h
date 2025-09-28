//
/// \file
/// Definition of TEventHandler and related classes & macros
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
// Copyright (c) 2013 Vidar Hasfjord
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//
//----------------------------------------------------------------------------

#if !defined(OWL_EVENTHAN_H)
#define OWL_EVENTHAN_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/signatur.h>
#include <owl/dispatch.h>

#if (defined(__TRACE) || defined(__WARN))
#include <owl/private/strmdefs.h>
#endif

namespace owl {

#include <owl/preclass.h>

/// \addtogroup base
/// @{

#if OWL_NEW_RESPONSE_TABLE // New response table implementation:

//
/// \class TResponseTableEntry
/// Consists of the information needed to lookup and dispatch a message to a selected handler.
///
/// See DECLARE_RESPONSE_TABLE and DEFINE_RESPONSE_TABLE for additional information
/// about the macros in the response tables.
//
class _OWLCLASS TResponseTableEntry
{
public:

  union
  {
    /// Stores the ID of the message handled by this entry.
    /// This can be a general message, command message or notification message, for example,
    /// WM_LBUTTONDOWN or LBN_SELCHANGE.
    //
    TMsgId Msg;

    /// Stores the control notification code (for example, ID_LISTBOX) handled by this entry.
    /// This can be a button, combo box, edit control or list box notification code.
    //
    uint NotifyCode;
  };

  /// Stores the menu or accelerator resource ID (CM_xxxx) handled by this entry.
  //
  uint Id;

  /// Stores the dispatcher used to handle messages by this entry.
  //
  TDispatchFunction Dispatcher;
};

typedef TResponseTableEntry TGenericTableEntry; // For backwards compatibility.

//
/// \class TEventHandler
/// TEventHandler is a base class from which you can derive classes that handle messages.
/// Specifically, TEventHandler performs the following event-handling tasks:
///
/// - 1.  Analyzes a window message.
/// - 2.  Searches the class's response table for an appropriate event-handling function.
/// - 3.  Dispatches the message to the designated event-handling function.
///
/// Most of ObjectWindows' classes are derived from TEventHandler and, therefore, inherit this
/// event-handling machinery. In addition, any user-defined class derived from TEventHandler can
/// handle particular Windows messages using a response table and member function handlers.
//
class _OWLCLASS TEventHandler
{
public:

  /// A nested class, TEventInfo provides specific information about the type of message sent, the
  /// class that contains the function to be handled, the corresponding response table entry, and
  /// the dispatch function that forwards the message.
  //
  class TEventInfo
  {
  public:

    /// Contains the type of message sent. These can be command messages, child ID
    /// messages, notify-based messages such as LBN_SELCHANGE, or windows messages such
    /// as LBUTTONDOWN.
    //
    const TMsgId Msg;

    /// Contains the menu or accelerator resource ID (CM_xxxx) for the message response
    /// member function.
    //
    const uint Id;

    /// Points to the object that contains the function to be handled.
    //
    void* Object;

    /// Points to the response table entry (for example, EvActivate).
    //
    const TResponseTableEntry* Entry;

    /// Constructs a TEventInfo object with the specified ID and message type.
    //
    TEventInfo(TMsgId msg, uint id = 0)
      : Msg(msg), Id(id), Entry(nullptr) {}
  };

  typedef bool (*TEqualOperator)(const TResponseTableEntry&, const TEventInfo&);

  /// Searches the list of response table entries looking for a match
  //
  virtual bool Find(TEventInfo& info, TEqualOperator op = 0);

  TResult Dispatch(TEventInfo& info, TParam1, TParam2 = 0);
  TResult DispatchMsg(TMsgId, uint id, TParam1, TParam2);

protected:

  bool SearchEntries(const TResponseTableEntry* entries, TEventInfo& info, TEqualOperator op);
};

#else // Old response table implementation:

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
/// - 1.  Analyzes a window message.
/// - 2.  Searches the class's response table entries for an appropriate
/// event-handling function.
/// - 3.  Dispatches the message to the designated event-handling function.
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
        //
        const TMsgId Msg;

        /// Contains the menu or accelerator resource ID (CM_xxxx) for the message response
        /// member function.
        const uint                  Id;

        /// Points to the object that contains the function to be handled.
        TGeneric*                    Object;

        /// Points to the response table entry (for example, EvActivate).
        const TGenericTableEntry* Entry;

        /// Constructs a TEventInfo object with the specified ID and message type.
        TEventInfo(TMsgId msg, uint id=0) : Msg(msg), Id(id) {Entry = 0;}
    };

    typedef bool(*TEqualOperator)(const TGenericTableEntry &, const TEventInfo&);

    /// Searches the list of response table entries looking for a match
    //
    virtual bool     Find(TEventInfo& info, TEqualOperator op = 0);

    TResult          Dispatch(TEventInfo& info, TParam1, TParam2 = 0);
    TResult          DispatchMsg(TMsgId, uint id, TParam1, TParam2);

  protected:
    bool             SearchEntries(const TGenericTableEntry * entries,
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

#endif

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

/// @}

#include <owl/posclass.h>

} // OWL namespace

#if OWL_NEW_RESPONSE_TABLE // New response table implementation:

//
// Macros to declare a response table
//
#define DECLARE_RESPONSE_TABLE(cls)\
  private: static ::owl::TResponseTableEntry __entries[];\
  private: typedef cls TMyClass;\
  public: auto Find(TEventInfo&, TEqualOperator = 0) -> bool override

#define DECLARE_EXPORT_RESPONSE_TABLE(cls)\
  private: static ::owl::TResponseTableEntry __entries[];\
  private: typedef cls TMyClass;\
  public: auto __declspec(dllexport) Find(TEventInfo&, TEqualOperator = 0) -> bool override

#define DECLARE_IMPORT_RESPONSE_TABLE(cls)\
  private: static ::owl::TResponseTableEntry __entries[];\
  private: typedef cls TMyClass;\
  public: auto __declspec(dllimport) Find(TEventInfo&, TEqualOperator = 0) -> bool override

//
// Macros to define the response table entries
//
#define DEFINE_RESPONSE_TABLE_ENTRIES(cls)\
  ::owl::TResponseTableEntry cls::__entries[] = {

#define END_RESPONSE_TABLE\
  {{0}, 0, nullptr}}

//
/// Macro to define a response table for a class with no base response tables
/// Use it like this:
///
/// \code
///   DEFINE_RESPONSE_TABLE(cls)
///     EV_WM_PAINT,
///     EV_WM_LBUTTONDOWN,
///   END_RESPONSE_TABLE;
/// \endcode
//
#define DEFINE_RESPONSE_TABLE(cls)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal);\
  }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

#define DEFINE_RESPONSE_TABLE1(cls, base1)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
      base1::Find(eventInfo, equal);\
  }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

#define DEFINE_RESPONSE_TABLE2(cls, base1, base2)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
      base1::Find(eventInfo, equal) ||\
      base2::Find(eventInfo, equal);\
  }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

#define DEFINE_RESPONSE_TABLE3(cls, base1, base2, base3)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
      base1::Find(eventInfo, equal) ||\
      base2::Find(eventInfo, equal) ||\
      base3::Find(eventInfo, equal);\
  }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

#define DEFINE_RESPONSE_TABLE4(cls, base1, base2, base3, base4)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
      base1::Find(eventInfo, equal) ||\
      base2::Find(eventInfo, equal) ||\
      base3::Find(eventInfo, equal) ||\
      base4::Find(eventInfo, equal);\
  }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

#define DEFINE_RESPONSE_TABLE5(cls, base1, base2, base3, base4, base5)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
      base1::Find(eventInfo, equal) ||\
      base2::Find(eventInfo, equal) ||\
      base3::Find(eventInfo, equal) ||\
      base4::Find(eventInfo, equal) ||\
      base5::Find(eventInfo, equal);\
  }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

#define DEFINE_RESPONSE_TABLE6(cls, base1, base2, base3, base4, base5, base6)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
      base1::Find(eventInfo, equal) ||\
      base2::Find(eventInfo, equal) ||\
      base3::Find(eventInfo, equal) ||\
      base4::Find(eventInfo, equal) ||\
      base5::Find(eventInfo, equal) ||\
      base6::Find(eventInfo, equal);\
  }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

#define DEFINE_RESPONSE_TABLE7(cls, base1, base2, base3, base4, base5, base6, base7)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
      base1::Find(eventInfo, equal) ||\
      base2::Find(eventInfo, equal) ||\
      base3::Find(eventInfo, equal) ||\
      base4::Find(eventInfo, equal) ||\
      base5::Find(eventInfo, equal) ||\
      base6::Find(eventInfo, equal) ||\
      base7::Find(eventInfo, equal);\
  }\
  DEFINE_RESPONSE_TABLE_ENTRIES(cls)

#define DEFINE_RESPONSE_TABLE8(cls, base1, base2, base3, base4, base5, base6, base7, base8)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
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

#define DEFINE_RESPONSE_TABLE9(cls, base1, base2, base3, base4, base5, base6, base7, base8, base9)\
  bool cls::Find(TEventInfo& eventInfo, TEqualOperator equal)\
  {\
    return SearchEntries(&__entries[0], (eventInfo.Object = this, eventInfo), equal) ||\
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

#else // Old response table implementation:

//
// Macros to declare a response table
//
#define DECLARE_RESPONSE_TABLE(cls)\
  private:\
    static ::owl::TResponseTableEntry< cls > __entries[];\
    typedef ::owl::TResponseTableEntry< cls >::PMF TMyPMF;\
    typedef cls                               TMyClass;\
  public:\
    auto  Find(TEventInfo&, TEqualOperator = 0) -> bool override

#define DECLARE_EXPORT_RESPONSE_TABLE(cls)\
  private:\
    static ::owl::TResponseTableEntry< cls > __entries[];\
    typedef ::owl::TResponseTableEntry< cls >::PMF TMyPMF;\
    typedef cls                               TMyClass;\
  public:\
    auto __declspec(dllexport) Find(TEventInfo&, TEqualOperator = 0) -> bool override

#define DECLARE_IMPORT_RESPONSE_TABLE(cls)\
  private:\
    static ::owl::TResponseTableEntry< cls > __entries[];\
    typedef ::owl::TResponseTableEntry< cls >::PMF TMyPMF;\
    typedef cls                               TMyClass;\
  public:\
    auto __declspec(dllimport) Find(TEventInfo&, TEqualOperator = 0) -> bool override

//
// Macros to define the response table entries
//
#define DEFINE_RESPONSE_TABLE_ENTRIES(cls)\
  ::owl::TResponseTableEntry< cls > cls::__entries[] = {

#define END_RESPONSE_TABLE\
  {{0}, 0, 0, 0}}

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
      return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal);\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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
       return SearchEntries((TGenericTableEntry *)&__entries[0], eventInfo, equal) ||\
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

#endif

//-----------------------------------------------------------------------------
// Common helper macros for response table entries

#if OWL_EV_SIGNATURE_CHECK

#define OWL_ID_EV_GENERIC_(msgId, key, id, method, dispatchTemplate)\
  {{static_cast<::owl::TMsgId>(key)}, static_cast<::owl::uint>(id),\
  (::owl::CheckSignature<TMyClass, static_cast<::owl::TMsgId>(msgId), dispatchTemplate>(&TMyClass::method),\
  OWL_DISPATCH(dispatchTemplate<static_cast<::owl::TMsgId>(msgId)>::Decode, method))}

#define OWL_ID_EV_(msgId, id, method) OWL_ID_EV_GENERIC_(msgId, msgId, id, method, ::owl::TDispatch)

#else

#define OWL_ID_EV_(msgId, id, method)\
  {{static_cast<::owl::TMsgId>(msgId)}, static_cast<::owl::uint>(id),\
  OWL_DISPATCH(::owl::TDispatch<static_cast<::owl::TMsgId>(msgId)>::Decode, method)}

#endif

#define OWL_EV_(msgId, method) OWL_ID_EV_(msgId, 0, method)

#endif  // OWL_EVENTHAN_H
