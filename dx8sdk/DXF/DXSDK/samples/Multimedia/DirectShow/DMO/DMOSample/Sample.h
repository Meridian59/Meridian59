//------------------------------------------------------------------------------
// File: Sample.h
//
// Desc: DirectShow sample code - definition of CSample class.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __SAMPLE_H_
#define __SAMPLE_H_


// df20ddfa-0d19-463a-ab46-e5d8ef6efd69
DEFINE_GUID(CLSID_Sample,
			0xdf20ddfa, 0x0d19, 0x463a, 0xab, 0x46, 0xe5, 0xd8, 0xef, 0x6e, 0xfd, 0x69);

/////////////////////////////////////////////////////////////////////////////
// CSample
class ATL_NO_VTABLE CSample :
    public IMediaObjectImpl<CSample, 1, 2>,        // 1 input, 2 outputs
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSample, &CLSID_Sample>
	
{
public:
	CSample()
	{
		m_pUnkMarshaler = NULL;
	}


DECLARE_REGISTRY_RESOURCEID(IDR_SAMPLE)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSample)
	COM_INTERFACE_ENTRY(IMediaObject)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
        //  Make sure we clean up
        FreeStreamingResources();
		m_pUnkMarshaler.Release();
	}

	//  IMediaObjectImpl callbacks
    HRESULT InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags);
    HRESULT InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags);
	HRESULT InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt);
    HRESULT InternalCheckOutputType(DWORD dwOutputStreamIndex, const DMO_MEDIA_TYPE *pmt);
    HRESULT InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex,
                             DMO_MEDIA_TYPE *pmt);
    HRESULT InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex,
                             DMO_MEDIA_TYPE *pmt);
    HRESULT InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize,
                             DWORD *pcbMaxLookahead, DWORD *pcbAlignment);
    HRESULT InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize,
                              DWORD *pcbAlignment);
    HRESULT InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency);
    HRESULT InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency);
    HRESULT InternalFlush();
    HRESULT InternalDiscontinuity(DWORD dwInputStreamIndex);
    HRESULT InternalAllocateStreamingResources();
    HRESULT InternalFreeStreamingResources();
    HRESULT InternalProcessInput(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer,
                                DWORD dwFlags, REFERENCE_TIME rtTimestamp,
                                REFERENCE_TIME rtTimelength);
    HRESULT InternalProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount,
                                DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
    	                       DWORD *pdwStatus);
	HRESULT InternalAcceptingInput(DWORD dwInputStreamIndex);


    //  Internal processing routine
    HRESULT Process();

	CComPtr<IUnknown> m_pUnkMarshaler;

    //  Streaming locals
    CComPtr<IMediaBuffer>  m_pBuffer;
    BYTE *                 m_pbData;
    DWORD                  m_cbData;

	//  Fabricate timestamps based on the average time per from if there isn't one in the stream
	REFERENCE_TIME         m_rtFrame;

    //  Current state info
	CStreamState           m_StreamState;
    bool                   m_bPicture;

public:
};

#endif //__SAMPLE_H_
