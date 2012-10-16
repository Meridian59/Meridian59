// ---------------------------------------------------------------------------
// File: CDVGraph.h
// 
// Desc: CDVGraph Class declaration, it supports DV Graph Building
//       This is the base class to build all AVC graphs using 
//       MSTape.sys
//      
// Copyright (c) 2000-2001, Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#define SAFE_RELEASE(pObject) if(pObject){ pObject->Release(); pObject = NULL;}

#define DVENCODER_WIDTH             720
#define PAL_DVENCODER_HEIGHT        576
#define NTSC_DVENCODER_HEIGHT       480


//track device mode or active 
enum DV_MODE
{
    CameraMode  = 0L,
    VcrMode     = 1L,
    UnknownMode = 2L
}; 

enum GRAPH_TYPE
{
    GRAPH_PREVIEW, 
    GRAPH_DV_TO_FILE, 
    GRAPH_DV_TO_FILE_NOPRE, 
    GRAPH_FILE_TO_DV, 
    GRAPH_FILE_TO_DV_NOPRE, 
    GRAPH_DV_TO_FILE_TYPE2, 
    GRAPH_DV_TO_FILE_NOPRE_TYPE2, 
    GRAPH_FILE_TO_DV_TYPE2, 
    GRAPH_FILE_TO_DV_NOPRE_TYPE2
};

class CDVGraph 
{
public:

    // variables
    // Basic DirectShow Interfaces needed for Graph Building
    IGraphBuilder           *m_pGraph;
    ICaptureGraphBuilder2   *m_pCaptureGraphBuilder;

    IMediaControl           *m_pMediaControl;
    IMediaEventEx           *m_pMediaEvent;
    IBaseFilter             *m_pDeviceFilter;
    IBaseFilter             *m_pInputFileFilter;  

    IVideoWindow            *m_pVideoWindow;
    IAMDroppedFrames        *m_pDroppedFrames;

    IAMExtDevice            *m_pIAMExtDevice;
    IAMExtTransport         *m_pIAMExtTransport;
    IAMTimecodeReader       *m_pIAMTCReader;

    TCHAR                   m_DeviceName[_MAX_PATH];
    // State maintaining member variables
    DV_MODE                 m_SubunitMode; // vcr or camera
    _DVENCODERVIDEOFORMAT   m_VideoFormat; //pal or ntsc
    LONG                    m_AvgTimePerFrame;              
    _DVRESOLUTION           m_DVResolution; //  resolution of DV decoder
    GRAPH_TYPE              m_iGraphType;
    
    // member functions
    //constructor & destructor
    CDVGraph(void);
    ~CDVGraph(void);
    
    // Graph Building Helper Methods
    HRESULT BuildBasicGraph(void);
    HRESULT GetTapeInfo(void);
    HRESULT StopGraph(void);
    HRESULT PauseGraph(void);
    HRESULT StartGraph(void);

    HRESULT MakePreviewGraph(void);
    
    // Type 1 File (capture\playback\transmit)
    HRESULT MakeDvToFileGraph_Type1(TCHAR*        OutputFileName);
    HRESULT MakeDvToFileGraph_NoPre_Type1(TCHAR*  OutputFileName);
    HRESULT MakeFileToDvGraph_Type1(TCHAR*        InputFileName);
    HRESULT MakeFileToDvGraph_NoPre_Type1(TCHAR*  InputFileName);
    
    // Type 2 File (capture\playback\transmit)
    HRESULT MakeDvToFileGraph_Type2(TCHAR*        OutputFileName);
    HRESULT MakeDvToFileGraph_NoPre_Type2(TCHAR*        OutputFileName);
    HRESULT MakeFileToDvGraph_Type2(TCHAR*  InputFileName);
    HRESULT MakeFileToDvGraph_NoPre_Type2(TCHAR*  InputFileName);

    HRESULT getDroppedFrameNum(BOOL *bIsModeTransmit, long* pDropped, long* pNotdropped);
    HRESULT ChangeFrameRate( BOOL bHalfFrameRate );
    HRESULT GetVideoWindowDimensions(int *pWidth, int *pHeight, BOOL bChangeResolution,HWND hwndApp);
    HRESULT SeekATN(int iHr, int iMn, int iSc, int iFr);
    HRESULT GetDVMode(DV_MODE *pSubunitMode );   
    HRESULT SaveGraphToFile(TCHAR* pFileName);                                                          
    HRESULT NukeFilters(IBaseFilter *pFilter, BOOL bNukeDownStream);

private:
    // variables

    // member functions
    void FreeFilters();
    HRESULT InitializeGraph(void);
    HRESULT AddDeviceFilter(void);
    HRESULT GetResolutionFromDVDecoderPropertyPage(HWND hwndApp, BOOL bChangeResolution);
    HRESULT SetAviOptions(IBaseFilter *ppf, InterleavingMode INTERLEAVE_MODE);

};

