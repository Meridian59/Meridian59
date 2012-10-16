//----------------------------------------------------------------------------
//  File:   Utils.h
//
//  Desc:   DirectShow sample code
//          Prototypes for external (global) utilities 
//          specific for VMRXcl app
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------

// global headers
#if !defined(UTILS_H)
#define UTILS_H

// helper function prototypes

DWORD MyMessage(char *sQuestion, char *sTitle);
const char * hresultNameLookup(HRESULT hres);
bool MySleep(DWORD  dwTime = 2500);

void ReportPixelFormat( DDPIXELFORMAT ddpf);
void ReportDDSCAPS2( DDSCAPS2 ddscaps );
void ReportDDrawSurfDesc( DDSURFACEDESC2 ddsd);
void PixelFormatHelper( DWORD dwFlags, char * pszFlags);
void SurfaceDescHelper( DWORD dwFlags, char * pszFlags );

//----------------------------------------------------------------------------
//  SceneSettings
//  
//  This structure defines demonstration settings
//----------------------------------------------------------------------------
struct SceneSettings
{
    bool    bRotateZ;           // rotate around Z axis if true
    int     nGradZ;             // angle of rotation around Z axis, in grades
                                // this is a 'time' variable for Z-rotation effect
    bool    bRotateY;           // rotate around Y axis if true
    int     nGradY;             // angle of rotation around Y axis, in grades
                                // this is a 'time' variable for Y-rotation effect
    bool    bShowStatistics;    // show FPS in the upper right corner if true
    bool    bShowTwist;         // show "twist" effect if true; THIS STATE OVERRIDES bRotateZ AND bRotateY
    int     nDy;                // vertical offset for the twist effect
                                // this is a 'time' variable for "twist" effect
    bool    bShowHelp;          // show text help hints if true (it is activated by right click on the control)
    int     nXHelp;             // coordinates for text help hint
    int     nYHelp;
    TCHAR   achHelp[MAX_PATH];  // help hint text
    TCHAR   achFPS[MAX_PATH];   // string representation of FramesPerSecons 
                                // (when bShowStatistics is on)

    SceneSettings() // constructor: no effects by default
    {
        bRotateZ = false;
        nGradZ = 0;
        bRotateY = false;
        nGradY = 0;
        bShowStatistics = false;
        bShowTwist = false;
        nDy = 0;
        bShowHelp = 0;
        nXHelp = 0;
        nYHelp = 0;
        lstrcpy(achHelp, TEXT(""));
    };

    ~SceneSettings()
    {
    };
};

#endif