//-----------------------------------------------------------------------------
// File: GameMenu.cpp
//
// Desc: Code for in-game menus
//
// Copyright (C) 1999-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#define STRICT
#define D3D_OVERLOADS
#include <D3D8.h>
#include <D3DX8Math.h>
#include <mmsystem.h>
#include "D3DFont.h"
#include "D3DUtil.h"
#include "GameMenu.h"




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CMenuItem::CMenuItem( TCHAR* strNewLabel, DWORD dwNewID )
{
	_tcscpy( strLabel, strNewLabel );
	dwID           = dwNewID;
	pParent        = NULL;
	dwNumChildren  = 0L;
	dwSelectedMenu = 0L;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CMenuItem::~CMenuItem()
{
	while( dwNumChildren )
		delete pChild[--dwNumChildren];
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
CMenuItem* CMenuItem::Add( CMenuItem* pNewChild )
{
	pChild[dwNumChildren++] = pNewChild;
	pNewChild->pParent = this;

	return pNewChild;
}



//-----------------------------------------------------------------------------
// Name: Render()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMenuItem::Render( LPDIRECT3DDEVICE8 pd3dDevice, CD3DFont* pFont )
{
	// Check parameters
	if( NULL==pd3dDevice || NULL==pFont )
		return E_INVALIDARG;

	// Save current matrices
	D3DXMATRIX matViewSaved, matProjSaved;
	pd3dDevice->GetTransform( D3DTS_VIEW,       &matViewSaved );
	pd3dDevice->GetTransform( D3DTS_PROJECTION, &matProjSaved );

	// Setup new view and proj matrices for head-on viewing
	D3DXMATRIX matView, matProj;
	D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3(0.0f,0.0f,-30.0f),
								  &D3DXVECTOR3(0.0f,0.0f,0.0f),
				 				  &D3DXVECTOR3(0.0f,1.0f,0.0f) );
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );
	pd3dDevice->SetTransform( D3DTS_VIEW,       &matView );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	// Establish colors for selected vs. normal menu items
	D3DMATERIAL8 mtrlNormal, mtrlSelected, mtrlTitle;
	D3DUtil_InitMaterial( mtrlTitle,    1.0f, 0.0f, 0.0f, 1.0f );
	D3DUtil_InitMaterial( mtrlNormal,   1.0f, 1.0f, 1.0f, 0.5f );
	D3DUtil_InitMaterial( mtrlSelected, 1.0f, 1.0f, 0.0f, 1.0f );

	pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	pd3dDevice->SetRenderState( D3DRS_AMBIENT,  0xffffffff );

	// Translate the menuitem into place
	D3DXMATRIX matWorld;
	D3DXMatrixScaling( &matWorld, 0.4f, 0.4f, 0.4f );
	matWorld._42 = (dwNumChildren*1.0f) + 2.0f;
	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	pd3dDevice->SetMaterial( &mtrlTitle );

	// Render the menuitem's label
	pFont->Render3DText( strLabel, D3DFONT_CENTERED|D3DFONT_TWOSIDED );

	// Loop through and render all menuitem lables
	for( DWORD i=0; i<dwNumChildren; i++ )
	{
		D3DXMATRIX matWorld;
		D3DXMatrixScaling( &matWorld, 0.3f, 0.3f, 0.3f );
		pd3dDevice->SetMaterial( &mtrlNormal );

		// Give a different effect for selected items
		if( dwSelectedMenu == i )
		{
			D3DXMATRIX matRotate;
			D3DXMatrixRotationY( &matRotate, (D3DX_PI/3)*sinf(timeGetTime()/200.0f) );
			D3DXMatrixMultiply( &matWorld, &matWorld, &matRotate );
			pd3dDevice->SetMaterial( &mtrlSelected );
		}

		// Translate the menuitem into place
		matWorld._42 = (dwNumChildren*1.0f) - (i*2.0f);
		pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

		// Render the menuitem's label
		pFont->Render3DText( pChild[i]->strLabel, 
						     D3DFONT_CENTERED|D3DFONT_TWOSIDED );
	}

	// Restore matrices
	pd3dDevice->SetTransform( D3DTS_VIEW,       &matViewSaved );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProjSaved );

	return S_OK;
}




