// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <cmath>
#include <chrono>
#include <limits>

using namespace std::chrono;

// Variables
extern room_type current_room;

static constexpr uint32_t PARTICLE_VERTICES = 2;
static constexpr uint32_t PARTICLE_PRIMITIVES = 1;

static steady_clock::time_point lastFrameTime = steady_clock::now();
// Elapsed time between frames (in seconds).
static float deltaTime_s = 0.0f;

// Xorshift32 provides improved randomization while remaining light on CPU cycles.
static uint32_t randomState = []()
{
	uint32_t seed = static_cast<uint32_t>(high_resolution_clock::now().time_since_epoch().count());
	return (seed == 0)? 0x3E71D159 : seed;
}();

static uint32_t xorshift32()
{
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;
    return randomState;
}

static float GetRandomFloatRange(float min, float max)
{
	if (min >= max)
	{
		return min;
	}
	static constexpr double uint32_maxValue = 1.0 / std::numeric_limits<uint32_t>::max(); 
	
    double normalized = xorshift32() * uint32_maxValue;
    return min + static_cast<float>(normalized) * (max - min);
}

// Returns XYZ after adjusting it with the emitter's variance range (if any).
static custom_xyz GetVariedXYZ(const custom_xyz& base, const custom_xyz& min, const custom_xyz& max)
{
	custom_xyz result = base;
	result.x += GetRandomFloatRange(min.x, max.x);
	result.y += GetRandomFloatRange(min.y, max.y);
	result.z += GetRandomFloatRange(min.z, max.z);
	return result;
}

void D3DParticleSystemReset(particle_system *pParticleSystem)
{
	list_destroy(pParticleSystem->emitterList);
	pParticleSystem->emitterList = nullptr;
}

emitter* D3DParticleEmitterInit(particle_system *pParticleSystem)
{
	emitter	*pEmitter = nullptr;

	if (pParticleSystem == nullptr)
	{
		return nullptr;
	}
	pEmitter = (emitter *)SafeMalloc(sizeof(emitter));

	if (pEmitter == nullptr)
	{
		return nullptr;
	}
	
	memset(pEmitter, 0, sizeof(emitter));
	
	return pEmitter;
}

void D3DParticleEmitterSetTimer(emitter *pEmitter, float time)
{
	pEmitter->timer_s = time;
	pEmitter->timerBase_s = time;
}

void D3DParticleEmitterAddToList(particle_system *pParticleSystem, emitter *pEmitter)
{
	if (pParticleSystem->emitterList == nullptr)
	{
		pParticleSystem->emitterList = list_create(pEmitter);
	}
	else
	{
		list_add_item(pParticleSystem->emitterList, pEmitter);
	}
}

void D3DParticleEmitterUpdate(emitter *pEmitter, float posX, float posY, float posZ)
{
	pEmitter->position.x += posX;
	pEmitter->position.y += posY;
	pEmitter->position.z += posZ;
}

void D3DParticleSystemUpdate(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
							 d3d_render_cache_system *pCacheSystem)
{
	D3DCacheSystemReset(pCacheSystem);
	D3DRenderPoolReset(pPool, &D3DMaterialParticlePool);
	
	// Calculate time (in seconds) since the last frame.
	auto currentFrameTime = steady_clock::now();
	auto elapsed = currentFrameTime - lastFrameTime;
	deltaTime_s = duration<float>(elapsed).count();
	lastFrameTime = currentFrameTime;

	for (list_type list = pParticleSystem->emitterList; list != nullptr; list = list->next)
	{
		emitter *pEmitter = (emitter *)list->data;
		
		// Update existing particles
		for (int curParticle = 0; curParticle < pEmitter->numParticles; curParticle++)
		{
			particle *pParticle = &pEmitter->particles[curParticle];			
			D3DParticleUpdate(pEmitter, pParticle, pPool);
		}

		// Creating new particles
		pEmitter->timer_s -= deltaTime_s;
		if (pEmitter->timer_s <= 0)
		{
			// Particles spawn one at a time and use circular buffing to track the next open particle.
			particle *pParticle = &pEmitter->particles[pEmitter->nextSlot];
			D3DParticleCreate(pEmitter, pParticle);
		}
	}

	D3DCacheFill(pCacheSystem, pPool, 0);
	D3DCacheFlush(pCacheSystem, pPool, 0, D3DPT_LINESTRIP);
}

void D3DParticleUpdate(emitter *pEmitter, particle *pParticle, d3d_render_pool_new *pPool)
{
	// Skip dead particles, or end particles that run out of energy.
	if (pParticle->energy <= 0 || --pParticle->energy <= 0)
	{
		return;
	}
	// Update lifetime for weather particles since only they track time.
	if (pEmitter->bDestroysOnSurface)
	{
		pParticle->currentAge_s += deltaTime_s;
		if (pParticle->currentAge_s >= pParticle->maxAge_s)
		{
			D3DParticleDestroy(pParticle);
			return;
		}
	}

	custom_xyzw velocity = {pParticle->velocity.x, pParticle->velocity.y, pParticle->velocity.z, 1.0f};

	D3DMATRIX rotate, matrix;
	MatrixRotateX(&matrix, pParticle->rotation.x);
	MatrixRotateY(&rotate, pParticle->rotation.y);
	MatrixMultiply(&rotate, &matrix, &rotate);
	MatrixRotateZ(&matrix, pParticle->rotation.z);
	MatrixMultiply(&rotate, &rotate, &matrix);

	MatrixMultiplyVector(&velocity, &rotate, &velocity);
	pParticle->velocity.x = velocity.x;
	pParticle->velocity.y = velocity.y;
	pParticle->velocity.z = velocity.z;
	pParticle->oldPosition = pParticle->position;
	pParticle->position.x += pParticle->velocity.x;
	pParticle->position.y += pParticle->velocity.y;
	pParticle->position.z += pParticle->velocity.z;
	
	d3d_render_packet_new *pPacket = D3DRenderPacketFindMatch(pPool, nullptr, nullptr, 0, 0, 0);
	assert(pPacket);
	pPacket->pMaterialFctn = &D3DMaterialParticlePacket;

	d3d_render_chunk_new *pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);
	pChunk->numIndices = PARTICLE_VERTICES;
	pChunk->numVertices = PARTICLE_VERTICES;
	pChunk->numPrimitives = PARTICLE_PRIMITIVES;
	pChunk->pMaterialFctn = &D3DMaterialParticleChunk;

	MatrixTranslate(&pChunk->xForm, pParticle->position.x, pParticle->position.z, pParticle->position.y);

	CHUNK_XYZ_SET(pChunk, 0, 0, 0, 0);
	CHUNK_XYZ_SET(pChunk, 1, -pParticle->velocity.x, -pParticle->velocity.y, -pParticle->velocity.z);
	CHUNK_BGRA_SET(pChunk, 0, pParticle->bgra.b, pParticle->bgra.g, pParticle->bgra.r, pParticle->bgra.a);
	CHUNK_BGRA_SET(pChunk, 1, pParticle->bgra.b, pParticle->bgra.g, pParticle->bgra.r, 0);
	CHUNK_INDEX_SET(pChunk, 0, 0);
	CHUNK_INDEX_SET(pChunk, 1, 1);
}

void D3DParticleCreate(emitter *pEmitter, particle *pParticle)
{
	pParticle->position = GetVariedXYZ(pEmitter->position, pEmitter->positionVarianceMin, pEmitter->positionVarianceMax);
	pParticle->velocity = GetVariedXYZ(pEmitter->velocity, pEmitter->velocityVarianceMin, pEmitter->velocityVarianceMax);
	
	// Each weather particle calculates the time it takes for them to land on the ground.
	if (pEmitter->bDestroysOnSurface)
	{						
		pParticle->currentAge_s = 0.0f;
		
		BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, pParticle->position.x, pParticle->position.y);
		if (leaf && leaf->sector->ceiling)
		{
			D3DParticleDestroy(pParticle);
			return;	
		}
		else if (leaf && leaf->sector->floor)
		{
			float floorHeight = GetFloorHeight(pParticle->position.x, pParticle->position.y, leaf->sector);

			if (pParticle->position.z < floorHeight)
			{
				D3DParticleDestroy(pParticle);
				return;
			}
			pParticle->maxAge_s = abs((pParticle->position.z - floorHeight) / pParticle->velocity.z);			
		}
		// Out-of-bound weather particles still spawn for a few seconds so they can still show normally
		// outdoors beyond areas like forest walls, or from outside windows.
		else
		{
			pParticle->maxAge_s = 2.0f;
		}
	}
	
	pParticle->rotation = GetVariedXYZ(pEmitter->rotation, pEmitter->rotationVarianceMin, pEmitter->rotationVarianceMax);
	pParticle->bgra = pEmitter->bgra;
	pParticle->energy = pEmitter->energy;
	
	// Advance to the next slot in the circular buffer.
	pEmitter->nextSlot = (pEmitter->nextSlot + 1) % MAX_PARTICLES;
	if (pEmitter->numParticles < MAX_PARTICLES)
	{
		pEmitter->numParticles++;
	}
	pEmitter->timer_s = pEmitter->timerBase_s;
}

void D3DParticleDestroy(particle *pParticle)
{
	pParticle->maxAge_s = 0;
	pParticle->energy = 0;
}
