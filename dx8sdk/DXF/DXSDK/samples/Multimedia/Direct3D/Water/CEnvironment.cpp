//
// CEnvironment
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//

#include <d3dx8.h>
#include "CEnvironment.h"



//////////////////////////////////////////////////////////////////////////////
// Types /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#pragma pack(4)
typedef struct ENV_VERTEX
{
    D3DXVECTOR3 m_vecPos;
    D3DXVECTOR2 m_vecTex;
} ENV_VERTEX;

#define ENV_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0))




//////////////////////////////////////////////////////////////////////////////
// CEnvironment implementation ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CEnvironment::CEnvironment()
{
    m_fSize = 1.0f;
    m_pDevice = NULL;

    memset(m_pSurf, 0x00, sizeof(m_pSurf));
}


CEnvironment::~CEnvironment()
{
}


HRESULT
CEnvironment::Initialize(float fSize)
{
    m_fSize = fSize;
    return S_OK;
}


HRESULT 
CEnvironment::OnCreateDevice(IDirect3DDevice8* pDevice)
{
    m_pDevice = pDevice;
    return S_OK;
}


HRESULT
CEnvironment::OnResetDevice()
{
    return S_OK;
}


HRESULT
CEnvironment::OnLostDevice()
{
    return S_OK;
}


HRESULT
CEnvironment::OnDestroyDevice()
{
    return S_OK;
}


HRESULT 
CEnvironment::SetSurfaces(
    IDirect3DTexture8* pXNeg, IDirect3DTexture8* pXPos, 
    IDirect3DTexture8* pYNeg, IDirect3DTexture8* pYPos,
    IDirect3DTexture8* pZNeg, IDirect3DTexture8* pZPos)
{
    m_pSurf[0] = pXNeg;
    m_pSurf[1] = pXPos;
    m_pSurf[2] = pYNeg;
    m_pSurf[3] = pYPos;
    m_pSurf[4] = pZNeg;
    m_pSurf[5] = pZPos;

    return S_OK;
}


HRESULT
CEnvironment::Draw()
{
    float f;
    ENV_VERTEX vert[4];

    f = 0.5f / 512.0f;

    vert[0].m_vecTex = D3DXVECTOR2(0.0f + f, 0.0f + f);
    vert[1].m_vecTex = D3DXVECTOR2(0.0f + f, 1.0f - f);
    vert[2].m_vecTex = D3DXVECTOR2(1.0f - f, 0.0f + f);
    vert[3].m_vecTex = D3DXVECTOR2(1.0f - f, 1.0f - f);

    m_pDevice->SetVertexShader(ENV_FVF);
    f = m_fSize * 0.5f;


    // XNeg
    vert[0].m_vecPos = D3DXVECTOR3(-f,  f,  f);
    vert[1].m_vecPos = D3DXVECTOR3(-f, -f,  f);
    vert[2].m_vecPos = D3DXVECTOR3(-f,  f, -f);
    vert[3].m_vecPos = D3DXVECTOR3(-f, -f, -f);

    m_pDevice->SetTexture(0, m_pSurf[0]);
    m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID) vert, sizeof(ENV_VERTEX));


    // XPos
    vert[0].m_vecPos = D3DXVECTOR3( f,  f, -f);
    vert[1].m_vecPos = D3DXVECTOR3( f, -f, -f);
    vert[2].m_vecPos = D3DXVECTOR3( f,  f,  f);
    vert[3].m_vecPos = D3DXVECTOR3( f, -f,  f);

    m_pDevice->SetTexture(0, m_pSurf[1]);
    m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID) vert, sizeof(ENV_VERTEX));


    // YNeg
    vert[1].m_vecPos = D3DXVECTOR3(-f, -f, -f);
    vert[3].m_vecPos = D3DXVECTOR3(-f, -f,  f);
    vert[0].m_vecPos = D3DXVECTOR3( f, -f, -f);
    vert[2].m_vecPos = D3DXVECTOR3( f, -f,  f);

    m_pDevice->SetTexture(0, m_pSurf[2]);
    m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID) vert, sizeof(ENV_VERTEX));

    
    // YPos
    vert[0].m_vecPos = D3DXVECTOR3(-f,  f,  f);
    vert[1].m_vecPos = D3DXVECTOR3(-f,  f, -f);
    vert[2].m_vecPos = D3DXVECTOR3( f,  f,  f);
    vert[3].m_vecPos = D3DXVECTOR3( f,  f, -f);

    m_pDevice->SetTexture(0, m_pSurf[3]);
    m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID) vert, sizeof(ENV_VERTEX));


    // ZNeg
    vert[0].m_vecPos = D3DXVECTOR3(-f,  f, -f);
    vert[1].m_vecPos = D3DXVECTOR3(-f, -f, -f);
    vert[2].m_vecPos = D3DXVECTOR3( f,  f, -f);
    vert[3].m_vecPos = D3DXVECTOR3( f, -f, -f);

    m_pDevice->SetTexture(0, m_pSurf[4]);
    m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID) vert, sizeof(ENV_VERTEX));


    // ZPos
    vert[0].m_vecPos = D3DXVECTOR3( f,  f,  f);
    vert[1].m_vecPos = D3DXVECTOR3( f, -f,  f);
    vert[2].m_vecPos = D3DXVECTOR3(-f,  f,  f);
    vert[3].m_vecPos = D3DXVECTOR3(-f, -f,  f);

    m_pDevice->SetTexture(0, m_pSurf[5]);
    m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID) vert, sizeof(ENV_VERTEX));

    return S_OK;
}


