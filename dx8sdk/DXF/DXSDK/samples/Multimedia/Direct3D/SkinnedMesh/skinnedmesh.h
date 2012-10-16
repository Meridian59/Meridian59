#pragma once

#ifndef __MVIEW_H__
#define __MVIEW_H__



/*//////////////////////////////////////////////////////////////////////////////
//
// File: mview.h
//
// Copyright (C) 2000-2001 Microsoft Corporation. All Rights Reserved.
//
//
//////////////////////////////////////////////////////////////////////////////*/

#define GXRELEASE(_p) do { if ((_p) != NULL) {(_p)->Release(); (_p) = NULL;} } while (0)

enum METHOD {
    D3DNONINDEXED,
    D3DINDEXED,
    SOFTWARE,
    D3DINDEXEDVS,
    NONE
};

struct SMeshContainer
{
    ID3DXMesh *pMesh;
    D3DMATERIAL8 *rgMaterials;
    LPDIRECT3DTEXTURE8 *pTextures;
    DWORD cpattr;
    DWORD cMaterials;
    DWORD iAttrSplit;

    SMeshContainer *pmcNext;

    char *szName;

    // Skin info
    LPD3DXSKINMESH m_pSkinMesh;
    D3DXATTRIBUTERANGE *m_pAttrTable;
    D3DXMATRIX** m_pBoneMatrix;
    LPD3DXBUFFER m_pBoneNamesBuf;
    LPD3DXBUFFER m_pBoneOffsetBuf;
    D3DXMATRIX* m_pBoneOffsetMat;
    DWORD* m_rgiAdjacency;
    DWORD m_numBoneComb;
    DWORD m_maxFaceInfl;
    LPD3DXBUFFER m_pBoneCombinationBuf;
    METHOD  m_Method;
    DWORD m_paletteSize;
    BOOL m_bUseSW;

	SMeshContainer()
		:pMesh(NULL),
            rgMaterials(NULL),
            pTextures(NULL),
            cpattr(0),
            iAttrSplit(0),
            cMaterials(0),
            pmcNext(NULL),
            szName(NULL),
            m_pSkinMesh(NULL),
            m_pAttrTable(NULL),
            m_pBoneMatrix(NULL),
            m_pBoneNamesBuf(NULL),
            m_pBoneOffsetBuf(NULL),
            m_pBoneOffsetMat(NULL),
            m_rgiAdjacency(NULL),
            m_numBoneComb(0),
            m_maxFaceInfl(0),
            m_pBoneCombinationBuf(NULL),
            m_Method(NONE),
            m_paletteSize(0),
            m_bUseSW(FALSE)
	{
	}

	~SMeshContainer()
	{
        delete []rgMaterials;

        if (pTextures)
        {
            for (DWORD i = 0; i < cMaterials; ++i)
            {
                GXRELEASE(pTextures[i]);
            }
            delete []pTextures;
        }

		GXRELEASE(pMesh);
        GXRELEASE(m_pSkinMesh);
        GXRELEASE(m_pBoneNamesBuf);
        GXRELEASE(m_pBoneOffsetBuf);
        GXRELEASE(m_pBoneCombinationBuf);

        delete[] m_pBoneMatrix;

        delete[] m_pAttrTable;

        delete []szName;

        delete[] m_rgiAdjacency;

        delete pmcNext;
	}
};

// X File formation rotate key
struct SRotateKeyXFile
{
	DWORD			dwTime;
	DWORD			dwFloats;	
	float			w;
	float			x;
	float			y;
	float			z;
};

struct SScaleKeyXFile
{
	DWORD	dwTime;
	DWORD	dwFloats;	
	D3DXVECTOR3	vScale;	
};


struct SPositionKeyXFile
{
	DWORD	dwTime;
	DWORD	dwFloats;	
	D3DXVECTOR3	vPos;	
};

struct SMatrixKeyXFile
{
	DWORD	dwTime;
	DWORD	dwFloats;	
	D3DXMATRIX	mat;	
};

// in memory versions

struct SRotateKey
{
	DWORD			dwTime;
	D3DXQUATERNION	quatRotate;	
};

struct SPositionKey
{
	DWORD	dwTime;
	D3DXVECTOR3	vPos;	
};

struct SScaleKey
{
	DWORD	dwTime;
	D3DXVECTOR3	vScale;	
};

struct SMatrixKey
{
	DWORD	dwTime;
	D3DXMATRIXA16	mat;	
};

struct SFrame
{
    SMeshContainer *pmcMesh;
    D3DXMATRIXA16 matRot;
    D3DXMATRIXA16 matTrans;
    D3DXMATRIXA16 matRotOrig;
    D3DXMATRIXA16 matCombined;

    // animation information
    SPositionKey *m_pPositionKeys;
    UINT m_cPositionKeys;
    SRotateKey *m_pRotateKeys;
    UINT m_cRotateKeys;
    SScaleKey *m_pScaleKeys;
    UINT m_cScaleKeys;
    SMatrixKey *m_pMatrixKeys;
    UINT m_cMatrixKeys;

    SFrame *pframeAnimNext;
    SFrame *pframeToAnimate;

    SFrame *pframeSibling;
    SFrame *pframeFirstChild;

    bool bAnimationFrame;
    char *szName;

    SFrame()
        :
            pmcMesh(NULL),
            m_pPositionKeys(NULL),
            m_cPositionKeys(0),
            m_pScaleKeys(NULL),
            m_cScaleKeys(0),
            m_pRotateKeys(NULL),
            m_cRotateKeys(0),
            m_pMatrixKeys(NULL),
            m_cMatrixKeys(0),
            pframeAnimNext(NULL),
            pframeToAnimate(NULL),
            pframeSibling(NULL),
            pframeFirstChild(NULL),
            bAnimationFrame(false),
            szName(NULL)
    {
        D3DXMatrixIdentity(&matRot);
        D3DXMatrixIdentity(&matRotOrig);
        D3DXMatrixIdentity(&matTrans);
    }

    ~SFrame()
    {
        delete []szName;
        delete pmcMesh;        
        delete pframeFirstChild;
        delete pframeSibling;

        delete []m_pPositionKeys;
        delete []m_pRotateKeys;
        delete []m_pScaleKeys;
        delete []m_pMatrixKeys;

        // do NOT delete pframeAnimNext
        // do NOT delete pframeToAnimate
    }

    void SetTime(float fTime);

    SFrame *FindFrame(char *szFrame)
    {
        SFrame *pframe;

        if ((szName != NULL) && (strcmp(szName, szFrame) == 0))
            return this;

        if (pframeFirstChild != NULL)
        {
            pframe = pframeFirstChild->FindFrame(szFrame);
            if (pframe != NULL)
                return pframe;
        }

        if (pframeSibling != NULL)
        {
            pframe = pframeSibling->FindFrame(szFrame);
            if (pframe != NULL)
                return pframe;
        }

        return NULL;
    }

    void ResetMatrix()
    {
        matRot = matRotOrig;
        D3DXMatrixIdentity(&matTrans);        

        if (pframeFirstChild != NULL)
        {
            pframeFirstChild->ResetMatrix();
        }

        if (pframeSibling != NULL)
        {
            pframeSibling->ResetMatrix();
        }
    }

    void AddFrame(SFrame *pframe)
    {
        if (pframeFirstChild == NULL)
        {
            pframeFirstChild = pframe;
        }
        else
        {
            pframe->pframeSibling = pframeFirstChild->pframeSibling;
            pframeFirstChild->pframeSibling = pframe;
        }
    }

    void AddMesh(SMeshContainer *pmc)
    {
        pmc->pmcNext = pmcMesh;
        pmcMesh = pmc;
    }
};

struct SDrawElement
{
    SFrame *pframeRoot;

    D3DXVECTOR3 vCenter;
    float fRadius;

	// name of element for selection purposes
	char *szName;

    // animation list
    SFrame *pframeAnimHead;

	// next element in list
	SDrawElement *pdeNext;

    float fCurTime;
    float fMaxTime;

	SDrawElement()
		:vCenter(0.0,0.0,0.0),
			fRadius(1.0),
			szName(NULL),
            pframeRoot(NULL),
            pframeAnimHead(NULL),
			pdeNext(NULL)
	{
	}

	~SDrawElement()
	{
        delete pframeRoot;
		delete pdeNext;
        delete [] szName;

        // do NOT delete pframeAnimHead;
	}

    void AddAnimationFrame(SFrame *pframeAnim)
    {
        pframeAnim->pframeAnimNext = pframeAnimHead;
        pframeAnimHead = pframeAnim;
    }

    SFrame *FindFrame(char *szName)
    {
        if (pframeRoot == NULL)
            return NULL;
        else
            return pframeRoot->FindFrame(szName);
    }
};

HRESULT CalculateBoundingSphere(SDrawElement *pdeCur);



extern DWORD      g_dwNumDeviceTypes;
//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont* m_pFont;

    METHOD m_method;

    DWORD m_dwFVF;

    CD3DArcBall   m_ArcBall;

	SMeshContainer *m_pmcSelectedMesh;
    SFrame *m_pframeSelected;
    SDrawElement *m_pdeSelected;
	SDrawElement *m_pdeHead;

    TCHAR m_szPath[MAX_PATH];
    LPD3DXMATRIXA16 m_pBoneMatrices;
    DWORD m_maxBones;
    DWORD m_dwIndexedVertexShader[4];
    D3DXMATRIXA16 m_mView;

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();
    HRESULT ConfirmDevice(D3DCAPS8*,DWORD,D3DFORMAT);

    HRESULT FindBones(SFrame *pframeCur, SDrawElement *pde);
    HRESULT LoadMeshHierarchy();
    HRESULT LoadAnimationSet(LPDIRECTXFILEDATA pxofobjCur, SDrawElement *pde,
                             DWORD options, DWORD fvf, LPDIRECT3DDEVICE8 pD3DDevice,
                             SFrame *pframeParent);
    HRESULT LoadAnimation(LPDIRECTXFILEDATA pxofobjCur, SDrawElement *pde,
                          DWORD options, DWORD fvf, LPDIRECT3DDEVICE8 pD3DDevice,
                          SFrame *pframeParent);
    HRESULT LoadFrames(LPDIRECTXFILEDATA pxofobjCur, SDrawElement *pde,
                       DWORD options, DWORD fvf, LPDIRECT3DDEVICE8 pD3DDevice,
                       SFrame *pframeParent);
    HRESULT LoadMesh(LPDIRECTXFILEDATA pxofobjCur,
                     DWORD options, DWORD fvf, LPDIRECT3DDEVICE8 pD3DDevice,
                     SFrame *pframeParent);
    HRESULT SetProjectionMatrix();
    HRESULT DeleteSelectedMesh();
    HRESULT DrawMeshContainer(SMeshContainer *pmcMesh);
    HRESULT UpdateFrames(SFrame *pframeCur, D3DXMATRIX &matCur);
    HRESULT GenerateMesh(SMeshContainer* pmcMesh);
    HRESULT DrawFrames(SFrame *pframeCur, UINT &cTriangles);

public:
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    CMyD3DApplication();

};


#endif

