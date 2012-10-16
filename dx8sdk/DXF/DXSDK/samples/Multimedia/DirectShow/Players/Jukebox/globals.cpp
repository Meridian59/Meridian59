//------------------------------------------------------------------------------
// File: Globals.h
//
// Desc: DirectShow sample code - global data for Jukebox application.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include <dshow.h>

#include "playvideo.h"

//
// Global data
//
IGraphBuilder *pGB = NULL;
IMediaSeeking *pMS = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IBasicVideo   *pBV = NULL;
IVideoWindow  *pVW = NULL;

FILTER_STATE g_psCurrent=State_Stopped;

BOOL g_bLooping=FALSE,
     g_bAudioOnly=FALSE,
     g_bDisplayEvents=FALSE,
     g_bGlobalMute=FALSE,
     g_bPlayThrough=FALSE;
