//------------------------------------------------------------------------------
// File: INull.h
//
// Desc: DirectShow sample code - custom interface to allow the user to
//       select media types.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __INULLIP__
#define __INULLIP__

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------------
// INullIP's GUID
//
// {43D849C0-2FE8-11cf-BCB1-444553540000}
DEFINE_GUID(IID_INullIPP,
0x43d849c0, 0x2fe8, 0x11cf, 0xbc, 0xb1, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// INullIPP
//----------------------------------------------------------------------------
DECLARE_INTERFACE_(INullIPP, IUnknown)
{

    STDMETHOD(put_MediaType) (THIS_
    				  CMediaType *pmt       /* [in] */	// the media type selected
				 ) PURE;

    STDMETHOD(get_MediaType) (THIS_
    				  CMediaType **pmt      /* [out] */	// the media type selected
				 ) PURE;

    STDMETHOD(get_IPin) (THIS_
    				  IPin **pPin          /* [out] */	// the source pin
				 ) PURE;


    STDMETHOD(get_State) (THIS_
    				  FILTER_STATE *state  /* [out] */	// the filter state
				 ) PURE;


};
//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __INULLIP__
