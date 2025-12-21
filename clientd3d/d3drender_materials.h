// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// This source file contains a collection of functions responsible for configuring and managing 
// various materials and rendering states within a Direct3D - based rendering engine.
//
#ifndef _D3DRENDERMATERIALS_H
#define _D3DRENDERMATERIALS_H

// material functions
bool D3DMaterialNone(d3d_render_chunk_new *pPool);

// world
bool D3DMaterialWorldPool(d3d_render_pool_new *pPool);
bool D3DMaterialWorldPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
bool D3DMaterialWorldDynamicChunk(d3d_render_chunk_new *pChunk);
bool D3DMaterialWorldStaticChunk(d3d_render_chunk_new *pChunk);
bool D3DMaterialWallMaskPool(d3d_render_pool_new *pPool);
bool D3DMaterialMaskChunk(d3d_render_chunk_new *pChunk);

// lmaps
bool D3DMaterialLMapDynamicPool(d3d_render_pool_new *pPool);
bool D3DMaterialLMapDynamicPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
bool D3DMaterialLMapDynamicChunk(d3d_render_chunk_new *pChunk);
bool D3DMaterialLMapStaticChunk(d3d_render_chunk_new *pChunk);

// objects
bool D3DMaterialObjectPool(d3d_render_pool_new *pPool);
bool D3DMaterialObjectPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
bool D3DMaterialObjectChunk(d3d_render_chunk_new *pChunk);

// invisible objects
bool D3DMaterialObjectInvisiblePool(d3d_render_pool_new *pPool);
bool D3DMaterialObjectInvisiblePacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
bool D3DMaterialObjectInvisibleChunk(d3d_render_chunk_new *pChunk);

// effects
bool D3DMaterialEffectPool(d3d_render_pool_new *pPool);
bool D3DMaterialEffectPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
bool D3DMaterialEffectChunk(d3d_render_chunk_new *pChunk);

// blur
bool D3DMaterialBlurPool(d3d_render_pool_new *pPool);
bool D3DMaterialBlurPacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
bool D3DMaterialBlurChunk(d3d_render_chunk_new *pChunk);

// particles
bool D3DMaterialParticlePool(d3d_render_pool_new *pPool);
bool D3DMaterialParticlePacket(d3d_render_packet_new *pPacket, d3d_render_cache_system *pCacheSystem);
bool D3DMaterialParticleChunk(d3d_render_chunk_new *pChunk);

#endif	/* #ifndef _D3DRENDERMATERIALS_H */
