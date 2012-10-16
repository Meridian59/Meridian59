//------------------------------------------------------------------------------
// File: Demonstration.h
//
// Desc: DirectShow sample code
//       Header file and class description for CDemonstration, 
//       "special effects" module to represent capabilities of VMR.
//       This class is called from CVMRMixDlg by the "Play" button.
//       CDemonstration contains CVMRCore member (VMR engine), through which 
//       it performs initialization of the graph builder and presentation.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef HDR_DEMONSTRATION
#define HDR_DEMONSTRATION

#include "stdafx.h"

class CVMRMixDlg;

class CDemonstration
{
private:
    CVMRMixDlg *    m_pDlg;             // 'parent' dialog
    char            m_szMsg[MAX_PATH];  // auxiliary string for debug messages
    bool            m_bInitialized;     // true if graph is build and files are rendered;
                                        // false otherwise
    CMediaList      m_MList;            // media list to be played
    CVMRCore *      m_pCore;            // 'main' class that implements VMR management
    VMRALPHABITMAP  m_sBmpParams;       // used in IVMRMixerBitmap; parameters 
                                        // of the bitmap overlapping the video
    HBITMAP         m_hbmp;             // handle to the bitmap overlapping the video

    HRESULT SetAlphaBitmapColorKey(UINT ImageID);   // set color key for overlapping bitmap
    HRESULT GetValidVMRBitmap(UINT ImageID);        // create bitmap compatible with renderer's settings

public:
    CDemonstration( CVMRMixDlg * pDlg,      // 'parent' dialog
                    CMediaList * pMajorList,// media list to play 
                    int nSize,              // size of pMajorList
                    HRESULT *phr) :         
        m_pDlg(pDlg),
        m_hbmp(NULL),
        m_pCore(NULL),
        m_bInitialized(false)
    {
        ASSERT(phr);
        ASSERT( nSize > 0);

        strcpy( m_szMsg, "");

        *phr = S_OK;

        ZeroMemory( &m_sBmpParams, sizeof(VMRALPHABITMAP));
        srand( clock());
        
        pMajorList->Shuffle();
        if( false == pMajorList->Clone(nSize, &m_MList))
        {
            *phr = E_INVALIDARG;
            return;
        }
    };
    virtual ~CDemonstration()
    {
        if( m_pCore )
        {
            delete m_pCore;
        }
        if( m_hbmp)
        {
            DeleteObject( m_hbmp );
        }
        if( m_sBmpParams.hdc )
        {
            DeleteDC( m_sBmpParams.hdc );
        }
        m_MList.Clean();
    };

    HRESULT Initialize(); 
    HRESULT UpdateStreams(clock_t tStart);
    virtual HRESULT Perform();

    // this function calculates parameters for the 'swirling windows' effect
    void FountainPath(  long t, 
                        long T, 
                        int n, 
                        NORMALIZEDRECT r0,  
                        double A0, 
                        NORMALIZEDRECT * pR, 
                        double * pA);
};

#endif