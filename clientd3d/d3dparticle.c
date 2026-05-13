// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <chrono>
#include <random>

///////////////
// Constants //
///////////////
static constexpr uint32_t PARTICLE_INDICES = 2;
static constexpr uint32_t PARTICLE_VERTICES = 2;
static constexpr uint32_t PARTICLE_PRIMITIVES = 1;

///////////////
// Variables //
///////////////
static std::mt19937 rng(static_cast<uint32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

////////////////////////
// Internal Functions //
////////////////////////
static float GetRandomFloatRange(float min, float max)
{
	if (min >= max)
	{
		return min;
	}
	std::uniform_real_distribution<float> dist(min, max);

    return dist(rng);
}

// Returns a new XYZ after adjusting the base XYZ with a specified variance range (if any).
static custom_xyz GetVariedXYZ(const custom_xyz& base, const custom_xyz& min, const custom_xyz& max)
{
	custom_xyz result = base;
	result.x += GetRandomFloatRange(min.x, max.x);
	result.y += GetRandomFloatRange(min.y, max.y);
	result.z += GetRandomFloatRange(min.z, max.z);
	return result;
}

static void D3DParticleUpdate(emitter *pEmitter, particle *pParticle, d3d_render_pool_new *pPool)
{
	// Skip inactive particles so they don't get added to rendering.
	if (pParticle->isActive == false)
		return;

	if (--pParticle->timeLeft <= 0)
	{
		pParticle->isActive = false;
		return;
	}

	D3DMATRIX rotate, matrix;
	MatrixRotateX(&matrix, pParticle->rotation.x);
	MatrixRotateY(&rotate, pParticle->rotation.y);
	MatrixMultiply(&rotate, &matrix, &rotate);
	MatrixRotateZ(&matrix, pParticle->rotation.z);
	MatrixMultiply(&rotate, &rotate, &matrix);

	custom_xyzw rotatedVelocity = {pParticle->velocity.x, pParticle->velocity.y, pParticle->velocity.z, 1.0f};
	MatrixMultiplyVector(&rotatedVelocity, &rotate, &rotatedVelocity);
	pParticle->velocity = {rotatedVelocity.x, rotatedVelocity.y, rotatedVelocity.z};

	pParticle->oldPosition = pParticle->position;
	pParticle->position.x += pParticle->velocity.x;
	pParticle->position.y += pParticle->velocity.y;
	pParticle->position.z += pParticle->velocity.z;

	auto *pPacket = D3DRenderPacketFindMatch(pPool, nullptr, nullptr, 0, 0, 0);
	assert(pPacket);
	pPacket->pMaterialFctn = &D3DMaterialParticlePacket;

	auto *pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);
	pChunk->numIndices = PARTICLE_INDICES;
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
static void D3DParticleInitialize(emitter *pEmitter, particle *pParticle)
{
	// Advance to the next slot in the circular buffer and reset the emitter's timer,
	// regardless if the recycled particle should show up in its new location or not.
	pEmitter->nextSlot = (pEmitter->nextSlot + 1) % MAX_PARTICLES_PER_EMITTER;
	if (pEmitter->numParticles < MAX_PARTICLES_PER_EMITTER)
	{
		pEmitter->numParticles++;
	}
	pEmitter->timer = pEmitter->timerBase;

	// Apply position/velocity/rotation settings to the particle, and with variance if any.
	pParticle->position = GetVariedXYZ(pEmitter->position,
		pEmitter->positionVarianceMin, pEmitter->positionVarianceMax);
	pParticle->velocity = GetVariedXYZ(pEmitter->velocity,
		pEmitter->velocityVarianceMin, pEmitter->velocityVarianceMax);
	pParticle->rotation = GetVariedXYZ(pEmitter->rotation,
		pEmitter->rotationVarianceMin, pEmitter->rotationVarianceMax);

	pParticle->bgra = pEmitter->bgra;
	pParticle->timeLeft = pEmitter->particleLifetime;

	// Once the particle is all set, make it active so it can be included in the rendering.
	pParticle->isActive = true;
}

//////////////////////
// Public Functions //
//////////////////////
void D3DParticleSystemClear(particle_system *pParticleSystem)
{
	if (pParticleSystem == nullptr)
		return;

	// Free memory from each emitter before clearing the pointers.
	for (auto pEmitter : pParticleSystem->emitterList)
	{
		SafeFree(pEmitter);
	}

	pParticleSystem->emitterList.clear();
}

emitter* D3DParticleEmitterInit(particle_system *pParticleSystem, int time)
{
	emitter	*pEmitter = reinterpret_cast<emitter*>(ZeroSafeMalloc(sizeof(emitter)));

	pEmitter->timer = time;
	pEmitter->timerBase = time;

	pParticleSystem->emitterList.push_back(pEmitter);
	return pEmitter;
}

void D3DParticleEmitterUpdate(emitter *pEmitter, custom_xyz deltaPosition)
{
	pEmitter->position.x += deltaPosition.x;
	pEmitter->position.y += deltaPosition.y;
	pEmitter->position.z += deltaPosition.z;
}

void D3DParticleSystemUpdate(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
							 d3d_render_cache_system *pCacheSystem)
{
	D3DCacheSystemReset(pCacheSystem);
	D3DRenderPoolReset(pPool, &D3DMaterialParticlePool);

	for (auto pEmitter : pParticleSystem->emitterList)
	{
		// Update existing particles.
		for (int i = 0; i < pEmitter->numParticles; i++)
		{
			D3DParticleUpdate(pEmitter, &pEmitter->particles[i], pPool);
		}

		// Initializing new particles
		if (--pEmitter->timer <= 0)
		{
			// Particles spawn one at a time and use circular buffing to track the next open particle.
			D3DParticleInitialize(pEmitter, &pEmitter->particles[pEmitter->nextSlot]);
		}
	}

	D3DCacheFill(pCacheSystem, pPool, 0);
	D3DCacheFlush(pCacheSystem, pPool, 0, D3DPT_LINESTRIP);
}
