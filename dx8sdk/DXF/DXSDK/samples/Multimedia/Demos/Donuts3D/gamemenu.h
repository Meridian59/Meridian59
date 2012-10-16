//-----------------------------------------------------------------------------
// File: GameMenu.h
//
// Desc: Code for in-game menus
//
// Copyright (C) 1999-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef GAMEMENU_H
#define GAMEMENU_H




//-----------------------------------------------------------------------------
// Name: class CMenuItem
// Desc: 
//-----------------------------------------------------------------------------
class CMenuItem
{
public:
	TCHAR       strLabel[80];
	DWORD       dwID;
	CMenuItem*  pParent;

	CMenuItem*  pChild[10];
	DWORD       dwNumChildren;

	DWORD       dwSelectedMenu;

public:
	HRESULT Render( LPDIRECT3DDEVICE8 pd3dDevice, CD3DFont* pFont );
	
	CMenuItem* Add( CMenuItem* );

	CMenuItem( TCHAR* strLabel, DWORD dwID );
	~CMenuItem();
};







#endif


