//-----------------------------------------------------------------------------
// File: D3DFile.h
//
// Desc: Support code for loading DirectX .X files.
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef D3DFILE_H
#define D3DFILE_H
#include <tchar.h>
#include <d3d8.h>
#include <d3dx8.h>





//-----------------------------------------------------------------------------
// Name: class CD3DMesh
// Desc: Class for loading and rendering file-based meshes
//-----------------------------------------------------------------------------
class CD3DMesh
{
public:
    TCHAR               m_strName[512];

    LPD3DXMESH          m_pSysMemMesh;    // SysMem mesh, lives through resize
    LPD3DXMESH          m_pLocalMesh;     // Local mesh, rebuilt on resize
    
    DWORD               m_dwNumMaterials; // Materials for the mesh
    D3DMATERIAL8*       m_pMaterials;
    LPDIRECT3DTEXTURE8* m_pTextures;
    BOOL                m_bUseMaterials;

public:
    // Rendering
    HRESULT Render( LPDIRECT3DDEVICE8 pd3dDevice, 
                    BOOL bDrawOpaqueSubsets = TRUE,
                    BOOL bDrawAlphaSubsets = TRUE );

    // Mesh access
    LPD3DXMESH GetSysMemMesh() { return m_pSysMemMesh; }
    LPD3DXMESH GetLocalMesh()  { return m_pLocalMesh; }

    // Rendering options
    VOID    UseMeshMaterials( BOOL bFlag ) { m_bUseMaterials = bFlag; }
    HRESULT SetFVF( LPDIRECT3DDEVICE8 pd3dDevice, DWORD dwFVF );

    // Initializing
    HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICE8 pd3dDevice );
    HRESULT InvalidateDeviceObjects();

    // Creation/destruction
    HRESULT Create( LPDIRECT3DDEVICE8 pd3dDevice, TCHAR* strFilename );
    HRESULT Create( LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTXFILEDATA pFileData );
    HRESULT Destroy();

    CD3DMesh( TCHAR* strName = _T("CD3DFile_Mesh") );
    virtual ~CD3DMesh();
};




//-----------------------------------------------------------------------------
// Name: class CD3DFrame
// Desc: Class for loading and rendering file-based meshes
//-----------------------------------------------------------------------------
class CD3DFrame
{
public:
    TCHAR      m_strName[512];
    D3DXMATRIX m_mat;
    CD3DMesh*  m_pMesh;
    TCHAR      m_strMeshName[512];

    CD3DFrame* m_pNext;
    CD3DFrame* m_pChild;

public:
    // Matrix access
    VOID        SetMatrix( D3DXMATRIX* pmat ) { m_mat = *pmat; }
    D3DXMATRIX* GetMatrix()                   { return &m_mat; }

    CD3DMesh*   FindMesh( TCHAR* strMeshName );
    CD3DFrame*  FindFrame( TCHAR* strFrameName );
    BOOL        EnumMeshes( BOOL (*EnumMeshCB)(CD3DMesh*,VOID*), 
                            VOID* pContext );

    HRESULT Destroy();
    HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICE8 pd3dDevice );
    HRESULT InvalidateDeviceObjects();
    HRESULT Render( LPDIRECT3DDEVICE8 pd3dDevice, 
                    BOOL bDrawOpaqueSubsets = TRUE,
                    BOOL bDrawAlphaSubsets = TRUE,
                    D3DXMATRIX* pmatWorldMartix = NULL);
    
    CD3DFrame( TCHAR* strName = _T("CD3DFile_Frame") );
    virtual ~CD3DFrame();
};




//-----------------------------------------------------------------------------
// Name: class CD3DFile
// Desc: Class for loading and rendering file-based meshes
//-----------------------------------------------------------------------------
class CD3DFile : public CD3DFrame
{
    HRESULT LoadMesh( LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTXFILEDATA pFileData, 
                      CD3DFrame* pParentFrame );
    HRESULT LoadFrame( LPDIRECT3DDEVICE8 pd3dDevice, LPDIRECTXFILEDATA pFileData, 
                       CD3DFrame* pParentFrame );
public:
    HRESULT Create( LPDIRECT3DDEVICE8 pd3dDevice, TCHAR* strFilename );
    HRESULT CreateFromResource( LPDIRECT3DDEVICE8 pd3dDevice, TCHAR* strResource, TCHAR* strType );
    // For pure devices, specify the world transform. If the world transform is not
    // specified on pure devices, this function will fail.
    HRESULT Render( LPDIRECT3DDEVICE8 pd3dDevice, D3DXMATRIX* pmatWorldMatrix = NULL );

    CD3DFile() : CD3DFrame( _T("CD3DFile_Root") ) {}
};



#endif



