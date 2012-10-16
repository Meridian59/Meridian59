//
// CWater
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//

#include <d3dx8.h>
#include "CWater.h"

#define WATER_SHIFT 6
#define WATER_SIZE  (1 << WATER_SHIFT)
#define WATER_AREA  (WATER_SIZE * WATER_SIZE)
#define WATER_MASK  (WATER_SIZE - 1)

#define WATER_SPHERE_HEIGHT   20.0f
#define WATER_SPHERE_RADIUS2  (35.0f * 35.0f)

#define WATER_INDEX(x, y) \
    ((x) | ((y) << WATER_SHIFT))

#define WATER_INDEX_WRAP(x, y) \
    (((x) & WATER_MASK) | (((y) & WATER_MASK) << WATER_SHIFT))


#if defined(_X86) && !defined(_WIN64)
inline int f2i(float flt) 
{
	volatile int n; 

	__asm 
	{
		fld flt
		fistp n
	}

	return n;
}
#else
inline int f2i(float flt) 
{
	return (int) flt;
}
#endif



//////////////////////////////////////////////////////////////////////////////
// Types /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#pragma pack(1)
typedef struct WATER_VERTEX
{
    D3DXVECTOR3 m_vecPos;
    D3DXVECTOR3 m_vecNormal;
    D3DCOLOR    m_dwDiffuse;
    D3DXVECTOR2 m_vecTex;

} WATER_VERTEX;

#define WATER_FVF   (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0))


typedef struct CAUSTICS_VERTEX
{
    D3DXVECTOR3 m_vecPos;
    D3DCOLOR    m_dwDiffuse;

} CAUSTICS_VERTEX;

#define CAUSTICS_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#pragma pack()


//////////////////////////////////////////////////////////////////////////////
// CWater implementation /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CWater::CWater()
{
    m_fDepth    = 0.0f;
    m_fScaleTex = 1.0f;

    m_uIndices  = 0;
    m_uVertices = 0;

    m_pRefract     = NULL;
    m_pSurface     = NULL;

    m_pDevice      = NULL;
    m_pibIndices   = NULL;
    m_pvbVertices  = NULL;
    m_pvbCaustics  = NULL;
}


CWater::~CWater()
{
    if(m_pSurface)
        delete [] m_pSurface;
}


HRESULT
CWater::Initialize(float fSize, float fDepth)
{
    m_fSize = fSize;
    m_fDepth = fDepth;
    m_fScaleTex = 1.0f / fSize;

    // Calculate number of vertices and indices
    m_uVertices = WATER_AREA;
    m_uIndices  = m_uVertices * 2;



    // Create refraction table
    static WATER_REFRACT Refract[512];

    if(!m_pRefract)
    {
        m_pRefract = &Refract[256];

        for(UINT u = 0; u < 256; u++)
        {        
            float fCos0 = (float) u / (float) 256.0f;
            float f0 = acosf(fCos0);
            float fSin0 = sinf(f0);

            float fSin1 = fSin0 / 1.333f; // water
            float f1 = asinf(fSin1);
            float fCos1 = cosf(f1);
        
            m_pRefract[u].fRefract = fSin0 / fSin1 * fCos1 - fCos0;
            m_pRefract[u].fRefractNorm = - fSin1 / fSin0;
            m_pRefract[u].dwDiffuse = ((((0xff - u)*(0xff - u)*(0xff - u)) << 8) & 0xff000000);

            Refract[u] = Refract[256];
        }
    }



    // Create maps
    if(!m_pSurface)
    {
        if(!(m_pSurface = new WATER_SURFACE[WATER_AREA]))
            return E_OUTOFMEMORY;

        memset(m_pSurface, 0x00, WATER_AREA * sizeof(WATER_SURFACE));
    }

    return S_OK;
}



HRESULT
CWater::OnCreateDevice(IDirect3DDevice8 *pDevice)
{
    m_pDevice = pDevice;
    return S_OK;
}


HRESULT
CWater::OnResetDevice()
{
    HRESULT hr;

    // Create indices
    if(!m_pibIndices)
    {
        WORD *pwIndices;

        if(FAILED(hr = m_pDevice->CreateIndexBuffer(m_uIndices * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pibIndices)))
            return hr;

        if(FAILED(hr = m_pibIndices->Lock(0, m_uIndices * sizeof(WORD), (BYTE**) &pwIndices, 0)))
            return hr;


        // Fill in indicies
        UINT uX = 0, uZ = 0;
        WORD *pwIndex = pwIndices;

        for(UINT uSize = WATER_SIZE; uSize != 0; uSize -= 2)
        {
            UINT u;

            // Top
            for(u = 0; u < uSize; u++)
            {
                *pwIndex++ = uX + uZ * WATER_SIZE;
                *pwIndex++ = uX + uZ * WATER_SIZE + WATER_SIZE;
                uX++;
            }

            uX--;
            uZ++;


            // Right
            for(u = 1; u < uSize; u++)
            {
                *pwIndex++ = uX + uZ * WATER_SIZE;
                *pwIndex++ = uX + uZ * WATER_SIZE - 1;
                uZ++;
            }

            uZ--;
            uX--;


            // Bottom
            for(u = 1; u < uSize; u++)
            {
                *pwIndex++ = uX + uZ * WATER_SIZE;
                *pwIndex++ = uX + uZ * WATER_SIZE - WATER_SIZE;
                uX--;
            }

            uX++;
            uZ--;


            // Left
            for(u = 2; u < uSize; u++)
            {
                *pwIndex++ = uX + uZ * WATER_SIZE;
                *pwIndex++ = uX + uZ * WATER_SIZE + 1;
                uZ--;
            }

            uZ++;
            uX++;
        }

        for(pwIndex = pwIndices; pwIndex < pwIndices + m_uIndices; pwIndex++)
        {
            if(*pwIndex >= m_uVertices)
                *pwIndex = 0;
        }

        m_pibIndices->Unlock();
    }

    
    // Create vertices
    if(!m_pvbVertices)
    {
        if(FAILED(hr = m_pDevice->CreateVertexBuffer(m_uVertices * sizeof(WATER_VERTEX), 
            D3DUSAGE_DYNAMIC, WATER_FVF, D3DPOOL_DEFAULT, &m_pvbVertices)))
        {
            return hr;
        }
    }


    // Create caustics
    if(!m_pvbCaustics)
    {
        if(FAILED(hr = m_pDevice->CreateVertexBuffer(m_uVertices * sizeof(CAUSTICS_VERTEX), 
            D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, CAUSTICS_FVF, D3DPOOL_DEFAULT, &m_pvbCaustics)))
        {
            return hr;
        }
    }

    return S_OK;
}

HRESULT
CWater::OnLostDevice()
{
    if(m_pibIndices)
    {
        m_pibIndices->Release();
        m_pibIndices = NULL;
    }

    if(m_pvbVertices)
    {
        m_pvbVertices->Release();
        m_pvbVertices = NULL;
    }

    if(m_pvbCaustics)
    {
        m_pvbCaustics->Release();
        m_pvbCaustics = NULL;
    }

    return S_OK;
}

HRESULT
CWater::OnDestroyDevice()
{
    m_pDevice = NULL;
    return S_OK;
}



HRESULT
CWater::Drop()
{
    UINT uX = rand();
    UINT uY = rand();

    m_pSurface[WATER_INDEX_WRAP(uX, uY)].fVelocity -= 0.25f;

    m_pSurface[WATER_INDEX_WRAP(uX - 1, uY)].fVelocity -= 0.125f;
    m_pSurface[WATER_INDEX_WRAP(uX + 1, uY)].fVelocity -= 0.125f;
    m_pSurface[WATER_INDEX_WRAP(uX, uY - 1)].fVelocity -= 0.125f;
    m_pSurface[WATER_INDEX_WRAP(uX, uY + 1)].fVelocity -= 0.125f;

    m_pSurface[WATER_INDEX_WRAP(uX - 1, uY - 1)].fVelocity -= 0.0625f;
    m_pSurface[WATER_INDEX_WRAP(uX + 1, uY + 1)].fVelocity -= 0.0625f;
    m_pSurface[WATER_INDEX_WRAP(uX + 1, uY - 1)].fVelocity -= 0.0625f;
    m_pSurface[WATER_INDEX_WRAP(uX - 1, uY + 1)].fVelocity -= 0.0625f;

    return S_OK;
}


HRESULT
CWater::Update(D3DXVECTOR3 &vecPos, D3DXVECTOR3 &vecLight, BOOL bCalcCaustics)
{
    HRESULT hr;
    UINT uXN, uX, uXP, uY, uYN, uY0, uYP;


    // Compute desired height
    m_fAvgHeight = 0.0f;
    WATER_SURFACE *pSurface = m_pSurface;



    uYN  = WATER_AREA - WATER_SIZE;
    uY0  = 0;
    uYP  = WATER_SIZE;

    do
    {
        uXN  = WATER_SIZE - 1;
        uX   = 0;
        uXP  = 1;

        do
        {
            // Bowl
            float fX = (float) uX - (WATER_SIZE >> 1);
            float fZ = (float) (uY0 >> WATER_SHIFT) - (WATER_SIZE >> 1);
            float fDesire;

            if((fX * fX + fZ * fZ) < (WATER_SPHERE_RADIUS2 -  WATER_SPHERE_HEIGHT * WATER_SPHERE_HEIGHT))
            {
                fDesire = 
                     (m_pSurface[uXN + uYN].fHeight + 
                      m_pSurface[uXP + uYN].fHeight + 
                      m_pSurface[uXN + uYP].fHeight + 
                      m_pSurface[uXP + uYP].fHeight) * (1.0f / 12.0f)
                      +
                     (m_pSurface[uX  + uYN].fHeight +
                      m_pSurface[uXN + uY0].fHeight +
                      m_pSurface[uXP + uY0].fHeight +
                      m_pSurface[uX  + uYP].fHeight) * (2.0f / 12.0f);
            }
            else
            {
                fDesire = 0.0f;
                pSurface->fHeight = 0.0f;
                pSurface->fVelocity = 0.0f;
            }


            // Update velocity
            if(pSurface->fVelocity > 0.01f || pSurface->fVelocity  < -0.01f)
                pSurface->fVelocity *= 0.99f;

            pSurface->fVelocity += 0.25f * (fDesire - pSurface->fHeight);
            m_fAvgHeight += pSurface->fHeight + pSurface->fVelocity;

            pSurface++;

            uXN = uX;
            uX  = uXP;
            uXP = (uXP + 1) & WATER_MASK;
        }
        while(uX);

        uYN = uY0;
        uY0 = uYP;
        uYP = (uYP + WATER_SIZE) & (WATER_MASK << WATER_SHIFT);
    }
    while(uY0);

    m_fAvgHeight /= (float) m_uVertices;




    // Calculate surface normals
    WATER_VERTEX *pVertices, *pVertex, *pVertexLim;

    D3DXVECTOR3 vec;
    D3DXVECTOR3 vecP, vecN;

    if(FAILED(hr = m_pvbVertices->Lock(0, m_uVertices * sizeof(WATER_VERTEX), (BYTE **) &pVertices, 0)))
        return hr;
    
    pVertex = pVertices;
    pVertexLim = pVertex + m_uVertices;
    pSurface = m_pSurface;

    float fInc = m_fSize / (float) (WATER_SIZE - 1);
    float fZ = m_fSize * -0.5f;

    uYN  = WATER_AREA - WATER_SIZE;
    uY0  = 0;
    uYP  = WATER_SIZE;

    do
    {
        float fX = m_fSize * -0.5f;

        uXN  = WATER_SIZE - 1;
        uX   = 0;
        uXP  = 1;

        do
        {
            // Update position and normal
            vecP.x = fX;
            vecP.y = pSurface->fHeight = pSurface->fHeight + pSurface->fVelocity - m_fAvgHeight;
            vecP.z = fZ;

            float f;
            f = m_pSurface[uXN + uYN].fHeight - m_pSurface[uXP + uYP].fHeight; vecN.x = vecN.z = f;           
            f = m_pSurface[uX  + uYN].fHeight - m_pSurface[uX  + uYP].fHeight; vecN.z += f;
            f = m_pSurface[uXP + uYN].fHeight - m_pSurface[uXN + uYP].fHeight; vecN.x -= f; vecN.z += f;
            f = m_pSurface[uXN + uY0].fHeight - m_pSurface[uXP + uY0].fHeight; vecN.x += f;

            vecN.y = 1.0f;
            D3DXVec3Normalize(&vecN, &vecN);

            pSurface++;


            // Update texture coords and diffuse based upon refraction
            D3DXVec3Subtract(&vec, &vecPos, &vecP);
            D3DXVec3Normalize(&vec, &vec);

            WATER_REFRACT *pRefract;
            pRefract = m_pRefract + f2i(D3DXVec3Dot(&vec, &vecN) * 255.0f);

            pVertex->m_vecPos = vecP;
            pVertex->m_vecNormal = vecN;
            pVertex->m_dwDiffuse = pRefract->dwDiffuse;


            // Bowl
            D3DXVECTOR3 vecD;
            vecD = (vecN * pRefract->fRefract + vec) * pRefract->fRefractNorm;
            vecP.y -= WATER_SPHERE_HEIGHT;

            float fC = D3DXVec3Dot(&vecP, &vecP) - WATER_SPHERE_RADIUS2;

            if(fC < 0.0f)
            {
                float fB = D3DXVec3Dot(&vecD, &vecP) * 2.0f;
                float fD = fB * fB - 4.0f * fC;
                float fScale = (-fB + sqrtf(fD)) * 0.5f;

                pVertex->m_vecTex.x = (vecD.x * fScale + vecP.x) * m_fScaleTex + 0.5f;
                pVertex->m_vecTex.y = (vecD.z * fScale + vecP.z) * m_fScaleTex + 0.5f;
            }
            else
            {
                pVertex->m_vecTex.x = vecP.x * m_fScaleTex + 0.5f;
                pVertex->m_vecTex.y = vecP.z * m_fScaleTex + 0.5f;
            }


            pVertex++;
            fX += fInc;

            uXN = uX;
            uX  = uXP;
            uXP = (uXP + 1) & WATER_MASK;
        }
        while(uX);

        fZ += fInc;

        uYN = uY0;
        uY0 = uYP;
        uYP = (uYP + WATER_SIZE) & (WATER_MASK << WATER_SHIFT);
    }
    while(uY0);



    // Calculate caustics
    if(bCalcCaustics)
    {
        CAUSTICS_VERTEX *pCaustics, *pCaustic;

        if(FAILED(hr = m_pvbCaustics->Lock(0, m_uVertices * sizeof(CAUSTICS_VERTEX), (BYTE **) &pCaustics, 0)))
            return hr;


        #define TABLE_SIZE 8
        static DWORD Table[TABLE_SIZE];
        if(!Table[0])
        {
            for(UINT u = 0; u < TABLE_SIZE; u++)
                Table[u] = (0x40 / (u + 1)) * 0x00010101;
        }


        pVertex = pVertices;
        pCaustic = pCaustics;

        for(uY = 0; uY < WATER_SIZE; uY++)
        {
            for(uX = 0; uX < WATER_SIZE; uX++)
            {
                WATER_REFRACT *pRefract;
                pRefract = m_pRefract + f2i(pVertex->m_vecNormal.y * 255.0f);


                // Bowl
                D3DXVECTOR3 vecD, vecP;
                vecD = (pVertex->m_vecNormal * pRefract->fRefract + vecLight) * pRefract->fRefractNorm;
                vecP = pVertex->m_vecPos;
                vecP.y -= WATER_SPHERE_HEIGHT;

                float fC = D3DXVec3Dot(&vecP, &vecP) - WATER_SPHERE_RADIUS2;

                if(fC < 0.0f)
                {
                    float fB = D3DXVec3Dot(&vecD, &vecP) * 2.0f;
                    float fD = fB * fB - 4.0f * fC;
                    float fScale = (-fB + sqrtf(fD)) * 0.5f;

                    pCaustic->m_vecPos.x = vecD.x * fScale + vecP.x;
                    pCaustic->m_vecPos.y = 0.0f;
                    pCaustic->m_vecPos.z = vecD.z * fScale + vecP.z;
                }
                else
                {
                    pCaustic->m_vecPos.x = vecP.x;
                    pCaustic->m_vecPos.y = 0.0f;
                    pCaustic->m_vecPos.z = vecP.z;
                }


                if(uX && uY)
                {
                    float fArea;
                    fArea = (pCaustic[-WATER_SIZE - 1].m_vecPos.x - pCaustic->m_vecPos.x) *
                            (pCaustic[-WATER_SIZE    ].m_vecPos.z - pCaustic->m_vecPos.z) -
                            (pCaustic[-WATER_SIZE - 1].m_vecPos.z - pCaustic->m_vecPos.z) *
                            (pCaustic[-WATER_SIZE    ].m_vecPos.x - pCaustic->m_vecPos.x);

                    if(fArea < 0.0f)
                        fArea = -fArea;

                    UINT u = f2i(fArea * fArea * 4.0f);
                    pCaustic->m_dwDiffuse = u < TABLE_SIZE ? Table[u] : 0;
                }

                pCaustic++;
                pVertex++;
            }

            pCaustic[-WATER_SIZE].m_dwDiffuse = pCaustic[-1].m_dwDiffuse;
        }


        for(uX = 0; uX < WATER_SIZE; uX++)
        {
            pCaustics[uX].m_dwDiffuse = pCaustics[uX + (WATER_AREA - WATER_SIZE)].m_dwDiffuse;
        }

        m_pvbCaustics->Unlock();
    }


    m_pvbVertices->Unlock();
    return S_OK;
}

    
HRESULT 
CWater::DrawCaustics()
{
    HRESULT hr;

    if(FAILED(hr = m_pDevice->SetVertexShader(CAUSTICS_FVF)))
        return hr;

    if(FAILED(hr = m_pDevice->SetStreamSource(0, m_pvbCaustics, sizeof(CAUSTICS_VERTEX))))
        return hr;

    if(FAILED(hr = m_pDevice->SetIndices(m_pibIndices, 0)))
        return hr;

    if(FAILED(hr = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_uVertices, 0, m_uIndices -2)))
        return hr;

    return S_OK;
}

HRESULT
CWater::DrawSurface()
{
    HRESULT hr;

    if(FAILED(hr = m_pDevice->SetStreamSource(0, m_pvbVertices, sizeof(WATER_VERTEX))))
        return hr;

    if(FAILED(hr = m_pDevice->SetIndices(m_pibIndices, 0)))
        return hr;

    if(FAILED(hr = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_uVertices, 0, m_uIndices -2)))
        return hr;

    return S_OK;
}

