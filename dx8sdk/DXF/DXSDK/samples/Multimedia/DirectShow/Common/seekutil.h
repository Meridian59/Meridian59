//------------------------------------------------------------------------------
// File: SeekUtil.h
//
// Desc: DirectShow sample code - prototypes for seeking utility functions
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//
// Constants
//
const int TICKLEN=100, TIMERID=55;

//
// Function prototypes
//
HRESULT ConfigureSeekbar(IMediaSeeking *pMS, CSliderCtrl Seekbar, CStatic& strPosition);

void StartSeekTimer();
void StopSeekTimer();
void UpdatePosition(IMediaSeeking *pMS, REFERENCE_TIME rtNow, CStatic& strPosition);
void ReadMediaPosition(IMediaSeeking *pMS, CSliderCtrl& Seekbar, CStatic& strPosition);

void HandleTrackbar(IMediaControl *pMC, IMediaSeeking *pMS, 
                    CSliderCtrl& Seekbar, CStatic& strPosition, WPARAM wReq);
