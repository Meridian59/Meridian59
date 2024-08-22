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
#ifndef __D3DRENDEROBJECTS_H__
#define __D3DRENDEROBJECTS_H__

void D3DRenderObjects(room_type* room, Draw3DParams* params, room_contents_node* pRNode);

#endif	// __D3DRENDEROBJECTS_H__
