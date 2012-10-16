//-----------------------------------------------------------------------------
// File: EchoTool.h
//
// Desc: Implements an object based on IDirectMusicTool
//       that provides echoing effects.
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _ECHOTOOL_H
#define _ECHOTOOL_H

#include <dmusici.h>

// Maximum echoes is 4 (the number of extra groups opened
// on the port in helper.cpp)
#define MAX_ECHOES	4

class CEchoTool : public IDirectMusicTool
{
public:
	CEchoTool();
    ~CEchoTool();

public:
// IUnknown
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

// IDirectMusicTool
	HRESULT STDMETHODCALLTYPE Init( IDirectMusicGraph* pGraph );
	HRESULT STDMETHODCALLTYPE GetMsgDeliveryType( DWORD* pdwDeliveryType );
	HRESULT STDMETHODCALLTYPE GetMediaTypeArraySize( DWORD* pdwNumElements );
	HRESULT STDMETHODCALLTYPE GetMediaTypes( DWORD** padwMediaTypes, DWORD dwNumElements) ;
	HRESULT STDMETHODCALLTYPE ProcessPMsg( IDirectMusicPerformance* pPerf, DMUS_PMSG* pDMUS_PMSG );
	HRESULT STDMETHODCALLTYPE Flush( IDirectMusicPerformance* pPerf, DMUS_PMSG* pDMUS_PMSG, REFERENCE_TIME rt );
private:
	long	m_cRef;			    // Reference counter
	DWORD	m_dwEchoNum;	    // Number of echoes to generate
	MUSIC_TIME	m_mtDelay;	    // Delay time between echoes
	CRITICAL_SECTION m_CrSec;	// To make SetEchoNum() and SetDelay() thread-safe

public:
// Public class methods
	void	SetEchoNum( DWORD dwEchoNum );
	void	SetDelay( MUSIC_TIME mtDelay );
};

#endif // _ECHOTOOL_H
