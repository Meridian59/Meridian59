//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Templatized class implementation of OLE component creation code for
/// TComponentFactory callback for ObjectWindows applications
/// All templates assume an OWLGetAppDictionary() global and a Registrar global
/// User can provide alternate implementation for any or all of the functions
/// The user's derived TApplication class is passed as the template parameter
//
//----------------------------------------------------------------------------

#if !defined(OCF_OLEFACTO_H)
#define OCF_OLEFACTO_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/defs.h>
#include <ocf/oleutil.h>
#include <ocf/ocreg.h>
#include <ocf/olewindo.h>
#include <owl/docmanag.h>
#include <owl/appdict.h>
#include <owl/pointer.h>

namespace ocf {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

//----------------------------------------------------------------------------
/// Non-automated application automation callout stub implementation
//
template <class T> struct TOleFactoryNoAuto {
  static void      AutomateApp(T* app, owl::uint32 options);
  static TUnknown* AutomateObject(T* app, const void* obj,const std::type_info& info);
  static void      UnregisterAutomation(T* app);
};

//
template<class T> void
TOleFactoryNoAuto<T>::AutomateApp(T* /*app*/, owl::uint32) {

}

//
template<class T> TUnknown*
TOleFactoryNoAuto<T>::AutomateObject(T*, const void*,const std::type_info&) {
  return 0;
}

//
template<class T> void
TOleFactoryNoAuto<T>::UnregisterAutomation(T* /*app*/) {

}

//----------------------------------------------------------------------------
/// Automated application default automation callout implementation
//
template <class T> struct TOleFactoryAuto {
  static void      AutomateApp(T* app, owl::uint32 options);
  static TUnknown* AutomateObject(T* app, const void* obj,const std::type_info& info);
  static void      UnregisterAutomation(T* app);
};

//
/// Default callout to aggregate an automation helper to an automated object
//
template <class T> TUnknown*
TOleFactoryAuto<T>::AutomateObject(T* app,const void* obj,const std::type_info& info)
{
  using ::Registrar;
  CHECK(Registrar);
  return Registrar->CreateAutoObject(obj, info, app, typeid(*app));
}

//
/// Default callout to unregister automation active object
//
template <class T> void
TOleFactoryAuto<T>::UnregisterAutomation(T* app)
{
  using ::Registrar;
  CHECK(Registrar);
  Registrar->ReleaseAutoApp(TAutoObject<T>(app));
}

//
/// Default callout to aggregate an automation helper to an automated app
//
template <class T> void
TOleFactoryAuto<T>::AutomateApp(T* app, owl::uint32 options)
{
  using ::Registrar;
  CHECK(Registrar);
  Registrar->CreateAutoApp(TAutoObjectDelete<T>(app), options);
}

//----------------------------------------------------------------------------
/// Application creation/destruction callouts shared for doc/view and non-d/v
//
template<class T, class Auto> struct TOleFactoryAppBase {
  static T*        CreateApp(owl::uint32 options);
  static void      DestroyApp(T* app);
};

//
template<class T, class Auto> T*
TOleFactoryAppBase<T, Auto>::CreateApp(owl::uint32 options)
{
  T* app = new T;
  if ((options & amEmbedding) || !(options & amExeMode))
    app->SetCmdShow(SW_HIDE);
  using ::Registrar;
  CHECK(Registrar);
  app->OcInit(dynamic_cast<TOcRegistrar&>(*Registrar), options);
  Auto::AutomateApp(app, options);
  app->Start();
  return app;
}

//
template<class T, class Auto>
void TOleFactoryAppBase<T, Auto>::DestroyApp(T* app)
{
  Auto::UnregisterAutomation(app);
  delete app;
}

//----------------------------------------------------------------------------
/// Non-docview application callout implementation, no CreateObject implemented
/// User must either provide an implementation of CreateOleObject for app class
/// or else reimplement this template function for the particular class
//
template<class T, class Auto> struct TOleFactoryNoDocView
                                   : TOleFactoryAppBase<T, Auto> {
  static TUnknown* CreateObject(T* app, owl::uint32 options, owl::TRegLink* link);
};

//
template <class T, class Auto> TUnknown*
TOleFactoryNoDocView<T, Auto>::CreateObject(T* app, owl::uint32 options,
                                            owl::TRegLink* link)
{
  return app->CreateOleObject(options, link);
}

//----------------------------------------------------------------------------
/// Docview application callout implementation - supplies CreateObject function
//
template <class T, class Auto> struct TOleFactoryDocView
                                    : TOleFactoryAppBase<T, Auto> {
  static TUnknown* CreateObject(T* app, owl::uint32 options, owl::TRegLink* link);
};

//
template <class T, class Auto> TUnknown*
TOleFactoryDocView<T, Auto>::CreateObject(T* app, owl::uint32 options, owl::TRegLink* link)
{
  TUnknown* obj = 0;
  if (!link)      // if not coming up embedded, we don't know what to make here
    return 0;
  owl::TDocManager* docMan = app->GetDocManager();
  if (!docMan)
    return 0;
  owl::TDocTemplate* tpl = static_cast<owl::TDocTemplate*>(link);
  owl::TDocument* doc = docMan->CreateDoc(tpl, 0, 0, owl::dtNewDoc | owl::dtNoAutoView);
  owl::TView* view = tpl->ConstructView(*doc);
  ocf::TOleWindow* ow = TYPESAFE_DOWNCAST(view, ocf::TOleWindow);
  if (ow) {
    obj = ow->CreateOcView(tpl, (options & amEmbedding) != 0, 0);
    ocf::TUnknown* autoObj;
    void* viewObj = dynamic_cast<void*>(view); // get address of derived most
    void* docObj = dynamic_cast<void*>(doc);
  if ((autoObj = Auto::AutomateObject(app, viewObj, typeid(*view))) != 0 ||
        (autoObj = Auto::AutomateObject(app, docObj,  typeid(*doc)))  != 0) {
      obj->SetOuter(&autoObj->Aggregate(*obj));
      obj->AdjustRefCount(-1);  // remove extra ref count
      obj = autoObj;
      obj->AdjustRefCount(+1);  // add it back to the new outer sub-object
    }
  }
  doc->InitView(view);
  return (options & amEmbedding) ? obj : 0;
}

//----------------------------------------------------------------------------
// Standard factory for OWL components, callouts supplied via template args
//
/// A template class, TOleFactoryBase<> creates callback code for ObjectWindows
/// classes. The main purpose of the factory code is to provide a callback function,
/// Create, that ObjectComponents calls to create objects.
///
/// Just as a recipe consists of a list of instructions about how to make a
/// particular kind of food, a template, such as TOleFactoryBase<>, contains
/// instructions about how to make an object, in this case, a factory object.
/// TOleFactoryBase<> includes two public member functions. The three additional
/// functions are passed as template arguments. Although these template arguments
/// actually belong to the class that is instantiated when you fill in the arguments
/// to TOleFactoryBase<>, they are described here for convenience.
///
/// Use TOleFactoryBase<> to manufacture objects in general, whether or not they are
/// embedded, OLE-enabled, or use the Doc/View model. These objects might or might
/// not be connected to OLE.
///
/// The callouts are supplied through the arguments passed to the template class.
/// The factory base class takes three template parameters: the application type, a
/// set of functions to create the object, and a set of functions to create an
/// automation object. Depending on the arguments passed, you can make the following
/// OLE-enabled components:
/// - Doc/View components that are automated
/// - Doc/View components that are not automated
/// - Non-Doc/View components that are automated
/// - Non-Doc/View components that are not automated
///
/// ObjectWindows provides a standard implementation for object creation and
/// automation. Factory Template Classes gives an overview of these classes.
///
/// By using TOleFactoryBase<> to obtain an OLE interface for your application, you
/// can make objects that are accessible to OLE. That is, TOleFactoryBase<> handles
/// any relationships with IUnknown, a standard OLE interface.
template <class T, class Obj> class TOleFactoryBase {
  public:
/// Converts the object into a pointer to the factory. ObjectComponents uses this
/// pointer to create the object.
    operator TComponentFactory() {return Create;}
    static IUnknown* Create(IUnknown* outer, owl::uint32 options, owl::uint32 id);
};

/*
//
template <class T, class Obj>
TOleFactoryBase<T, Obj>::operator TComponentFactory()
{
  return Create;
}
*/

//
// Main Create callback function called to create app and/or object
//
/// A TComponentFactory callback function that creates or destroys the application
/// or creates objects. If an application object does not already exist, Create
/// creates a new one. The outer argument points to the OLE 2 IUnknown  interface
/// with which this object aggregates itself. If outer is 0,  If outer is 0, the
/// object will become an independent object.
/// The options argument indicates the application's mode while it is running. The
/// values for options are either set from the command line or set by
/// ObjectComponents. They are passed in by the Registrar to this callback. The
/// application looks at these flags in order to know how to operate, and the
/// factory callback looks at them in order to know what to do. For example, a value
/// of amExeMode indicates that the server is running as an .EXE either because it
/// was built as an .EXE or because it is a .DLL that was launched by an .EXE stub
/// and is now running as an executable program. The TOcAppMode enum description
/// shows the possible values for the options argument.
/// If the application already exists and the object ID (id) equals 0, Create
/// returns the application's OLE interface. Otherwise, it calls OCInit to create a
/// new TOcApp and register the options from TOcAppMode enum, which contains
/// OLE-related flags used in the application's command line. (These flags tell the
/// application whether it has been run as a server, whether it needs to register
/// itself, and so on.) If a component ID was passed, that becomes the component;
/// otherwise, Create runs the application itself based on the values of the
/// TOcAppMode enum and returns the OLE interface.
///
/// \image html bm59.BMP
template <class T, class Obj> IUnknown*
TOleFactoryBase<T, Obj>::Create(IUnknown* outer, owl::uint32 options, owl::uint32 id)
{
  owl::TRegLink* link = reinterpret_cast<owl::TRegLink*>(id);

  // Look for the app object for this process, creating one if necessary
  //
  owl::TApplication* existingApp = owl::OWLGetAppDictionary().GetApplication();
  T* app;
  if (!existingApp) {
    if (options & amShutdown)   // app already destructed
      return 0;
    app = Obj::CreateApp(options);
    if (!link && !(options & amEmbedding)) {
      Obj::CreateObject(app, options, link);
    }
  } else {
    app = TYPESAFE_DOWNCAST(existingApp, T);
    if (options & amShutdown) {
      Obj::DestroyApp(app);
      return 0;  // any interface present when passed in has now been released
    }
  }
  TUnknown* obj = app->OcApp;   // app's COM interface, used if not doc object

  // If a component ID was passed, make that component, otherwise return app
  //
  if (link) {
    TUnknown* doc = Obj::CreateObject(app, options, link);
    if (doc)
      obj = doc;
    else if (!(options & amEmbedding))  // run DLL in ExeMode from doc factory
      app->OcApp->SetOption(amServedApp, true);
    else
      return 0;  // doc factory failed
  }
  IUnknown* ifc = obj ? obj->SetOuter(outer) : 0; // aggregate to passed outer

  // Run message look if ExeMode, either EXE server or DLL server force to run
  // EXE servers come through here twice, no Run if 2nd pass from factory call
  //
  if (options & amRun)
    app->Run();

  return ifc;
}

//----------------------------------------------------------------------------
///  Factory for OWL non-Doc/View, non-automated OLE components
//
template <class T> class TOleFactory
: public TOleFactoryBase<T, TOleFactoryNoDocView<T, TOleFactoryNoAuto<T> > >
{
};

//
///  Factory for OWL Doc/View, non-automated OLE components
//
template <class T> class TOleDocViewFactory
: public TOleFactoryBase<T, TOleFactoryDocView<T, TOleFactoryNoAuto<T> > >
{
};

//
///  Factory for OWL non-Doc/View, automated OLE components
//
template <class T> class TOleAutoFactory
: public TOleFactoryBase<T, TOleFactoryNoDocView<T, TOleFactoryAuto<T> > >
{
};

//
///  Factory for OWL Doc/View, automated OLE components
//
template <class T> class TOleDocViewAutoFactory
: public TOleFactoryBase<T, TOleFactoryDocView<T, TOleFactoryAuto<T> > >
{
};

//----------------------------------------------------------------------------
//  Factory for OWL automated OLE components, no linking/embedding support
//

/// A template class, TAutoFactory<> creates callback code for ObjectWindows
/// classes. The application class is passed as the argument to the template. By
/// itself, TAutoFactory<> does not provide linking or embedding support for
/// ObjectWindows automated applications.
///
/// Although TAutoFactory<> simplifies the process of creating the callback
/// function, you can write your own callback function or provide alternate
/// implementation for any or all of TAutoFactory<>'s functions.
template <class T> class TAutoFactory {
  public:
/// Converts the object into a pointer to the factory. ObjectComponents uses this
/// pointer to create the automated object.
    operator TComponentFactory() {return Create;}

    // Callouts to allow replacement of individual creation steps
    //
    static T*        CreateApp(owl::uint32 options);
    static void      DestroyApp(T* app);

    // Main Create callback function called to create app and/or object
    //
    static IUnknown* Create(IUnknown* outer, owl::uint32 options, owl::uint32 id);
};

/*
//
template <class T>
TAutoFactory<T>::operator TComponentFactory()
{
  return Create;
}
*/

//
/// Called when the app is not found and needs to be created
//
/// CreateApp creates a new automated application. By default, it creates a new
/// application of template type T with no arguments. The options argument is one of
/// the TOcAppMode enum values, for example, amRun, amAutomation, and so on that
/// indicate the application's mode when running.
//
template <class T> T*
TAutoFactory<T>::CreateApp(owl::uint32 options)
{
  T* app = new T;
  if ((options & amEmbedding) || !(options & amExeMode))
    app->nCmdShow = SW_HIDE;
  app->Start();
  return app;
}

//
/// Destroys the previously created application referred to in app.
//
template <class T> void
TAutoFactory<T>::DestroyApp(T* app)
{
  delete app;
}

//
/// Create is a TComponentFactory callback function that creates or destroys the
/// application or creates objects. If an application object does not already exist,
/// Create creates a new one. The outer argument points to the OLE 2 IUnknown
/// interface with which this object aggregates itself. If outer is 0, the new
/// object is not aggregated, or it will become the main object.
/// The options argument indicates the application's mode while it is running. The
/// values for options are either set from the command line or set by
/// ObjectComponents. They are passed in by the "Registrar" to this callback. The
/// application looks at these flags to know how to operate, and the factory
/// callback looks at them to know what to do. For example, a value of amExeMode
/// indicates that the server is running as an .EXE either because it was built as
/// an .EXE or because it is a .DLL that was launched by an .EXE stub and is now
/// running as an executable program. See TOcAppMode enum for a description of the
/// possible values for the options argument.
/// If the application already exists, Create returns the application's OLE
/// interface and registers the options from TOcAppMode enum which contains
/// OLE-related flags used in the application's command line. For example, the
/// amAutomation flag tells the server to register itself as a single-user
/// application. (In general, these flags tell the application whether it has been
/// run as a server, whether it needs to register itself, and so on.)
/// The id argument, which is not used for TAutoFactory, is always 0.
///
/// \image html bm59.BMP
//
template <class T> IUnknown*
TAutoFactory<T>::Create(IUnknown* outer, owl::uint32 options, owl::uint32 /*id*/)
{
  IUnknown* ifc = 0;
  owl::TApplication* existingApp = owl::OWLGetAppDictionary().GetApplication();
  T* app;
  if (!existingApp) {
    if (options & amShutdown)   // app already destructed
      return 0;
    app = CreateApp(options);
  }
  else {
    app = TYPESAFE_DOWNCAST(existingApp, T);
    if (options & amShutdown) {
      DestroyApp(app);
      return (options & amServedApp) ? 0 : outer;
    }
  }

  if (options & amServedApp) {
    using ::Registrar;
    CHECK(Registrar);
    TUnknown* obj = Registrar->CreateAutoApp(TAutoObjectDelete<T>(app),
                                               options, outer);
    ifc = *obj;  // does an AddRef, reference count owned by container
  }

  if (options & amRun) {
    app->Run();
  }
  return ifc;
}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


} // OCF namespace


#endif  // OWL_OLEFACTO_H
