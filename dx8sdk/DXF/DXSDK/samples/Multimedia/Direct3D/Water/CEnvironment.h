//
// CEnvironment
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//

#pragma once
#ifndef __CENVIRONMENT_H__
#define __CENVIRONMENT_H__


class CEnvironment
{
public:
    CEnvironment();
   ~CEnvironment();

    HRESULT Initialize(float fSize);

    HRESULT OnCreateDevice(IDirect3DDevice8* pDevice);
    HRESULT OnResetDevice();
    HRESULT OnLostDevice();
    HRESULT OnDestroyDevice();

    HRESULT SetSurfaces(
        IDirect3DTexture8* pXNeg, IDirect3DTexture8* pXPos, 
        IDirect3DTexture8* pYNeg, IDirect3DTexture8* pYPos,
        IDirect3DTexture8* pZNeg, IDirect3DTexture8* pZPos); 
        
    HRESULT Draw();

protected:
    float m_fSize;

    IDirect3DDevice8* m_pDevice;
    IDirect3DTexture8* m_pSurf[6];
};

#endif // __CENVIRONMENT_H__