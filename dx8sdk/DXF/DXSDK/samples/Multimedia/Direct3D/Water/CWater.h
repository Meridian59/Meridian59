//
// CWater
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//

#pragma once
#ifndef __CWATER_H__
#define __CWATER_H__


typedef struct WATER_REFRACT
{
    // Vrefract = (V + refract * N) * norm
    float fRefract;
    float fRefractNorm; 
    DWORD dwDiffuse;

} WATER_REFRACT;


typedef struct WATER_SURFACE
{
    float fHeight;
    float fVelocity;

} WATER_SURFACE;



class CWater
{
    FLOAT m_fSize;
    FLOAT m_fDepth;
    FLOAT m_fScaleTex;
    FLOAT m_fAvgHeight;

    FLOAT m_fSphereHeight;
    FLOAT m_fSphereRadius2;

    UINT m_uIndices;
    UINT m_uVertices;

    WATER_SURFACE *m_pSurface;
    WATER_REFRACT *m_pRefract;

    IDirect3DDevice8       *m_pDevice;
    IDirect3DIndexBuffer8  *m_pibIndices;
    IDirect3DVertexBuffer8 *m_pvbVertices;
    IDirect3DVertexBuffer8 *m_pvbCaustics;

public:
    CWater();
   ~CWater();

    HRESULT Initialize(float fSize, float fDepth);

    HRESULT OnCreateDevice(IDirect3DDevice8 *pDevice);
    HRESULT OnResetDevice();
    HRESULT OnLostDevice();
    HRESULT OnDestroyDevice();

    HRESULT Drop();
    HRESULT Update(D3DXVECTOR3 &vecPos, D3DXVECTOR3 &vecLight, BOOL bCalcCaustics);

    HRESULT DrawCaustics();
    HRESULT DrawSurface();
};

#endif // __CWATER_H__