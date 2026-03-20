// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Variables
static particle_system sandParticleSystem;
static particle_system rainParticleSystem;
static particle_system snowParticleSystem;

// Slight variation on weather particles fall velocity.
static constexpr float WEATHER_JITTER_MIN = -0.5f;
static constexpr float WEATHER_JITTER_MAX = 1.5f;

// Default colors for particles
static constexpr custom_bgra SANDSTORM_COLOR = {6,153,226,255};
static constexpr custom_bgra RAIN_COLOR = {249,228,175,150};
static constexpr custom_bgra SNOW_COLOR = {255,255,255,220};

// Interfaces

static void SandstormInit(void);
static void RainInit(void);
static void SnowInit(void);

// Implementations

/**
* Initializes the particle effects, specifically the particle emitters.
*/
void D3DFxInit()
{
	SandstormInit();
	RainInit();
	SnowInit();
}

/**
* Updates and renders all active particle emitters, including the sandstorm effect, 
* in the current frame.
*/
void D3DRenderParticles(const ParticleSystemStructure& pss)
{
	// Update position of emitters.
	for (list_type list = sandParticleSystem.emitterList; list != nullptr; list = list->next)
	{
		emitter *pEmitter = (emitter *)list->data;
		if (pEmitter)
		{
			D3DParticleEmitterUpdate(pEmitter, pss.playerDeltaPos.x, pss.playerDeltaPos.y, pss.playerDeltaPos.z);
		}
	}

	for (list_type list = rainParticleSystem.emitterList; list != nullptr; list = list->next)
	{
		emitter *pEmitter = (emitter *)list->data;
		if (pEmitter)
		{
			D3DParticleEmitterUpdate(pEmitter, pss.playerDeltaPos.x, pss.playerDeltaPos.y, pss.playerDeltaPos.z);
		}
	}

	for (list_type list = snowParticleSystem.emitterList; list != nullptr; list = list->next)
	{
		emitter *pEmitter = (emitter *)list->data;
		if (pEmitter)
		{
			D3DParticleEmitterUpdate(pEmitter, pss.playerDeltaPos.x, pss.playerDeltaPos.y, pss.playerDeltaPos.z);
		}
	}

	if (effects.sand)
	{
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, nullptr);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, pss.vertexDeclaration);

		D3DParticleSystemUpdate(&sandParticleSystem, pss.particlePool, pss.particleCacheSystem);
	}

	if (effects.raining)
	{
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, nullptr);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, pss.vertexDeclaration);

		D3DParticleSystemUpdate(&rainParticleSystem, pss.particlePool, pss.particleCacheSystem);
	}

	if (effects.snowing)
	{
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, nullptr);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, pss.vertexDeclaration);

		D3DParticleSystemUpdate(&snowParticleSystem, pss.particlePool, pss.particleCacheSystem);
	}
}

/**
* Initializes the sandstorm particle emitters with predefined positions, energy, and colors for the 
* particle system.
*/
void SandstormInit(void)
{
	static constexpr int SAND_EMITTER_COUNT = 16;
	
	static constexpr int SAND_EMITTER_ENERGY = 40;
	static constexpr float SAND_EMITTER_RADIUS = 10000.0f;
	static constexpr float SAND_Z_VARIANCE = 1000.0f;
	static constexpr float SAND_VELOCITY = 500.0f;
	static constexpr float SAND_TIMER_S = 0.015f;  // In seconds
	static constexpr float SAND_RAND_ROT = PI / 1000.0f;
	
	D3DParticleSystemReset(&sandParticleSystem);
	emitter* newEmitter = nullptr;
	for (int i = 0; i < SAND_EMITTER_COUNT; i++)
	{
		newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_TIMER_S);
		newEmitter->energy = SAND_EMITTER_ENERGY;
		newEmitter->positionVarianceMin = {-SAND_EMITTER_RADIUS, -SAND_EMITTER_RADIUS, -SAND_Z_VARIANCE};
		newEmitter->positionVarianceMax = {SAND_EMITTER_RADIUS, SAND_EMITTER_RADIUS, SAND_Z_VARIANCE * 2.0f};
		newEmitter->rotationVarianceMin = {-SAND_RAND_ROT, -SAND_RAND_ROT, -SAND_RAND_ROT};
		newEmitter->rotationVarianceMax = {SAND_RAND_ROT, SAND_RAND_ROT, SAND_RAND_ROT};
		
		// Each emitters fire sand particles at 22.5 degrees in a full circle.
		float angle = (static_cast<float>(i) * 2.0f * PI) / static_cast<float>(SAND_EMITTER_COUNT);
		float directionX = cosf(angle);
		float directionY = sinf(angle);
		newEmitter->velocity.x = directionX * SAND_VELOCITY;
		newEmitter->velocity.y = directionY * SAND_VELOCITY;
		newEmitter->bgra = SANDSTORM_COLOR;
	}
}

/**
* Initializes the rain particle emitters with predefined positions, energy, and colors for the 
* particle system.
*/
void RainInit(void)
{
	static constexpr int RAIN_EMITTER_COUNT = 16;
	
	static constexpr float RAIN_TIMER_S = 0.015f;  // In seconds
	static constexpr int RAIN_EMITTER_ENERGY = 200;
	static constexpr float RAIN_EMITTER_HEIGHT = 4000.0f;
	// Sets minimum z-position variance so weather particles can be primed at the start.
	static constexpr float RAIN_Z_SPAWN_OFFSET = -(RAIN_EMITTER_HEIGHT / 2);
	static constexpr float RAIN_VELOCITY = -200.0f;
	static constexpr float RAIN_EMITTER_RADIUS = 7500.0f;

	D3DParticleSystemReset(&rainParticleSystem);
	emitter* newEmitter = nullptr;
	for (int i = 0; i < RAIN_EMITTER_COUNT; i++)
	{
		newEmitter = D3DParticleEmitterInit(&rainParticleSystem, RAIN_TIMER_S);
		newEmitter->bDestroysOnSurface = true;
		newEmitter->energy = RAIN_EMITTER_ENERGY;
		newEmitter->position = {0.0f, 0.0f, RAIN_EMITTER_HEIGHT};
		//  Half of the emitters spawn weather particles twice as far.
		if (i < (RAIN_EMITTER_COUNT / 2))
		{
			newEmitter->positionVarianceMin = {-RAIN_EMITTER_RADIUS, -RAIN_EMITTER_RADIUS, RAIN_Z_SPAWN_OFFSET};
			newEmitter->positionVarianceMax = {RAIN_EMITTER_RADIUS, RAIN_EMITTER_RADIUS, 0.0f};
		}
		else
		{
			newEmitter->positionVarianceMin = {-RAIN_EMITTER_RADIUS * 2.0f, -RAIN_EMITTER_RADIUS * 2.0f, RAIN_Z_SPAWN_OFFSET};
			newEmitter->positionVarianceMax = {RAIN_EMITTER_RADIUS * 2.0f, RAIN_EMITTER_RADIUS * 2.0f, 0.0f};
		}
		newEmitter->velocity = {0.0f, 0.0f, RAIN_VELOCITY};
		newEmitter->velocityVarianceMin = {0.0f, 0.0f, WEATHER_JITTER_MIN};
		newEmitter->velocityVarianceMax = {0.0f, 0.0f, WEATHER_JITTER_MAX};
		newEmitter->bgra = RAIN_COLOR;
	}
}

/**
* Initializes the snow particle emitters with predefined positions, energy, and colors for the 
* particle system.
*/
void SnowInit(void)
{
	static constexpr int SNOW_EMITTER_COUNT = 16;
	
	static constexpr float SNOW_EMITTER_RADIUS = 7500.0f;
	static constexpr int SNOW_EMITTER_ENERGY = 800;
	static constexpr float SNOW_EMITTER_HEIGHT = 4000.0f;
	// Sets minimum z-position variance so weather particles can be primed at the start.
	static constexpr float SNOW_Z_SPAWN_OFFSET = -(SNOW_EMITTER_HEIGHT * 0.875f);
	static constexpr float SNOW_FALL_SPEED = -30.0f;
	static constexpr float SNOW_TIMER_S = 0.015f; // In seconds

	D3DParticleSystemReset(&snowParticleSystem);
	emitter* newEmitter = nullptr;
	for(int i = 0; i < SNOW_EMITTER_COUNT; i++)
	{
		newEmitter = D3DParticleEmitterInit(&snowParticleSystem, SNOW_TIMER_S);
		newEmitter->bDestroysOnSurface = true;
		newEmitter->energy = SNOW_EMITTER_ENERGY;
		newEmitter->position = {0.0f, 0.0f, SNOW_EMITTER_HEIGHT};
		//  Half of the emitters spawn weather particles twice as far.
		if (i < (SNOW_EMITTER_COUNT / 2))
		{
			newEmitter->positionVarianceMin = {-SNOW_EMITTER_RADIUS, -SNOW_EMITTER_RADIUS, SNOW_Z_SPAWN_OFFSET};
			newEmitter->positionVarianceMax = {SNOW_EMITTER_RADIUS, SNOW_EMITTER_RADIUS, 0.0f};
		}
		else
		{
			newEmitter->positionVarianceMin = {-SNOW_EMITTER_RADIUS * 2.0f, -SNOW_EMITTER_RADIUS * 2.0f, SNOW_Z_SPAWN_OFFSET};
			newEmitter->positionVarianceMax = {SNOW_EMITTER_RADIUS * 2.0f, SNOW_EMITTER_RADIUS * 2.0f, 0.0f};
		}
		newEmitter->velocity = {0.0f, 0.0f, SNOW_FALL_SPEED};
		newEmitter->velocityVarianceMin = {0.0f, 0.0f, WEATHER_JITTER_MIN};
		newEmitter->velocityVarianceMax = {0.0f, 0.0f, WEATHER_JITTER_MAX};
		newEmitter->bgra = SNOW_COLOR;
	}
}

/**
* Applies post-render overlay effects such as alpha blending, screen flashes, and pain effects, 
* and resets the rendering cache.
*/
void D3DPostOverlayEffects(const FxRenderSystemStructure& fxrss)
{
	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, 1, D3DCMP_GREATEREQUAL);
	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, FALSE);

	D3DCacheSystemReset(fxrss.objectCacheSystem);
	D3DRenderPoolReset(fxrss.objectPool, &D3DMaterialObjectPool);

	// Flash of XLAT.  Could be color, blindness, whatever.
	if (effects.flashxlat != XLAT_IDENTITY)
	{
		custom_bgra	bgra;
		
		effects.duration -= GetDeltaTimeMs();
		
		switch (effects.flashxlat)
		{
			case XLAT_BLEND10RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND25RED:
			case XLAT_BLEND30RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND75RED:
			case XLAT_BLEND80RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND10WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND30WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND80WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25YELLOW:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 192;
			break;
			case XLAT_BLEND25BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 192;
			break;

			default:
				bgra.b = 0; bgra.g = 0; bgra.r = 0; bgra.a = 255;
			break;
		}

		if (effects.duration <= 0)
		{
			effects.flashxlat = XLAT_IDENTITY;
			effects.duration = 0;
		}

		d3d_render_packet_new *pPacket = D3DRenderPacketFindMatch(fxrss.objectPool, nullptr, nullptr, 0, 0, 0);
		if (pPacket == nullptr)
			return;
		d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (int i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, bgra.b, bgra.g, bgra.r, bgra.a);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	if (effects.xlatOverride > 0)
	{
		custom_bgra	bgra;

		switch (effects.xlatOverride)
		{
			case XLAT_BLEND10RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND25RED:
			case XLAT_BLEND30RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND75RED:
			case XLAT_BLEND80RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND10WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND30WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND80WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25YELLOW:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 192;
			break;
			case XLAT_BLEND25BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 192;
			break;

			default:
				bgra.b = 0; bgra.g = 0; bgra.r = 0; bgra.a = 255;
			break;
		}

		d3d_render_packet_new *pPacket = D3DRenderPacketFindMatch(fxrss.objectPool, nullptr, nullptr, 0, 0, 0);
		if (pPacket == nullptr)
			return;
		d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (int i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, bgra.b, bgra.g, bgra.r, bgra.a);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	// May be drawn over room or map.

	// whiteout
	if (effects.whiteout)
	{
		int whiteout = std::min(effects.whiteout, 500);

		whiteout = whiteout * COLOR_MAX / 500;
		whiteout = std::max(whiteout, 200);

		d3d_render_packet_new *pPacket = D3DRenderPacketFindMatch(fxrss.objectPool, nullptr, nullptr, 0, 0, 0);
		if (pPacket == nullptr)
			return;
		d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (int i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, COLOR_MAX, COLOR_MAX, COLOR_MAX, whiteout);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	// Pain (always drawn last).
	if (config.pain)
	{
		if (effects.pain)
		{
			int	pain = std::min(effects.pain, 2000);

			pain = pain * 204 / 2000;

			d3d_render_packet_new *pPacket = D3DRenderPacketFindMatch(fxrss.objectPool, nullptr, nullptr, 0, 0, 0);
			if (pPacket == nullptr)
				return;
			d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);
			pChunk->numIndices = 4;
			pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
			pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
			MatrixIdentity(&pChunk->xForm);

			for (int i = 0; i < 4; i++)
			{
				CHUNK_BGRA_SET(pChunk, i, 0, 0, COLOR_MAX, pain);
			}

			CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));
			CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
			CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
			CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));

			pChunk->indices[0] = 1;
			pChunk->indices[1] = 2;
			pChunk->indices[2] = 0;
			pChunk->indices[3] = 3;
		}
	}

	D3DCacheFill(fxrss.objectCacheSystem, fxrss.objectPool, 1);
	D3DCacheFlush(fxrss.objectCacheSystem, fxrss.objectPool, 1, D3DPT_TRIANGLESTRIP);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);
}

/**
* Creates a blur and wave distortion effects using frame buffers, applying it to the current screen 
* using a series of textures.
*/
void D3DFxBlurWaver(const FxRenderSystemStructure& fxRss)
{
	static int offset = 0;
	static int offsetDir = 1;

	int t = fxRss.frame & 7;

	if (fxRss.frame & 63)
	{
		offset += offsetDir;

		if ((offset > 31) || (offset < 0))
		{
			offsetDir = -offsetDir;
			offset += offsetDir;
		}
	}

	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, FALSE, 1, D3DCMP_GREATEREQUAL);
	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, FALSE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

	IDirect3DDevice9_SetVertexShader(gpD3DDevice, nullptr);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, fxRss.vertexDeclaration);

	D3DRenderFramebufferTextureCreate(fxRss.backBufferTexFull, fxRss.backBufferTex[t],
		fxRss.smallTextureSize, fxRss.smallTextureSize);

	D3DCacheSystemReset(fxRss.effectCacheSystem);
	D3DRenderPoolReset(fxRss.effectPool, &D3DMaterialBlurPool);

	for (int i = 0; i <= 7; i++)
	{
		d3d_render_packet_new *pPacket = D3DRenderPacketNew(fxRss.effectPool);
		if (nullptr == pPacket)
			return;
		d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
		pPacket->pMaterialFctn = D3DMaterialBlurPacket;
		pChunk->pMaterialFctn = D3DMaterialBlurChunk;
		pPacket->pTexture = fxRss.backBufferTex[i];
		pChunk->numIndices = pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		MatrixIdentity(&pChunk->xForm);

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(-offset, fxRss.screenWidth),
			0, D3DRENDER_SCREEN_TO_CLIP_Y(-offset, fxRss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(-offset, fxRss.screenWidth),
			0, D3DRENDER_SCREEN_TO_CLIP_Y(fxRss.screenHeight + offset, fxRss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxRss.screenWidth + offset, fxRss.screenWidth),
			0, D3DRENDER_SCREEN_TO_CLIP_Y(fxRss.screenHeight + offset, fxRss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxRss.screenWidth + offset, fxRss.screenWidth),
			0, D3DRENDER_SCREEN_TO_CLIP_Y(-offset, fxRss.screenHeight));

		CHUNK_ST0_SET(pChunk, 0, 0.0f, 0.0f);
		CHUNK_ST0_SET(pChunk, 1, 0.0f, 1.0f);
		CHUNK_ST0_SET(pChunk, 2, 1.0f, 1.0f);
		CHUNK_ST0_SET(pChunk, 3, 1.0f, 0.0f);

		CHUNK_BGRA_SET(pChunk, 0, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
		CHUNK_BGRA_SET(pChunk, 1, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
		CHUNK_BGRA_SET(pChunk, 2, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
		CHUNK_BGRA_SET(pChunk, 3, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);

		CHUNK_INDEX_SET(pChunk, 0, 1);
		CHUNK_INDEX_SET(pChunk, 1, 2);
		CHUNK_INDEX_SET(pChunk, 2, 0);
		CHUNK_INDEX_SET(pChunk, 3, 3);
	}

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &fxRss.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &fxRss.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &fxRss.transformMatrix);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, FALSE);

	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, FALSE, 1, D3DCMP_GREATEREQUAL);
	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

	D3DCacheFill(fxRss.effectCacheSystem, fxRss.effectPool, 1);
	D3DCacheFlush(fxRss.effectCacheSystem, fxRss.effectPool, 1, D3DPT_TRIANGLESTRIP);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);
}
