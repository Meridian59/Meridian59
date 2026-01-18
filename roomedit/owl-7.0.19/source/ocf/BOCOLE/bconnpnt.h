//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
//  Bolero helper for Connection Points
//
//    BEventClass is the base implementation for Connection Point.
//    BEventClass deals with sinks collection and with Advise and
//    Unadvise member functions.
//    BEventClass keeps also a collection of void pointers which are
//    those resulted from QueryInterface on sinks. Control implementors
//    can ask for this list.
//----------------------------------------------------------------------------
#ifndef _BCONNPNT_H
#define _BCONNPNT_H 1

#ifndef _BOLE_H
#include "bole.h"
#endif

#define BOLE_DEFAULT_ENUM_SIZE    10



//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//      Class BEventHandler
//
//      Purpose:
//              This class is mainly the implementation for IConnectionPoint
//              It deals with all is releted to with sinks handling. This class
//              contains a nested class which is the object implementing
//              the two lists of sinks.
//
//
////\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\
class _ICLASS BEventHandler : public IBEventClass,
                      public IConnectionPoint
{
  public:
    BEventHandler (REFIID, LPUNKNOWN, UINT = BOLE_DEFAULT_ENUM_SIZE);
    ~BEventHandler();

    //\\// IUnknown methods \\//\\
    ULONG _IFUNC AddRef ();
    ULONG _IFUNC Release ();
    HRESULT _IFUNC QueryInterface(REFIID, LPVOID FAR*);

    //\\// IBEventClass methods \\//\\
    HRESULT _IFUNC GetSinkList (IBSinkList **);
    HRESULT _IFUNC FreezeEvents (bool);

    //\\// IConnectionPoint methods \\//\\
    HRESULT _IFUNC GetConnectionInterface (IID FAR*);
    HRESULT _IFUNC GetConnectionPointContainer (LPCONNECTIONPOINTCONTAINER FAR*);
    HRESULT _IFUNC Advise (LPUNKNOWN, DWORD FAR*);
    HRESULT _IFUNC Unadvise (DWORD);
    HRESULT _IFUNC EnumConnections (LPENUMCONNECTIONS FAR*);

  private:


    //\\//\\//\\//\\//\\//\\//\\//\\//\\//
    //      Connections list class
    //\\//\\//\\//\\//\\//\\//\\//\\//\\//

    class BSinkList : public IBSinkList,
                  public IEnumConnections {

      public:
        BSinkList (UINT = BOLE_DEFAULT_ENUM_SIZE);
        ~BSinkList ();

        //\\// IUnknown \\//\\
        ULONG _IFUNC AddRef ();
        ULONG _IFUNC Release ();
        HRESULT _IFUNC QueryInterface (REFIID, LPVOID FAR*);

        //\\// IBSinkList \\//\\
        HRESULT _IFUNC NextSink (LPVOID FAR*);
        HRESULT _IFUNC GetSinkAt (int, LPVOID FAR*);

        //\\// /IEnumConnections \\//\\
        HRESULT _IFUNC Next (ULONG, LPCONNECTDATA, ULONG FAR*);
        HRESULT _IFUNC Skip (ULONG);
        HRESULT _IFUNC Clone (LPENUMCONNECTIONS FAR*);

        //\\// IBSinkList & IEnumConnection \\//\\
        HRESULT _IFUNC Reset ();

        // public member functions
        inline IUnknown * operator [] (int nPos) { return pUnkList[nPos]; }
        inline int Size() { return nSize; }
        inline void SetFreeze (bool fF) { fFreeze = fF; }

        void SetSinkAt (IUnknown* pU, LPVOID pObj, int nPos);
        void DeleteSinkAt (int);
        // inline int Position() { return nCurrPos; } // should be unused now
        int Expand ();

      private:
        BSinkList (BSinkList*); // used by ::Clone function

      private:
        LPUNKNOWN FAR* pUnkList; // contains IUnknown
        LPVOID FAR* pDirectList; // contains real interface pointer
        int nCurrPos;
        int nSize;
        bool fFreeze;

        DWORD cRef;

    } * pSinkList; // sink list

    // Connection Point information
    REFIID EventIID; // event iid
    LPUNKNOWN pBackToCPC; // ConnectionPointContainer back pointer

    DWORD cRef; // lifetime counter
};

#endif


