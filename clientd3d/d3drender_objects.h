// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// The render world object interface is responsible for rendering world objects 
// (such as monsters, players, ornaments and vegetation) in the game world.
// World objects are always drawn when the d3drenderer `draw_objects` is true.
//
#ifndef _D3DRENDEROBJECTS_H
#define _D3DRENDEROBJECTS_H

/*
struct VisibleObjectsParams {
	int smallTextureSize;
	int sectorDepths[];
	long nitems;
	DrawItem& drawdata[];
	ObjectRange& visibleObject[];
	int numVisibleObjects;
	int numObjects;
	VisibleObjectsParams(
		int smallTextureSizeParam, 
		int sectorDepthsParam[], 
		long nitemsParam, 
		DrawItem& drawdataParam[], 
		ObjectRange& visibleObjectParam[], 
		int numVisibleObjectsParam, 
		int numObjectsParam)
		: smallTextureSize(smallTextureSizeParam),
		  sectorDepths(sectorDepthsParam),
		  nitems(nitemsParam),
		  drawdata(drawdataParam),
		  visibleObject(visibleObjectParam),
		  numVisibleObjects(numVisibleObjectsParam),
		  numObjects(numObjectsParam)
	{}
};
*/
struct ObjectsRenderParams {
	LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;
	LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationInvisible;
	d3d_driver_profile driverProfile;
	mutable d3d_render_pool_new renderPool;
	mutable d3d_render_cache_system cacheSystem;
	D3DMATRIX view;
	D3DMATRIX proj;

	ObjectsRenderParams(
		LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationParam,
		LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationInvisibleParam,
		d3d_driver_profile driverProfileParam,
		d3d_render_pool_new renderPoolParam,
		d3d_render_cache_system cacheSystemParam,
		D3DMATRIX viewParam,
		D3DMATRIX projParam)
		: vertexDeclaration(vertexDeclarationParam),
		  vertexDeclarationInvisible(vertexDeclarationInvisibleParam),
		  driverProfile(driverProfileParam),
		  renderPool(renderPoolParam),
		  cacheSystem(cacheSystemParam),
		  view(viewParam),
		  proj(projParam)
	{}
};

void D3DRenderObjects(room_type* room, Draw3DParams* params, room_contents_node* pRNode, 
	const ObjectsRenderParams& objectsRenderParams);

#endif	/* #ifndef _D3DRENDEROBJECTS_H */
