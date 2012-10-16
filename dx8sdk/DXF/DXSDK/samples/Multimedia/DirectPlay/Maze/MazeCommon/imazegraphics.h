//----------------------------------------------------------------------------
// File: imazegraphics.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _IMAZEGRAPHICS_
#define _IMAZEGRAPHICS_


class CMazeApp;
class CDPlay8Client;
class CMazeClient;
enum EnumLineType { LINE_PROMPT, LINE_INPUT, LINE_LOG, LINE_CMD };



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
interface IMazeGraphics
{
public:
    virtual VOID    Init( CMazeApp* pMazeApp, CDPlay8Client* pDP8Client, CMazeClient* pMazeClient ) = 0; 
    virtual HRESULT Create( HINSTANCE hInstance ) = 0;
    virtual INT     Run() = 0;
    virtual void    HandleOutputMsg( EnumLineType enumLineType, TCHAR* strLine ) = 0;
    virtual BOOL    IsPreview() = 0;
    virtual void    Shutdown() = 0;
};


#endif
