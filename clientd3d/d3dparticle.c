// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

particle_system			gParticleSystemSand;
particle_system			gParticleSystemRain;
particle_system			gParticleSystemSnow;
particle_system			gParticleSystemFireworks;

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
							int randomPos, int randomRot, Bool bGroundDestroy, Bool bWeatherEffect,
                     int maxParticles)
{
	emitter	*pEmitter = NULL;

	if (pParticleSystem == NULL)
		return;

	pEmitter = (emitter *)SafeMalloc(sizeof(emitter));

	if (pEmitter == NULL)
		return;

   pEmitter->maxParticles = maxParticles;
   pEmitter->particles = (particle *)SafeMalloc(maxParticles * sizeof(particle));
   memset(pEmitter->particles, 0, maxParticles * sizeof(particle));
	pEmitter->numParticles = 0;
   pEmitter->numAlive = 0;
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
							 d3d_render_cache_system *pCacheSystem, Draw3DParams *params)
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
            // If this particle is hidden from the player's view, don't let D3D try to draw it.
            if (IsHidden(params, (long)pParticle->pos.x, (long)pParticle->pos.y, (long)pParticle->pos.x, (long)pParticle->pos.y))
               continue;
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

      if (pEmitter->numParticles < pEmitter->maxParticles)
		{
			int	curParticle;

			if (--pEmitter->timer <= 0)
			{
            for (curParticle = 0; curParticle < pEmitter->maxParticles; curParticle++)
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

void D3DFireworksSystemUpdate(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
							 d3d_render_cache_system *pCacheSystem, Draw3DParams *params)
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
      if (pEmitter->numParticles >= pEmitter->maxParticles && pEmitter->numAlive > 0)
      {
         //debug(("numparticles is %i\n",pEmitter->numParticles));
         for (curParticle = 0; curParticle < pEmitter->numParticles; curParticle++)
         {
            pParticle = &pEmitter->particles[curParticle];

            if (pParticle->energy == 0)
               continue;

            if (--pParticle->energy <= 0)
            {
               pParticle->energy = 0;
               pEmitter->numAlive--;
            }
            else
            {
               custom_xyzw	velocity;

               if (pParticle->velocity.x > 0)
                  velocity.x = pParticle->velocity.x - 0.1f;
               else if (pParticle->velocity.x < 0)
                  velocity.x = pParticle->velocity.x + 0.1f;
               else
                  velocity.x = pParticle->velocity.x;
               
               if (pParticle->velocity.y > 0)
                  velocity.y = pParticle->velocity.y - 0.1f;
               else if (pParticle->velocity.y < 0)
                  velocity.y = pParticle->velocity.y + 0.1f;
               else
                  velocity.y = pParticle->velocity.y;

               velocity.z = pParticle->velocity.z - 0.1f;
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

               // If this particle is hidden from the player's view, don't let D3D try to draw it.
               if (IsHidden(params, (long)pParticle->pos.x, (long)pParticle->pos.y, (long)pParticle->pos.x, (long)pParticle->pos.y))
                  continue;

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
               CHUNK_BGRA_SET(pChunk, 0, (pParticle->bgra.b), pParticle->bgra.g, pParticle->bgra.r,
                  pParticle->bgra.a);
               CHUNK_BGRA_SET(pChunk, 1, pParticle->bgra.b, pParticle->bgra.g, pParticle->bgra.r,
                  0);
               CHUNK_INDEX_SET(pChunk, 0, 0);
               CHUNK_INDEX_SET(pChunk, 1, 1);
               if ((int)rand() % 10 == 1)
               {
                  --pParticle->bgra.b;
                  --pParticle->bgra.g;
                  --pParticle->bgra.r;
               }
            }
         }
         if (pEmitter->pos.x > 0)
         {
            if ((int)rand() & 1)
               pEmitter->pos.x += (((int)rand() % 96));
            else
               pEmitter->pos.x -= (((int)rand() % 96));
         }
         if (pEmitter->pos.y > 0)
         {
            if ((int)rand() & 1)
               pEmitter->pos.y += (((int)rand() % 128));
            else
               pEmitter->pos.y -= (((int)rand() % 128));
         }
      }

      // Check timer.
      if (--pEmitter->timer <= 0)
      {
         // Only start another batch if none remaining alive in old.
         if (pEmitter->numAlive <= 0)
         {
            pEmitter->numAlive = 0;
            pEmitter->numParticles = 0;
            int curParticle;
            // Re-randomize the color.
            pEmitter->bgra.b = FIREWORKS_B;
            pEmitter->bgra.g = FIREWORKS_G;
            pEmitter->bgra.r = FIREWORKS_R;

            for (curParticle = 0; curParticle < pEmitter->maxParticles; curParticle++)
            {
               pParticle = &pEmitter->particles[curParticle];

               if (pParticle->energy == 0)
               {
                  float sign;
                  sign = 1;
                  pParticle->pos.x = pEmitter->pos.x;
                  pParticle->pos.y = pEmitter->pos.y;
                  pParticle->pos.z = pEmitter->pos.z;
                  if ((int)rand() & 1)
                     sign = -sign;
                  pParticle->velocity.x = pEmitter->velocity.x + (((int)rand() % 10) * sign);
                  if ((int)rand() & 1)
                     sign = -sign;
                  pParticle->velocity.y = pEmitter->velocity.y + (((int)rand() % 10) * sign);
                  if ((int)rand() & 1)
                     sign = -sign;
                  pParticle->velocity.z = pEmitter->velocity.z + (((int)rand() % 10) * sign);
                  //debug(("%4.2f, %4.2f\n",pEmitter->velocity.z,pEmitter->velocity.z));
                  pParticle->rotation.x = pEmitter->rotation.x;
                  pParticle->rotation.y = pEmitter->rotation.y;
                  pParticle->rotation.z = pEmitter->rotation.z;

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
                     //pParticle->pos.x += (int)rand() % FINENESS;

                     random = (int)rand() % pEmitter->randomRot;
                     if (random <= 1)
                        random = 2;
                     if ((int)rand() & 1)
                        sign = -sign;
                     pParticle->rotation.y += (pEmitter->rotation.y * random * sign);
                     //pParticle->pos.y += (int)rand() % FINENESS;

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
                  pEmitter->numAlive++;
               }
            }
         }
         pEmitter->timer = pEmitter->timerBase;
      }
	}

	D3DCacheFill(pCacheSystem, pPool, 0);
   D3DCacheFlush(pCacheSystem, pPool, 0, D3DPT_LINESTRIP);
}

void D3DParticleDestroy(particle *pParticle)
{
	pParticle->energy = 0;
}

void SandstormInit(void)
{
#define EMITTER_RADIUS	(12)
#define EMITTER_ENERGY	(40)
#define EMITTER_HEIGHT	(0)

   D3DParticleSystemReset(&gParticleSystemSand);
   // four corners, blowing around the perimeter
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      0, 500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      500.0f, 0, 0,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      0, -500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      -500.0f, 0, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);

   // four corners, blowing towards player
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      353.55f, 353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      353.55f, -353.55f, 0,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      -353.55f, -353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      -353.55f, 353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);

   // forward, left, right, and back, blowing towards player
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -1024.0f, 0, EMITTER_HEIGHT,
      500.0f, 0.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 1024.0f, 0, EMITTER_HEIGHT,
      -500.0f, 0, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      0, EMITTER_RADIUS * 1024.0f, EMITTER_HEIGHT,
      0, -500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      0, EMITTER_RADIUS * -1024.0f, EMITTER_HEIGHT,
      0, 500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);

   // four corners, blowing around the perimeter
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      0, 500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      500.0f, 0, 0,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      0, -500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      -500.0f, 0, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);

   // four corners, blowing towards player
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      353.55f, 353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      353.55f, -353.55f, 0,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      -353.55f, -353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      -353.55f, 353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);

   // forward, left, right, and back, blowing towards player
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -1024.0f, 0, EMITTER_HEIGHT,
      500.0f, 0.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 1024.0f, 0, EMITTER_HEIGHT,
      -500.0f, 0, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      0, EMITTER_RADIUS * 1024.0f, EMITTER_HEIGHT,
      0, -500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
   D3DParticleEmitterInit(&gParticleSystemSand,
      0, EMITTER_RADIUS * -1024.0f, EMITTER_HEIGHT,
      0, 500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, 1,
      0, -PI / 500.0f, -PI / 500.0f,
      1, 1, 1024, 2,
      0, 0, MAX_PARTICLES);
}

void RainInit(void)
{
#define RAIN_EMITTER_RADIUS	(16384)
#define RAIN_EMITTER_ENERGY	(400)
#define RAIN_EMITTER_HEIGHT	(2500)

   int i;

   D3DParticleSystemReset(&gParticleSystemRain);

   for (i = 0; i < 16; i++)
   {
      D3DParticleEmitterInit(&gParticleSystemRain,
         0, 0, RAIN_EMITTER_HEIGHT,
         0.0f, 0, -300.0f,
         RAIN_B, RAIN_G, RAIN_R, RAIN_A,
         RAIN_EMITTER_ENERGY, 1,
         0, 0, 0,
         1, 0, RAIN_EMITTER_RADIUS, 0,
         1, 1, MAX_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemRain,
         0, 0, RAIN_EMITTER_HEIGHT * 2,
         0.0f, 0, -300.0f,
         RAIN_B, RAIN_G, RAIN_R, RAIN_A,
         RAIN_EMITTER_ENERGY, 1,
         0, 0, 0,
         1, 0, RAIN_EMITTER_RADIUS, 0,
         1, 1, MAX_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemRain,
         0, 0, RAIN_EMITTER_HEIGHT,
         0.0f, 0, -300.0f,
         RAIN_B, RAIN_G, RAIN_R, RAIN_A,
         RAIN_EMITTER_ENERGY, 1,
         0, 0, 0,
         1, 0, RAIN_EMITTER_RADIUS / 4, 0,
         1, 1, MAX_PARTICLES);
   }
}

void SnowInit(void)
{
   // Distance from the player to spawn particles.
#define SNOW_EMITTER_RADIUS	(16384)
   // Amount of energy to give them.
#define SNOW_EMITTER_ENERGY	(400)
   // Default height to spawn them at.
#define SNOW_EMITTER_HEIGHT	(2500)

   int i;

   D3DParticleSystemReset(&gParticleSystemSnow);

   for (i = 0; i < 9; i++)
   {
      // Normal height.
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         5.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, 1,
         0, 0, 0,
         1, 0, SNOW_EMITTER_RADIUS, 0,
         1, 1, MAX_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         -5.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, 1,
         0, 0, 0,
         1, 0, SNOW_EMITTER_RADIUS, 0,
         1, 1, MAX_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         0.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, 1,
         0, 0, 0,
         1, 0, SNOW_EMITTER_RADIUS, 0,
         1, 1, MAX_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         0.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, 1,
         0, 0, 0,
         1, 0, SNOW_EMITTER_RADIUS, 0,
         1, 1, MAX_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         0.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, 1,
         0, 0, 0,
         1, 0, SNOW_EMITTER_RADIUS, 0,
         1, 1, MAX_PARTICLES);
      // These ones are higher, but disappear faster (filler).
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT * 2,
         0.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY / 3, 1,
         0, 0, 0,
         1, 0, SNOW_EMITTER_RADIUS, 0,
         1, 1, MAX_PARTICLES);
   }
}

void FireworksInit(void)
{
   // Distance from the player to spawn particles.
#define FIREWORKS_EMITTER_RADIUS	(512)
   // Amount of energy to give them.
#define FIREWORKS_EMITTER_ENERGY	(80)
   // Default height to spawn them at.
#define FIREWORKS_EMITTER_HEIGHT	(2048)

   int i;

   D3DParticleSystemReset(&gParticleSystemFireworks);

   for (i = 0; i < 8; i++)
   {
      // Normal height.
      D3DParticleEmitterInit(&gParticleSystemFireworks,
         4096.0f, 0.0f, FIREWORKS_EMITTER_HEIGHT,
         0.0f, 0.0f, -2.0f,
         FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
         FIREWORKS_EMITTER_ENERGY, 3,
         0, 0, 0,
         0, 0, FIREWORKS_EMITTER_RADIUS, 0,
         0, 0, MAX_FIREWORK_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemFireworks,
         4096.0f, 0.0f, FIREWORKS_EMITTER_HEIGHT,
         0.0f, 0.0f, -2.0f,
         FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
         FIREWORKS_EMITTER_ENERGY, 6,
         0, 0, 0,
         0, 0, FIREWORKS_EMITTER_RADIUS, 0,
         0, 0, MAX_FIREWORK_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemFireworks,
         0, 4096.0f, FIREWORKS_EMITTER_HEIGHT,
         0.0f, 0.0f, -2.0f,
         FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
         FIREWORKS_EMITTER_ENERGY, 9,
         0, 0, 0,
         0, 0, FIREWORKS_EMITTER_RADIUS, 0,
         0, 0, MAX_FIREWORK_PARTICLES);
      D3DParticleEmitterInit(&gParticleSystemFireworks,
         0, 4096.0f, FIREWORKS_EMITTER_HEIGHT,
         0.0f, 0.0f, -3.0f,
         FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
         FIREWORKS_EMITTER_ENERGY, 12,
         0, 0, 0,
         0, 0, FIREWORKS_EMITTER_RADIUS, 0,
         0, 0, MAX_FIREWORK_PARTICLES);
   }
   for (i = 0; i < 3; ++i)
      D3DParticleEmitterInit(&gParticleSystemFireworks,
         4096.0f, 4096.0f, FIREWORKS_EMITTER_HEIGHT * 2,
         0.0f, 0.0f, -1.0f,
         FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
         FIREWORKS_EMITTER_ENERGY, 2,
         0, 0, 0,
         0, 0, FIREWORKS_EMITTER_RADIUS, 0,
         0, 0, MAX_FIREWORK_PARTICLES * 3);
}
