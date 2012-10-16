//-----------------------------------------------------------------------------
// File: tutorial2.cpp
//
// Desc: DirectMusic tutorial to show how to get an object from
//       an audiopath, and set the 3D position of a DirectMusic segment
//
// Copyright (c) 2000-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define INITGUID
#include <windows.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <cguid.h>




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
IDirectMusicLoader8*      g_pLoader         = NULL;
IDirectMusicPerformance8* g_pPerformance    = NULL;
IDirectMusicSegment8*     g_pSegment        = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Plays a single wave file using DirectMusic on the default audiopath.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    // Initialize COM
    CoInitialize(NULL);
    
    // Create loader object
    CoCreateInstance( CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, 
                      IID_IDirectMusicLoader8, (void**)&g_pLoader );

    // Create performance object
    CoCreateInstance( CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, 
                      IID_IDirectMusicPerformance8, (void**)&g_pPerformance );

    // This initializes both DirectMusic and DirectSound and 
    // sets up the synthesizer. 
    g_pPerformance->InitAudio( NULL, NULL, NULL, 
                               DMUS_APATH_DYNAMIC_STEREO, 64,
                               DMUS_AUDIOF_ALL, NULL );

    CHAR strPath[MAX_PATH];
    GetWindowsDirectory( strPath, MAX_PATH );
    strcat( strPath, "\\media" );

    // Tell DirectMusic where the default search path is
    WCHAR wstrSearchPath[MAX_PATH];
    MultiByteToWideChar( CP_ACP, 0, strPath, -1, 
                         wstrSearchPath, MAX_PATH );

	g_pLoader->SetSearchDirectory( GUID_DirectMusicAllTypes, 
	                               wstrSearchPath, FALSE );
    
    // Load the segment from the file
    WCHAR wstrFileName[MAX_PATH] = L"The Microsoft Sound.wav";   
    if( FAILED( g_pLoader->LoadObjectFromFile( CLSID_DirectMusicSegment,
                                               IID_IDirectMusicSegment8,
                                               wstrFileName,
                                               (LPVOID*) &g_pSegment ) ) )
    {
        MessageBox( NULL, "Media not found, sample will now quit", 
                          "DirectMusic Tutorial", MB_OK );
        return 0;
    }

    // Download the segment's instruments to the synthesizer
    g_pSegment->Download( g_pPerformance );

    // Tell DirectMusic to repeat this segment forever
    g_pSegment->SetRepeats( DMUS_SEG_REPEAT_INFINITE );

    // Create an 3D audiopath with a 3d buffer.
    // We can then play all segments into this buffer and directly control its
    // 3D parameters.
    IDirectMusicAudioPath8* p3DAudioPath = NULL;
    g_pPerformance->CreateStandardAudioPath( DMUS_APATH_DYNAMIC_3D, 
                                             64, TRUE, &p3DAudioPath );

    // Play segment on the 3D audiopath
    g_pPerformance->PlaySegmentEx( g_pSegment, NULL, NULL, 0, 
                                   0, NULL, NULL, p3DAudioPath );

    // Now DirectMusic will play in the backgroud, 
    // so continue on with our task
    MessageBox( NULL, "The music is now playing in center. " \
                "Click OK to pan music to left.", "DirectMusic Tutorial", MB_OK );

    // Get the IDirectSound3DBuffer8 from the 3D audiopath
    IDirectSound3DBuffer8* pDSB = NULL;
    p3DAudioPath->GetObjectInPath( DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, 
                                 GUID_NULL, 0, IID_IDirectSound3DBuffer, 
                                 (LPVOID*) &pDSB );

    // Set the position of sound a little to the left
    pDSB->SetPosition( -0.1f, 0.0f, 0.0f, DS3D_IMMEDIATE );

    // Wait for input
    MessageBox( NULL, "The music is now playing on the left. " \
                "Click OK to pan music to right.", "DirectMusic Tutorial", MB_OK );

    // Set the position of sound a little to the right
    pDSB->SetPosition( 0.1f, 0.0f, 0.0f, DS3D_IMMEDIATE );

    // Wait for input
    MessageBox( NULL, "The music is now playing on the right. " \
                "Click OK to exit.", "DirectMusic Tutorial", MB_OK );

    // Stop the music
    g_pPerformance->Stop( NULL, NULL, 0, 0 );

    // Cleanup all interfaces
    pDSB->Release();
    p3DAudioPath->Release();
    g_pLoader->Release(); 
    g_pSegment->Release();

    // Close down DirectMusic after releasing the DirectSound buffers
    g_pPerformance->CloseDown();
    g_pPerformance->Release();
    
    // Close down COM
    CoUninitialize();
        
    return 0;
}

