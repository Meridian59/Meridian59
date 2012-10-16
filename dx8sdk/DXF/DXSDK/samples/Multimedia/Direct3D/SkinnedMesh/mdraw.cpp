//-----------------------------------------------------------------------------
// File: mdraw.cpp
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include <d3d8.h>
#include <d3dx8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "SkinnedMesh.h"


HRESULT CMyD3DApplication::DrawMeshContainer(SMeshContainer *pmcMesh)
{
    UINT ipattr;
    HRESULT hr = S_OK;
    LPD3DXBONECOMBINATION pBoneComb;
    DWORD AttribIdPrev;

    if (pmcMesh->m_pSkinMesh)
    {
        if (m_method != pmcMesh->m_Method)
        {
            GenerateMesh(pmcMesh);
        }

        if (m_method == D3DNONINDEXED)
        {
            AttribIdPrev = UNUSED32; 
            pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pmcMesh->m_pBoneCombinationBuf->GetBufferPointer());
            // Draw using default vtx processing of the device (typically HW)
            for (ipattr = 0; ipattr < pmcMesh->cpattr; ipattr++)
            {
                DWORD numBlend = 0;
                for (DWORD i = 0; i < pmcMesh->m_maxFaceInfl; ++i)
                {
                    if (pBoneComb[ipattr].BoneId[i] != UINT_MAX)
                    {
                        numBlend = i;
                    }
                }

                if (m_d3dCaps.MaxVertexBlendMatrices >= numBlend + 1)
                {
                    for (DWORD i = 0; i < pmcMesh->m_maxFaceInfl; ++i)
                    {
                        DWORD matid = pBoneComb[ipattr].BoneId[i];
                        if (matid != UINT_MAX)
                        {
                            m_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(i), pmcMesh->m_pBoneMatrix[matid]);
                            m_pd3dDevice->MultiplyTransform(D3DTS_WORLDMATRIX(i), &pmcMesh->m_pBoneOffsetMat[matid]);
                        }
                    }

                    m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, numBlend);

                    if ((AttribIdPrev != pBoneComb[ipattr].AttribId) || (AttribIdPrev == UNUSED32))
                    {
                        m_pd3dDevice->SetMaterial(&(pmcMesh->rgMaterials[pBoneComb[ipattr].AttribId]));
                        m_pd3dDevice->SetTexture(0, pmcMesh->pTextures[pBoneComb[ipattr].AttribId]);
                        AttribIdPrev = pBoneComb[ipattr].AttribId;
                    }

		            hr = pmcMesh->pMesh->DrawSubset( ipattr );
                    if(FAILED(hr))
                        return hr;
                }

            }

            // If necessary, draw parts that HW could not handle using SW
            if (pmcMesh->iAttrSplit < pmcMesh->cpattr)
            {
                AttribIdPrev = UNUSED32; 
                m_pd3dDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, TRUE);
                for (ipattr = pmcMesh->iAttrSplit; ipattr < pmcMesh->cpattr; ipattr++)
                {
                    DWORD numBlend = 0;
                    for (DWORD i = 0; i < pmcMesh->m_maxFaceInfl; ++i)
                    {
                        if (pBoneComb[ipattr].BoneId[i] != UINT_MAX)
                        {
                            numBlend = i;
                        }
                    }

                    if (m_d3dCaps.MaxVertexBlendMatrices < numBlend + 1)
                    {
                        for (DWORD i = 0; i < pmcMesh->m_maxFaceInfl; ++i)
                        {
                            DWORD matid = pBoneComb[ipattr].BoneId[i];
                            if (matid != UINT_MAX)
                            {
                                m_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(i), pmcMesh->m_pBoneMatrix[matid]);
                                m_pd3dDevice->MultiplyTransform(D3DTS_WORLDMATRIX(i), &pmcMesh->m_pBoneOffsetMat[matid]);
                            }
                        }

                        m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, numBlend);

                        if ((AttribIdPrev != pBoneComb[ipattr].AttribId) || (AttribIdPrev == UNUSED32))
                        {
                            m_pd3dDevice->SetMaterial(&(pmcMesh->rgMaterials[pBoneComb[ipattr].AttribId]));
                            m_pd3dDevice->SetTexture(0, pmcMesh->pTextures[pBoneComb[ipattr].AttribId]);
                            AttribIdPrev = pBoneComb[ipattr].AttribId;
                        }

		                hr = pmcMesh->pMesh->DrawSubset( ipattr );
                        if(FAILED(hr))
                            return hr;
                    }
                }
                m_pd3dDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, FALSE);
            }
            m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, 0);
        }
        else if (m_method == D3DINDEXEDVS)
        {
            // Use COLOR instead of UBYTE4 since Geforce3 does not support it
            // vConst.w should be 3, but due to about hack, mul by 255 and add epsilon
            D3DXVECTOR4 vConst( 1.0f, 0.0f, 0.0f, 765.01f );
            LPDIRECT3DVERTEXBUFFER8 pVB;
            LPDIRECT3DINDEXBUFFER8 pIB;

            if (pmcMesh->m_bUseSW)
            {
                m_pd3dDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, TRUE);
            }

            pmcMesh->pMesh->GetVertexBuffer(&pVB);
            pmcMesh->pMesh->GetIndexBuffer(&pIB);
            hr = m_pd3dDevice->SetStreamSource(0, pVB, D3DXGetFVFVertexSize(pmcMesh->pMesh->GetFVF()));
            if(FAILED(hr))
                return hr;
            hr = m_pd3dDevice->SetIndices(pIB, 0);
            if(FAILED(hr))
                return hr;
            pVB->Release();
            pIB->Release();

            hr = m_pd3dDevice->SetVertexShader(m_dwIndexedVertexShader[pmcMesh->m_maxFaceInfl - 1]);
            if(FAILED(hr))
                return hr;

            pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pmcMesh->m_pBoneCombinationBuf->GetBufferPointer());
            for (ipattr = 0; ipattr < pmcMesh->cpattr; ipattr++)
            {
                for (DWORD i = 0; i < pmcMesh->m_paletteSize; ++i)
                {
                    DWORD matid = pBoneComb[ipattr].BoneId[i];
                    if (matid != UINT_MAX)
                    {
                        D3DXMATRIXA16 mat;
                        D3DXMatrixMultiply(&mat, &pmcMesh->m_pBoneOffsetMat[matid], pmcMesh->m_pBoneMatrix[matid]);
                        D3DXMatrixMultiplyTranspose(&mat, &mat, &m_mView);
                        m_pd3dDevice->SetVertexShaderConstant(i*3 + 9, &mat, 3);
                    }
                }

                // Sum of all ambient and emissive contribution
                D3DXCOLOR ambEmm;
                D3DXColorModulate(&ambEmm, &D3DXCOLOR(pmcMesh->rgMaterials[pBoneComb[ipattr].AttribId].Ambient),&D3DXCOLOR(.25, .25, .25, 1.0));
                ambEmm += D3DXCOLOR(pmcMesh->rgMaterials[pBoneComb[ipattr].AttribId].Emissive);
                m_pd3dDevice->SetVertexShaderConstant(8, &(pmcMesh->rgMaterials[pBoneComb[ipattr].AttribId].Diffuse), 1);
                m_pd3dDevice->SetVertexShaderConstant(7, &ambEmm, 1);
                vConst.y = pmcMesh->rgMaterials[pBoneComb[ipattr].AttribId].Power;
                m_pd3dDevice->SetVertexShaderConstant(0, &vConst, 1);

                m_pd3dDevice->SetTexture(0, pmcMesh->pTextures[pBoneComb[ipattr].AttribId]);

                hr = m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
                                             pBoneComb[ipattr].VertexStart, pBoneComb[ipattr].VertexCount,
                                             pBoneComb[ipattr].FaceStart * 3, pBoneComb[ipattr].FaceCount);
                if(FAILED(hr))
                    return hr;
            }

            if (pmcMesh->m_bUseSW)
            {
                m_pd3dDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, FALSE);
            }
        }
        else if (m_method == D3DINDEXED)
        {
            if (pmcMesh->m_bUseSW)
            {
                m_pd3dDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, TRUE);
            }

            if (pmcMesh->m_maxFaceInfl == 1)
                m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS);
            else
                m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, pmcMesh->m_maxFaceInfl - 1);
            if (pmcMesh->m_maxFaceInfl)
                m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
            pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pmcMesh->m_pBoneCombinationBuf->GetBufferPointer());
            for (ipattr = 0; ipattr < pmcMesh->cpattr; ipattr++)
            {
                for (DWORD i = 0; i < pmcMesh->m_paletteSize; ++i)
                {
                    DWORD matid = pBoneComb[ipattr].BoneId[i];
                    if (matid != UINT_MAX)
                    {
                        m_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(i), pmcMesh->m_pBoneMatrix[matid]);
                        m_pd3dDevice->MultiplyTransform(D3DTS_WORLDMATRIX(i), &pmcMesh->m_pBoneOffsetMat[matid]);
                    }
                }
                
                m_pd3dDevice->SetMaterial(&(pmcMesh->rgMaterials[pBoneComb[ipattr].AttribId]));
                m_pd3dDevice->SetTexture(0, pmcMesh->pTextures[pBoneComb[ipattr].AttribId]);

                hr = pmcMesh->pMesh->DrawSubset( ipattr );
                if(FAILED(hr))
                    return hr;
            }
            m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
            m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, 0);

            if (pmcMesh->m_bUseSW)
            {
                m_pd3dDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, FALSE);
            }
        }
        else if (m_method == SOFTWARE)
        {
            D3DXMATRIX  Identity;
            DWORD       cBones  = pmcMesh->m_pSkinMesh->GetNumBones();

            // set up bone transforms
            for (DWORD iBone = 0; iBone < cBones; ++iBone)
            {
                D3DXMatrixMultiply
                (
                    &m_pBoneMatrices[iBone],                 // output
                    &pmcMesh->m_pBoneOffsetMat[iBone], 
                    pmcMesh->m_pBoneMatrix[iBone]
                );
            }

            // set world transform
            D3DXMatrixIdentity(&Identity);
            hr = m_pd3dDevice->SetTransform(D3DTS_WORLD, &Identity);
            if (FAILED(hr))
                return hr;

            // generate skinned mesh
            hr = pmcMesh->m_pSkinMesh->UpdateSkinnedMesh(m_pBoneMatrices, NULL, pmcMesh->pMesh);
            if (FAILED(hr))
                return hr;

            for (ipattr = 0; ipattr < pmcMesh->cpattr; ipattr++)
            {
                m_pd3dDevice->SetMaterial(&(pmcMesh->rgMaterials[pmcMesh->m_pAttrTable[ipattr].AttribId]));
                m_pd3dDevice->SetTexture(0, pmcMesh->pTextures[pmcMesh->m_pAttrTable[ipattr].AttribId]);
		        hr  = pmcMesh->pMesh->DrawSubset(pmcMesh->m_pAttrTable[ipattr].AttribId);
                if (FAILED(hr))
                    return hr;
            }
            return hr;
        }
    }
    else
    {
        for (ipattr = 0; ipattr < pmcMesh->cpattr; ipattr++)
        {
            m_pd3dDevice->SetMaterial(&(pmcMesh->rgMaterials[ipattr]));
            m_pd3dDevice->SetTexture(0, pmcMesh->pTextures[ipattr]);
		    hr = pmcMesh->pMesh->DrawSubset( ipattr );
            if(FAILED(hr))
                return hr;
        }
    }

    return S_OK;
}




HRESULT CMyD3DApplication::UpdateFrames(SFrame *pframeCur, D3DXMATRIX &matCur)
{
    HRESULT hr = S_OK;
    pframeCur->matCombined = matCur;
    D3DXMatrixMultiply(&pframeCur->matCombined, &pframeCur->matRot, &matCur);
    D3DXMatrixMultiply(&pframeCur->matCombined, &pframeCur->matCombined, &pframeCur->matTrans );
    SFrame *pframeChild = pframeCur->pframeFirstChild;
    while (pframeChild != NULL)
    {
        hr = UpdateFrames(pframeChild, pframeCur->matCombined);
        if (FAILED(hr))
            return hr;

        pframeChild = pframeChild->pframeSibling;
    }
    return S_OK;
}




HRESULT CMyD3DApplication::DrawFrames(SFrame *pframeCur, UINT &cTriangles)
{
    HRESULT hr = S_OK;
    SMeshContainer *pmcMesh;
    SFrame *pframeChild;

    if (pframeCur->pmcMesh != NULL)
    {
        hr = m_pd3dDevice->SetTransform(D3DTS_WORLD, &pframeCur->matCombined);
        if(FAILED(hr))
            return hr;
    }

    pmcMesh = pframeCur->pmcMesh;
    while (pmcMesh != NULL)
    {
        hr = DrawMeshContainer(pmcMesh);
        if (FAILED(hr))
            return hr;

        cTriangles += pmcMesh->pMesh->GetNumFaces();

        pmcMesh = pmcMesh->pmcNext;
    }

    pframeChild = pframeCur->pframeFirstChild;
    while (pframeChild != NULL)
    {
        hr = DrawFrames(pframeChild, cTriangles);
        if (FAILED(hr))
            return hr;

        pframeChild = pframeChild->pframeSibling;
    }

    return S_OK;
}




void SFrame::SetTime(float fGlobalTime)
{
    UINT iKey;
    UINT dwp2;
    UINT dwp3;
    D3DXMATRIXA16 matResult;
    D3DXMATRIXA16 matTemp;
    float fTime1;
    float fTime2;
    float fLerpValue;
    D3DXVECTOR3 vScale;
    D3DXVECTOR3 vPos;
    D3DXQUATERNION quat;
    BOOL bAnimate = false;
    float fTime;

    if (m_pMatrixKeys )
    {
        fTime = (float)fmod(fGlobalTime, m_pMatrixKeys[m_cMatrixKeys-1].dwTime);

        for (iKey = 0 ;iKey < m_cMatrixKeys ; iKey++)
        {
            if ((float)m_pMatrixKeys[iKey].dwTime > fTime)
            {
                dwp3 = iKey;

                if (iKey > 0)
                {
                    dwp2= iKey - 1;
                }
                else  // when iKey == 0, then dwp2 == 0
                {
                    dwp2 = iKey;
                }

                break;
            }
        }
        fTime1 = (float)m_pMatrixKeys[dwp2].dwTime;
        fTime2 = (float)m_pMatrixKeys[dwp3].dwTime;

        if ((fTime2 - fTime1) ==0)
            fLerpValue = 0;
        else
            fLerpValue =  (fTime - fTime1)  / (fTime2 - fTime1);

        if (fLerpValue > 0.5)
        {
            iKey = dwp3;
        }
        else
        {
            iKey = dwp2;
        }

        pframeToAnimate->matRot = m_pMatrixKeys[iKey].mat;
    }
    else
    {
        D3DXMatrixIdentity(&matResult);

        if (m_pScaleKeys)
        {
            dwp2 = dwp3 = 0;

            fTime = (float)fmod(fGlobalTime, m_pScaleKeys[m_cScaleKeys-1].dwTime);

            for (iKey = 0 ;iKey < m_cScaleKeys ; iKey++)
            {
                if ((float)m_pScaleKeys[iKey].dwTime > fTime)
                {
                    dwp3 = iKey;

                    if (iKey > 0)
                    {
                        dwp2= iKey - 1;
                    }
                    else  // when iKey == 0, then dwp2 == 0
                    {
                        dwp2 = iKey;
                    }

                    break;
                }
            }
            fTime1 = (float)m_pScaleKeys[dwp2].dwTime;
            fTime2 = (float)m_pScaleKeys[dwp3].dwTime;

            if ((fTime2 - fTime1) ==0)
                fLerpValue = 0;
            else
                fLerpValue =  (fTime - fTime1)  / (fTime2 - fTime1);

            D3DXVec3Lerp(&vScale,
                    &m_pScaleKeys[dwp2].vScale,
                    &m_pScaleKeys[dwp3].vScale,
                    fLerpValue);


            D3DXMatrixScaling(&matTemp, vScale.x, vScale.y, vScale.z);

            D3DXMatrixMultiply(&matResult, &matResult, &matTemp);

            bAnimate = true;
        }

        //check rot keys
        if (m_pRotateKeys )
        {
            int i1 = 0;
            int i2 = 0;

            fTime = (float)fmod(fGlobalTime, m_pRotateKeys[m_cRotateKeys-1].dwTime);

            for (iKey = 0 ;iKey < m_cRotateKeys ; iKey++)
            {
                if ((float)m_pRotateKeys[iKey].dwTime > fTime)
                {
                    i1 = (iKey > 0) ? iKey - 1 : 0;
                    i2 = iKey;
                    break;
                }
            }

            fTime1 = (float)m_pRotateKeys[i1].dwTime;
            fTime2 = (float)m_pRotateKeys[i2].dwTime;

            if ((fTime2 - fTime1) ==0)
                fLerpValue = 0;
            else
                fLerpValue =  (fTime - fTime1)  / (fTime2 - fTime1);

#define USE_SQUAD
#ifdef USE_SQUAD
            int i0 = i1 - 1;
            int i3 = i2 + 1;

            if(i0 < 0)
                i0 += m_cRotateKeys;

            if(i3 >= (INT) m_cRotateKeys)
                i3 -= m_cRotateKeys;

            D3DXQUATERNION qA, qB, qC;
            D3DXQuaternionSquadSetup(&qA, &qB, &qC, 
                &m_pRotateKeys[i0].quatRotate, &m_pRotateKeys[i1].quatRotate, 
                &m_pRotateKeys[i2].quatRotate, &m_pRotateKeys[i3].quatRotate);

            D3DXQuaternionSquad(&quat, &m_pRotateKeys[i1].quatRotate, &qA, &qB, &qC, fLerpValue);
#else
            D3DXQuaternionSlerp(&quat, &m_pRotateKeys[i1].quatRotate, &m_pRotateKeys[i2].quatRotate, fLerpValue);
#endif

            quat.w = -quat.w;
            D3DXMatrixRotationQuaternion(&matTemp, &quat);
            D3DXMatrixMultiply(&matResult, &matResult, &matTemp);

            bAnimate = true;
        }

        if (m_pPositionKeys)
        {
            dwp2=dwp3=0;

            fTime = (float)fmod(fGlobalTime, m_pPositionKeys[m_cPositionKeys-1].dwTime);

            for (iKey = 0 ;iKey < m_cPositionKeys ; iKey++)
            {
                if ((float)m_pPositionKeys[iKey].dwTime > fTime)
                {
                    dwp3 = iKey;

                    if (iKey > 0)
                    {
                        dwp2= iKey - 1;
                    }
                    else  // when iKey == 0, then dwp2 == 0
                    {
                        dwp2 = iKey;
                    }

                    break;
                }
            }
            fTime1 = (float)m_pPositionKeys[dwp2].dwTime;
            fTime2 = (float)m_pPositionKeys[dwp3].dwTime;

            if ((fTime2 - fTime1) ==0)
                fLerpValue = 0;
            else
                fLerpValue =  (fTime - fTime1)  / (fTime2 - fTime1);


            D3DXVec3Lerp((D3DXVECTOR3*)&vPos,
                    &m_pPositionKeys[dwp2].vPos,
                    &m_pPositionKeys[dwp3].vPos,
                    fLerpValue);

            D3DXMatrixTranslation(&matTemp, vPos.x, vPos.y, vPos.z);

            D3DXMatrixMultiply(&matResult, &matResult, &matTemp);
            bAnimate = true;
        }
        else
        {
            D3DXMatrixTranslation(&matTemp, pframeToAnimate->matRotOrig._41, pframeToAnimate->matRotOrig._42, pframeToAnimate->matRotOrig._43);

            D3DXMatrixMultiply(&matResult, &matResult, &matTemp);
        }

        if (bAnimate)
        {
            pframeToAnimate->matRot = matResult;
        }
    }
}




