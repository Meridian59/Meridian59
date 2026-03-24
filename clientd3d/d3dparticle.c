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
#include <random>
#include <vector>


// Variables
extern room_type current_room;

static constexpr uint32_t PARTICLE_VERTICES = 2;
static constexpr uint32_t PARTICLE_PRIMITIVES = 1;

static std::mt19937 gen(static_cast<uint32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

static float GetRandomFloatRange(float min, float max)
{
	if (min >= max)
	{
		return min;
	}
	std::uniform_real_distribution<float> dist(min, max);
	
    return dist(gen);
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
	// Reset priming seting so it won't potentially leak between room changes.
	pParticleSystem->isPriming = false;
	
	// Free memory from each emitter before clearing the pointers.
	for (auto pEmitter : pParticleSystem->emitterList)
	{
		if (pEmitter != nullptr)
		{
			SafeFree(pEmitter);
		}
	}
	
	pParticleSystem->emitterList.clear();
}

emitter* D3DParticleEmitterInit(particle_system *pParticleSystem, float time)
{
	emitter	*pEmitter = (emitter *)SafeMalloc(sizeof(emitter));
	memset(pEmitter, 0, sizeof(emitter));
	
	pEmitter->timer_s = time;
	pEmitter->timerBase_s = time;
	
	pParticleSystem->emitterList.push_back(pEmitter);
	return pEmitter;
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

	for (auto pEmitter : pParticleSystem->emitterList)
	{	
		// Update existing particles
		for (int i = 0; i < pEmitter->numParticles; i++)
		{			
			D3DParticleUpdate(pEmitter, &pEmitter->particles[i], pPool);
		}
		
		// Initializing new particles
		pEmitter->timer_s -= GetDeltaTime();
		if (pEmitter->timer_s <= 0.0f)
		{
			// Particles spawn one at a time and use circular buffing to track the next open particle.
			D3DParticleInitialize(pEmitter, &pEmitter->particles[pEmitter->nextSlot], pParticleSystem->isPriming);
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
		pParticle->currentAge_s += GetDeltaTime();
		if (pParticle->currentAge_s >= pParticle->maxAge_s)
		{
			D3DParticleHide(pParticle);
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

// Gets a particle from the object pool 
void D3DParticleInitialize(emitter *pEmitter, particle *pParticle, bool &isPriming)
{
	// Advance to the next slot in the circular buffer and reset the emitter's timer, 
	// regardless if the recycled particle should show up in its new location or not.
	pEmitter->nextSlot = (pEmitter->nextSlot + 1) % MAX_PARTICLES;
	if (pEmitter->numParticles < MAX_PARTICLES)
	{
		pEmitter->numParticles++;
	}
	pEmitter->timer_s = pEmitter->timerBase_s;

	pParticle->position = GetVariedXYZ(pEmitter->position, pEmitter->positionVarianceMin, pEmitter->positionVarianceMax);
	pParticle->velocity = GetVariedXYZ(pEmitter->velocity, pEmitter->velocityVarianceMin, pEmitter->velocityVarianceMax);
	
	// Each weather particle first checks if their new spawn location is valid.
	// If so, calculate the time it takes to land on a surface.
	if (pEmitter->bDestroysOnSurface)
	{						
		pParticle->currentAge_s = 0.0f;
		BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, pParticle->position.x, pParticle->position.y);
		
		// Weather particles are hidden at ceiling sectors because a single BSP lookup cannot detect one-sided ceiling
		// textures from the top-down.  From the player's view, the hidden particles are barely noticable from outdoors.
		if (leaf && leaf->sector->ceiling)
		{
			D3DParticleHide(pParticle);
			return;	
		}
		else if (leaf && leaf->sector->floor)
		{
			float floorHeight = GetFloorHeight(pParticle->position.x, pParticle->position.y, leaf->sector);

			if (pParticle->position.z < floorHeight)
			{
				D3DParticleHide(pParticle);
				return;
			}
			
			pParticle->maxAge_s = abs((pParticle->position.z - floorHeight) / pParticle->velocity.z);

		}
		// Out-of-bound weather particles still spawn for a few seconds so they can still show normally
		// outdoors beyond areas like forest walls, or from outside windows.
		else
		{
			pParticle->maxAge_s = 3.0f;
		}
		
		// Now check if we are priming particles if we just loaded into a room or teleported around.
		if (isPriming)
		{
			// If so, jump start the particle to be somewhere between the start of its spawn and near the end.
			pParticle->currentAge_s = pParticle->maxAge_s * GetRandomFloatRange(0.0f, 0.95f);
			pParticle->position.z += (pParticle->velocity.z * pParticle->currentAge_s);
			
			if (pEmitter->numParticles >= MAX_PARTICLES)
			{
				isPriming = false;
			}
		}
	}
	
	pParticle->rotation = GetVariedXYZ(pEmitter->rotation, pEmitter->rotationVarianceMin, pEmitter->rotationVarianceMax);
	pParticle->bgra = pEmitter->bgra;
	pParticle->energy = pEmitter->energy;
}

void D3DParticleHide(particle *pParticle)
{
	pParticle->maxAge_s = 0;
	pParticle->energy = 0;
}
