//------------------------------------------------------------------------------
// File: IContrst.h
//
// Desc: DirectShow sample code - custom interface allowing the user
//       to adjust the contrast level.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __ICONTRAST__
#define __ICONTRAST__

#ifdef __cplusplus
extern "C" {
#endif

    // {fd5010a2-8ebe-11ce-8183-00aa00577da1}
    DEFINE_GUID(IID_IContrast,
    0xfd5010a2, 0x8ebe, 0x11ce, 0x81, 0x83, 0x00, 0xaa, 0x00, 0x57, 0x7d, 0xa1);

    DECLARE_INTERFACE_(IContrast, IUnknown)
    {
        STDMETHOD(get_ContrastLevel) (THIS_
            signed char *ContrastLevel      // The current contrast level
        ) PURE;

        STDMETHOD(put_ContrastLevel) (THIS_
            signed char ContrastLevel,      // Change to the contrast level
            unsigned long ChangeTime        // Time from now to do the change
        ) PURE;

        STDMETHOD(put_DefaultContrastLevel) (THIS) PURE;
    };

#ifdef __cplusplus
}
#endif

#endif // __ICONTRAST__

