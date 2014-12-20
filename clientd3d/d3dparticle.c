// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

void D3DParticleDestroy(particle *pParticle);

void D3DParticleSystemReset(particle_system *pParticleSystem)
{
	list_destroy(pParticleSystem->emitterList);
	pParticleSystem->emitterList = NULL;
}

void D3DParticleEmitterInit(particle_system *pParticleSystem, float posX, float posY, float posZ,
							float velX, float velY, float velZ, unsigned char b, unsigned char g,
							unsigned char r, unsigned char a, int energy, int timerBase,
							float rotX, float rotY, float rotZ, Bool bRandomizeXY, Bool bRandomizeZ,
							int randomPos, int randomRot, Bool bGroundDestroy, Bool bWeatherEffect)
{
	emitter	*pEmitter = NULL;

	if (pParticleSystem == NULL)
		return;

	pEmitter = (emitter *)SafeMalloc(sizeof(emitter));

	if (pEmitter == NULL)
		return;

	memset(pEmitter, 0, sizeof(emitter));

	pEmitter->numParticles = 0;
	pEmitter->bRandomizeXY = bRandomizeXY;
	pEmitter->bRandomizeZ = bRandomizeZ;
	pEmitter->bGroundDestroy = bGroundDestroy;
	pEmitter->bWeatherEffect = bWeatherEffect;
	pEmitter->pos.x = posX;
	pEmitter->pos.y = posY;
	pEmitter->pos.z = posZ;
	pEmitter->rotation.x = rotX;
	pEmitter->rotation.y = rotY;
	pEmitter->rotation.z = rotZ;
	pEmitter->velocity.x = velX;
	pEmitter->velocity.y = velY;
	pEmitter->velocity.z = velZ;
	pEmitter->energy = energy;
	pEmitter->timer = timerBase;
	pEmitter->timerBase = timerBase;
	pEmitter->bgra.b = b;
	pEmitter->bgra.g = g;
	pEmitter->bgra.r = r;
	pEmitter->bgra.a = a;
	pEmitter->randomPos = randomPos;
	pEmitter->randomRot = randomRot;

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
		//debug(("numparticles is %i\n",pEmitter->numParticles));

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
				PDIB pdibCeiling = NULL;
				pdibCeiling = GetPointCeilingTexture(pParticle->pos.x, pParticle->pos.y);
				if (pEmitter->bWeatherEffect && pdibCeiling)
				{
					D3DParticleDestroy(pParticle);
					pEmitter->numParticles--;

					continue;
				}

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

				// If we don't allow this type of particle to survive inside the ground, destroy it.
				if ((pEmitter->bGroundDestroy) && (pParticle->pos.z < GetPointFloor(pParticle->pos.x, pParticle->pos.y)))
				{
					//debug(("destroying particle, floor: %i, pHeight: %6.1f \n",GetPointFloor(pParticle->pos.x, pParticle->pos.y), pParticle->pos.z));
					pParticle->energy = 0;

					continue;
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

						if (pEmitter->bRandomizeXY)
						{
							int	sign = 1;

							if ((int)rand() & 1)
								sign = -sign;
							pParticle->pos.x += sign * ((int)rand() % pEmitter->randomPos);
							if ((int)rand() & 1)
								sign = -sign;
							pParticle->pos.y += sign * ((int)rand() % pEmitter->randomPos);

							if (pEmitter->bRandomizeZ)
							{
								if ((int)rand() & 1)
									sign = -sign;
								pParticle->pos.z += sign * ((int)rand() % pEmitter->randomPos);
							}
						}

						pParticle->velocity.x = pEmitter->velocity.x;
						pParticle->velocity.y = pEmitter->velocity.y;
						pParticle->velocity.z = pEmitter->velocity.z;

						pParticle->rotation.x = pEmitter->rotation.x;
						pParticle->rotation.y = pEmitter->rotation.y;
						pParticle->rotation.z = pEmitter->rotation.z;

						// Weather effect randomizing.
						if (pEmitter->bWeatherEffect)
						{
							// Small randomization of Z velocity.
							pParticle->velocity.z *= ((float)((int)rand() % 11 + 5)) / 10.0f;

								// Half particles start at the ceiling, half start at
								// a lower point. This gives a nicer effect on screens
								// with large ceilings (TODO: standardise ceiling height).
								if ((int)rand() & 1)
								{
									pParticle->pos.z = GetPointCeiling(pParticle->pos.x, pParticle->pos.y);
								}
						}

						if (pEmitter->randomRot)
						{
							float	random, sign;

							sign = 1;

/*							random = (int)rand() % 10;
							random = DEGREES_TO_RADIANS(random);
							pParticle->rotation.x += random * sign;

							random = (int)rand() % 10;
							random = DEGREES_TO_RADIANS(random);
							pParticle->rotation.y += random * sign;

							random = (int)rand() % 10;
							random = DEGREES_TO_RADIANS(random);
							pParticle->rotation.z += random * sign;*/
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
