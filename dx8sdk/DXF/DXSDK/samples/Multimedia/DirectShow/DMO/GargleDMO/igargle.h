//------------------------------------------------------------------------------
// File: IGargle.h
//
// Desc: DirectShow sample code - custom interface to allow the user
//       to adjust the modulation rate.  It defines the interface between
//       the user interface component (the property sheet) and the filter
//       itself.  This interface is exported by the code in Gargle.cpp and
//       is used by the code in GargProp.cpp.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __IGARGLEDMO__
#define __IGARGLEDMO__

#ifdef __cplusplus
extern "C" {
#endif


//
// IGargleDMOSample's GUID
//
// {9AE9B11C-4EF8-42bc-9484-65EA0008964F}
DEFINE_GUID(IID_IGargleDMOSample, 
0x9ae9b11c, 0x4ef8, 0x42bc, 0x94, 0x84, 0x65, 0xea, 0x0, 0x8, 0x96, 0x4f);

typedef struct _GargleFX
{
    DWORD       dwRateHz;               // Rate of modulation in hz
    DWORD       dwWaveShape;            // GARGLE_FX_WAVE_xxx
} GargleFX, *LPGargleFX;

#define GARGLE_FX_WAVE_TRIANGLE        0
#define GARGLE_FX_WAVE_SQUARE          1

typedef const GargleFX *LPCGargleFX;

#define GARGLE_FX_RATEHZ_MIN           1
#define GARGLE_FX_RATEHZ_MAX           1000

//
// IGargleDMOSample
//
DECLARE_INTERFACE_(IGargleDMOSample, IUnknown) {

    // IGargleDMOSample methods
    STDMETHOD(SetAllParameters)     (THIS_ LPCGargleFX pcGargleFx) PURE;
    STDMETHOD(GetAllParameters)     (THIS_ LPGargleFX pGargleFx) PURE;

};


#ifdef __cplusplus
}
#endif

#endif // __IGARGLEDMO__
