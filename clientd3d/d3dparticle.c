// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Variables
extern room_type current_room;

void D3DParticleDestroy(particle *pParticle);

void D3DParticleSystemReset(particle_system *pParticleSystem)
{
	list_destroy(pParticleSystem->emitterList);
	pParticleSystem->emitterList = NULL;
}

emitter* D3DParticleEmitterInit(particle_system *pParticleSystem, int energy, int timerBase, 
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
	pEmitter->energy = energy;
	pEmitter->timer = timerBase;
	pEmitter->timerBase = timerBase;
	
	// bWeatherEffect makes particles clear when hitting ceilings or floors. It also
	// randomzies velocity.z a bit, and also ignores randomPos for the z-axis.
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
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;
	D3DMATRIX				rotate, matrix;

	D3DCacheSystemReset(pCacheSystem);
	D3DRenderPoolReset(pPool, &D3DMaterialParticlePool);

	for (list = pParticleSystem->emitterList; list != NULL; list = list->next)
	{
		pEmitter = (emitter *)list->data;

		for (curParticle = 0; curParticle < pEmitter->numParticles; curParticle++)
		{
			pParticle = &pEmitter->particles[curParticle];

			if (--pParticle->energy <= 0)
			{
				D3DParticleDestroy(pParticle);
				pEmitter->numParticles--;
			}
			else
			{
				custom_xyzw	velocity;

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

				// bWeatherEffect means that every frame, each particle checks in their leaf sector if they
				// hit a ceiling or floor. If so, the particle is cleared.
				if (pEmitter->bWeatherEffect)
				{
					BSPleaf *leaf = BSPFindLeafByPoint(current_room.tree, pParticle->pos.x, pParticle->pos.y);
					if (leaf && leaf->sector->ceiling)
					{
						D3DParticleDestroy(pParticle);
						pEmitter->numParticles--;
						continue;
					}
					if (leaf && (pParticle->pos.z < GetFloorHeight(pParticle->pos.x, pParticle->pos.y, leaf->sector)))
					{
						pParticle->energy = 0;
						continue;
					}
				}

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
		}

		if (pEmitter->numParticles < MAX_PARTICLES)
		{
			int	curParticle;

			if (--pEmitter->timer <= 0)
			{
				for (curParticle = 0; curParticle < MAX_PARTICLES; curParticle++)
				{
					pParticle = &pEmitter->particles[curParticle];

					if (pParticle->energy == 0)
					{
						pParticle->pos.x = pEmitter->pos.x;
						pParticle->pos.y = pEmitter->pos.y;
						pParticle->pos.z = pEmitter->pos.z;

						if (pEmitter->randomPos)
						{
							int	sign = 1;

							if ((int)rand() & 1)
								sign = -sign;
							pParticle->pos.x += sign * ((int)rand() & pEmitter->randomPos);

							if ((int)rand() & 1)
								sign = -sign;
							pParticle->pos.y += sign * ((int)rand() & pEmitter->randomPos);
							
							// Weather particles spawn randomly between half height to max height.
							if (pEmitter->bWeatherEffect)
							{
								pParticle->pos.z -= ((int)rand() & (pEmitter->randomPos)/2);
							}
							// Otherwise, randomize z-position if this isn't a weather particle.
							else
							{
								if ((int)rand() & 1)
									sign = -sign;
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
							if (random <= 1)
								random = 2;
							if ((int)rand() & 1)
								sign = -sign;
							pParticle->rotation.y += (pEmitter->rotation.y * random * sign);
							pParticle->pos.y += (int)rand() % FINENESS;

							random = (int)rand() % pEmitter->randomRot;
							if (random <= 1)
								random = 2;
							if ((int)rand() & 1)
								sign = -sign;
							pParticle->rotation.z += (pEmitter->rotation.z * random * sign);
							pParticle->pos.z += (int)rand() % FINENESS;
						}

						pParticle->bgra.b = pEmitter->bgra.b;
						pParticle->bgra.g = pEmitter->bgra.g;
						pParticle->bgra.r = pEmitter->bgra.r;
						pParticle->bgra.a = pEmitter->bgra.a;

						pParticle->energy = pEmitter->energy;

						pEmitter->numParticles++;
						curParticle = MAX_PARTICLES;
					}
				}

				pEmitter->timer = pEmitter->timerBase;
			}
		}
	}

	D3DCacheFill(pCacheSystem, pPool, 0);
	D3DCacheFlush(pCacheSystem, pPool, 0, D3DPT_LINESTRIP);
}

void D3DParticleDestroy(particle *pParticle)
{
	pParticle->energy = 0;
}
