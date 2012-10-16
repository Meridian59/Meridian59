//-----------------------------------------------------------------------------
// File: BumpSelfShadow.cpp
//
// Desc: Shows how to implement self-shadowing bump mapping
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <Windows.h>
#include <tchar.h>
#include <d3dx8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "DXUtil.h"

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------

// A complex vertex, if we use vertex shaders we can handle this data ourselves
struct BiNormalVertex
{
    FLOAT x,y,z;
    FLOAT u,v;
    FLOAT nx,ny,nz;
    FLOAT bx,by,bz;
    DWORD col;
    INT RefCount;
};

struct ShaderVertex
{
    FLOAT x,y,z;
    FLOAT u,v;
    FLOAT nx,ny,nz;
    FLOAT bx,by,bz;
};

struct NormalVertex
{
    FLOAT x,y,z;
    FLOAT u,v;
    FLOAT nx,ny,nz;
};


struct BasicVertex
{
    FLOAT x,y,z;
    DWORD col;
    FLOAT u,v;
};
#define FVF_BASICVERTEX D3DFVF_XYZ|D3DFVF_DIFFUSE| D3DFVF_TEX1 


//A BasicVertex FVF2
struct BasicVertex2
{
    FLOAT x,y,z;
    DWORD col;
    FLOAT u,v;
    FLOAT u1,v2;
};
#define FVF_BASICVERTEX2 D3DFVF_XYZ|D3DFVF_DIFFUSE| D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1)

#define MAXTRIANGLES 4096
#define BASISMAPSIZE 32
#define RENDERTARGSIZE 256

struct sTriangle
{
    WORD ind1,ind2,ind3;
};


class ShadowSet 
{
    sTriangle m_Triangles[MAXTRIANGLES];
    LPDIRECT3DVERTEXBUFFER8 m_pScreenVB;
    LPDIRECT3DVERTEXBUFFER8 m_pShad1;
    LPDIRECT3DVERTEXBUFFER8 m_pVertices;

    BiNormalVertex m_TriVertices[MAXTRIANGLES*3];
    
    INT m_iNumTriangles,m_iNumVertices;
    DWORD m_iHeightWidth,m_iHeightHeight;

    IDirect3DDevice8 *m_pDev;

    // our interleaved horizon maps with the precomputed basis textures
    LPDIRECT3DTEXTURE8 m_pHorizonTextures[4];
    LPDIRECT3DTEXTURE8 m_pBasisTextures[4];

    // we render into this buffer and use it as a texture
    LPDIRECT3DTEXTURE8 m_pRenderBuffer;
    LPDIRECT3DTEXTURE8 m_pRenderBuffer2;
    LPDIRECT3DTEXTURE8 m_pTempBuffer;

    LPDIRECT3DSURFACE8 m_pOldRenderTarget;
    LPDIRECT3DSURFACE8 m_pOldStencilZ;

    LPDIRECT3DTEXTURE8 m_pNormalMap;

    LPDIRECT3DTEXTURE8     m_pColorTexture;
    LPDIRECT3DINDEXBUFFER8 m_pIndexBuffer;

    D3DXMATRIX  m_matObject;
    D3DXMATRIX  m_matView;
    D3DXMATRIX  m_matProj;
    D3DXMATRIX  m_matTot;

    // our shaders
    DWORD m_dwBasisShader;
    DWORD m_dwShadowShader;
    DWORD m_dwBumpShader;
    DWORD m_dwPixShader;

    TCHAR m_cColorName[160];
    TCHAR m_cHeightName[160];
    DWORD m_dwHeightLevels;

    BOOL m_bInitalize;
    BOOL m_bLoaded;
    BOOL m_bUsePix;

    // scratch space for loading/computing the textures
    BYTE* m_pHeightMap[16];
    BYTE* m_pHorizonMap[16];
    BYTE* m_pBasisMap[16];
    BYTE* m_pHorizonComp[4];
    BYTE* m_pBasisComp[4];

    // linear basis map
    VOID BuildBasisMap(BYTE *pBasis,FLOAT PrimAngle,FLOAT Angle2);

    BOOL BuildInterleavedMap(LPDIRECT3DTEXTURE8 &newTex, BYTE *r, BYTE *g, BYTE *b,
                             BYTE *a, INT width, INT height);

    VOID BuildHorizonMap(BYTE *pHor, BYTE*, FLOAT dx, FLOAT dy, INT StepLength,
                         INT iWidth, INT iHeight, BOOL bWrapU, BOOL bWrapV);

public:
    ShadowSet(TCHAR *HeightMapName, TCHAR *ColorTexName);
    ShadowSet::~ShadowSet();

    VOID ComputeLight(D3DXVECTOR3 &lightDir, D3DXMATRIX *pObject, D3DXMATRIX *pView, D3DXMATRIX *pProj);
    VOID Render(BOOL,BOOL, BOOL, INT left, INT top, INT right, INT bottom);
    VOID SetBuffers(LPDIRECT3DINDEXBUFFER8 pIndex, LPDIRECT3DVERTEXBUFFER8 pVertBuffer,
                    INT iNumFaces, INT iNumVertices);
   
    HRESULT Init(IDirect3DDevice8*, BOOL bUsePix);
    VOID DeleteDeviceObj();
    VOID Restore();
    VOID Invalidate();
};




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
public:
    CMyD3DApplication();

    ShadowSet *m_pShadow;

    D3DXMATRIX  m_matObject;
    D3DXMATRIX  m_matView;
    D3DXMATRIX  m_matProj;
    D3DXMATRIX  m_matPosition;

    D3DXVECTOR3 m_LightDir;
    BOOL m_bLMouseDown;
    BOOL m_bRMouseDown;
    BOOL m_bFirstTime;
    BOOL m_bRotate;
    FLOAT m_xPos, m_yPos;
    FLOAT m_oPosX, m_oPosY;
    INT m_iIndex;

    BYTE  m_bKey[256];
    FLOAT m_fSpeed;
    FLOAT m_fAngularSpeed;
    FLOAT m_fTheta;
    FLOAT m_fTheta2;

    BOOL m_bDebug;
    BOOL m_bShadow;
    BOOL m_bBump;
    BOOL m_bUsePix;

    D3DXVECTOR3     m_vecVelocity;
    D3DXVECTOR3     m_vecAngularVelocity;

    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual LRESULT MsgProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual HRESULT ConfirmDevice(D3DCAPS8*,DWORD,D3DFORMAT);
    virtual HRESULT FinalCleanup();

private:
    FLOAT m_fAspectRatio;

    VOID CreateSphere();
    VOID OnMouseMove();
    VOID LoadXFile(TCHAR*);

    CD3DFont* m_pFont;
    CD3DFont* m_pFontSmall;
};



//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    CMyD3DApplication d3dApp;

    if( FAILED( d3dApp.Create( hInst ) ) )
        return 0;

    return d3dApp.Run();
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
    if( (dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
        (dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) )
    {
        if( pCaps->VertexShaderVersion < D3DVS_VERSION(1,0) )
            return E_FAIL;
    }
      
    if(!(pCaps->TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3))
        return E_FAIL;
    if(pCaps->MaxTextureBlendStages < 2)
        return E_FAIL;

    if( pCaps->PixelShaderVersion < D3DPS_VERSION(1,1) )
    {
        if( FAILED( m_pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, 
            pCaps->DeviceType, Format, D3DUSAGE_RENDERTARGET,
            D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8 ) ) )
        {
            return E_FAIL;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    D3DXMATRIX matT;
    m_fElapsedTime = 0.0f;
    m_fAspectRatio = 0.0f;

    m_dwCreationWidth   = 400;   // Width used to create window
    m_dwCreationHeight  = 400;

    m_bUseDepthBuffer   = TRUE;
    m_bShowCursorWhenFullscreen = TRUE;

    m_strWindowTitle    = _T("Self-Shadow Bumpmap");

    m_pShadow = NULL;
    m_pFont         = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pFontSmall    = new CD3DFont( _T("Arial"),  9, D3DFONT_BOLD );

    for(INT i=0; i<256; i++)
        m_bKey[i] = FALSE;

    m_bLMouseDown = FALSE;
    m_bRMouseDown = FALSE;
    m_bFirstTime  = TRUE;

    m_iIndex = 0;

    m_bDebug = m_bShadow = m_bBump = TRUE;
  
    m_fTheta             = 0;
    m_fTheta2            = 0;

    D3DXMatrixIdentity(&m_matObject);
    D3DXMatrixIdentity(&m_matView);
    D3DXMatrixIdentity(&m_matPosition);

    //initalize the camera and keyboard speeds
    //this should probally change depending on the size
    //of the object being viewed
    m_fSpeed             = 20.00f;
    m_fAngularSpeed      =  0.10f;
    m_vecVelocity        = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_vecAngularVelocity = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_LightDir           = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );

    D3DXMatrixTranslation( &matT, 0,0, -40);

    D3DXMatrixMultiply( &m_matPosition, &matT, &m_matPosition );
    D3DXMatrixInverse( &m_matView, NULL, &m_matPosition );
}




HRESULT CMyD3DApplication::FinalCleanup()
{
    SAFE_DELETE(m_pShadow);
    
    return S_OK;
}




//track mouse movement
//just a crude interface to let the user move the object
VOID CMyD3DApplication::OnMouseMove()
{
    RECT r;
    FLOAT dx,dy;
    D3DXMATRIX mat1,mat2,mat3;

    GetWindowRect(m_hWnd,&r);
    dx = ((FLOAT)m_xPos)/((FLOAT)r.right-r.left)*2 - 1;
    dy = ((FLOAT)m_yPos)/((FLOAT)r.right-r.left)*2 - 1;
    
    //right mouse button moves the light around
    if(m_bRMouseDown)
    {
        m_LightDir.x = -dx;
        m_LightDir.y = dy;
        m_LightDir.z = .25f;
    }
    
    //left button rotates the object
    if(m_bLMouseDown)
    {   
        m_fTheta  = -dy+m_oPosY;
        m_fTheta2 = -dx+m_oPosX;
    
        D3DXMatrixRotationX(&mat1,m_fTheta*.1f);
        D3DXMatrixRotationY(&mat2,m_fTheta2*.1f);
        D3DXMatrixMultiply(&mat3,&mat1,&mat2);
        D3DXMatrixMultiply(&m_matObject,&m_matObject,&mat3);
    }
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Message proc function to handle key and menu input
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT r;
    D3DXMATRIX matT;

    // Trap the context menu
    if( WM_CONTEXTMENU==uMsg )
        return 0;

    switch(uMsg)
    {
    case WM_KEYUP:
        //mark a key up
        m_bKey[wParam] = FALSE;
        break;

    case WM_KEYDOWN:
        //mark a key down
         m_bKey[wParam] = TRUE;
             
         //turn shadowing off
        if(wParam == '2')
            m_bShadow = !m_bShadow;

        //turn bump mapping off
        if(wParam == '3')
            m_bBump = !m_bBump;

        //auto rotate
        if(wParam == '5')
            m_bRotate = !m_bRotate;
    
        //reset matrices
        if(wParam == '4')
        {
            D3DXMatrixTranslation( &m_matPosition, 0,0, -40);
            D3DXMatrixInverse( &m_matView, NULL, &m_matPosition );
            D3DXMatrixIdentity(&m_matObject);
        }
       break;

       //track left mouse button down
    case WM_LBUTTONDOWN:
        m_bLMouseDown=TRUE;
        m_xPos = LOWORD(lParam);
        m_yPos = HIWORD(lParam);

        GetWindowRect(m_hWnd,&r);
        
        m_oPosX = ((FLOAT)m_xPos)/((FLOAT)r.right-r.left)*2 - 1;
        m_oPosY = ((FLOAT)m_yPos)/((FLOAT)r.right-r.left)*2 - 1;
        break;

        //track right mouse button 
    case WM_RBUTTONDOWN:
        m_xPos = LOWORD(lParam);
        m_yPos = HIWORD(lParam);
    
        m_bRMouseDown = TRUE;
        break;

        //track mouse movement
    case  WM_MOUSEMOVE:
        if(m_bRMouseDown || m_bLMouseDown)
        {
            m_xPos = LOWORD(lParam);
            m_yPos = HIWORD(lParam);
        }

        OnMouseMove();
        break;

    case WM_RBUTTONUP:
        m_bRMouseDown=FALSE;
        break;

    case WM_LBUTTONUP:
        m_bLMouseDown=FALSE;
        break;
    }

    return CD3DApplication::MsgProc( hwnd, uMsg, wParam, lParam );
}




//load an X file
VOID CMyD3DApplication::LoadXFile(TCHAR* name)
{
    LPD3DXMESH           pMeshSysMem=NULL,pMeshSysMem2=NULL;
    LPDIRECT3DINDEXBUFFER8 pIndexBuffer=NULL;
   
    HRESULT hr;
    DWORD                dwFaces,dwVertices;

    LPDIRECT3DVERTEXBUFFER8 pVertexBuffer = NULL;
  
    hr = D3DXLoadMeshFromX(name, D3DXMESH_SYSTEMMEM, 
                          m_pd3dDevice, NULL, NULL, NULL, &pMeshSysMem);
    if(FAILED(hr))
        goto FAIL;

    //reform the mesh to one which will have space for the tangent vectors
    hr = pMeshSysMem->CloneMeshFVF(D3DXMESH_MANAGED,  D3DFVF_NORMAL
                                    | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(1) | D3DFVF_XYZ, 
                                    m_pd3dDevice, &pMeshSysMem2);
 
    //compute the normals
    hr = D3DXComputeNormals(pMeshSysMem,NULL);
    if(FAILED(hr))
        goto FAIL;

    //compute the tangent vectors in the texture space and load them into
    //the 1'rd texture stage (which is 3d), read the texture coords from the 0th texture
    //stage. Don't need the V direction vector since it is assumed to be U x N.
    hr = D3DXComputeTangent(pMeshSysMem,0,pMeshSysMem2,1,D3DX_COMP_TANGENT_NONE,TRUE,NULL);
    if(FAILED(hr))
        goto FAIL;
         
    SAFE_RELEASE(pMeshSysMem);

    hr = pMeshSysMem2->GetIndexBuffer(&pIndexBuffer);
    if(FAILED(hr))
        goto FAIL;
    hr = pMeshSysMem2->GetVertexBuffer(&pVertexBuffer);
    if(FAILED(hr))
        goto FAIL;

    dwFaces    = pMeshSysMem2->GetNumFaces();
    dwVertices = pMeshSysMem2->GetNumVertices();
    
    //give them to the shadow object
    m_pShadow->SetBuffers(pIndexBuffer,pVertexBuffer,dwFaces,dwVertices);

    SAFE_RELEASE(pVertexBuffer)
    SAFE_RELEASE(pIndexBuffer)

    //cleanup
FAIL:
      
    SAFE_RELEASE(pMeshSysMem)
    SAFE_RELEASE(pMeshSysMem2)
    SAFE_RELEASE(pVertexBuffer)
    SAFE_RELEASE(pIndexBuffer)
}



//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Animates the scene
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    D3DXMATRIX matWorld,mat1,mat2;
 
    // Update world matrix
    
     // Process keyboard input
    D3DXVECTOR3 vecT(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vecR(0.0f, 0.0f, 0.0f);

    vecT =     D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    if(m_bKey['W'])                            vecT.z += 2.0f; // Move Forward
    if(m_bKey['S'])                            vecT.z -= 2.0f; // Move Backward
   
    m_vecVelocity = m_vecVelocity * 0.9f - vecT * 0.1f;
    m_vecAngularVelocity = m_vecAngularVelocity * 0.9f - vecR * 0.1f;

       // Update position and view matricies
    D3DXMATRIX matT, matR;
    D3DXQUATERNION qR;

    vecT = m_vecVelocity * m_fElapsedTime * m_fSpeed;
    vecR = m_vecAngularVelocity * m_fElapsedTime * m_fAngularSpeed;

    D3DXMatrixTranslation( &matT, -vecT.x, -vecT.y, -vecT.z );
     
    D3DXMatrixMultiply( &m_matPosition, &matT, &m_matPosition );

    D3DXQuaternionRotationYawPitchRoll( &qR, vecR.y, vecR.x, vecR.z );
    D3DXMatrixRotationQuaternion( &matR, &qR );
    D3DXMatrixMultiply( &m_matPosition, &matR, &m_matPosition );

    D3DXMatrixInverse( &m_matView, NULL, &m_matPosition );

    //apply rotation
    if(m_bRotate)
    {
        D3DXMatrixRotationY(&matR, -m_fElapsedTime*.5f);
        D3DXMatrixMultiply(&m_matObject,&matR,&m_matObject);
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    HRESULT hr;
    
    FLOAT fAspectRatio = (FLOAT)m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
  
    // Update projection matrix
    if(m_fAspectRatio != fAspectRatio)
    {
        D3DXMATRIX matProjection;
        D3DXMatrixPerspectiveFovLH(&m_matProj, 0.05f, fAspectRatio, 1.0f, 10000.0f);
        m_fAspectRatio = fAspectRatio;
    }

    m_pShadow->ComputeLight(m_LightDir,&m_matObject,&m_matView,&m_matProj); 
   
    if(FAILED(hr = m_pd3dDevice->BeginScene()))
        return hr;

    
    if(m_bWindowed)
    {
        m_pShadow->Render(m_bDebug, m_bShadow, m_bBump, m_rcWindowClient.left,
                          m_rcWindowClient.top, m_rcWindowClient.right - m_rcWindowClient.left,
                          m_rcWindowClient.bottom - m_rcWindowClient.top);
    }
    else
    {
        m_pShadow->Render(m_bDebug, m_bShadow, m_bBump, 0, 0, 
                          m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height);
    }

    // Output statistics
    m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
    m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );
    m_pFontSmall->DrawText( 2, 40, D3DCOLOR_ARGB(255,255,255,255), TEXT("2: Toggle shadow") );
    m_pFontSmall->DrawText( 2, 60, D3DCOLOR_ARGB(255,255,255,255), TEXT("3: Toggle bumpmapping") );
    m_pFontSmall->DrawText( 2, 80, D3DCOLOR_ARGB(255,255,255,255), TEXT("4: Reset object") );
    m_pFontSmall->DrawText( 2,100, D3DCOLOR_ARGB(255,255,255,255), TEXT("5: Toggle spin") );

    if(FAILED(hr = m_pd3dDevice->EndScene()))
        return hr;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()

{
    // Set cursor to indicate that user can move the object with the mouse
#ifdef _WIN64
    SetClassLongPtr( m_hWnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor( NULL, IDC_SIZEALL ) );
#else
    SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)LoadCursor( NULL, IDC_SIZEALL ) );
#endif

    TCHAR strBump[160], strColor[160];
    DXUtil_FindMediaFile(strBump, _T("earthbump.bmp"));
    DXUtil_FindMediaFile(strColor,_T("earth.bmp"));

    m_pShadow = new ShadowSet(strBump, strColor);
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    D3DXMATRIX matT;

    m_pFont->InitDeviceObjects( m_pd3dDevice );
    m_pFontSmall->InitDeviceObjects( m_pd3dDevice );

    D3DXMatrixIdentity(&m_matObject);
    D3DXMatrixIdentity(&m_matView);
    D3DXMatrixIdentity(&m_matPosition);

    m_fSpeed             = 20.0f;
    m_fAngularSpeed      = .10f;
    m_vecVelocity        = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_vecAngularVelocity = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    m_LightDir           = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );

    D3DXMatrixTranslation( &matT, 0,0, -40);

    D3DXMatrixMultiply( &m_matPosition, &matT, &m_matPosition );
    D3DXMatrixInverse( &m_matView, NULL, &m_matPosition );
   
    D3DCAPS8 Caps;
    m_pd3dDevice->GetDeviceCaps(&Caps);

    //detect if the hardware supports pixel shaders
    //if it does, then the shadow term can be done in one pass,
    //resulting in much faster and better results
    if(Caps.PixelShaderVersion < D3DPS_VERSION(1,1) )
        m_bUsePix = FALSE;
    else
        m_bUsePix = TRUE;

    if(FAILED( m_pShadow->Init(m_pd3dDevice, m_bUsePix)))
       return E_FAIL;
 
    TCHAR strFile[160];
    DXUtil_FindMediaFile(strFile, _T("sphere.x"));
    LoadXFile(strFile);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pFont->RestoreDeviceObjects();
    m_pFontSmall->RestoreDeviceObjects();

    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);     
   
    m_pShadow->Restore();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    m_pFontSmall->InvalidateDeviceObjects();
    m_pShadow->Invalidate();
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    m_pFont->DeleteDeviceObjects();
    m_pFontSmall->DeleteDeviceObjects();
    m_pShadow->DeleteDeviceObj();
    return S_OK;
}




ShadowSet::ShadowSet(TCHAR *name,TCHAR *colorName)
{
    DWORD i,x,y;
    FLOAT fRange = D3DX_PI/4;

    m_bInitalize = FALSE;
    m_bLoaded = FALSE;
 
    DWORD dwIndex =0;
   
    m_pOldStencilZ=NULL;

    //set everything to NULL
    for(i = 0;i < 16;i++)
    {
        m_pBasisMap[i] = NULL;
        m_pHorizonMap[i] = NULL;
        m_pHeightMap[i] = NULL;
        if(i<4)
        {
            m_pBasisTextures[i] = NULL;
            m_pHorizonTextures[i] = NULL;
        }
    }   

    m_pRenderBuffer  = NULL;
    m_pRenderBuffer2 = NULL;  

    m_pNormalMap        = NULL;
    m_pColorTexture     = NULL;
  
    if(colorName)
        _tcscpy(m_cColorName,colorName);
    else
        m_cColorName[0] = 0;

    if(name)
        _tcscpy(m_cHeightName,name);
    else
        m_cColorName[0] = 0;

    //Load Bitmap
    RGBQUAD rgbq[256];
    BITMAP bp;
    BYTE *bits;
    BYTE *tb;
    HBITMAP texmap;
    HDC tDC;
    BYTE *temploc,*srcloc;
    
    texmap = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if(texmap==0)
        goto FAIL;
    GetObject(texmap, sizeof(BITMAP), (LPSTR) &bp);

    m_iHeightWidth  = bp.bmWidth;
    m_iHeightHeight = bp.bmHeight;

    //XXX - need to  check for powers of 2
    tDC = CreateCompatibleDC(NULL);
    SelectObject(tDC, texmap);
    GetObject(tDC, sizeof(BITMAP), (LPSTR) &bp);
    bits = (BYTE *)bp.bmBits;

    GetDIBColorTable(tDC,0,256,rgbq);

    tb = m_pHeightMap[0] = new BYTE[m_iHeightWidth*m_iHeightHeight];
    if(tb==NULL)
        goto FAIL;

    if(bp.bmBitsPixel == 8)
    {
      for(y = 0; y < m_iHeightHeight; y++)
      { 
        for(x = 0; x < m_iHeightWidth ; x++)
        {
            temploc = &tb[(y * (m_iHeightWidth) + x) ];
            srcloc = &bits[(m_iHeightHeight-y-1) * (bp.bmWidthBytes) + x];
            temploc[0] = rgbq[*srcloc].rgbRed;
        }
      }
    }
    else
    {
        for(y = 0; y < m_iHeightHeight; y++)
        { 
            for(x = 0; x < m_iHeightWidth ; x++)
            {
                temploc = &tb[(y * (m_iHeightWidth) + x)];
                srcloc = &bits[(m_iHeightHeight-y-1) * (bp.bmWidthBytes) + x*3];
                temploc[0] = srcloc[2];
            }
        }
    }

    DeleteObject(texmap);
    DeleteDC(tDC);

    DWORD curHeight,curWidth;

    curHeight = m_iHeightHeight;
    curWidth  = m_iHeightWidth;

    curHeight = curHeight >> 1;
    curWidth  = curWidth  >> 1;

 /*   for(i = 1;(curHeight > 2) && (curWidth > 2);i++)
    {
   
        m_pHeightMap[i] = new BYTE[curHeight*curWidth];
        if(!m_pHeightMap)
            goto FAIL;

         //do an average of the four neighboring pixels of the previous level
        for(y = 0;y < curHeight;y++)
        {
            pLine = &m_pHeightMap[i-1][y*2*(2*curWidth)];
            for(x = 0; x < curWidth;x++)
            {
                m_pHeightMap[i][y*curWidth+x] = (pLine[0] + pLine[1] +
                                                       pLine[2*curWidth]+pLine[2*curWidth+1])/4;    
                pLine+=2;
            }
        }
        curHeight = curHeight >> 1;
        curWidth  = curWidth  >> 1;
    }
    m_dwHeightLevels = i;

  */
    //now set up the basis maps
    for(i = 0;i < 8;i++)
    {
        m_pBasisMap[i] = new BYTE[BASISMAPSIZE*BASISMAPSIZE];
        m_pHorizonMap[i] = new BYTE[m_iHeightWidth*m_iHeightHeight];

        if(m_pBasisMap[i] == NULL ||
           m_pHorizonMap[i] == NULL)
            goto FAIL;
    
        BuildBasisMap(m_pBasisMap[i],((FLOAT)(D3DX_PI*i))/4.0f,fRange);
        BuildHorizonMap(m_pHorizonMap[i],m_pHeightMap[0],cosf((FLOAT)(D3DX_PI*i)/4.0f), sinf((FLOAT)(D3DX_PI*i)/4.0f),15,m_iHeightWidth,
                        m_iHeightHeight,TRUE,TRUE);
    }

    m_pShad1 = NULL;
    m_pRenderBuffer = NULL;
    m_pIndexBuffer = NULL;
    m_pScreenVB=NULL;

    m_iNumTriangles = 0;
    m_iNumVertices  = 0;
    m_pVertices = NULL;

    m_bLoaded = TRUE;
    return;

FAIL:
    return;
}




ShadowSet::~ShadowSet()
{
    INT i;
  
    for(i = 0;i < 16;i++)
    {
        if(m_pBasisMap[i])
            delete []m_pBasisMap[i]; 
        if(m_pHorizonMap[i])
            delete []m_pHorizonMap[i];
        if(m_pHeightMap[i])
            delete []m_pHeightMap[i];
    }
}




HRESULT ShadowSet::Init(IDirect3DDevice8 *pDev, BOOL bUsePix)
{
    HRESULT hr;
    LPD3DXBUFFER pCode=NULL;
    INT iWidth,iHeight;
    m_pDev = pDev;

    LPDIRECT3DTEXTURE8 pHeightMap;
    D3DSURFACE_DESC    desc;
    
    DWORD dwDecl[] = 
    {
        D3DVSD_STREAM(0),
        D3DVSD_REG(0,   D3DVSDT_FLOAT3),
        D3DVSD_REG(3,   D3DVSDT_FLOAT3),
        D3DVSD_REG(7,   D3DVSDT_FLOAT2),
        D3DVSD_REG(8,   D3DVSDT_FLOAT3),
        D3DVSD_END()
    };

    TCHAR Shad1[255],Shad2[255],Shad3[255];

    m_bUsePix = bUsePix;
    if(!m_bLoaded)
        goto FAIL;

    if(FAILED(hr = D3DXCreateTextureFromFile(m_pDev,m_cColorName,&m_pColorTexture)))
        goto FAIL;

    if(FAILED(hr = D3DXCreateTextureFromFile(m_pDev,m_cHeightName,&pHeightMap)))
        goto FAIL;

    pHeightMap->GetLevelDesc(0,&desc);

    iWidth  = desc.Width;
    iHeight = desc.Height;
    
    if(FAILED(hr = D3DXCreateTexture(m_pDev,iWidth,iHeight,0,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED, &m_pNormalMap)))
        goto FAIL;

    D3DXComputeNormalMap(m_pNormalMap,pHeightMap,NULL,0,D3DX_CHANNEL_RED,10);
    
    pHeightMap->Release();

    //two methods
    if(!m_bUsePix)
    {
        //don't have a dp4, so need to put the maps into 3 basis texture and 3 horizon maps
        //basis maps are not-content specific - they are just LERP tables

        //build the interleave maps
        if(!BuildInterleavedMap(m_pBasisTextures[0],m_pBasisMap[0],m_pBasisMap[1],m_pBasisMap[2],NULL,BASISMAPSIZE,BASISMAPSIZE))
            goto FAIL;
        if(!BuildInterleavedMap(m_pBasisTextures[1],m_pBasisMap[3],m_pBasisMap[4],m_pBasisMap[5],NULL,BASISMAPSIZE,BASISMAPSIZE))
            goto FAIL;
        if(!BuildInterleavedMap(m_pBasisTextures[2],m_pBasisMap[6],m_pBasisMap[7],NULL,NULL,BASISMAPSIZE,BASISMAPSIZE))
            goto FAIL;

        if(!BuildInterleavedMap(m_pHorizonTextures[0],m_pHorizonMap[0],m_pHorizonMap[1],m_pHorizonMap[2],NULL,m_iHeightWidth,m_iHeightHeight))
            goto FAIL;
        if(!BuildInterleavedMap(m_pHorizonTextures[1],m_pHorizonMap[3],m_pHorizonMap[4],m_pHorizonMap[5],NULL,m_iHeightWidth,m_iHeightHeight))
           goto FAIL;
        if(!BuildInterleavedMap(m_pHorizonTextures[2],m_pHorizonMap[6],m_pHorizonMap[7],NULL,NULL,m_iHeightWidth,m_iHeightHeight))
           goto FAIL;

        if(FAILED(hr = D3DXFilterTexture(m_pHorizonTextures[0],NULL,0,D3DX_FILTER_BOX)))
            goto FAIL;
        if(FAILED(hr = D3DXFilterTexture(m_pHorizonTextures[1],NULL,0,D3DX_FILTER_BOX)))
            goto FAIL;
        if(FAILED(hr = D3DXFilterTexture(m_pHorizonTextures[2],NULL,0,D3DX_FILTER_BOX)))
            goto FAIL;
  
        DXUtil_FindMediaFile(Shad1,_T("bumpshader.vsh"));
        DXUtil_FindMediaFile(Shad2,_T("bumpshader2.vsh"));
        DXUtil_FindMediaFile(Shad3,_T("bumpshader3.vsh"));

        if( FAILED( hr = D3DXAssembleShaderFromFile(Shad2, 0, NULL, &pCode, NULL ) ) )
            goto FAIL;
        if( FAILED( hr = m_pDev->CreateVertexShader( dwDecl,(DWORD*)pCode->GetBufferPointer(), &m_dwBasisShader, 0 ) ) )
            goto FAIL;      
        SAFE_RELEASE(pCode);
        
        if( FAILED( hr = D3DXAssembleShaderFromFile(Shad1, 0, NULL, &pCode, NULL ) ) )
            goto FAIL;
        if( FAILED( hr = m_pDev->CreateVertexShader( dwDecl,(DWORD*)pCode->GetBufferPointer(), &m_dwShadowShader, 0 ) ) )
            goto FAIL;
        SAFE_RELEASE(pCode);
          
        if( FAILED( hr = D3DXAssembleShaderFromFile(Shad3, 0, NULL, &pCode, NULL ) ) )
            goto FAIL;
        if( FAILED(hr = m_pDev->CreateVertexShader( dwDecl,(DWORD*)pCode->GetBufferPointer(), &m_dwBumpShader, 0 ) ) )
            goto FAIL;

        SAFE_RELEASE(pCode);

        //create render targets for pic-shader emulation
        if(FAILED(hr = m_pDev->CreateTexture(RENDERTARGSIZE,RENDERTARGSIZE,0,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pRenderBuffer)))
           goto FAIL;

        if(FAILED(hr = m_pDev->CreateTexture(RENDERTARGSIZE,RENDERTARGSIZE,0,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&m_pRenderBuffer2)))
           goto FAIL;
    }
    else
    {
        //build the interleave maps, use the Alpha to
        //basis maps are not-content specific - they are just LERP tables
        if(!BuildInterleavedMap(m_pBasisTextures[0],m_pBasisMap[0],m_pBasisMap[1],m_pBasisMap[2],m_pBasisMap[3],BASISMAPSIZE,BASISMAPSIZE))
            goto FAIL;
        if(!BuildInterleavedMap(m_pBasisTextures[1],m_pBasisMap[4],m_pBasisMap[5],m_pBasisMap[6],m_pBasisMap[7],BASISMAPSIZE,BASISMAPSIZE))
            goto FAIL;

        if(!BuildInterleavedMap(m_pHorizonTextures[0],m_pHorizonMap[0],m_pHorizonMap[1],m_pHorizonMap[2],m_pHorizonMap[3],m_iHeightWidth,m_iHeightHeight))
            goto FAIL;
        if(!BuildInterleavedMap(m_pHorizonTextures[1],m_pHorizonMap[4],m_pHorizonMap[5],m_pHorizonMap[6],m_pHorizonMap[7],m_iHeightWidth,m_iHeightHeight))
           goto FAIL;

        if(FAILED(hr = D3DXFilterTexture(m_pHorizonTextures[0],NULL,0,D3DX_FILTER_BOX)))
            goto FAIL;
        if(FAILED(hr = D3DXFilterTexture(m_pHorizonTextures[1],NULL,0,D3DX_FILTER_BOX)))
            goto FAIL;

        //load the required shaders
        DXUtil_FindMediaFile(Shad1,_T("bumpshader4.vsh"));
        DXUtil_FindMediaFile(Shad2,_T("shadowbumpshader.psh"));
        DXUtil_FindMediaFile(Shad3,_T("bumpshader3.vsh"));

        if(FAILED(hr = D3DXAssembleShaderFromFile(Shad1,0,NULL, &pCode,NULL) ) )
            goto FAIL;
        if( FAILED( hr = m_pDev->CreateVertexShader( dwDecl,(DWORD*)pCode->GetBufferPointer(), &m_dwShadowShader, 0 ) ) )
            goto FAIL;
        SAFE_RELEASE(pCode);

        if(FAILED(hr = D3DXAssembleShaderFromFile(Shad2,0,NULL, &pCode,NULL) ) )
            goto FAIL;
        if( FAILED( hr = m_pDev->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), &m_dwPixShader) ) )
            goto FAIL;
        SAFE_RELEASE(pCode);

        if( FAILED( hr = D3DXAssembleShaderFromFile(Shad3, 0, NULL, &pCode, NULL ) ) )
            goto FAIL;
        if( FAILED(hr = m_pDev->CreateVertexShader( dwDecl,(DWORD*)pCode->GetBufferPointer(), &m_dwBumpShader, 0 ) ) )
            goto FAIL;

        SAFE_RELEASE(pCode);
    }
        
    m_bInitalize = TRUE;

    return S_OK;
FAIL:

    SAFE_RELEASE(pCode);
    return E_FAIL;
}




VOID ShadowSet::DeleteDeviceObj()
{
    for(INT i=0;i<4;i++)
    {
        SAFE_RELEASE(m_pHorizonTextures[i]);
        SAFE_RELEASE(m_pBasisTextures[i]);
    }
    SAFE_RELEASE(m_pNormalMap);
    SAFE_RELEASE(m_pColorTexture);
    SAFE_RELEASE(m_pRenderBuffer);
    SAFE_RELEASE(m_pRenderBuffer2);
    SAFE_RELEASE(m_pIndexBuffer);
    SAFE_RELEASE(m_pVertices);
    SAFE_RELEASE(m_pScreenVB);
    SAFE_RELEASE(m_pShad1);
    
    if(m_pDev)
    {
        if(!m_bUsePix)
        {
            m_pDev->DeleteVertexShader(m_dwBumpShader);
            m_pDev->DeleteVertexShader(m_dwShadowShader);
            m_pDev->DeleteVertexShader(m_dwBasisShader);
        }
        else
        {
            m_pDev->DeleteVertexShader(m_dwShadowShader);
            m_pDev->DeletePixelShader(m_dwPixShader);
        }
    }
}  




VOID ShadowSet::Restore()
{
    HRESULT hr;
    SAFE_RELEASE(m_pRenderBuffer);
    SAFE_RELEASE(m_pRenderBuffer2);

    if(!m_bUsePix)
    {
        hr = m_pDev->CreateTexture(RENDERTARGSIZE, RENDERTARGSIZE, 0, 
            D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pRenderBuffer);
        hr = m_pDev->CreateTexture(RENDERTARGSIZE, RENDERTARGSIZE, 0, 
            D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pRenderBuffer2);
    }

    m_pDev->GetRenderTarget(&m_pOldRenderTarget);
    hr = m_pDev->GetDepthStencilSurface(&m_pOldStencilZ);
}




VOID ShadowSet::Invalidate()
{
    SAFE_RELEASE(m_pRenderBuffer);
    SAFE_RELEASE(m_pRenderBuffer2);
    SAFE_RELEASE(m_pOldRenderTarget);
    SAFE_RELEASE(m_pOldStencilZ);
}




// this builds a basis map
// the basis maps will be interleaved, or we will pack 3 of them into one texture
// (we could pack 4, but we can't use the alpha channel in our dot multiplies)
// but we will assume that the u,v represent the angle with respect to the center of the
// of the texture (.5,.5), So if we want the basis function for the light vector 
// (1,.5) we'd look at the point (1,.75)  
VOID ShadowSet::BuildBasisMap(BYTE *pBasis,FLOAT PrimAngle,FLOAT fAngle2)
{
    INT x,y;
    FLOAT fEndX,fEndY,fAngle,fPercent;
    FLOAT u,v,dot,nx,ny,fsq;
    BYTE *pCurLine;

    u = cosf(PrimAngle);
    v = sinf(PrimAngle);

    for(y = 0;y < BASISMAPSIZE;y++)
    {
        pCurLine = &pBasis[y*BASISMAPSIZE];

        for(x = 0;x < BASISMAPSIZE;x++)
        {
            fEndX =  x - .5f*BASISMAPSIZE;
            fEndY =  y - .5f*BASISMAPSIZE;
            
            //take the dot product of the normalized vectors
            fsq = sqrtf(fEndX*fEndX+fEndY*fEndY);
            if(fsq == 0)
            {
                fPercent = 128;
            }
            else
            {
                // we remember our definiton of dot product,
                // cos(Angle) = DotProduct of Normalzed vectors
                // so Angle = acos(DotProduct)
                nx = fEndX/fsq;
                ny = fEndY/fsq;
                dot = nx*u + ny*v;
                fAngle = acosf(dot);
                if(fabs(fAngle) < fAngle2)
                {
                    fPercent = 128 + fabsf(fAngle-fAngle2)*127/fAngle2;
                    if(fPercent>255)
                        fPercent=255;
                }
                else
                {
                    fPercent = 128;
                }
            }
            *pCurLine = (BYTE)fPercent;
            pCurLine++;
        }
    }
}




BOOL ShadowSet::BuildInterleavedMap(LPDIRECT3DTEXTURE8 &newTex,
                                    BYTE *r,
                                    BYTE *g,
                                    BYTE *b,
                                    BYTE *a,INT width,INT height)
{
    HRESULT hr;
    LPDIRECT3DTEXTURE8 pTexture;
    D3DLOCKED_RECT LockedRect;
    BYTE *pBase,*pLine;
    INT x,y;

    hr = D3DXCreateTexture(m_pDev, width, height, 0, 0, 
        D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture);
    if(hr != S_OK)
        return(FALSE);

    hr = pTexture->LockRect(0,&LockedRect,NULL,0);
    if(hr != S_OK)
    {
        pTexture->Release();
        return(FALSE);
    }

    pBase = (BYTE *)LockedRect.pBits;

    for(y = 0;y < height;y++)
    {
        pLine = &pBase[y*LockedRect.Pitch];
        for(x = 0;x < width;x++)
        {
            if(r)
                pLine[2] = r[y*width + x];
            else
                pLine[2] = 128;
            if(g)
                pLine[1] = g[y*width + x];
            else
                pLine[1] = 128;
            if(b)
                pLine[0] = b[y*width + x];
            else
                pLine[0] = 128;
            if(a)
                pLine[3] = a[y*width + x];
            else
                pLine[3] = 128;
        pLine+=4;
        }
    }

    pTexture->UnlockRect(0);

    newTex = pTexture;

    return TRUE;
}




#define MAPHEIGHT .175f
#define MAPHEIGHT2 .175f

VOID ShadowSet::BuildHorizonMap(BYTE *pHor,BYTE* pHeight,FLOAT dx,FLOAT dy,INT iStepLength,
                         INT iWidth,INT iHeight,BOOL bWrapU,BOOL bWrapV)
{
    INT x,y;
    INT imx,imy,i;
    FLOAT fpx,fpy;
    BYTE *pCurHurLine;
    FLOAT fMaxAngle,fNewAngle,fDeltaX,fDeltaY,fHeight;

    for(y = 0;y < iHeight;y++)
    {
        pCurHurLine = &pHor[y*iWidth];
        for(x = 0;x < iWidth;x ++)
        {
            //we assume its completly visible at zero
            fMaxAngle = 0;
            fpx = (FLOAT)x;
            fpy = (FLOAT)y;
            for(i = 0;i < iStepLength;i++)
            {
                fpx += dx;
                fpy += dy;

                imx = (INT) fpx;
                imy = (INT) fpy;
                //don't check against ourselves for blocking
                if(imx == x && imy == y)
                    continue;
                //if we are out of bounds, we're done
                //if the texture is repeating - this should
                //probally wrap
                if(imx < 0)
                {
                    if(bWrapU)
                        imx = iWidth-1;
                    else continue;
                }
                
                if(imx >= iWidth)
                {
                    if(bWrapU)
                       imx = 0;
                    else continue;
                }
                
                if(imy < 0)
                {
                    if(bWrapV)
                       imy = iHeight-1;
                    else continue;
                }
                
                if(imy >= iHeight)
                {
                    if(bWrapV)
                        imy = 0;
                    else continue;
                }

                fDeltaX = fpx - x;
                fDeltaY = fpy - y;
                fHeight = pHeight[imy*iWidth+imx]*MAPHEIGHT-pHeight[y*iWidth+x]*MAPHEIGHT;
                
                //this is the dot prodoct
                //Since we are dotting against the normal, (0,0,1) this is just
                // the Height normalized, or the Height over the length of the vector
                fNewAngle = fHeight/sqrtf(fDeltaX*fDeltaX + fDeltaY*fDeltaY + fHeight*fHeight);
            
                //if we found a further obstruction, use it
                if(fNewAngle > fMaxAngle)
                    fMaxAngle = fNewAngle;
            }

            //centered around 128
            *pCurHurLine = 128 + (BYTE) ((FLOAT)fMaxAngle*127); 
            pCurHurLine++;
        }
    }

}




VOID ShadowSet::ComputeLight(D3DXVECTOR3 &lightDir, 
    D3DXMATRIX *pObject, D3DXMATRIX *pView, D3DXMATRIX *pProj)
{

    D3DXVECTOR3 U,V,W,Pos;
    D3DXVECTOR4 light,one(1,1,1,1),half(0.5f,0.5f,.5f,.5f);//.5f);
    
    if(!m_bInitalize)
        return;

    light.x = lightDir.x;
    light.y = lightDir.y;
    light.z = lightDir.z;
    light.w = 0;

    D3DXVec4Normalize(&light,&light);
    
    m_matObject = *pObject;
    m_matView   = *pView;
    m_matProj   = *pProj;

    D3DXMATRIX  temp;

    D3DXMatrixMultiply(&m_matTot, pObject, pView);

    D3DXMatrixTranspose(&temp, &m_matTot);

    //the inverse transform
    m_pDev->SetVertexShaderConstant(   16,pObject, 4);
    
    D3DXMatrixMultiply(&m_matTot,&m_matTot,pProj);

    D3DXMatrixTranspose(&m_matTot,&m_matTot);
    D3DXMatrixTranspose(&m_matObject,pObject);

    //load the values into the registers
    m_pDev->SetVertexShaderConstant(  12, &light,  1 );
    m_pDev->SetVertexShaderConstant(  32, &one,    1);
    m_pDev->SetVertexShaderConstant(  33, &half,  1);
    m_pDev->SetVertexShaderConstant(   0, &m_matObject, 4);
    m_pDev->SetVertexShaderConstant(   8,&m_matTot, 4);
}




VOID ShadowSet::Render(BOOL bShow, BOOL bShad, BOOL bBump, INT iLeft, INT iTop,
                       INT iRight, INT iBottom)
{
    if(!m_pVertices || !m_pIndexBuffer || !m_pDev)
        return;

    HRESULT hr;
    INT i;

    VOID* pVerts;
    
    LPDIRECT3DSURFACE8 pSurface;
    
    D3DVIEWPORT8 ViewPort,OldPort;

    if(!m_bInitalize)
        return;

    static struct { FLOAT x,y,z,w, u1,v1;} s_Verts2[] =
    {
        {  RENDERTARGSIZE,   0.0f, 0.0f, 1.0f,  1,0,},
        {  RENDERTARGSIZE, RENDERTARGSIZE, 0.0f, 1.0f,  1,1,},
        {  0.0f,     0.0f, 0.0f, 1.0f,  0,0,},
        {  0.0f,   RENDERTARGSIZE, 0.0f, 1.0f,  0,1,}
    };

    static struct { FLOAT x,y,z,w; DWORD color;} s_Verts3[] =
    {
        {750.0f,  0.0f, 0.0f,0, D3DCOLOR_ARGB( 0xa0,0xff, 0x00, 0x00 ),},
        {750.0f, 375.0f, 0.0f,0, D3DCOLOR_ARGB( 0xa0,0xff,00, 0x00),},
        { 0.0f,  0.0f, 0.0f, 0, D3DCOLOR_ARGB( 0xa0,0xff,0x00, 0x00),},
        { 0.0f, 375.0f, 0.0f,0, D3DCOLOR_ARGB( 0xa0,0xff, 0x00, 0x00),},
    };

    m_pDev->SetIndices(m_pIndexBuffer,0);

    if(!m_bUsePix)
    {
        //sets up polygons that
        //are the same size as the screen
        //(well... almost)
        OldPort.X = iLeft;
        OldPort.Y = iTop;
        OldPort.Width = iRight-iLeft;
        OldPort.Height = iBottom-iTop;
        OldPort.MaxZ = 1;
        OldPort.MinZ = 0; 

        s_Verts2[0].x = (FLOAT) OldPort.Width;
        s_Verts2[1].x = (FLOAT)OldPort.Width;
        s_Verts2[1].y =(FLOAT) OldPort.Height;
        s_Verts2[3].y = (FLOAT)OldPort.Height; 

        s_Verts3[0].x = (FLOAT)OldPort.Width;
        s_Verts3[1].x = (FLOAT)OldPort.Width;
        s_Verts3[1].y = (FLOAT)OldPort.Height;
        s_Verts3[3].y =(FLOAT) OldPort.Height; 

        if(!bShow)
        {
            s_Verts3[0].color = D3DCOLOR_ARGB( 0xa0,0x00, 0x00, 0x00 );
            s_Verts3[1].color = D3DCOLOR_ARGB( 0xa0,0x00, 0x00, 0x00 );
            s_Verts3[2].color = D3DCOLOR_ARGB( 0xa0,0x00, 0x00, 0x00 );
            s_Verts3[3].color = D3DCOLOR_ARGB( 0xa0,0x00, 0x00, 0x00 ); 
        }
        else
        {
            s_Verts3[0].color = D3DCOLOR_ARGB( 0x70,0xff, 0x00, 0x00 );
            s_Verts3[1].color = D3DCOLOR_ARGB( 0x70,0xff, 0x00, 0x00 );
            s_Verts3[2].color = D3DCOLOR_ARGB( 0x70,0xff, 0x00, 0x00 );
            s_Verts3[3].color = D3DCOLOR_ARGB( 0x70,0xff, 0x00, 0x00 ); 
        }   

        //fill the VBs
        if(m_pShad1==NULL)
        {
           if ( FAILED( hr = m_pDev->CreateVertexBuffer( sizeof(s_Verts2),
                                 0x0, D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_SYSTEMMEM, &m_pShad1 ) ) )
           {
                   goto Fail;
           }
        }

        if ( FAILED(hr = m_pShad1->Lock( 0, sizeof(s_Verts2), (BYTE **)(&pVerts), 0 )) )
            goto Fail;

        memcpy( (VOID*)pVerts, (VOID*)s_Verts2, sizeof(s_Verts2) );
    
        if (FAILED(hr = m_pShad1->Unlock()) )
           goto Fail;

        if(m_pScreenVB==NULL)
        {
            if ( FAILED( hr = m_pDev->CreateVertexBuffer( sizeof(s_Verts3),
                               0x0, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_SYSTEMMEM, &m_pScreenVB) ) )
            {
                 goto Fail;
            }
        }

        if ( FAILED(hr = m_pScreenVB->Lock( 0, sizeof(s_Verts3), (BYTE **)(&pVerts), 0 )) )
            goto Fail;

        memcpy( (VOID*)pVerts, (VOID*)s_Verts3, sizeof(s_Verts3) );
    
        if (FAILED(hr = m_pScreenVB->Unlock()) )
             goto Fail;

        m_pDev->SetRenderState(D3DRS_ZENABLE,FALSE);

        if(bShad)
        {
           //first set the Rendertarget
            hr =  m_pRenderBuffer->GetSurfaceLevel(0,&pSurface);
            hr =  m_pDev->SetRenderTarget(pSurface,NULL);

            ViewPort.X = 0;
            ViewPort.Y = 0;
            ViewPort.Height = RENDERTARGSIZE;
            ViewPort.Width =  RENDERTARGSIZE;
            ViewPort.MaxZ = 1;
            ViewPort.MinZ = 0;

            m_pDev->SetViewport(&ViewPort);
      
            m_pDev->Clear(0, NULL, D3DCLEAR_TARGET , D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0, 0);
    
            m_pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);  
            m_pDev->SetRenderState( D3DRS_ALPHATESTENABLE,FALSE);

            m_pDev->SetRenderState(D3DRS_WRAP0,D3DWRAPCOORD_0 | D3DWRAPCOORD_1);
            m_pDev->SetRenderState(D3DRS_WRAP1,D3DWRAPCOORD_0 | D3DWRAPCOORD_1);

            m_pDev->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
            m_pDev->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

            m_pDev->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
            m_pDev->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
     
            m_pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
            m_pDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            m_pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    
            m_pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
            m_pDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3);
            m_pDev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DOTPRODUCT3);   

            m_pDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            m_pDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT); //D3DTA_CURRENT);
            m_pDev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            m_pDev->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

            m_pDev->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
            m_pDev->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

            m_pDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
            m_pDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);

            m_pDev->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

            m_pDev->SetRenderState(D3DRS_WRAP0, D3DWRAPCOORD_0 | D3DWRAPCOORD_1);
            m_pDev->SetRenderState(D3DRS_WRAP1, D3DWRAPCOORD_0 | D3DWRAPCOORD_1);

            //render all the basis functions times all the images
            m_pDev->SetStreamSource( 0, m_pVertices, sizeof(ShaderVertex) );
            m_pDev->SetVertexShader(m_dwBasisShader);
            for(i = 0;i < 3;i++)
            {      
                m_pDev->SetTexture(0, m_pHorizonTextures[i]);
                m_pDev->SetTexture(1, m_pBasisTextures[i]);
                hr = m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,m_iNumVertices,0,m_iNumTriangles);
            }

            m_pDev->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE);

            pSurface->Release(); 

            //now we have to render a simple alpha texture to subtract off for the shadow
            hr =  m_pRenderBuffer2->GetSurfaceLevel(0,&pSurface);
            hr =  m_pDev->SetRenderTarget(pSurface,NULL);

            m_pDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0, 0);

            m_pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);  
            m_pDev->SetRenderState( D3DRS_ALPHATESTENABLE,FALSE);

            m_pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

            m_pDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
            m_pDev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

            m_pDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            m_pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

            m_pDev->SetStreamSource( 0, m_pVertices, sizeof(ShaderVertex) );
            m_pDev->SetVertexShader(m_dwShadowShader);
    
            hr = m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,m_iNumVertices,0,m_iNumTriangles);

            //restore our states
            m_pDev->SetRenderTarget(m_pOldRenderTarget,m_pOldStencilZ);
            pSurface->Release();
            m_pDev->SetViewport(&OldPort);
        }
    }

    m_pDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0x00, 0x00, 0x00, 0x00), 1.0, 0);
    m_pDev->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pDev->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR); 

    m_pDev->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pDev->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

    m_pDev->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
    m_pDev->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

    m_pDev->SetRenderState(D3DRS_WRAP0,D3DWRAPCOORD_0 | D3DWRAPCOORD_1);
    m_pDev->SetRenderState(D3DRS_WRAP1,D3DWRAPCOORD_0 | D3DWRAPCOORD_1);
    //important to unset a pixel shader!
    m_pDev->SetPixelShader(0);

    //diffuse, DOT3 lighting. 
    if(bBump)
    {
        m_pDev->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
        m_pDev->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
        m_pDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3);
        m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        m_pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        m_pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        m_pDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

        m_pDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pDev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        m_pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);
        m_pDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_pDev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        
        m_pDev->SetTexture(1,m_pColorTexture);
        m_pDev->SetTexture(0,m_pNormalMap);    
    }
    else
    {
        //no lighting, just base color texture
        m_pDev->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
        m_pDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        m_pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        m_pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        m_pDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        
        m_pDev->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
        m_pDev->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

        m_pDev->SetTexture(0,m_pColorTexture);
    }

    m_pDev->SetStreamSource( 0, m_pVertices, sizeof(ShaderVertex) );
    
    m_pDev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
    m_pDev->SetVertexShader(m_dwBumpShader);
    hr = m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,m_iNumVertices,0,m_iNumTriangles);

    m_pDev->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
    m_pDev->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    m_pDev->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    m_pDev->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_NONE);

    //now apply the black mask
    if(bShad)
    {
        //emmulate pixel shaders
        //the first two passes (the basis computation and the 
        //light angle computation) were done before
        if(!m_bUsePix)
        {
            //now, subtract the light angle from the she shadow angle
            //if the value is non zero, then the pixel is in shaddow
            m_pDev->SetStreamSource(0, m_pShad1,6*sizeof(FLOAT));
            m_pDev->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);

            m_pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

            m_pDev->SetRenderState(D3DRS_WRAP0,0);
            m_pDev->SetRenderState(D3DRS_WRAP1,0);

            m_pDev->SetRenderState(D3DRS_TEXTUREFACTOR,s_Verts3[0].color);
            m_pDev->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            m_pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            m_pDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);

            if(bShow)
                m_pDev->SetRenderState(D3DRS_TEXTUREFACTOR,D3DCOLOR_ARGB( 0xff,0xff, 0x00, 0x00 ));
            else
                m_pDev->SetRenderState(D3DRS_TEXTUREFACTOR,0x80202020);

            m_pDev->SetRenderState(D3DRS_TEXTUREFACTOR,0x60606060);
            m_pDev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            m_pDev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SUBTRACT);
            m_pDev->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            m_pDev->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
            m_pDev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TFACTOR);

            //use an alpha test - if alpha is non zero, 
            m_pDev->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);
            m_pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);    
            m_pDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
            m_pDev->SetRenderState(D3DRS_ALPHAREF,0);

            m_pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
            m_pDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO);
            m_pDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);

            m_pDev->SetTexture(0, m_pRenderBuffer2);
            m_pDev->SetTexture(1, m_pRenderBuffer);
            m_pDev->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

            m_pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
        }
        else
        {
             //use the pixel shader, much faster...

             //turn wrapping and blending on all the stages
             m_pDev->SetRenderState(D3DRS_WRAP0,D3DWRAPCOORD_0 | D3DWRAPCOORD_1);
             m_pDev->SetRenderState(D3DRS_WRAP1,D3DWRAPCOORD_0 | D3DWRAPCOORD_1);
             
             m_pDev->SetRenderState(D3DRS_WRAP2,D3DWRAPCOORD_0 | D3DWRAPCOORD_1);
             m_pDev->SetRenderState(D3DRS_WRAP3,D3DWRAPCOORD_0 | D3DWRAPCOORD_1);

             m_pDev->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
             m_pDev->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

             m_pDev->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
             m_pDev->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

             m_pDev->SetTextureStageState( 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
             m_pDev->SetTextureStageState( 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
             
             m_pDev->SetTextureStageState( 3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
             m_pDev->SetTextureStageState( 3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

             m_pDev->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
             m_pDev->SetTextureStageState( 2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
             m_pDev->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
             m_pDev->SetTextureStageState( 3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
             
             //bind the textures
             m_pDev->SetTexture(0, m_pHorizonTextures[0]);
             m_pDev->SetTexture(1, m_pBasisTextures[0]);
             m_pDev->SetTexture(2, m_pHorizonTextures[1]);
             m_pDev->SetTexture(3, m_pBasisTextures[1]);

             //render all the basis functions times all the images
             m_pDev->SetStreamSource( 0, m_pVertices, sizeof(ShaderVertex) );
             m_pDev->SetVertexShader(m_dwShadowShader);
             m_pDev->SetPixelShader(m_dwPixShader);

             m_pDev->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
             m_pDev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);    
             m_pDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCCOLOR);
             m_pDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ZERO);

             //run the pixel shader/vertex shader
             //these will summ up all the basis maps and then compare at a per 
             //pixel level with the light angle.
             //the output is the (inverse) shadow term
             hr = m_pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,m_iNumVertices,0,m_iNumTriangles);
        }
    }

Fail:
    return;
}




//set the index and vertex buffer for the object being rendered.
VOID ShadowSet::SetBuffers(LPDIRECT3DINDEXBUFFER8 pIndex,LPDIRECT3DVERTEXBUFFER8 pVertBuffer,
                           INT iNumFaces,INT iNumVertices)
{
    m_pIndexBuffer = pIndex;
    pIndex->AddRef();
    
    m_pVertices    = pVertBuffer;
    pVertBuffer->AddRef();

    m_iNumTriangles = iNumFaces;
    m_iNumVertices  = iNumVertices;
}
