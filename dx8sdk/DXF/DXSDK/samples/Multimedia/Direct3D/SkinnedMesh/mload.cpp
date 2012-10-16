//-----------------------------------------------------------------------------
// File: mload.cpp
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <mmsystem.h>
#include <objbase.h>
#include <malloc.h> // _alloca
#include <stdio.h>
#include <d3d8.h>
#include <d3dx8.h>
#include "resource.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "D3DRes.h"
#include "rmxfguid.h"
#include "rmxftmpl.h"
#include "dxutil.h"
#include "SkinnedMesh.h"


HRESULT CalculateSum(SFrame *pframe, D3DXMATRIX *pmatCur, D3DXVECTOR3 *pvCenter, 
                     UINT *pcVertices)
{
    HRESULT hr = S_OK;
    PBYTE pbPoints = NULL;
    UINT cVerticesLocal = 0;
    PBYTE pbCur;
    D3DXVECTOR3 *pvCur;
    D3DXVECTOR3 vTransformedCur;
    UINT iPoint;
    SMeshContainer *pmcCur;
    SFrame *pframeCur;
    UINT cVertices;
    D3DXMATRIXA16 matLocal;
    
    D3DXMatrixMultiply(&matLocal, &pframe->matRot, pmatCur);
    
    pmcCur = pframe->pmcMesh;
    while (pmcCur != NULL)
    {
        DWORD fvfsize = D3DXGetFVFVertexSize(pmcCur->pMesh->GetFVF());
        
        cVertices = pmcCur->pMesh->GetNumVertices();
        
        hr = pmcCur->pMesh->LockVertexBuffer(0, &pbPoints);
        if (FAILED(hr))
            goto e_Exit;
        
        for( iPoint=0, pbCur = pbPoints; iPoint < cVertices; iPoint++, pbCur += fvfsize)
        {
            pvCur = (D3DXVECTOR3*)pbCur;
            
            if ((pvCur->x != 0.0) || (pvCur->y != 0.0) || (pvCur->z != 0.0))
            {
                cVerticesLocal++;
                
                D3DXVec3TransformCoord(&vTransformedCur, pvCur, &matLocal);
                
                pvCenter->x += vTransformedCur.x;
                pvCenter->y += vTransformedCur.y;
                pvCenter->z += vTransformedCur.z;
            }
        }
        
        
        pmcCur->pMesh->UnlockVertexBuffer();
        pbPoints = NULL;
        
        pmcCur = pmcCur->pmcNext;
    }
    
    *pcVertices += cVerticesLocal;
    
    pframeCur = pframe->pframeFirstChild;
    while (pframeCur != NULL)
    {
        hr = CalculateSum(pframeCur, &matLocal, pvCenter, pcVertices);
        if (FAILED(hr))
            goto e_Exit;
        
        pframeCur = pframeCur->pframeSibling;
    }
    
e_Exit:
    if (pbPoints != NULL)
    {
        pmcCur->pMesh->UnlockVertexBuffer();
    }
    
    return hr;
}




HRESULT CalculateRadius(SFrame *pframe, D3DXMATRIX *pmatCur, D3DXVECTOR3 *pvCenter, 
                        float *pfRadiusSq)
{
    HRESULT hr = S_OK;
    PBYTE pbPoints = NULL;
    PBYTE pbCur;
    D3DXVECTOR3 *pvCur;
    D3DXVECTOR3 vDist;;
    UINT iPoint;
    UINT cVertices;
    SMeshContainer *pmcCur;
    SFrame *pframeCur;
    float fRadiusLocalSq;
    float fDistSq;
    D3DXMATRIXA16 matLocal;
    
    D3DXMatrixMultiply(&matLocal, &pframe->matRot, pmatCur);
    
    pmcCur = pframe->pmcMesh;
    fRadiusLocalSq = *pfRadiusSq;
    while (pmcCur != NULL)
    {
        DWORD fvfsize = D3DXGetFVFVertexSize(pmcCur->pMesh->GetFVF());
        
        cVertices = pmcCur->pMesh->GetNumVertices();
        
        hr = pmcCur->pMesh->LockVertexBuffer(0, &pbPoints);
        if (FAILED(hr))
            goto e_Exit;
        
        for( iPoint=0, pbCur = pbPoints; iPoint < cVertices; iPoint++, pbCur += fvfsize )
        {
            pvCur = (D3DXVECTOR3*)pbCur;
            
            if ((pvCur->x == 0.0) && (pvCur->y == 0.0) && (pvCur->z == 0.0))
                continue;
            
            D3DXVec3TransformCoord(&vDist, pvCur, &matLocal);
            
            vDist -= *pvCenter;
            
            fDistSq = D3DXVec3LengthSq(&vDist);
            
            if( fDistSq > fRadiusLocalSq )
                fRadiusLocalSq = fDistSq;
        }
        
        
        pmcCur->pMesh->UnlockVertexBuffer();
        pbPoints = NULL;
        
        pmcCur = pmcCur->pmcNext;
    }
    
    *pfRadiusSq = fRadiusLocalSq;
    
    pframeCur = pframe->pframeFirstChild;
    while (pframeCur != NULL)
    {
        hr = CalculateRadius(pframeCur, &matLocal, pvCenter, pfRadiusSq);
        if (FAILED(hr))
            goto e_Exit;
        
        pframeCur = pframeCur->pframeSibling;
    }
    
e_Exit:
    if (pbPoints != NULL)
    {
        pmcCur->pMesh->UnlockVertexBuffer();
    }
    
    return hr;
}




HRESULT CalculateBoundingSphere(SDrawElement *pdeCur)
{
    HRESULT hr = S_OK;
    D3DXVECTOR3 vCenter(0,0,0);
    UINT cVertices = 0;
    float fRadiusSq = 0;
    D3DXMATRIXA16 matCur;
    
    D3DXMatrixIdentity(&matCur);
    hr = CalculateSum(pdeCur->pframeRoot, &matCur, &vCenter, &cVertices);
    if (FAILED(hr))
        goto e_Exit;
    
    if (cVertices > 0)
    {
        vCenter /= (float)cVertices;
        
        D3DXMatrixIdentity(&matCur);
        hr = CalculateRadius(pdeCur->pframeRoot, &matCur, &vCenter, &fRadiusSq);
        if (FAILED(hr))
            goto e_Exit;
    }
    
    pdeCur->fRadius = (float)sqrt((double)fRadiusSq);;
    pdeCur->vCenter = vCenter;
e_Exit:
    return hr;
}




HRESULT CMyD3DApplication::FindBones(SFrame *pframeCur, SDrawElement *pde)
{
    HRESULT hr = S_OK;
    SMeshContainer *pmcMesh;
    SFrame *pframeChild;
    
    pmcMesh = pframeCur->pmcMesh;
    while (pmcMesh != NULL)
    {
        if (pmcMesh->m_pSkinMesh)
        {
            char** pBoneName = static_cast<char**>(pmcMesh->m_pBoneNamesBuf->GetBufferPointer());
            for (DWORD i = 0; i < pmcMesh->m_pSkinMesh->GetNumBones(); ++i)
            {
                SFrame* pFrame = pde->FindFrame(pBoneName[i]);
                pmcMesh->m_pBoneMatrix[i] = &(pFrame->matCombined);
            }
        }
        pmcMesh = pmcMesh->pmcNext;
    }
    
    pframeChild = pframeCur->pframeFirstChild;
    while (pframeChild != NULL)
    {
        hr = FindBones(pframeChild, pde);
        if (FAILED(hr))
            return hr;
        
        pframeChild = pframeChild->pframeSibling;
    }
    
    return S_OK;
}




HRESULT CMyD3DApplication::LoadMeshHierarchy()
{
    TCHAR* pszFile = m_szPath;
    SDrawElement *pdeMesh = NULL;
    HRESULT hr = S_OK;
    LPDIRECTXFILE pxofapi = NULL;
    LPDIRECTXFILEENUMOBJECT pxofenum = NULL;
    LPDIRECTXFILEDATA pxofobjCur = NULL;
    DWORD dwOptions;
    int cchFileName;

    if (pszFile == NULL)
        return E_INVALIDARG;
    
    pdeMesh = new SDrawElement();
    
    delete pdeMesh->pframeRoot;
    pdeMesh->pframeAnimHead = NULL;
    
    pdeMesh->pframeRoot = new SFrame();
    if (pdeMesh->pframeRoot == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    
    
    dwOptions = 0;
    
    cchFileName = strlen(pszFile);
    if (cchFileName < 2)
    {
        hr = E_FAIL;
        goto e_Exit;
    }
    
    hr = DirectXFileCreate(&pxofapi);
    if (FAILED(hr))
        goto e_Exit;
    
    // Register templates for d3drm.
    hr = pxofapi->RegisterTemplates((LPVOID)D3DRM_XTEMPLATES,
        D3DRM_XTEMPLATE_BYTES);
    if (FAILED(hr))
        goto e_Exit;
    
    // Create enum object.
    hr = pxofapi->CreateEnumObject((LPVOID)pszFile,
        DXFILELOAD_FROMFILE,
        &pxofenum);
    if (FAILED(hr))
        goto e_Exit;
    
    
    // Enumerate top level objects.
    // Top level objects are always data object.
    while (SUCCEEDED(pxofenum->GetNextDataObject(&pxofobjCur)))
    {
        hr = LoadFrames(pxofobjCur, pdeMesh, dwOptions, m_dwFVF,
            m_pd3dDevice,
            pdeMesh->pframeRoot);
        GXRELEASE(pxofobjCur);
        
        if (FAILED(hr))
            goto e_Exit;
    }
    
    hr = FindBones(pdeMesh->pframeRoot, pdeMesh);
    if (FAILED(hr))
        goto e_Exit;
    
    
    delete []pdeMesh->szName;
    pdeMesh->szName = new char[cchFileName+1];
    if (pdeMesh->szName == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    memcpy(pdeMesh->szName, pszFile, cchFileName+1);

    // delete the current mesh, now that the load has succeeded
    DeleteSelectedMesh();

    // link into the draw list
    pdeMesh->pdeNext = m_pdeHead;
    m_pdeHead = pdeMesh;
    
    m_pdeSelected = pdeMesh;
    m_pmcSelectedMesh = pdeMesh->pframeRoot->pmcMesh;
    
    
    m_pframeSelected = pdeMesh->pframeRoot;
    
    hr = CalculateBoundingSphere(pdeMesh);
    if (FAILED(hr))
        goto e_Exit;
    
    SetProjectionMatrix();
    
    m_pdeSelected->fCurTime = 0.0f;
    m_pdeSelected->fMaxTime = 200.0f;
    
    D3DXMatrixTranslation(&m_pdeSelected->pframeRoot->matRot,
        -pdeMesh->vCenter.x, -pdeMesh->vCenter.y, -pdeMesh->vCenter.z);
    m_pdeSelected->pframeRoot->matRotOrig = m_pdeSelected->pframeRoot->matRot;
    
e_Exit:
    GXRELEASE(pxofobjCur);
    GXRELEASE(pxofenum);
    GXRELEASE(pxofapi);
    
    if (FAILED(hr))
    {
        delete pdeMesh;
    }
    
    return hr;
}




HRESULT CMyD3DApplication::LoadAnimation(LPDIRECTXFILEDATA pxofobjCur, SDrawElement *pde,
                                         DWORD options, DWORD fvf, LPDIRECT3DDEVICE8 pD3DDevice,
                                         SFrame *pframeParent)
{
    HRESULT hr = S_OK;
    SRotateKeyXFile *pFileRotateKey;
    SScaleKeyXFile *pFileScaleKey;
    SPositionKeyXFile *pFilePosKey;
    SMatrixKeyXFile *pFileMatrixKey;
    SFrame *pframeCur;
    LPDIRECTXFILEDATA pxofobjChild = NULL;
    LPDIRECTXFILEOBJECT pxofChild = NULL;
    LPDIRECTXFILEDATAREFERENCE pxofobjChildRef = NULL;
    const GUID *type;
    DWORD dwSize;
    PBYTE pData;
    DWORD dwKeyType;
    DWORD cKeys;
    DWORD iKey;
    DWORD cchName;
    char *szFrameName;
    
    pframeCur = new SFrame();
    if (pframeCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    pframeCur->bAnimationFrame = true;
    
    pframeParent->AddFrame(pframeCur);
    pde->AddAnimationFrame(pframeCur);
    
    // Enumerate child objects.
    // Child object can be data, data reference or binary.
    // Use QueryInterface() to find what type of object a child is.
    while (SUCCEEDED(pxofobjCur->GetNextObject(&pxofChild)))
    {
        // Query the child for it's FileDataReference
        hr = pxofChild->QueryInterface(IID_IDirectXFileDataReference,
            (LPVOID *)&pxofobjChildRef);
        if (SUCCEEDED(hr))
        {
            hr = pxofobjChildRef->Resolve(&pxofobjChild);
            if (SUCCEEDED(hr))
            {
                hr = pxofobjChild->GetType(&type);
                if (FAILED(hr))
                    goto e_Exit;
                
                if( TID_D3DRMFrame == *type )
                {
                    if (pframeCur->pframeToAnimate != NULL)
                    {
                        hr = E_INVALIDARG;
                        goto e_Exit;
                    }
                    
                    hr = pxofobjChild->GetName(NULL, &cchName);
                    if (FAILED(hr))
                        goto e_Exit;
                    
                    if (cchName == 0)
                    {
                        hr = E_INVALIDARG;
                        goto e_Exit;
                        
                    }
                    
                    szFrameName = (char*)_alloca(cchName);
                    if (szFrameName == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                        goto e_Exit;
                    }
                    
                    hr = pxofobjChild->GetName(szFrameName, &cchName);
                    if (FAILED(hr))
                        goto e_Exit;
                    
                    pframeCur->pframeToAnimate = pde->FindFrame(szFrameName);
                    if (pframeCur->pframeToAnimate == NULL)
                    {
                        hr = E_INVALIDARG;
                        goto e_Exit;
                    }
                }
                
                GXRELEASE(pxofobjChild);
            }
            
            GXRELEASE(pxofobjChildRef);
        }
        else
        {
            
            // Query the child for it's FileData
            hr = pxofChild->QueryInterface(IID_IDirectXFileData,
                (LPVOID *)&pxofobjChild);
            if (SUCCEEDED(hr))
            {
                hr = pxofobjChild->GetType(&type);
                if (FAILED(hr))
                    goto e_Exit;
                
                if ( TID_D3DRMFrame == *type )
                {
                    hr = LoadFrames(pxofobjChild, pde, options, fvf, pD3DDevice, pframeCur);
                    if (FAILED(hr))
                        goto e_Exit;
                }
                else if ( TID_D3DRMAnimationOptions == *type )
                {
                    //ParseAnimOptions(pChildData,pParentFrame);
                    //i=2;
                }
                else if ( TID_D3DRMAnimationKey == *type )
                {
                    hr = pxofobjChild->GetData( NULL, &dwSize, (PVOID*)&pData );
                    if (FAILED(hr))
                        goto e_Exit;
                    
                    dwKeyType = ((DWORD*)pData)[0];
                    cKeys = ((DWORD*)pData)[1];
                    
                    if (dwKeyType == 0)
                    {
                        if (pframeCur->m_pRotateKeys != NULL)
                        {
                            hr = E_INVALIDARG;
                            goto e_Exit;
                        }
                        
                        pframeCur->m_pRotateKeys = new SRotateKey[cKeys];
                        if (pframeCur->m_pRotateKeys == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            goto e_Exit;
                        }
                        
                        pframeCur->m_cRotateKeys = cKeys;
                        //NOTE x files are w x y z and QUATERNIONS are x y z w
                        
                        pFileRotateKey =  (SRotateKeyXFile*)(pData + (sizeof(DWORD) * 2));
                        for (iKey = 0;iKey < cKeys; iKey++)
                        {
                            pframeCur->m_pRotateKeys[iKey].dwTime = pFileRotateKey->dwTime;
                            pframeCur->m_pRotateKeys[iKey].quatRotate.x = pFileRotateKey->x;
                            pframeCur->m_pRotateKeys[iKey].quatRotate.y = pFileRotateKey->y;
                            pframeCur->m_pRotateKeys[iKey].quatRotate.z = pFileRotateKey->z;
                            pframeCur->m_pRotateKeys[iKey].quatRotate.w = pFileRotateKey->w;
                            
                            pFileRotateKey += 1;
                        }
                    }
                    else if (dwKeyType == 1)
                    {
                        if (pframeCur->m_pScaleKeys != NULL)
                        {
                            hr = E_INVALIDARG;
                            goto e_Exit;
                        }
                        
                        pframeCur->m_pScaleKeys = new SScaleKey[cKeys];
                        if (pframeCur->m_pScaleKeys == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            goto e_Exit;
                        }
                        
                        pframeCur->m_cScaleKeys = cKeys;
                        
                        pFileScaleKey =  (SScaleKeyXFile*)(pData + (sizeof(DWORD) * 2));
                        for (iKey = 0;iKey < cKeys; iKey++)
                        {
                            pframeCur->m_pScaleKeys[iKey].dwTime = pFileScaleKey->dwTime;
                            pframeCur->m_pScaleKeys[iKey].vScale = pFileScaleKey->vScale;
                            
                            pFileScaleKey += 1;
                        }
                    }
                    else if (dwKeyType == 2)
                    {
                        if (pframeCur->m_pPositionKeys != NULL)
                        {
                            hr = E_INVALIDARG;
                            goto e_Exit;
                        }
                        
                        pframeCur->m_pPositionKeys = new SPositionKey[cKeys];
                        if (pframeCur->m_pPositionKeys == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            goto e_Exit;
                        }
                        
                        pframeCur->m_cPositionKeys = cKeys;
                        
                        pFilePosKey =  (SPositionKeyXFile*)(pData + (sizeof(DWORD) * 2));
                        for (iKey = 0;iKey < cKeys; iKey++)
                        {
                            pframeCur->m_pPositionKeys[iKey].dwTime = pFilePosKey->dwTime;
                            pframeCur->m_pPositionKeys[iKey].vPos = pFilePosKey->vPos;
                            
                            pFilePosKey += 1;
                        }
                    }
                    else if (dwKeyType == 4)
                    {
                        if (pframeCur->m_pMatrixKeys != NULL)
                        {
                            hr = E_INVALIDARG;
                            goto e_Exit;
                        }
                        
                        pframeCur->m_pMatrixKeys = new SMatrixKey[cKeys];
                        if (pframeCur->m_pMatrixKeys == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            goto e_Exit;
                        }
                        
                        pframeCur->m_cMatrixKeys = cKeys;
                        
                        pFileMatrixKey =  (SMatrixKeyXFile*)(pData + (sizeof(DWORD) * 2));
                        for (iKey = 0;iKey < cKeys; iKey++)
                        {
                            pframeCur->m_pMatrixKeys[iKey].dwTime = pFileMatrixKey->dwTime;
                            pframeCur->m_pMatrixKeys[iKey].mat = pFileMatrixKey->mat;
                            
                            pFileMatrixKey += 1;
                        }
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                        goto e_Exit;
                    }
                }
                
                GXRELEASE(pxofobjChild);
            }
        }
        
        GXRELEASE(pxofChild);
    }
    
e_Exit:
    GXRELEASE(pxofobjChild);
    GXRELEASE(pxofChild);
    GXRELEASE(pxofobjChildRef);
    return hr;
}




HRESULT CMyD3DApplication::LoadAnimationSet(LPDIRECTXFILEDATA pxofobjCur, SDrawElement *pde,
                                            DWORD options, DWORD fvf, LPDIRECT3DDEVICE8 pD3DDevice,
                                            SFrame *pframeParent)
{
    SFrame *pframeCur;
    const GUID *type;
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pxofobjChild = NULL;
    LPDIRECTXFILEOBJECT pxofChild = NULL;
    DWORD cchName;
    
    pframeCur = new SFrame();
    if (pframeCur == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    pframeCur->bAnimationFrame = true;
    
    pframeParent->AddFrame(pframeCur);
    
    hr = pxofobjCur->GetName(NULL, &cchName);
    if (FAILED(hr))
        goto e_Exit;
    
    if (cchName > 0)
    {
        pframeCur->szName = new char[cchName];
        if (pframeCur->szName == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
        
        hr = pxofobjCur->GetName(pframeCur->szName, &cchName);
        if (FAILED(hr))
            goto e_Exit;
    }
    
    
    // Enumerate child objects.
    // Child object can be data, data reference or binary.
    // Use QueryInterface() to find what type of object a child is.
    while (SUCCEEDED(pxofobjCur->GetNextObject(&pxofChild)))
    {
        // Query the child for it's FileData
        hr = pxofChild->QueryInterface(IID_IDirectXFileData,
            (LPVOID *)&pxofobjChild);
        if (SUCCEEDED(hr))
        {
            hr = pxofobjChild->GetType(&type);
            if (FAILED(hr))
                goto e_Exit;
            
            if( TID_D3DRMAnimation == *type )
            {
                hr = LoadAnimation(pxofobjChild, pde, options, fvf, pD3DDevice, pframeCur);
                if (FAILED(hr))
                    goto e_Exit;
            }
            
            GXRELEASE(pxofobjChild);
        }
        
        GXRELEASE(pxofChild);
    }
    
e_Exit:
    GXRELEASE(pxofobjChild);
    GXRELEASE(pxofChild);
    return hr;
}





HRESULT CMyD3DApplication::GenerateMesh(SMeshContainer *pmcMesh)
{
    // ASSUMPTION:  pmcMesh->m_rgiAdjacency contains the current adjacency
    HRESULT hr = S_OK;
    LPDIRECT3DDEVICE8 pDevice = NULL;
    DWORD cFaces = pmcMesh->m_pSkinMesh->GetNumFaces();

    hr  = pmcMesh->m_pSkinMesh->GetDevice(&pDevice);
    if (FAILED(hr))
        goto e_Exit;

    GXRELEASE(pmcMesh->pMesh);
    delete [] m_pBoneMatrices;

    pmcMesh->pMesh      = NULL;
    m_pBoneMatrices     = NULL;
    
    if (m_method == D3DNONINDEXED)
    {
        LPD3DXBONECOMBINATION   rgBoneCombinations;

        hr = pmcMesh->m_pSkinMesh->ConvertToBlendedMesh
                                   (
                                       D3DXMESH_MANAGED|D3DXMESHOPT_VERTEXCACHE, 
                                       pmcMesh->m_rgiAdjacency, 
                                       NULL, 
                                       &pmcMesh->cpattr, 
                                       &pmcMesh->m_pBoneCombinationBuf, 
                                       NULL,
                                       NULL,
                                       &pmcMesh->pMesh
                                   );
        if (FAILED(hr))
            goto e_Exit;

        // calculate the max face influence count

        if ((pmcMesh->pMesh->GetFVF() & D3DFVF_POSITION_MASK) != D3DFVF_XYZ)
        {
            pmcMesh->m_maxFaceInfl = 1 + ((pmcMesh->pMesh->GetFVF() & D3DFVF_POSITION_MASK) - D3DFVF_XYZRHW) / 2;
        }
        else
        {
            pmcMesh->m_maxFaceInfl = 1;
        }

        /* If the device can only do 2 matrix blends, ConvertToBlendedMesh cannot approximate all meshes to it
           Thus we split the mesh in two parts: The part that uses at most 2 matrices and the rest. The first is
           drawn using the device's HW vertex processing and the rest is drawn using SW vertex processing. */
        if (m_d3dCaps.MaxVertexBlendMatrices <= 2)       
        {
            // calculate the index of the attribute table to split on
            rgBoneCombinations  = reinterpret_cast<LPD3DXBONECOMBINATION>(pmcMesh->m_pBoneCombinationBuf->GetBufferPointer());

            for (pmcMesh->iAttrSplit = 0; pmcMesh->iAttrSplit < pmcMesh->cpattr; pmcMesh->iAttrSplit++)
            {
                DWORD   cInfl   = 0;

                for (DWORD iInfl = 0; iInfl < pmcMesh->m_maxFaceInfl; iInfl++)
                {
                    if (rgBoneCombinations[pmcMesh->iAttrSplit].BoneId[iInfl] != UINT_MAX)
                    {
                        ++cInfl;
                    }
                }

                if (cInfl > m_d3dCaps.MaxVertexBlendMatrices)
                {
                    break;
                }
            }

            // if there is both HW and SW, add the Software Processing flag
            if (pmcMesh->iAttrSplit < pmcMesh->cpattr)
            {
                LPD3DXMESH pMeshTmp;

                hr = pmcMesh->pMesh->CloneMeshFVF(D3DXMESH_SOFTWAREPROCESSING|pmcMesh->pMesh->GetOptions(), 
                                                    pmcMesh->pMesh->GetFVF(),
                                                    pDevice, &pMeshTmp);
                if (FAILED(hr))
                {
                    goto e_Exit;
                }

                pmcMesh->pMesh->Release();
                pmcMesh->pMesh = pMeshTmp;
                pMeshTmp = NULL;
            }
        }
        else
        {
            pmcMesh->iAttrSplit = pmcMesh->cpattr;
        }
    }
    else if (m_method == D3DINDEXEDVS)
    {
        // Get palette size
        pmcMesh->m_paletteSize = min(28, pmcMesh->m_pSkinMesh->GetNumBones());

        DWORD flags = D3DXMESHOPT_VERTEXCACHE;
        if (m_d3dCaps.VertexShaderVersion >= D3DVS_VERSION(1, 1))
        {
            pmcMesh->m_bUseSW = false;
            flags |= D3DXMESH_MANAGED;
        }
        else
        {
            pmcMesh->m_bUseSW = true;
            flags |= D3DXMESH_SYSTEMMEM;
        }

        hr = pmcMesh->m_pSkinMesh->ConvertToIndexedBlendedMesh(flags, pmcMesh->m_rgiAdjacency, pmcMesh->m_paletteSize, NULL,
            &pmcMesh->cpattr, &pmcMesh->m_pBoneCombinationBuf, NULL, NULL, &pmcMesh->pMesh);
        if (FAILED(hr))
            goto e_Exit;

        if ((pmcMesh->pMesh->GetFVF() & D3DFVF_POSITION_MASK) != D3DFVF_XYZ)
        {
            pmcMesh->m_maxFaceInfl = ((pmcMesh->pMesh->GetFVF() & D3DFVF_POSITION_MASK) - D3DFVF_XYZRHW) / 2;
        }
        else
        {
            pmcMesh->m_maxFaceInfl = 1;
        }

        // FVF has to match our declarator. Vertex shaders are not as forgiving as FF pipeline
        DWORD newFVF = (pmcMesh->pMesh->GetFVF() & D3DFVF_POSITION_MASK) | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4;
        if (newFVF != pmcMesh->pMesh->GetFVF())
        {
            LPD3DXMESH pMesh;
            hr = pmcMesh->pMesh->CloneMeshFVF(pmcMesh->pMesh->GetOptions(), newFVF, pDevice, &pMesh);
            if (!FAILED(hr))
            {
                pmcMesh->pMesh->Release();
                pmcMesh->pMesh = pMesh;
                pMesh = NULL;
            }
        }
    }
    else if (m_method == D3DINDEXED)
    {
        DWORD maxFaceInfl;
        DWORD flags = D3DXMESHOPT_VERTEXCACHE;

        hr = pmcMesh->m_pSkinMesh->GetMaxFaceInfluences(&maxFaceInfl);
        if (FAILED(hr))
            goto e_Exit;

        // 12 entry palette guarantees that any triangle (4 independent influences per vertex of a tri)
        // can be handled
        maxFaceInfl = min(maxFaceInfl, 12);

        if (m_d3dCaps.MaxVertexBlendMatrixIndex + 1 < maxFaceInfl)
        {
            // HW does not support indexed vertex blending. Use SW instead
            pmcMesh->m_paletteSize = min(256, pmcMesh->m_pSkinMesh->GetNumBones());
            pmcMesh->m_bUseSW = true;
            flags |= D3DXMESH_SYSTEMMEM;
        }
        else
        {
            pmcMesh->m_paletteSize = min(m_d3dCaps.MaxVertexBlendMatrixIndex + 1, pmcMesh->m_pSkinMesh->GetNumBones());
            pmcMesh->m_bUseSW = false;
            flags |= D3DXMESH_MANAGED;
        }

        hr = pmcMesh->m_pSkinMesh->ConvertToIndexedBlendedMesh(flags, pmcMesh->m_rgiAdjacency, pmcMesh->m_paletteSize, NULL,
            &pmcMesh->cpattr, &pmcMesh->m_pBoneCombinationBuf, NULL, NULL, &pmcMesh->pMesh);
        if (FAILED(hr))
            goto e_Exit;

        // Here we are talking of max vertex influence which we determine from 
        // the FVF of the returned mesh
        if ((pmcMesh->pMesh->GetFVF() & D3DFVF_POSITION_MASK) != D3DFVF_XYZ)
        {
            pmcMesh->m_maxFaceInfl = ((pmcMesh->pMesh->GetFVF() & D3DFVF_POSITION_MASK) - D3DFVF_XYZRHW) / 2;
        }
        else
        {
            pmcMesh->m_maxFaceInfl = 1;
        }
    }
    else if (m_method == SOFTWARE)
    {
        hr = pmcMesh->m_pSkinMesh->GenerateSkinnedMesh
                                   (
                                       D3DXMESH_WRITEONLY,          // options
                                       0.0f,                        // minimum bone weight allowed
                                       pmcMesh->m_rgiAdjacency,     // adjacency of in-mesh
                                       NULL,     // adjacency of out-mesh
                                       NULL,     // face remap array
                                       NULL,     // vertex remap buffer
                                       &pmcMesh->pMesh              // out-mesh
                                   );
        if (FAILED(hr))
            goto e_Exit;

        hr = pmcMesh->pMesh->GetAttributeTable(NULL, &pmcMesh->cpattr);
        if (FAILED(hr))
            goto e_Exit;

        delete[] pmcMesh->m_pAttrTable;
        pmcMesh->m_pAttrTable  = new D3DXATTRIBUTERANGE[pmcMesh->cpattr];
        if (pmcMesh->m_pAttrTable == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        hr = pmcMesh->pMesh->GetAttributeTable(pmcMesh->m_pAttrTable, NULL);
        if (FAILED(hr))
            goto e_Exit;

        hr = pmcMesh->m_pSkinMesh->GetMaxFaceInfluences(&pmcMesh->m_maxFaceInfl);
        if (FAILED(hr))
            goto e_Exit;

        // Allocate space for blend matrices
        m_pBoneMatrices  = new D3DXMATRIXA16[m_maxBones];
        if (m_pBoneMatrices == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
    }
    pmcMesh->m_Method = m_method;

e_Exit:

    GXRELEASE(pDevice);
    return hr;
}




HRESULT CMyD3DApplication::LoadMesh(LPDIRECTXFILEDATA pxofobjCur,
                                    DWORD options, DWORD fvf, LPDIRECT3DDEVICE8 pD3DDevice,
                                    SFrame *pframeParent)
{
    HRESULT hr = S_OK;
    SMeshContainer *pmcMesh = NULL;
    LPD3DXBUFFER pbufMaterials = NULL;
    LPD3DXBUFFER pbufAdjacency = NULL;
    DWORD cchName;
    UINT cFaces;
    UINT iMaterial;
    LPDIRECT3DDEVICE8 m_pDevice = m_pd3dDevice;
    LPDWORD pAdjacencyIn;
    
    pmcMesh = new SMeshContainer();
    if (pmcMesh == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }
    
    hr = pxofobjCur->GetName(NULL, &cchName);
    if (FAILED(hr))
        goto e_Exit;
    
    if (cchName > 0)
    {
        pmcMesh->szName = new char[cchName];
        if (pmcMesh->szName == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
        
        hr = pxofobjCur->GetName(pmcMesh->szName, &cchName);
        if (FAILED(hr))
            goto e_Exit;
    }
    
    hr = D3DXLoadSkinMeshFromXof(pxofobjCur, options, pD3DDevice, &pbufAdjacency, &pbufMaterials, &pmcMesh->cMaterials, 
        &pmcMesh->m_pBoneNamesBuf, &pmcMesh->m_pBoneOffsetBuf, &pmcMesh->m_pSkinMesh);
    if (FAILED(hr))
    {
        if (hr == D3DXERR_LOADEDMESHASNODATA)
            hr = S_OK;
        goto e_Exit;
    }
    
    cFaces = pmcMesh->m_pSkinMesh->GetNumFaces();

    pAdjacencyIn = static_cast<LPDWORD>(pbufAdjacency->GetBufferPointer());

    pmcMesh->m_rgiAdjacency = new DWORD[cFaces * 3];

    if (pmcMesh->m_rgiAdjacency == NULL)
    {
        hr = E_OUTOFMEMORY;

        goto e_Exit;
    }

    memcpy(pmcMesh->m_rgiAdjacency, pAdjacencyIn, cFaces * 3 * sizeof(DWORD));

    // Process skinning data
    if (pmcMesh->m_pSkinMesh->GetNumBones())
    {
        // Update max bones of any mesh in the app
        m_maxBones = max(pmcMesh->m_pSkinMesh->GetNumBones(), m_maxBones);
        
        pmcMesh->m_pBoneMatrix = new D3DXMATRIX*[pmcMesh->m_pSkinMesh->GetNumBones()];
        if (pmcMesh->m_pBoneMatrix == NULL)
            goto e_Exit;
        pmcMesh->m_pBoneOffsetMat = reinterpret_cast<D3DXMATRIX*>(pmcMesh->m_pBoneOffsetBuf->GetBufferPointer());
        
        hr = GenerateMesh(pmcMesh);

        if (FAILED(hr))
            goto e_Exit;
    }
    else
    {
        pmcMesh->m_pSkinMesh->GetOriginalMesh(&(pmcMesh->pMesh));
        pmcMesh->m_pSkinMesh->Release();
        pmcMesh->m_pSkinMesh = NULL;
        pmcMesh->cpattr = pmcMesh->cMaterials;
    }
    
    if ((pbufMaterials == NULL) || (pmcMesh->cMaterials == 0))
    {
        pmcMesh->rgMaterials = new D3DMATERIAL8[1];
        pmcMesh->pTextures = new LPDIRECT3DTEXTURE8[1];
        if (pmcMesh->rgMaterials == NULL || pmcMesh->pTextures == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
        
        memset(pmcMesh->rgMaterials, 0, sizeof(D3DMATERIAL8));
        pmcMesh->rgMaterials[0].Diffuse.r = 0.5f;
        pmcMesh->rgMaterials[0].Diffuse.g = 0.5f;
        pmcMesh->rgMaterials[0].Diffuse.b = 0.5f;
        pmcMesh->rgMaterials[0].Specular = pmcMesh->rgMaterials[0].Diffuse;
        pmcMesh->pTextures[0] = NULL;
    }
    else
    {
        pmcMesh->rgMaterials = new D3DMATERIAL8[pmcMesh->cMaterials];
        pmcMesh->pTextures = new LPDIRECT3DTEXTURE8[pmcMesh->cMaterials];
        if (pmcMesh->rgMaterials == NULL || pmcMesh->pTextures == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
        
        LPD3DXMATERIAL pMaterials = (LPD3DXMATERIAL)pbufMaterials->GetBufferPointer();
        
        for (iMaterial = 0; iMaterial < pmcMesh->cMaterials; iMaterial++)
        {
            
            pmcMesh->rgMaterials[iMaterial] = pMaterials[iMaterial].MatD3D;
            
            pmcMesh->pTextures[iMaterial] = NULL;
            if (pMaterials[iMaterial].pTextureFilename != NULL)
            {
                TCHAR szPath[MAX_PATH];
                DXUtil_FindMediaFile(szPath, pMaterials[iMaterial].pTextureFilename);

                hr = D3DXCreateTextureFromFile(m_pDevice, szPath, &(pmcMesh->pTextures[iMaterial]));
                if (FAILED(hr))
                    pmcMesh->pTextures[iMaterial] = NULL;
            }
        }
    }
    
    // add the mesh to the parent frame
    pframeParent->AddMesh(pmcMesh);
    pmcMesh = NULL;
    
e_Exit:
    delete pmcMesh;
    
    GXRELEASE(pbufAdjacency);
    GXRELEASE(pbufMaterials);

    return hr;
}




HRESULT CMyD3DApplication::LoadFrames(LPDIRECTXFILEDATA pxofobjCur, SDrawElement *pde,
                                      DWORD options, DWORD fvf, LPDIRECT3DDEVICE8 pD3DDevice,
                                      SFrame *pframeParent)
{
    HRESULT hr = S_OK;
    LPDIRECTXFILEDATA pxofobjChild = NULL;
    LPDIRECTXFILEOBJECT pxofChild = NULL;
    const GUID *type;
    DWORD cbSize;
    D3DXMATRIX *pmatNew;
    SFrame *pframeCur;
    DWORD cchName;
    
    // Get the type of the object
    hr = pxofobjCur->GetType(&type);
    if (FAILED(hr))
        goto e_Exit;
    
    
    if (*type == TID_D3DRMMesh)
    {
        hr = LoadMesh(pxofobjCur, options, fvf, pD3DDevice, pframeParent);
        if (FAILED(hr))
            goto e_Exit;
    }
    else if (*type == TID_D3DRMFrameTransformMatrix)
    {
        hr = pxofobjCur->GetData(NULL, &cbSize, (PVOID*)&pmatNew);
        if (FAILED(hr))
            goto e_Exit;
        
        // update the parents matrix with the new one
        pframeParent->matRot = *pmatNew;
        pframeParent->matRotOrig = *pmatNew;
    }
    else if (*type == TID_D3DRMAnimationSet)
    {
        LoadAnimationSet(pxofobjCur, pde, options, fvf, pD3DDevice, pframeParent);
    }
    else if (*type == TID_D3DRMAnimation)
    {
        LoadAnimation(pxofobjCur, pde, options, fvf, pD3DDevice, pframeParent);
    }
    else if (*type == TID_D3DRMFrame)
    {
        pframeCur = new SFrame();
        if (pframeCur == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
        
        hr = pxofobjCur->GetName(NULL, &cchName);
        if (FAILED(hr))
            goto e_Exit;
        
        if (cchName > 0)
        {
            pframeCur->szName = new char[cchName];
            if (pframeCur->szName == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto e_Exit;
            }
            
            hr = pxofobjCur->GetName(pframeCur->szName, &cchName);
            if (FAILED(hr))
                goto e_Exit;
        }
        
        pframeParent->AddFrame(pframeCur);
        
        // Enumerate child objects.
        // Child object can be data, data reference or binary.
        // Use QueryInterface() to find what type of object a child is.
        while (SUCCEEDED(pxofobjCur->GetNextObject(&pxofChild)))
        {
            // Query the child for it's FileData
            hr = pxofChild->QueryInterface(IID_IDirectXFileData,
                (LPVOID *)&pxofobjChild);
            if (SUCCEEDED(hr))
            {
                hr = LoadFrames(pxofobjChild, pde, options, fvf, pD3DDevice, pframeCur);
                if (FAILED(hr))
                    goto e_Exit;
                
                GXRELEASE(pxofobjChild);
            }
            
            GXRELEASE(pxofChild);
        }
        
    }
    
e_Exit:
    GXRELEASE(pxofobjChild);
    GXRELEASE(pxofChild);
    return hr;
}


         
                                      
HRESULT  CMyD3DApplication::DeleteSelectedMesh()
{
    if (m_pdeSelected != NULL)
    {
        SDrawElement *pdeCur = m_pdeHead;
        SDrawElement *pdePrev = NULL;
        while ((pdeCur != NULL) && (pdeCur != m_pdeSelected))
        {
            pdePrev = pdeCur;
            pdeCur = pdeCur->pdeNext;
        }

        if (pdePrev == NULL)
        {
            m_pdeHead = m_pdeHead->pdeNext;
        }
        else
        {
            pdePrev->pdeNext = pdeCur->pdeNext;
        }

        m_pdeSelected->pdeNext = NULL;
        if (m_pdeHead == m_pdeSelected)
            m_pdeHead = NULL;
        delete m_pdeSelected;
        m_pdeSelected = NULL;
    }

    return S_OK;
}
