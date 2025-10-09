//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// NOTE: I do *not* claim ownership of these three classes; the copyright
// notice is intended only to claim those rights not explicitly claimed
// by Mr. Meyers on this property. I typed it in; that's the extent of
// the work I've provided here.
//
// Ref-counted auto-managing template from Meyers' "More Effective C++"
// pp. 203-205
//
//-------------------------------------------------------------------
#ifndef __OWLEXT_RCPTR_H
#define __OWLEXT_RCPTR_H

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                      RCPtr<T>
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <class T>
class RCPtr
{
    //
    // Ctors, Dtors & Canonical Form
    //
public:
    RCPtr(T* realPtr = 0)
        : _pointee(realPtr)
    { init(); }
    RCPtr(const RCPtr<T>& src)
        : _pointee(src._pointee)
    { init(); }
    ~RCPtr()
    { if (_pointee) _pointee->RemoveReference(); }
    RCPtr<T>& operator=(const RCPtr<T>& rhs)
    {
        if (_pointee != rhs._pointee)
        {
            if (_pointee)
                _pointee->RemoveReference();

            _pointee = rhs._pointee;
            init();
        }
        return *this;
    }

    //
    // smart pointer interface
    //
    T* operator->() const
    { return _pointee; }
    T& operator*() const
    { return *_pointee; }
    T* c_ptr() const
    { return _pointee; }

    //
    // Data & implementational methods
    //
private:
    T* _pointee;

    void init()
    {
        if (_pointee==0)
            return;

        if (_pointee->IsShareable() == false)
            _pointee = new T(*_pointee);

        _pointee->AddReference();
    }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                      RCObject
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class RCObject
{
    //
    // Ctors, Dtors & Canonical form
    //
protected:
    RCObject()
        : _refCount(0), _shareable(true)
    { }
    RCObject(const RCObject&)
        : _refCount(0), _shareable(true)
    { }
    RCObject& operator=(const RCObject& rhs)
    { return *this; }
    virtual ~RCObject()
    { }

    //
    // Reference() methods
    //
public:
    void AddReference()
    { ++_refCount; }
    void RemoveReference()
    { if (--_refCount==0) delete this; }

    void MarkUnshareable()
    { _shareable = false; }
    bool IsShareable()
    { return _shareable; }
    bool IsShared() const
    { return _refCount > 1; }

    //
    // Data & implementational methods
    //
private:
    int _refCount;
    bool _shareable;
};

/* Sample usage
Derive your letter class from RCObject. Make your letter class accept
RCPtr<letter_class> as a friend. Instantiate an object of RCPtr<letter_class> in
your object, and away you go!

Example:
class String
{
public:
    String(const char* initValue = "")
        : value(new StringValue(initValue))
    { }

    char operator[](owl::uint index) const
    { return value->data[index]; }
    char& operator[](owl::uint index)
    {
        if (value->IsShared())
        {
            value->RemoveReference();
            value = new StringValue(value->data);
        }
        value->MarkUnshareable();

        return value->data[index];
    }

private:
    struct StringValue : public RCObject
    {
        char* data;

        StringValue(const char* initValue)
        { init(initValue); }
        StringValue(const StringValue& rhs)
        { init(rhs.data); }
        void init(const char* initValue)
        { data = new char[strlen(initValue)+1]; strcpy(data, initValue); }
        ~StringValue()
        { delete [] data; }

        friend class RCPtr<StringValue>;
    };
    RCPtr<StringValue> value;
};

Note that no copy constructor, no destructor are required: the default C++
implementations of those methods are all that are necessary. [Meyers96, p. 205]
*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                     RCIPtr<T>
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <class T>
class RCIPtr
{
    // Object lifetime methods
    //
public:
    RCIPtr(T* realPtr = 0)
        : _counter(new CountHolder)
    { _counter->_pointee = realPtr; init(); }
    RCIPtr(const RCIPtr<T>& src)
        : _counter(src._counter)
    { init(); }
    ~RCIPtr()
    { if (_counter) _counter->RemoveReference(); }

    // Mutators
    //
public:
    RCIPtr<T>& operator=(const RCIPtr<T>& rhs)
    {
        if (_counter != rhs._counter)
        {
            if (_counter)
                _counter->RemoveReference();
            _counter = rhs._counter;
            init();
        }
        return *this;
    }

    //
    // Smart-pointer interface
    //
public:
    T* operator->() const
    { return _counter->_pointee; }
    T& operator*() const
    { return *(_counter->_pointee); }
    T* c_ptr() const
    { return _counter->_pointee; }

    //
    // Data & implementational methods
    //
private:
    struct CountHolder : public RCObject
    {
        ~CountHolder()
        { delete _pointee; }

        T* _pointee;
    };
  CountHolder* _counter;

    void init()
    {
        if (_counter==0)
            return;

        if (_counter->IsShareable() == false)
        {
            _counter = new CountHolder;
            _counter->_pointee = new T(*_counter->_pointee);
        }

        _counter->AddReference();
    }
};


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

/*
Usage:

Given a class:

    class Widget
    {
    public:
        Widget(int size);
        Widget(const Widget& rhs);
        ~Widget();
        Widget& operator=(const Widget& rhs);

        void doThis();
        int showThat() const;
    };

Either use RCIPtr<> directly, as in:

    RCIPtr<Widget> rcwidget(new Widget(12));

Or create a class:

    class RCWidget
    {
    public:
        RCWidget(int size);
            : value(new Widget(size))
        { }

        void doThis()
        { value->doThis(); }
        int showThat() const
        { return value->showThat(); }

    private:
        RCIPtr<Widget> value;
    };

Use it as follows:

    RCWidget widget(12);
    widget.doThis;
- or -
    RCIPtr<Widget> rcwidget(new Widget(12));
    rcwidget->doThis;

 */

#endif
