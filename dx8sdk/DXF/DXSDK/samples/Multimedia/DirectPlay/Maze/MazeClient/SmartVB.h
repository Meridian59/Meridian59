//----------------------------------------------------------------------------
// File: SmartVB.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _SMARTVB_H
#define _SMARTVB_H


#include "DXUtil.h"


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
// Smart vertex buffer template class. This performs the usual NOOVERWRITE/DISCARDCONTENTS
// filling loop. Simply call Begin(), then multiple instances of MakeRoom() to be given
// pointers to where to store vertex and index data, then End() when done. The class
// automatically flushes rendering as the buffer fills.
//
// By default rendering is performed by calling DrawIndexedPrimitive with a TRIANGELIST
// consisting of the submitted data, but the client can supply a callback if more complex
// rendering is required (perhaps for multipass?).
typedef void (__stdcall *SmartVBRenderCallback)( LPDIRECT3DVERTEXBUFFER8 pVB,
                                                 DWORD dwStartVertex,
                                                 DWORD dwNumVertices,
                                                 LPDIRECT3DINDEXBUFFER8 pIB,
                                                 DWORD dwStartIndex,
                                                 DWORD dwIndexCount,
                                                 void* pParam );




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
template< class VertType, DWORD VertFVF, DWORD NumIndex > class SmartVB
{
public:
    //-----------------------------------------------------------------------------
    // Name: 
    // Desc:
    //-----------------------------------------------------------------------------
    SmartVB() : m_pVB(NULL), m_pd3dDevice(NULL), m_pIB(NULL)
    { 
        SetRenderCallback(); 
    };



    //-----------------------------------------------------------------------------
    // Name: 
    // Desc:
    //-----------------------------------------------------------------------------
    ~SmartVB()
    {
        Uninit();
    };



    //-----------------------------------------------------------------------------
    // Name: 
    // Desc:
    //-----------------------------------------------------------------------------
    HRESULT Init( IDirect3D8* pD3D, IDirect3DDevice8* pDevice, DWORD dwNumVerts )
    {
        HRESULT hr;

        Uninit();

        m_dwNumVerts    = dwNumVerts;
        m_pd3dDevice    = pDevice;

        if( m_pd3dDevice )
            m_pd3dDevice->AddRef();
        else
            return E_INVALIDARG;

        m_pNextIndex    = NULL;
        m_dwNextIndex   = 0;
        if( FAILED( hr = m_pd3dDevice->CreateIndexBuffer( NumIndex * sizeof(WORD),
                                                     D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, 
                                                     D3DPOOL_DEFAULT, &m_pIB ) ) )
            return hr;

        m_pNextVert     = NULL;
        m_dwNextVert    = 0;
        DWORD dwFVFSize = D3DXGetFVFVertexSize( VertFVF );
        if( FAILED( hr = m_pd3dDevice->CreateVertexBuffer( dwFVFSize * dwNumVerts, 
                                                    D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, VertFVF, 
                                                    D3DPOOL_DEFAULT, &m_pVB ) ) )
            return hr;

        return S_OK;
    };



    //-----------------------------------------------------------------------------
    // Name: 
    // Desc:
    //-----------------------------------------------------------------------------
    void Uninit()
    {
        SAFE_RELEASE( m_pVB );
        SAFE_RELEASE( m_pIB );
        SAFE_RELEASE( m_pd3dDevice );
    };



    //-----------------------------------------------------------------------------
    // Name: 
    // Desc: Set rendering callback. Passing NULL for the callback gets you a default
    //       call to DrawIndexedPrimitive. pParam is passed through to the callback.
    //-----------------------------------------------------------------------------
    void    SetRenderCallback( SmartVBRenderCallback pCallback = NULL, void* pParam = NULL )
    {
        if ( pCallback != NULL )
        {
            m_pCallback      = pCallback;
            m_pCallbackParam = pParam;
        }
        else
        {
            m_pCallback      = DefaultRenderCallback;
            m_pCallbackParam = this;
        }
    };



    //-----------------------------------------------------------------------------
    // Name: 
    // Desc:
    //-----------------------------------------------------------------------------
    HRESULT Begin()
    {
        // Check we haven't already begun
        if ( m_pNextVert != NULL && m_pNextIndex != NULL )
            return E_FAIL;

        // Lock buffer, use D3DLOCK_DISCARD flag
        m_pVB->Lock( 0, 0, (BYTE**)&m_pNextVert, D3DLOCK_DISCARD );
        m_pNextVert     += m_dwNextVert;
        m_dwFirstVert   = m_dwNextVert;
        m_dwVertexCount = 0;

        m_pIB->Lock( 0, 0, (BYTE**)&m_pNextIndex, D3DLOCK_DISCARD );
        m_pNextIndex    += m_dwNextIndex;
        m_dwFirstIndex  = m_dwNextIndex;
        m_dwIndexCount  = 0;

        return S_OK;
    };



    //-----------------------------------------------------------------------------
    // Name: 
    // Desc: Request space to submit data, may cause a 'flush' for rendering
    //-----------------------------------------------------------------------------
    HRESULT MakeRoom( DWORD dwNumVert, DWORD dwNumIndex, VertType** pVertPtr,
                      WORD** pIndexPtr, WORD* wIndexOffset )
    {
        Flush();
        // Have we room left in the buffer?
        if ( (dwNumVert  + m_dwNextVert  >= m_dwNumVerts) ||
             (dwNumIndex + m_dwNextIndex >= NumIndex) )
        {
            // Nope, so flush current batch
            Flush();

            // Check we could fit this in at all
            if ( (dwNumVert > m_dwNumVerts) || (dwNumIndex > NumIndex) )
                return E_INVALIDARG;
        }

        // Got room, so just return position in buffer
        *pVertPtr       = m_pNextVert;
        *pIndexPtr      = m_pNextIndex;
        *wIndexOffset   = WORD(m_dwVertexCount);

        // Update position
        m_pNextVert     += dwNumVert;
        m_dwNextVert    += dwNumVert;
        m_dwVertexCount += dwNumVert;

        m_pNextIndex    += dwNumIndex;
        m_dwNextIndex   += dwNumIndex;
        m_dwIndexCount  += dwNumIndex;

        // Done
        return S_OK;
    };



    //-----------------------------------------------------------------------------
    // Name: 
    // Desc: End data filling, and submit for rendering via callback
    //-----------------------------------------------------------------------------
    void    End()
    {
        // Unlock VB
        m_pVB->Unlock();
        m_pIB->Unlock();

        // Submit for rendering
        if ( m_dwVertexCount > 0 && m_dwIndexCount > 0 )
            m_pCallback( m_pVB, m_dwFirstVert, m_dwVertexCount, 
                         m_pIB, m_dwFirstIndex, m_dwIndexCount, 
                         m_pCallbackParam );

        m_pNextVert  = NULL;
        m_pNextIndex = NULL;
    };



    //-----------------------------------------------------------------------------
    // Name: 
    // Desc: Flush data if we overflowed
    //-----------------------------------------------------------------------------
    void    Flush()
    {
        // Unlock VB
        m_pVB->Unlock();
        m_pIB->Unlock();

        // Submit for rendering
        if ( m_dwVertexCount > 0 && m_dwIndexCount > 0 )
            m_pCallback( m_pVB, m_dwFirstVert, m_dwVertexCount, 
                         m_pIB, m_dwFirstIndex, m_dwIndexCount, 
                         m_pCallbackParam );

        // Lock VB again
        m_pVB->Lock( 0, 0, (BYTE**)&m_pNextVert, D3DLOCK_DISCARD );
        m_dwFirstVert   = 0;
        m_dwVertexCount = 0;
        m_dwNextVert    = 0;

        m_pIB->Lock( 0, 0, (BYTE**)&m_pNextIndex, D3DLOCK_DISCARD );
        m_dwFirstIndex  = 0;
        m_dwIndexCount  = 0;
        m_dwNextIndex   = 0;
    };

protected:
    IDirect3DDevice8*       m_pd3dDevice;
    DWORD                   m_dwNumVerts;

    SmartVBRenderCallback   m_pCallback;
    void*                   m_pCallbackParam;

    // Current position of 'write cursor' in buffer
    LPDIRECT3DINDEXBUFFER8  m_pIB;
    WORD*                   m_pNextIndex;
    DWORD                   m_dwNextIndex;
    DWORD                   m_dwFirstIndex;

    LPDIRECT3DVERTEXBUFFER8 m_pVB;
    VertType*               m_pNextVert;
    DWORD                   m_dwNextVert;
    DWORD                   m_dwFirstVert;

    // Counts of vertices and indicies in the current batch
    DWORD                   m_dwVertexCount;
    DWORD                   m_dwIndexCount;

    static void __stdcall DefaultRenderCallback( LPDIRECT3DVERTEXBUFFER8 pVB,
                                                 DWORD dwStartVertex,
                                                 DWORD dwNumVertices,
                                                 LPDIRECT3DINDEXBUFFER8 pIB,
                                                 DWORD dwStartIndex,
                                                 DWORD dwIndexCount,
                                                 void* pParam )
    {
        ((SmartVB*)pParam)->m_pd3dDevice->SetVertexShader( VertFVF );
        ((SmartVB*)pParam)->m_pd3dDevice->SetStreamSource( 0, pVB, sizeof(VertType) );
        ((SmartVB*)pParam)->m_pd3dDevice->SetIndices( pIB, 0 );
        ((SmartVB*)pParam)->m_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, dwStartVertex, 
                                                                dwNumVertices, dwStartIndex, dwIndexCount/3 );
    };

private:
    SmartVB( const SmartVB& );
    void operator=( const SmartVB& );
};




#endif
