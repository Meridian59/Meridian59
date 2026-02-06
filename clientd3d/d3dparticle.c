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

using namespace std::chrono;

// Variables
extern room_type current_room;

static steady_clock::time_point lastFrameTime = steady_clock::now();
// Using int64_t for delta time to match steady_clock's type.
static int64_t msDeltaTime = 0;

void D3DParticleDestroy(particle *pParticle);

void D3DParticleSystemReset(particle_system *pParticleSystem)
{
	list_destroy(pParticleSystem->emitterList);
	pParticleSystem->emitterList = NULL;
}

emitter* D3DParticleEmitterInit(particle_system *pParticleSystem, int energy, int timerBaseMs, 
								bool bWeatherEffect)
{
	emitter	*pEmitter = NULL;

	if (pParticleSystem == NULL)
		return nullptr;

	pEmitter = (emitter *)SafeMalloc(sizeof(emitter));

	if (pEmitter == NULL)
		return nullptr;

	memset(pEmitter, 0, sizeof(emitter));

	pEmitter->numParticles = 0;
	pEmitter->nextSlot = 0;
	pEmitter->energy = energy;
	pEmitter->timerMs = timerBaseMs;
	pEmitter->timerBaseMs = timerBaseMs;
	
	pEmitter->bWeatherEffect = bWeatherEffect;
	
	return pEmitter;
}
void D3DParticleEmitterSetPos(emitter *pEmitter, float posX, float posY, float posZ)
{
	pEmitter->pos.x = posX;
	pEmitter->pos.y = posY;
	pEmitter->pos.z = posZ;
}
void D3DParticleEmitterSetVel(emitter *pEmitter, float velX, float velY, float velZ)
{
	pEmitter->velocity.x = velX;
	pEmitter->velocity.y = velY;
	pEmitter->velocity.z = velZ;
}
void D3DParticleEmitterSetRot(emitter *pEmitter, float rotX, float rotY, float rotZ)
{
	pEmitter->rotation.x = rotX;
	pEmitter->rotation.y = rotY;
	pEmitter->rotation.z = rotZ;	
}
// Randomizes particle position and rotation. Setting any one of them to '0' disables it.
void D3DParticleEmitterSetRandom(emitter *pEmitter, int randomPos, int randomRot)
{
	pEmitter->randomPos = randomPos;
	pEmitter->randomRot = randomRot;
}
void D3DParticleEmitterSetBGRA(emitter *pEmitter, const custom_bgra &newBGRA)
{
	pEmitter->bgra.b = newBGRA.b;
	pEmitter->bgra.g = newBGRA.g;
	pEmitter->bgra.r = newBGRA.r;
	pEmitter->bgra.a = newBGRA.a;	
}
void D3DParticleEmitterAddToList(particle_system *pParticleSystem, emitter *pEmitter)
{
	if (NULL == pParticleSystem->emitterList)
		pParticleSystem->emitterList =
			list_create(pEmitter);
	else
		list_add_item(pParticleSystem->emitterList, pEmitter);
}

void D3DParticleEmitterUpdate(emitter *pEmitter, float posX, float posY, float posZ)
{
	pEmitter->pos.x += posX;
	pEmitter->pos.y += posY;
	pEmitter->pos.z += posZ;
}

void D3DParticleSystemUpdate(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
							 d3d_render_cache_system *pCacheSystem)
{
	int						curParticle;
	list_type				list;
	emitter					*pEmitter;
	particle				*pParticle;

	D3DCacheSystemReset(pCacheSystem);
	D3DRenderPoolReset(pPool, &D3DMaterialParticlePool);
	
	// Calculate time (in milliseconds) since the last frame.
	auto currentFrameTime = steady_clock::now();
	auto duration = duration_cast<milliseconds>(currentFrameTime - lastFrameTime);
	msDeltaTime = duration.count();
	lastFrameTime = currentFrameTime;

	for (list = pParticleSystem->emitterList; list != NULL; list = list->next)
	{
		pEmitter = (emitter *)list->data;
		
		// Update existing particles
		for (curParticle = 0; curParticle < pEmitter->numParticles; curParticle++)
		{
			pParticle = &pEmitter->particles[curParticle];			
			D3DParticleUpdate(pEmitter, pParticle, pPool);
		}

		// Creating new particles
		pEmitter->timerMs -= msDeltaTime;
		if (pEmitter->timerMs <= 0)
		{
			// Particles spawn one at a time and use circular buffing to track the next open particle.
			pParticle = &pEmitter->particles[pEmitter->nextSlot];
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
	if (pEmitter->bWeatherEffect)
	{
		pParticle->lifetimeMs += static_cast<int32_t>(msDeltaTime);
		if (pParticle->lifetimeMs >= pParticle->maxTimeMs)
		{
			D3DParticleDestroy(pParticle);
			return;
		}
	}

	custom_xyzw	velocity;
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;
	D3DMATRIX				rotate, matrix;

	velocity.x = pParticle->velocity.x;
	velocity.y = pParticle->velocity.y;
	velocity.z = pParticle->velocity.z;
	velocity.w = 1.0f;

	MatrixRotateX(&matrix, pParticle->rotation.x);
	MatrixRotateY(&rotate, pParticle->rotation.y);
	MatrixMultiply(&rotate, &matrix, &rotate);
	MatrixRotateZ(&matrix, pParticle->rotation.z);
	MatrixMultiply(&rotate, &rotate, &matrix);

	MatrixMultiplyVector(&velocity, &rotate, &velocity);

	pParticle->velocity.x = velocity.x;
	pParticle->velocity.y = velocity.y;
	pParticle->velocity.z = velocity.z;

	pParticle->oldPos.x = pParticle->pos.x;
	pParticle->oldPos.y = pParticle->pos.y;
	pParticle->oldPos.z = pParticle->pos.z;

	pParticle->pos.x += pParticle->velocity.x;
	pParticle->pos.y += pParticle->velocity.y;
	pParticle->pos.z += pParticle->velocity.z;
	
	pPacket = D3DRenderPacketFindMatch(pPool, NULL, NULL, 0, 0, 0);
	assert(pPacket);
	pPacket->pMaterialFctn = &D3DMaterialParticlePacket;

	pChunk = D3DRenderChunkNew(pPacket);
	assert(pChunk);
	pChunk->numIndices = 2;
	pChunk->numVertices = 2;
	pChunk->numPrimitives = 1;
	pChunk->pMaterialFctn = &D3DMaterialParticleChunk;

	MatrixTranslate(&pChunk->xForm, pParticle->pos.x, pParticle->pos.z, pParticle->pos.y);

	CHUNK_XYZ_SET(pChunk, 0, 0, 0, 0);
	CHUNK_XYZ_SET(pChunk, 1, -pParticle->velocity.x, -pParticle->velocity.y,
		-pParticle->velocity.z);
	CHUNK_BGRA_SET(pChunk, 0, pParticle->bgra.b, pParticle->bgra.g, pParticle->bgra.r,
		pParticle->bgra.a);
	CHUNK_BGRA_SET(pChunk, 1, pParticle->bgra.b, pParticle->bgra.g, pParticle->bgra.r,
		0);
	CHUNK_INDEX_SET(pChunk, 0, 0);
	CHUNK_INDEX_SET(pChunk, 1, 1);
}

void D3DParticleCreate(emitter *pEmitter, particle *pParticle)
{
	pParticle->pos.x = pEmitter->pos.x;
	pParticle->pos.y = pEmitter->pos.y;
	pParticle->pos.z = pEmitter->pos.z;

	if (pEmitter->randomPos)
	{
		int	sign = 1;

		if ((int)rand() & 1) sign = -sign;
		pParticle->pos.x += sign * ((int)rand() & pEmitter->randomPos);

		if ((int)rand() & 1) sign = -sign;
		pParticle->pos.y += sign * ((int)rand() & pEmitter->randomPos);
		
		// Weather particles spawn randomly between half height to max height.
		if (pEmitter->bWeatherEffect)
		{
			pParticle->pos.z -= ((int)rand() & (pEmitter->randomPos)/2);
		}
		// Otherwise, randomize z-position if this isn't a weather particle.
		else
		{
			if ((int)rand() & 1) sign = -sign;
			pParticle->pos.z += sign * ((int)rand() & pEmitter->randomPos);
		}
	}

	pParticle->velocity.x = pEmitter->velocity.x;
	pParticle->velocity.y = pEmitter->velocity.y;
	pParticle->velocity.z = pEmitter->velocity.z;

	pParticle->rotation.x = pEmitter->rotation.x;
	pParticle->rotation.y = pEmitter->rotation.y;
	pParticle->rotation.z = pEmitter->rotation.z;

	// Randomizes z-velocity a bit for weather effects.
	if (pEmitter->bWeatherEffect)
	{
		pParticle->velocity.z *= ((float)((int)rand() % 11 + 5)) / 10.0f;
	}
	
	// Each weather particle calculates the time it takes for them to land on the ground.
	if (pEmitter->bWeatherEffect)
	{				
		pParticle->lifetimeMs = 0;
		
		BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, pParticle->pos.x, pParticle->pos.y);
		if (leaf && leaf->sector->ceiling)
		{
			D3DParticleDestroy(pParticle);
			return;	
		}
		else if (leaf && leaf->sector->floor)
		{
			float floorHeight = GetFloorHeight(pParticle->pos.x, pParticle->pos.y, leaf->sector);

			if (pParticle->pos.z < floorHeight)
			{
				D3DParticleDestroy(pParticle);
				return;
			}
			pParticle->maxTimeMs = abs((pParticle->pos.z - floorHeight) / pParticle->velocity.z) * 1000;			
		}
		// Out-of-bound weather particles still spawn for a few seconds so they can still show normally
		// outdoors beyond areas like forest walls, or from outside windows.
		else
		{
			pParticle->maxTimeMs = 2000;
		}
	}

	if (pEmitter->randomRot)
	{
		float	random, sign;
		sign = 1;

		random = (int)rand() % pEmitter->randomRot;
		if (random <= 1)
			random = 2;
		if ((int)rand() & 1)
			sign = -sign;
		pParticle->rotation.x += (pEmitter->rotation.x * random * sign);
		pParticle->pos.x += (int)rand() % FINENESS;

		random = (int)rand() % pEmitter->randomRot;
		if (random <= 1) random = 2;
		if ((int)rand() & 1) sign = -sign;
		pParticle->rotation.y += (pEmitter->rotation.y * random * sign);
		pParticle->pos.y += (int)rand() % FINENESS;

		random = (int)rand() % pEmitter->randomRot;
		if (random <= 1) random = 2;
		if ((int)rand() & 1) sign = -sign;
		pParticle->rotation.z += (pEmitter->rotation.z * random * sign);
		pParticle->pos.z += (int)rand() % FINENESS;
	}

	pParticle->bgra.b = pEmitter->bgra.b;
	pParticle->bgra.g = pEmitter->bgra.g;
	pParticle->bgra.r = pEmitter->bgra.r;
	pParticle->bgra.a = pEmitter->bgra.a;

	pParticle->energy = pEmitter->energy;
	
	pEmitter->nextSlot = (pEmitter->nextSlot + 1) % MAX_PARTICLES;
	if (pEmitter->numParticles < MAX_PARTICLES)
	{
		pEmitter->numParticles++;
	}
	pEmitter->timerMs = pEmitter->timerBaseMs;
}

void D3DParticleDestroy(particle *pParticle)
{
	pParticle->maxTimeMs = 0;
	pParticle->energy = 0;
}
