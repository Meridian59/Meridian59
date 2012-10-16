//-----------------------------------------------------------------------------
// File: CWaterApp.cpp
//
// Desc: 
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <stdio.h>
#include <d3dx8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "DXUtil.h"
#include "CEnvironment.h"
#include "CWater.h"
#include "resource.h"

#define WATER_DEPTH 20.0f
#define WATER_CAUSTICS_SIZE 128



//-----------------------------------------------------------------------------
// Name: CMyD3DApplication
// Desc: 
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
public:
    CMyD3DApplication();
    HRESULT GetNextTechnique(INT nDir, BOOL bBypassValidate);

    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();

    virtual LRESULT MsgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


private:
    // Water    
    CWater                 m_Water;
    CEnvironment           m_Environment;

    LPD3DXEFFECT           m_pEffect;
    UINT_PTR                   m_iTechnique;
    LPD3DXRENDERTOSURFACE  m_pRenderToSurface;

    D3DXVECTOR4            m_vecLight;
    D3DXCOLOR              m_colorLight;

    LPDIRECT3DTEXTURE8     m_pFloorTex;
    LPDIRECT3DTEXTURE8     m_pCausticTex;
    LPDIRECT3DSURFACE8     m_pCausticSurf;
    LPDIRECT3DTEXTURE8     m_pSkyTex[6];
    LPDIRECT3DCUBETEXTURE8 m_pSkyCubeTex;

    CD3DFont*              m_pFont;
    CD3DFont*              m_pFontSmall;

    // Interface
    BYTE        m_bKey[256];

    BOOL        m_bPause;
    BOOL        m_bDrawWater;
    BOOL        m_bDrawCaustics;
    BOOL        m_bDrawEnvironment;
    BOOL        m_bShowHelp;

    FLOAT       m_fSpeed;
    FLOAT       m_fAngularSpeed;
    FLOAT       m_fTime;
    FLOAT       m_fSecsPerFrame;

    D3DXVECTOR3 m_vecVelocity;
    D3DXVECTOR3 m_vecAngularVelocity;

    D3DXMATRIX  m_matIdentity;
    D3DXMATRIX  m_matView;
    D3DXMATRIX  m_matPosition;
    D3DXMATRIX  m_matProjection;
};




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    CMyD3DApplication d3dApp;

    if(FAILED(d3dApp.Create(hInst)))
        return 0;

    return d3dApp.Run();
}




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication
// Desc:
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_strWindowTitle    = _T("Water");
    m_bUseDepthBuffer   = TRUE;

    m_pFont         = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pFontSmall    = new CD3DFont( _T("Arial"),  9, D3DFONT_BOLD );

    // Water
    m_pEffect       = NULL;
    m_iTechnique    = 0;

    m_vecLight      = D3DXVECTOR4(0.5f, -1.0f, 1.0f, 0.0f);
    m_colorLight    = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

    m_pFloorTex     = NULL;
    m_pCausticTex   = NULL;
    m_pCausticSurf  = NULL;
    m_pSkyCubeTex   = NULL;

    for(UINT i = 0; i < 6; i++)
        m_pSkyTex[i] = NULL;

    // Misc
    memset(m_bKey, 0x00, sizeof(m_bKey));

    m_fSpeed        = 25.0f;
    m_fAngularSpeed = 1.0f;
    m_fSecsPerFrame = 0.0f;
    m_fTime         = 0.0f;

    m_vecVelocity        = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_vecAngularVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    D3DXMatrixIdentity(&m_matIdentity);
    D3DXMatrixIdentity(&m_matView);
    D3DXMatrixIdentity(&m_matPosition);
    D3DXMatrixIdentity(&m_matProjection);

    m_bShowHelp         = FALSE;
    m_bPause            = FALSE;
}




//-----------------------------------------------------------------------------
// Name: GetNextTechnique
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::GetNextTechnique(INT nDir, BOOL bBypassValidate)
{
    D3DXEFFECT_DESC effect;
    UINT_PTR iTechnique = m_iTechnique;

    m_pEffect->GetDesc(&effect);

    for(;;)
    {
        iTechnique += nDir;

        if(((INT_PTR) iTechnique) < 0)
            iTechnique = effect.Techniques - 1;

        if(iTechnique >= effect.Techniques)
            iTechnique = 0;

        if(nDir && (iTechnique == m_iTechnique))
            break;

        if(!nDir)
            nDir = 1;


        m_pEffect->SetTechnique((LPCSTR) iTechnique);

        if(bBypassValidate || (iTechnique == effect.Techniques - 1) || 
          (m_bDrawCaustics || !m_pEffect->IsParameterUsed("tCAU")) && SUCCEEDED(m_pEffect->Validate()))
        {
            m_iTechnique = iTechnique;

            char szText[256];
            sprintf(szText, "Water - Technique %d", m_iTechnique);
            SetWindowText(m_hWnd, szText);

            return S_OK;
        }
    }

    m_pEffect->SetTechnique((LPCSTR) m_iTechnique);
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    HRESULT hr;

    // Initialize Water
    if(FAILED(hr = m_Water.Initialize(64.0f, WATER_DEPTH)))
        return hr;

    // Initialize Environment
    if(FAILED(hr = m_Environment.Initialize(1000.0f)))
        return hr;

    // Misc stuff
    D3DXMatrixRotationX(&m_matPosition, D3DX_PI * -0.3f);
    m_matPosition._42 = 15.0f;
    m_matPosition._43 = 15.0f;

    D3DXMatrixInverse(&m_matView, NULL, &m_matPosition);
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    HRESULT hr;
    TCHAR sz[512];

    m_bDrawWater        = TRUE;
    m_bDrawCaustics     = TRUE;
    m_bDrawEnvironment  = TRUE;


    // Initialize the font's internal textures
    m_pFont->InitDeviceObjects( m_pd3dDevice );
    m_pFontSmall->InitDeviceObjects( m_pd3dDevice );

    // Floor
    DXUtil_FindMediaFile(sz, _T("Water.bmp"));
    D3DXCreateTextureFromFileEx(m_pd3dDevice, sz, D3DX_DEFAULT, D3DX_DEFAULT, 
        D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, 
        D3DX_DEFAULT, 0, NULL, NULL, &m_pFloorTex);


    // Sky
    TCHAR* szSkyTex[6] =
    {
        _T("lobbyxpos.bmp"), _T("lobbyxneg.bmp"),
        _T("lobbyypos.bmp"), _T("lobbyyneg.bmp"),
        _T("lobbyzneg.bmp"), _T("lobbyzpos.bmp")
    };

    for(UINT i = 0; i < 6; i++)
    {
        DXUtil_FindMediaFile(sz, szSkyTex[i]);
        D3DXCreateTextureFromFileEx(m_pd3dDevice, sz, D3DX_DEFAULT, D3DX_DEFAULT, 
            1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 
            0, NULL, NULL, &m_pSkyTex[i]);
    }

    if(SUCCEEDED(D3DXCreateCubeTexture(m_pd3dDevice, 128, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, &m_pSkyCubeTex)))
    {
        for(UINT i = 0; i < 6; i++)
        {
            if(m_pSkyTex[i])
            {
                IDirect3DSurface8 *pSrc;
                IDirect3DSurface8 *pDest;

                m_pSkyTex[i]->GetSurfaceLevel(0, &pSrc);
                m_pSkyCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES) i, 0, &pDest);

                if(pSrc && pDest)
                    D3DXLoadSurfaceFromSurface(pDest, NULL, NULL, pSrc, NULL, NULL, D3DX_DEFAULT, 0);

                SAFE_RELEASE(pDest);
                SAFE_RELEASE(pSrc);
            }
        }

        D3DXFilterCubeTexture(m_pSkyCubeTex, NULL, 0, D3DX_DEFAULT);
    }



    // OnCreateDevice
    if(FAILED(hr = m_Water.OnCreateDevice(m_pd3dDevice)))
        return hr;

    if(FAILED(hr = m_Environment.OnCreateDevice(m_pd3dDevice)))
        return hr;

    return S_OK;
}



    
//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    HRESULT hr;

    // Restore the font
    m_pFont->RestoreDeviceObjects();
    m_pFontSmall->RestoreDeviceObjects();

    // Create light
    D3DLIGHT8 light;
    ZeroMemory(&light, sizeof(light));

    light.Type        = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r   = m_colorLight.r;
    light.Diffuse.g   = m_colorLight.g;
    light.Diffuse.b   = m_colorLight.b;
    light.Diffuse.a   = m_colorLight.a;
    light.Specular.r  = 1.0f;
    light.Specular.g  = 1.0f;
    light.Specular.b  = 1.0f;
    light.Specular.a  = 0.0f;
    light.Direction.x = m_vecLight.x;
    light.Direction.y = m_vecLight.y;
    light.Direction.z = m_vecLight.z;

    m_pd3dDevice->SetLight(0, &light);
    m_pd3dDevice->LightEnable(0, TRUE);


    // Create material
    D3DMATERIAL8 material;
    ZeroMemory(&material, sizeof(material));

    material.Diffuse.a  = 1.0f;
    material.Specular.r = 0.5f;
    material.Specular.g = 0.5f;
    material.Specular.b = 0.5f;
    material.Power      = 20.0f;

    m_pd3dDevice->SetMaterial(&material);

    // Setup render states
    m_pd3dDevice->SetVertexShader(D3DFVF_XYZ);

    m_pd3dDevice->SetTransform(D3DTS_VIEW,  &m_matView);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_matIdentity);

    m_pd3dDevice->SetRenderState(D3DRS_LIGHTING,       FALSE);
    m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);

    m_pd3dDevice->SetRenderState(D3DRS_ZFUNC,     D3DCMP_LESSEQUAL);
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE,  D3DCULL_CW);
    m_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
    m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_DISABLE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

    m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
    m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);




    // Create caustic texture
    D3DDISPLAYMODE mode;
    m_pd3dDevice->GetDisplayMode(&mode);

    if(FAILED(hr = D3DXCreateTexture(m_pd3dDevice, WATER_CAUSTICS_SIZE, WATER_CAUSTICS_SIZE, 1, D3DUSAGE_RENDERTARGET, mode.Format, D3DPOOL_DEFAULT, &m_pCausticTex)) &&
       FAILED(hr = D3DXCreateTexture(m_pd3dDevice, WATER_CAUSTICS_SIZE, WATER_CAUSTICS_SIZE, 1, 0, mode.Format, D3DPOOL_DEFAULT, &m_pCausticTex)))
    {
        return hr;
    }

    D3DSURFACE_DESC desc;
    m_pCausticTex->GetSurfaceLevel(0, &m_pCausticSurf);
    m_pCausticSurf->GetDesc(&desc);

    if(FAILED(hr = D3DXCreateRenderToSurface(m_pd3dDevice, desc.Width, desc.Height, 
        desc.Format, FALSE, D3DFMT_UNKNOWN, &m_pRenderToSurface)))
    {
        return hr;
    }



    // Shader
    TCHAR sz[512];
    DXUtil_FindMediaFile(sz, _T("water.sha"));

    if(FAILED(hr = D3DXCreateEffectFromFile(m_pd3dDevice, sz, &m_pEffect, NULL)))
        return hr;

    m_pEffect->SetMatrix("mID",  &m_matIdentity);
    m_pEffect->SetMatrix("mENV", &m_matIdentity);

    m_pEffect->SetTexture("tFLR", m_pFloorTex);
    m_pEffect->SetTexture("tCAU", m_pCausticTex);
    m_pEffect->SetTexture("tENV", m_pSkyCubeTex);

    if(FAILED(hr = GetNextTechnique(0, FALSE)))
        return hr;


    // Set surfaces
    if(FAILED(hr = m_Environment.SetSurfaces(
        m_pSkyTex[D3DCUBEMAP_FACE_NEGATIVE_X], m_pSkyTex[D3DCUBEMAP_FACE_POSITIVE_X], 
        m_pSkyTex[D3DCUBEMAP_FACE_NEGATIVE_Y], m_pSkyTex[D3DCUBEMAP_FACE_POSITIVE_Y],
        m_pSkyTex[D3DCUBEMAP_FACE_POSITIVE_Z], m_pSkyTex[D3DCUBEMAP_FACE_NEGATIVE_Z])))
    {
        return hr;
    }


    // OnResetDevice
    if(FAILED(hr = m_Water.OnResetDevice()))
        return hr;

    if(FAILED(hr = m_Environment.OnResetDevice()))
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    HRESULT hr;

    m_pFont->InvalidateDeviceObjects();
    m_pFontSmall->InvalidateDeviceObjects();

    if(FAILED(hr = m_Water.OnLostDevice()))
        return hr;

    if(FAILED(hr = m_Environment.OnLostDevice()))
        return hr;

    SAFE_RELEASE(m_pRenderToSurface);
    SAFE_RELEASE(m_pEffect);
    SAFE_RELEASE(m_pCausticSurf);
    SAFE_RELEASE(m_pCausticTex);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    HRESULT hr;

    m_pFont->DeleteDeviceObjects();
    m_pFontSmall->DeleteDeviceObjects();

    if(FAILED(hr = m_Water.OnDestroyDevice()))
        return hr;

    if(FAILED(hr = m_Environment.OnDestroyDevice()))
        return hr;

    SAFE_RELEASE(m_pFloorTex);
    SAFE_RELEASE(m_pSkyCubeTex);

    for(UINT i = 0; i < 6; i++)
        SAFE_RELEASE(m_pSkyTex[i]);


    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    HRESULT hr;

    //
    // Process keyboard input
    //

    D3DXVECTOR3 vecT(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vecR(0.0f, 0.0f, 0.0f);

    if(m_bKey[VK_NUMPAD1] || m_bKey[VK_LEFT])  vecT.x -= 1.0f; // Slide Left
    if(m_bKey[VK_NUMPAD3] || m_bKey[VK_RIGHT]) vecT.x += 1.0f; // Slide Right
    if(m_bKey[VK_DOWN])                        vecT.y -= 1.0f; // Slide Down
    if(m_bKey[VK_UP])                          vecT.y += 1.0f; // Slide Up
    if(m_bKey['W'])                            vecT.z -= 2.0f; // Move Forward
    if(m_bKey['S'])                            vecT.z += 2.0f; // Move Backward
    if(m_bKey['A'] || m_bKey[VK_NUMPAD8])      vecR.x -= 1.0f; // Pitch Down
    if(m_bKey['Z'] || m_bKey[VK_NUMPAD2])      vecR.x += 1.0f; // Pitch Up
    if(m_bKey['E'] || m_bKey[VK_NUMPAD6])      vecR.y -= 1.0f; // Turn Right
    if(m_bKey['Q'] || m_bKey[VK_NUMPAD4])      vecR.y += 1.0f; // Turn Left
    if(m_bKey[VK_NUMPAD9])                     vecR.z -= 2.0f; // Roll CW
    if(m_bKey[VK_NUMPAD7])                     vecR.z += 2.0f; // Roll CCW

    m_vecVelocity = m_vecVelocity * 0.9f + vecT * 0.1f;
    m_vecAngularVelocity = m_vecAngularVelocity * 0.9f + vecR * 0.1f;



    //
    // Update position and view matricies
    //

    D3DXMATRIX matT, matR;
    D3DXQUATERNION qR;

    vecT = m_vecVelocity * m_fElapsedTime * m_fSpeed;
    vecR = m_vecAngularVelocity * m_fElapsedTime * m_fAngularSpeed;

    D3DXMatrixTranslation(&matT, vecT.x, vecT.y, vecT.z);
    D3DXMatrixMultiply(&m_matPosition, &matT, &m_matPosition);

    D3DXQuaternionRotationYawPitchRoll(&qR, vecR.y, vecR.x, vecR.z);
    D3DXMatrixRotationQuaternion(&matR, &qR);

    D3DXMatrixMultiply(&m_matPosition, &matR, &m_matPosition);
    D3DXMatrixInverse(&m_matView, NULL, &m_matPosition);


    //
    // Update simulation
    //

    if(!m_bPause && m_bDrawWater)
    {
        BOOL bCaustics = m_bDrawCaustics && m_pEffect->IsParameterUsed("tCAU");
        D3DXVECTOR3 vecPos(m_matPosition._41, m_matPosition._42, m_matPosition._43);
        D3DXVECTOR3 vecLight(0.0f, 1.0f, 0.0f);

        m_Water.Update(vecPos, vecLight, bCaustics);
        m_fTime += m_fSecsPerFrame;

        if(bCaustics)
        {
            if(SUCCEEDED(m_pRenderToSurface->BeginScene(m_pCausticSurf, NULL)))
            {
                D3DXMATRIX matProj;
                D3DXMATRIX matView;

                D3DXMatrixOrthoRH(&matProj, 63.0f, 63.0f, 1.0f, 100.0f);
                D3DXMatrixRotationX(&matView, 0.5f * D3DX_PI);
                matView._43 = -50.0f;

                m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
                m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

                m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 0.0f, 0);

                m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
                m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
                m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

                m_Water.DrawCaustics();

                m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
                m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
                m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

                m_pRenderToSurface->EndScene();
            }
            else
            {
                m_bDrawCaustics = FALSE;
                m_pEffect->SetTexture("tCAU", NULL);

                if(FAILED(hr = GetNextTechnique(0, FALSE)))
                    return hr;
            }
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{   
    HRESULT hr;

    if(FAILED(hr = m_pd3dDevice->BeginScene()))
        return hr;

    // Draw Environment
    FLOAT fAspectRatio = (FLOAT)m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovRH(&m_matProjection, D3DXToRadian(60.0f), fAspectRatio, 0.1f, 2000.0f);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProjection);    

    if(m_bDrawEnvironment)
    {
        D3DXMATRIX mat(m_matView);
        mat._41 = mat._42 = mat._43 = 0.0f;
        m_pd3dDevice->SetTransform(D3DTS_VIEW, &mat);

        m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

        m_Environment.Draw();

        m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    }
    else
    {
        m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
    }

    m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_matView);

    // Draw water
    if(m_bDrawWater)
    {
        // Setup matrices
        if(m_pEffect->IsParameterUsed("mENV"))
        {
            D3DXMATRIX matP(m_matPosition);
            matP._41 = matP._42 = matP._43 = 0.0f;

            D3DXMATRIX mat;
            D3DXMatrixScaling(&mat, 1.0f, 1.0f, -1.0f);

            D3DXMatrixMultiply(&mat, &matP, &mat);

            // matCube
            m_pEffect->SetMatrix("mENV", &mat);
        }

        // Draw water
        UINT uPasses;
        m_pEffect->Begin(&uPasses, 0);

        for(UINT uPass = 0; uPass < uPasses; uPass++)
        {
            m_pEffect->Pass(uPass);
            m_Water.DrawSurface();
        }

        m_pEffect->End();
    }

    // Show info
    m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
    m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

    TCHAR szText[100];
    wsprintf( szText, _T("Using Technique %d"), m_iTechnique );
    m_pFontSmall->DrawText( 2, 40, D3DCOLOR_ARGB(255,255,100,100), szText );
    
    if( m_bShowHelp )
    {
        m_pFontSmall->DrawText(  2, 60, D3DCOLOR_ARGB(255,100,100,200),
                                _T("Keyboard controls:") );
        m_pFontSmall->DrawText( 20, 80, D3DCOLOR_ARGB(255,100,100,200),
                                _T("Add Drop\n")
                                _T("Next Technique\n")
                                _T("Next Tech. (no validate)\n")
                                _T("Prev Technique\n")
                                _T("Prev Tech. (no validate)\n")
                                _T("Move\nTurn\nPitch\nSlide\n")
                                _T("Help\nChange device\nExit") );
        m_pFontSmall->DrawText( 210, 80, D3DCOLOR_ARGB(255,100,100,200),
                                _T("D\n")
                                _T("PageDn\nShift-PageDn\n")
                                _T("PageUp\nShift-PageUp\n")
                                _T("W,S\nE,Q\nA,Z\nArrow keys\n")
                                _T("F1\nF2\nEsc") );
    }
    else
    {
        m_pFontSmall->DrawText(  2, 60, D3DCOLOR_ARGB(255,100,100,200), 
                           _T("Press F1 for help") );
    }


    if(FAILED(hr = m_pd3dDevice->EndScene()))
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: MsgProc
// Desc:
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, 
                                    LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_KEYDOWN:
            m_bKey[wParam] = TRUE;
            break;

        case WM_KEYUP:
            m_bKey[wParam] = FALSE;
            break;

    case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
            case IDM_ADDDROP:
                m_Water.Drop();
                break;

            case IDM_NEXT_TECHNIQUE:
                GetNextTechnique(1, FALSE);
                break;

            case IDM_NEXT_TECHNIQUE_NOVALIDATE:
                GetNextTechnique(1, TRUE);
                break;

            case IDM_PREV_TECHNIQUE:
                GetNextTechnique(-1, FALSE);
                break;

            case IDM_PREV_TECHNIQUE_NOVALIDATE:
                GetNextTechnique(-1, TRUE);
                break;

            case IDM_TOGGLEHELP:
                m_bShowHelp = !m_bShowHelp;
                break;
            }
        }

    }

    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}


