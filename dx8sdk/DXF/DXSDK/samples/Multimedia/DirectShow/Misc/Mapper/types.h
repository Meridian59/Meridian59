//------------------------------------------------------------------------------
// File: Types.h
//
// Desc: DirectShow sample code - an MFC based C++ filter mapper application.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//
// Merit for pins
//
typedef struct _meritinfo
{
    DWORD dwMerit;
    TCHAR szName[64];

} MERITINFO;

// Minimum merit must be specified, so there is no <Don't Care> entry
const MERITINFO merittypes[] = {

    MERIT_HW_COMPRESSOR,        TEXT("Hardware compressor"),
    MERIT_SW_COMPRESSOR,        TEXT("Software compressor"),
    MERIT_DO_NOT_USE,           TEXT("Do not use"),
    MERIT_UNLIKELY,             TEXT("Unlikely"),
    MERIT_NORMAL,               TEXT("Normal"),
    MERIT_PREFERRED,            TEXT("Preferred"),

};

#define NUM_MERIT_TYPES     (sizeof(merittypes) / sizeof(merittypes[0]))

//
// Media types
//
typedef struct _guidinfo
{
    const GUID *pGUID;
    TCHAR szName[64];

} GUIDINFO;

const GUIDINFO pintypes[] = {

    0                           ,TEXT("<Don't care>"),
    &PIN_CATEGORY_ANALOGVIDEOIN ,TEXT("Analog video in"),
    &PIN_CATEGORY_CAPTURE       ,TEXT("Capture"),
    &PIN_CATEGORY_CC            ,TEXT("Closed Captioning (Line21)"),
    &PIN_CATEGORY_EDS           ,TEXT("EDS (Line 21)"),
    &PIN_CATEGORY_NABTS         ,TEXT("NABTS"),
    &PIN_CATEGORY_PREVIEW       ,TEXT("Preview"),
    &PIN_CATEGORY_STILL         ,TEXT("Still"),
    &PIN_CATEGORY_TELETEXT      ,TEXT("Teletext (CC)"),
    &PIN_CATEGORY_TIMECODE      ,TEXT("Timecode"),
    &PIN_CATEGORY_VBI           ,TEXT("VBI"),
    &PIN_CATEGORY_VIDEOPORT     ,TEXT("VideoPort (connect to Overlay Mixer)"),
    &PIN_CATEGORY_VIDEOPORT_VBI ,TEXT("VideoPort VBI"),

};

#define NUM_PIN_TYPES       (sizeof(pintypes)   / sizeof(pintypes[0]))

const GUIDINFO majortypes[] = {

    0                           ,TEXT("<Don't care>"),  /* No selection */
    &MEDIATYPE_AnalogAudio      ,TEXT("Analog audio"), 
    &MEDIATYPE_AnalogVideo      ,TEXT("Analog video"),
    &MEDIATYPE_Audio            ,TEXT("Audio"),
    &MEDIATYPE_AUXLine21Data    ,TEXT("Line 21 data (CC)"),
    &MEDIATYPE_File             ,TEXT("File (CC)"),
    &MEDIATYPE_Interleaved      ,TEXT("Interleaved (DV)"),
    &MEDIATYPE_LMRT             ,TEXT("LMRT (Obsolete)"),
    &MEDIATYPE_Midi             ,TEXT("MIDI"),
    &MEDIATYPE_MPEG2_PES        ,TEXT("MPEG2 (DVD)"),
    &MEDIATYPE_ScriptCommand    ,TEXT("ScriptCommand (CC)"),
    &MEDIATYPE_Stream           ,TEXT("Byte stream (no time stamps)"),
    &MEDIATYPE_Text             ,TEXT("Text"),
    &MEDIATYPE_Timecode         ,TEXT("Timecode data"),
    &MEDIATYPE_URL_STREAM       ,TEXT("URL_STREAM (Obsolete)"),
    &MEDIATYPE_Video            ,TEXT("Video"),

};

#define NUM_MAJOR_TYPES     (sizeof(majortypes) / sizeof(majortypes[0]))

//
// Media subtypes
//
const GUIDINFO audiosubtypes[] = {

    &MEDIASUBTYPE_PCM           ,TEXT("PCM audio"), 
    &MEDIASUBTYPE_MPEG1Packet   ,TEXT("MPEG1 Audio Packet"), 
    &MEDIASUBTYPE_MPEG1Payload  ,TEXT("MPEG1 Audio Payload"), 
    0, 0
};

const GUIDINFO line21subtypes[] = {

    &MEDIASUBTYPE_Line21_BytePair       ,TEXT("BytePairs"),
    &MEDIASUBTYPE_Line21_GOPPacket      ,TEXT("DVD GOP Packet"),
    &MEDIASUBTYPE_Line21_VBIRawData     ,TEXT("VBI Raw Data"),
    0, 0
};

const GUIDINFO mpeg2subtypes[] = {

    &MEDIASUBTYPE_DVD_SUBPICTURE        ,TEXT("DVD Subpicture"),
    &MEDIASUBTYPE_DVD_LPCM_AUDIO        ,TEXT("DVD Audio (LPCM)"),
    &MEDIASUBTYPE_DOLBY_AC3             ,TEXT("Dolby AC3"),
    &MEDIASUBTYPE_MPEG2_AUDIO           ,TEXT("MPEG-2 Audio"),
    &MEDIASUBTYPE_MPEG2_TRANSPORT       ,TEXT("MPEG-2 Transport Stream"),
    &MEDIASUBTYPE_MPEG2_PROGRAM         ,TEXT("MPEG-2 Program Stream"),
    0, 0
};

const GUIDINFO streamsubtypes[] = {

    &MEDIASUBTYPE_AIFF              ,TEXT("AIFF"),
    &MEDIASUBTYPE_Asf               ,TEXT("ASF"),
    &MEDIASUBTYPE_Avi               ,TEXT("AVI"),
    &MEDIASUBTYPE_AU                ,TEXT("AU"),
    &MEDIASUBTYPE_DssAudio          ,TEXT("DSS Audio"),
    &MEDIASUBTYPE_DssVideo          ,TEXT("DSS Video"),
    &MEDIASUBTYPE_MPEG1Audio        ,TEXT("MPEG1 Audio"),
    &MEDIASUBTYPE_MPEG1System       ,TEXT("MPEG1 System"),
    // &MEDIASUBTYPE_MPEG1SystemStream ,TEXT("MPEG1 System Stream"),
    &MEDIASUBTYPE_MPEG1Video        ,TEXT("MPEG1 Video"),
    &MEDIASUBTYPE_MPEG1VideoCD      ,TEXT("MPEG1 VideoCD"),
    &MEDIASUBTYPE_WAVE              ,TEXT("Wave"),
    0, 0
};

const GUIDINFO videosubtypes[] = {

    &MEDIASUBTYPE_YVU9              ,TEXT("YVU9"),
    &MEDIASUBTYPE_Y411              ,TEXT("YUV 411"),
    &MEDIASUBTYPE_Y41P              ,TEXT("Y41P"),
    &MEDIASUBTYPE_YUY2              ,TEXT("YUY2"),
    &MEDIASUBTYPE_YVYU              ,TEXT("YVYU"),
    &MEDIASUBTYPE_UYVY              ,TEXT("UYVY"),
    &MEDIASUBTYPE_Y211              ,TEXT("YUV 211"),
    &MEDIASUBTYPE_CLJR              ,TEXT("Cirrus YUV 411"),
    &MEDIASUBTYPE_IF09              ,TEXT("Indeo YVU9"),
    &MEDIASUBTYPE_CPLA              ,TEXT("Cinepak UYVY"),
    &MEDIASUBTYPE_MJPG              ,TEXT("Motion JPEG"),
    &MEDIASUBTYPE_TVMJ              ,TEXT("TrueVision MJPG"),
    &MEDIASUBTYPE_WAKE              ,TEXT("MJPG (Wake)"),
    &MEDIASUBTYPE_CFCC              ,TEXT("MJPG (CFCC)"),
    &MEDIASUBTYPE_IJPG              ,TEXT("Intergraph JPEG"),
    &MEDIASUBTYPE_Plum              ,TEXT("Plum MJPG"),
    &MEDIASUBTYPE_RGB1              ,TEXT("RGB1 (Palettized)"),
    &MEDIASUBTYPE_RGB4              ,TEXT("RGB4 (Palettized)"),
    &MEDIASUBTYPE_RGB8              ,TEXT("RGB8 (Palettized)"),
    &MEDIASUBTYPE_RGB565            ,TEXT("RGB565"),
    &MEDIASUBTYPE_RGB555            ,TEXT("RGB555"),
    &MEDIASUBTYPE_RGB24             ,TEXT("RGB24"),
    &MEDIASUBTYPE_RGB32             ,TEXT("RGB32"),
    &MEDIASUBTYPE_ARGB32            ,TEXT("ARGB32"),
    &MEDIASUBTYPE_Overlay           ,TEXT("Overlay video (from HW)"),
    &MEDIASUBTYPE_QTMovie           ,TEXT("Apple QuickTime"),
    &MEDIASUBTYPE_QTRpza            ,TEXT("QuickTime RPZA"),
    &MEDIASUBTYPE_QTSmc             ,TEXT("QuickTime SMC"),
    &MEDIASUBTYPE_QTRle             ,TEXT("QuickTime RLE"),
    &MEDIASUBTYPE_QTJpeg            ,TEXT("QuickTime JPEG"),
    &MEDIASUBTYPE_dvsd              ,TEXT("Standard DV"),
    &MEDIASUBTYPE_dvhd              ,TEXT("High Definition DV"),
    &MEDIASUBTYPE_dvsl              ,TEXT("Long Play DV"),
    &MEDIASUBTYPE_MPEG1Packet       ,TEXT("MPEG1 Video Packet"),
    &MEDIASUBTYPE_MPEG1Payload      ,TEXT("MPEG1 Video Payload"),
    //&MEDIASUBTYPE_VideoPort         ,TEXT("Video Port (DVD)"),
    &MEDIASUBTYPE_VPVideo           ,TEXT("Video port video"),
    &MEDIASUBTYPE_VPVBI             ,TEXT("Video port VBI"),
    0, 0
};

const GUIDINFO analogvideosubtypes[] = {

    &MEDIASUBTYPE_AnalogVideo_NTSC_M   ,TEXT("(M) NTSC"),
    &MEDIASUBTYPE_AnalogVideo_PAL_B    ,TEXT("(B) PAL"),
    &MEDIASUBTYPE_AnalogVideo_PAL_D    ,TEXT("(D) PAL"),
    &MEDIASUBTYPE_AnalogVideo_PAL_G    ,TEXT("(G) PAL"),
    &MEDIASUBTYPE_AnalogVideo_PAL_H    ,TEXT("(H) PAL"),
    &MEDIASUBTYPE_AnalogVideo_PAL_I    ,TEXT("(I) PAL"),
    &MEDIASUBTYPE_AnalogVideo_PAL_M    ,TEXT("(M) PAL"),
    &MEDIASUBTYPE_AnalogVideo_PAL_N    ,TEXT("(N) PAL"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_B  ,TEXT("(B) SECAM"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_D  ,TEXT("(D) SECAM"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_G  ,TEXT("(G) SECAM"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_H  ,TEXT("(H) SECAM"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_K  ,TEXT("(K) SECAM"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_K1 ,TEXT("(K1) SECAM"),
    &MEDIASUBTYPE_AnalogVideo_SECAM_L  ,TEXT("(L) SECAM"),
    0, 0
};


const GUIDINFO *pSubTypes[] = {

    audiosubtypes,      // Analog audio
    analogvideosubtypes,// Analog video
    audiosubtypes,      // Audio
    line21subtypes,     // Line21 data
    NULL,               // File. Used by closed captions
    NULL,               // Interleaved.  Used by Digital Video (DV)
    NULL,               // Obsolete. Do not use.
    NULL,               // MIDI format
    mpeg2subtypes,      // MPEG-2.  Used by DVD.
    NULL,               // Script command, used by closed captions
    streamsubtypes,     // Byte stream with no time stamps
    NULL,               // Text
    NULL,               // Timecode data
    NULL,               // Obsolete.  Do not use.
    videosubtypes,      // Video
};

