//------------------------------------------------------------------------------
// File: Contrast.h
//
// Desc: DirectShow sample code - class definition for simple 
//       contrast-adjusting transform filter.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


const signed char MaxContrastLevel = 127;
const signed char MinContrastLevel = -127;

class CContrast : public CTransformFilter,
		  public IContrast,
		  public ISpecifyPropertyPages
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    // Reveals IContrast & ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    DECLARE_IUNKNOWN;

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    // IContrast methods

    STDMETHODIMP get_ContrastLevel(signed char *ContrastLevel);
    STDMETHODIMP put_ContrastLevel(signed char ContrastLevel, unsigned long ChangeTime);
    STDMETHODIMP put_DefaultContrastLevel(void);

    // ISpecifyPropertyPages method

    STDMETHODIMP GetPages(CAUUID *pPages);

private:

    // Constructor
    CContrast(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

    // Look after the transformation of media samples
    BOOL CanChangeContrastLevel(const CMediaType *pMediaType) const;
    HRESULT Copy(IMediaSample *pSource, IMediaSample *pDest) const;
    HRESULT Transform(IMediaSample *pMediaSample);
    HRESULT Transform(AM_MEDIA_TYPE *pType, const signed char ContrastLevel) const;

    // Handle increasing and decreasing contrast
    void IncreaseContrast(RGBQUAD *pElem, const int Low, const int High, const float Grad) const;
    void IncreaseContrast(BYTE *pByte, const int Low, const int High, const float Grad) const;
    void DecreaseContrast(RGBQUAD *pElem, const int Level, const float Grad) const;
    void DecreaseContrast(BYTE *pByte, const int Level, const float Grad) const;

    // The default (no-change) level
    const signed char m_DefaultContrastLevel;

    // The current level to set the palette to
    signed char	m_ContrastLevel;	

    // The level the last frame's palette was set at
    signed char	m_PrevLevel;		

    // The number of buffers to request on the output allocator
    const long m_lBufferRequest;

    // Non interface locking critical section
    CCritSec m_ContrastLock;

}; // CContrast

