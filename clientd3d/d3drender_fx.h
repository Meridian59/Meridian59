// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
//
// Render area visual effects such as sandstorm, rain, etc as well as visual effects such 
// as being dazzled or blinded are provided here.
//
#ifndef _D3DRENDERFX_H
#define _D3DRENDERFX_H

struct ParticleSystemStructure {
    LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;
    const custom_xyz& playerDeltaPos;
    d3d_render_pool_new* particlePool;
    d3d_render_cache_system* particleCacheSystem;

    ParticleSystemStructure(
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationParam,
        const custom_xyz& playerDeltaPosParam,
        d3d_render_pool_new* particlePoolParam,
        d3d_render_cache_system* particleCacheSystemParam)
        : vertexDeclaration(vertexDeclarationParam),
          playerDeltaPos(playerDeltaPosParam),
          particlePool(particlePoolParam),
          particleCacheSystem(particleCacheSystemParam)
    {}
};

struct FxRenderSystemStructure {
    LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;
    d3d_render_pool_new* objectPool;
    d3d_render_cache_system* objectCacheSystem;
    d3d_render_pool_new* effectPool;
    d3d_render_cache_system* effectCacheSystem;
    LPDIRECT3DTEXTURE9 (&backBufferTex)[16];
    LPDIRECT3DTEXTURE9 backBufferTexFull;
    int fullTextureSize;
    int smallTextureSize;
    D3DMATRIX& mat;
    unsigned int frame;
    int screenWidth;
    int	screenHeight;

    FxRenderSystemStructure(
        LPDIRECT3DVERTEXDECLARATION9 vertexDeclarationParam,
        d3d_render_pool_new* objectPoolParam,
        d3d_render_cache_system* objectCacheSystemParam,
        d3d_render_pool_new* effectPoolParam,
        d3d_render_cache_system* effectCacheSystemParam,
        LPDIRECT3DTEXTURE9 (&backBufferTexParam)[16],
        LPDIRECT3DTEXTURE9 backBufferTexFullParam,
        int fullTextureSizeParam,
        int smallTextureSizeParam,
        D3DMATRIX& matParam,
        unsigned int frameParam,
        int screenWidthParam,
        int screenHeightParam)
        : vertexDeclaration(vertexDeclarationParam),
          objectPool(objectPoolParam),
          objectCacheSystem(objectCacheSystemParam),
          effectPool(effectPoolParam),
          effectCacheSystem(effectCacheSystemParam),
          backBufferTex(backBufferTexParam),
          backBufferTexFull(backBufferTexFullParam),
          fullTextureSize(fullTextureSizeParam),
          smallTextureSize(smallTextureSizeParam),
          mat(matParam),
		  frame(frameParam),
          screenWidth(screenWidthParam),
          screenHeight(screenHeightParam)
    {}
};

void D3DFxInit();
void D3DFxBlurWaver(FxRenderSystemStructure renderSystemStructure);
void D3DPostOverlayEffects(FxRenderSystemStructure fxrss);
void D3DRenderParticles(ParticleSystemStructure pss);

#endif	/* #ifndef _D3DRENDERFX_H */
