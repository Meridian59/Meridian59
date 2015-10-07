// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

#define MAX_PARTICLES 256

particle_system gParticleSystemSand;
particle_system gParticleSystemRain;
particle_system gParticleSystemSnow;
particle_system gParticleSystemFireworks;

extern player_info player;

void D3DParticleInitPosSpeed(emitter *pEmitter, particle *pParticle);
void D3DParticleInitPosSpeedSphere(emitter *pEmitter, particle *pParticle);
void D3DParticleInitPosSpeedRandomCircle(emitter *pEmitter, particle *pParticle);
void D3DParticleInitPosSpeedCircleX(emitter *pEmitter, particle *pParticle);
void D3DParticleInitPosSpeedCircleY(emitter *pEmitter, particle *pParticle);
void D3DParticleInitPosSpeedCircleZ(emitter *pEmitter, particle *pParticle);
void D3DParticleInitRotation(emitter *pEmitter, particle *pParticle);
void D3DParticleInitColorEnergy(emitter *pEmitter, particle *pParticle);
void D3DParticleAddToRenderer(d3d_render_pool_new *pPool, particle *pParticle);
void D3DParticleVelocityUpdate(emitter *pEmitter, particle *pParticle);
void D3DParticleRandomizeEmitterPosition(emitter *pEmitter);

Bool D3DParticleIsAlive(emitter *pEmitter, particle *pParticle);
void D3DParticleDestroy(particle *pParticle);

void SandstormInit(void);
void RainInit(void);
void SnowInit(void);
void FireworksInit(void);

/*
 * D3DParticlesReset: Initializes all particle systems.
 */
void D3DParticlesInit(bool reset)
{
   int height = PlayerGetHeight();

   SandstormInit();
   RainInit();
   SnowInit();
   FireworksInit();
   if (reset)
      D3DParticleSystemSetPlayerPos((float)player.x, (float)player.y, (float)height);
}

void D3DParticleSystemReset(particle_system *pParticleSystem)
{
   list_type list;
   emitter *pEmitter;

   for (list = pParticleSystem->emitterList; list != NULL; list = list->next)
   {
      pEmitter = (emitter *)list->data;
      if (pEmitter->particles)
         SafeFree(pEmitter->particles);
   }
   // list_destroy frees the emitters.
   list_destroy(pParticleSystem->emitterList);
   pParticleSystem->emitterList = NULL;
}

void D3DParticleEmitterInit(particle_system *pParticleSystem, float posX, float posY, float posZ,
                           float velX, float velY, float velZ, unsigned char b, unsigned char g,
                           unsigned char r, unsigned char a, int energy, int timerBase,
                           float rotX, float rotY, float rotZ, int randomPos, int randomRot,
                           int maxParticles, int emitterFlags)
{
   emitter	*pEmitter = NULL;

   if (pParticleSystem == NULL)
      return;

   pEmitter = (emitter *)SafeMalloc(sizeof(emitter));

   if (pEmitter == NULL)
      return;
   // User can choose how many particles to display.
   pEmitter->maxParticles = (maxParticles * config.particles) / 100;
   if (pEmitter->maxParticles <= 0)
      return;

   // Allocate particle mem and set to 0.
   pEmitter->particles = (particle *)SafeMalloc(pEmitter->maxParticles * sizeof(particle));
   memset(pEmitter->particles, 0, pEmitter->maxParticles * sizeof(particle));

   pEmitter->numParticles = 0;
   pEmitter->numAlive = 0;
   pEmitter->emitterFlags = emitterFlags;
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

   if (!pParticleSystem->emitterList)
      pParticleSystem->emitterList = list_create(pEmitter);
   else
      list_add_item(pParticleSystem->emitterList, pEmitter);
}

/*
 * D3DParticleEmitterUpdate: Called when the player moves, to update emitter positions.
 */
void D3DParticleEmitterUpdate(emitter *pEmitter, float posX, float posY, float posZ)
{
   pEmitter->pos.x += posX;
   pEmitter->pos.y += posY;
   pEmitter->pos.z += posZ;
}

/*
 * D3DParticleEmittersSetPlayerPos: Called to set emitter initial pos from player pos.
 */
void D3DParticleSystemSetPlayerPos(float posX, float posY, float posZ)
{
   list_type list;
   emitter *pEmitter;

   for (list = gParticleSystemSand.emitterList; list != NULL; list = list->next)
   {
      pEmitter = (emitter *)list->data;

      if (pEmitter)
         D3DParticleEmitterUpdate(pEmitter, posX, posY, posZ);
   }
   for (list = gParticleSystemRain.emitterList; list != NULL; list = list->next)
   {
      pEmitter = (emitter *)list->data;

      if (pEmitter)
         D3DParticleEmitterUpdate(pEmitter, posX, posY, posZ);
   }
   for (list = gParticleSystemSnow.emitterList; list != NULL; list = list->next)
   {
      pEmitter = (emitter *)list->data;

      if (pEmitter)
         D3DParticleEmitterUpdate(pEmitter, posX, posY, posZ);
   }
   for (list = gParticleSystemFireworks.emitterList; list != NULL; list = list->next)
   {
      pEmitter = (emitter *)list->data;

      if (pEmitter)
         D3DParticleEmitterUpdate(pEmitter, posX, posY, posZ);
   }
}

/*
 * D3DParticleIsAlive: Decrements particle energy, checks if particle
 *                     is alive and can be displayed. Doesn't check if
 *                     particle is behind the player, because position
 *                     and velocity still need to be updated for these.
 */
Bool D3DParticleIsAlive(emitter *pEmitter, particle *pParticle)
{
   if (pParticle->energy == 0)
      return false;

   if (--pParticle->energy <= 0)
   {
      pParticle->energy = 0;
      pEmitter->numAlive--;

      return false;
   }

   if (pEmitter->emitterFlags & PS_WEATHER_EFFECT)
   {
      PDIB pdibCeiling = NULL;
      pdibCeiling = GetPointCeilingTexture(pParticle->pos.x, pParticle->pos.y);
      if (pdibCeiling)
      {
         pParticle->energy = 0;
         pEmitter->numAlive--;

         return false;
      }
   }

   // If we don't allow this type of particle to survive inside the ground, destroy it.
   if ((pEmitter->emitterFlags & PS_GROUND_DESTROY)
      && (pParticle->pos.z < GetPointFloor(pParticle->pos.x, pParticle->pos.y)))
   {
      //debug(("destroying particle, floor: %i, pHeight: %6.1f \n",
      //   GetPointFloor(pParticle->pos.x, pParticle->pos.y), pParticle->pos.z));
      pParticle->energy = 0;
      pEmitter->numAlive--;

      return false;
   }

   return true;
}

/*
 * D3DParticleSystemUpdateFluid: This functions updates any 'continuously' updating
 *   particle systems, such as the weather-related particle systems. Particles
 *   are constantly renewed as they run out of energy.
 */
void D3DParticleSystemUpdateFluid(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
                                  d3d_render_cache_system *pCacheSystem, Draw3DParams *params)
{
   list_type list;
   emitter *pEmitter;
   particle *pParticle;

   D3DCacheSystemReset(pCacheSystem);
   D3DRenderPoolReset(pPool, &D3DMaterialParticlePool);

   for (list = pParticleSystem->emitterList; list != NULL; list = list->next)
   {
      pEmitter = (emitter *)list->data;
      //debug(("numparticles is %i\n",pEmitter->numParticles));
      for (int curParticle = 0; curParticle < pEmitter->maxParticles; ++curParticle)
      {
         pParticle = &pEmitter->particles[curParticle];

         if (!D3DParticleIsAlive(pEmitter, pParticle))
         {
            if (--pEmitter->timer <= 0)
            {
               // Reset the particle if the timer is up.
               D3DParticleInitPosSpeed(pEmitter, pParticle);
               D3DParticleInitRotation(pEmitter, pParticle);
               D3DParticleInitColorEnergy(pEmitter, pParticle);
               pEmitter->timer = pEmitter->timerBase;
            }
            continue;
         }

         D3DParticleVelocityUpdate(pEmitter, pParticle);

         // If this particle is hidden from the player's view, don't let D3D try to draw it.
         if (!IsHidden(params, (long)pParticle->pos.x, (long)pParticle->pos.y,
               (long)pParticle->pos.x, (long)pParticle->pos.y))
            D3DParticleAddToRenderer(pPool, pParticle);
      }
   }

   D3DCacheFill(pCacheSystem, pPool, 0);
   D3DCacheFlush(pCacheSystem, pPool, 0, D3DPT_LINESTRIP);
}

/*
* D3DParticleSystemUpdateBurst: This functions updates any 'burst' updating
*   particle systems, such as fireworks. Particles are built-up and then
*   released all at once.
*/
void D3DParticleSystemUpdateBurst(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
                                  d3d_render_cache_system *pCacheSystem, Draw3DParams *params)
{
   int curParticle;
   list_type list;
   emitter *pEmitter;
   particle *pParticle;
   bool playedSound = false;

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

            if (!D3DParticleIsAlive(pEmitter, pParticle))
               continue;

            // TODO: standardize particle sound effects, for now hardcoded fireworks sound.
            if (!playedSound && curParticle == 0
               && pParticle->energy == pEmitter->energy - 1
               && rand() % 2)
            {
               SoundPlayFile("firework.wav", 0, pParticle->pos.x, pParticle->pos.y);
               playedSound = true;
            }

            D3DParticleVelocityUpdate(pEmitter, pParticle);

            // If this particle is hidden from the player's view, don't let D3D try to draw it.
            if (!IsHidden(params, (long)pParticle->pos.x, (long)pParticle->pos.y,
                     (long)pParticle->pos.x, (long)pParticle->pos.y))
               D3DParticleAddToRenderer(pPool, pParticle);

            // 10% chance to dim color.
            if ((int)rand() % 10 == 1)
            {
               --pParticle->bgra.b;
               --pParticle->bgra.g;
               --pParticle->bgra.r;
            }
         }

         // Emitter moves x, y coords for random fireworks positions.
         D3DParticleRandomizeEmitterPosition(pEmitter);
      }

      // Check timer.
      if (--pEmitter->timer <= 0)
      {
         // Only start another batch if none remaining alive in old.
         if (pEmitter->numAlive <= 0)
         {
            pEmitter->numAlive = 0;
            pEmitter->numParticles = 0;

            // Re-randomize the color.
            pEmitter->bgra.b = FIREWORKS_B;
            pEmitter->bgra.g = FIREWORKS_G;
            pEmitter->bgra.r = FIREWORKS_R;

            for (curParticle = 0; curParticle < pEmitter->maxParticles; curParticle++)
            {
               pParticle = &pEmitter->particles[curParticle];

               if (pParticle->energy == 0)
               {
                  D3DParticleInitPosSpeedSphere(pEmitter, pParticle);
                  D3DParticleInitRotation(pEmitter, pParticle);
                  D3DParticleInitColorEnergy(pEmitter, pParticle);
               }
            }
         }
         pEmitter->timer = pEmitter->timerBase;
      }
   }

   D3DCacheFill(pCacheSystem, pPool, 0);
   D3DCacheFlush(pCacheSystem, pPool, 0, D3DPT_LINESTRIP);
}

/*
* D3DParticleInitPosSpeed: Sets initial position and speed for a particle.
*/
void D3DParticleInitPosSpeed(emitter *pEmitter, particle *pParticle)
{
   int sign = 1;

   pParticle->pos.x = pEmitter->pos.x;
   pParticle->pos.y = pEmitter->pos.y;
   pParticle->pos.z = pEmitter->pos.z;

   if (pEmitter->emitterFlags & PS_RANDOM_XY)
   {
      if (rand() & 1)
         sign = -sign;
      pParticle->pos.x += sign * ((int)rand() % pEmitter->randomPos);
      if (rand() & 1)
         sign = -sign;
      pParticle->pos.y += sign * ((int)rand() % pEmitter->randomPos);
   }

   if (pEmitter->emitterFlags & PS_RANDOM_Z)
   {
      if (rand() & 1)
         sign = -sign;
      pParticle->pos.z += sign * ((int)rand() % pEmitter->randomPos);
   }

   pParticle->velocity.x = pEmitter->velocity.x;
   pParticle->velocity.y = pEmitter->velocity.y;
   pParticle->velocity.z = pEmitter->velocity.z;

   // Weather effect randomizing.
   if (pEmitter->emitterFlags & PS_WEATHER_EFFECT)
   {
      // Small randomization of Z velocity.
      pParticle->velocity.z *= ((float)(rand() % 11 + 5)) / 10.0f;

      // Half particles start at the ceiling, half start at
      // a lower point. This gives a nicer effect on screens
      // with large ceilings (TODO: standardise ceiling height).
      if (rand() & 1)
      {
         pParticle->pos.z = GetPointCeiling(pParticle->pos.x, pParticle->pos.y);
      }
   }
}

/*
 * D3DParticleInitPosSpeedSphere: Sets initial particle position and speed for a sphere emitter.
 */
void D3DParticleInitPosSpeedSphere(emitter *pEmitter, particle *pParticle)
{
   float angle = (float)(rand() % 360);
   float speed = (rand() % 9);

   pParticle->pos.x = pEmitter->pos.x;
   pParticle->pos.y = pEmitter->pos.y;
   pParticle->pos.z = pEmitter->pos.z;
   pParticle->velocity.z = (float)(rand() % 11 - 5);

   if (angle < 90)
   {
      pParticle->velocity.x = (float)cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
   else if (angle < 180)
   {
      pParticle->velocity.x = (float)-cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
   else if (angle < 270)
   {
      pParticle->velocity.x = (float)cos(angle) * speed;
      pParticle->velocity.y = (float)-sin(angle) * speed;
   }
   else if (angle < 360)
   {
      pParticle->velocity.x = (float)-cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
}

/*
* D3DParticleInitPosSpeedCircleZ: Sets initial particle position and speed for a circle emitter.
*/
void D3DParticleInitPosSpeedCircleZ(emitter *pEmitter, particle *pParticle)
{
   float angle = (float)(rand() % 360);
   float speed = 5.0f;

   pParticle->pos.x = pEmitter->pos.x;
   pParticle->pos.y = pEmitter->pos.y;
   pParticle->pos.z = pEmitter->pos.z;

   pParticle->velocity.z = 0.0f;

   if (angle < 90)
   {
      pParticle->velocity.x = (float)cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
   else if (angle < 180)
   {
      pParticle->velocity.x = (float)-cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
   else if (angle < 270)
   {
      pParticle->velocity.x = (float)cos(angle) * speed;
      pParticle->velocity.y = (float)-sin(angle) * speed;
   }
   else if (angle < 360)
   {
      pParticle->velocity.x = (float)-cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
}

/*
* D3DParticleInitPosSpeedCircleX: Sets initial particle position and speed for a circle emitter.
*/
void D3DParticleInitPosSpeedCircleX(emitter *pEmitter, particle *pParticle)
{
   float angle = (float)(rand() % 360);
   float speed = 5.0f;

   pParticle->pos.x = pEmitter->pos.x;
   pParticle->pos.y = pEmitter->pos.y;
   pParticle->pos.z = pEmitter->pos.z;

   pParticle->velocity.x = 0.0f;
   if (angle < 90)
   {
      pParticle->velocity.z = (float)cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
   else if (angle < 180)
   {
      pParticle->velocity.z = (float)-cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
   else if (angle < 270)
   {
      pParticle->velocity.z = (float)cos(angle) * speed;
      pParticle->velocity.y = (float)-sin(angle) * speed;
   }
   else if (angle < 360)
   {
      pParticle->velocity.z = (float)-cos(angle) * speed;
      pParticle->velocity.y = (float)sin(angle) * speed;
   }
}

/*
* D3DParticleInitPosSpeedCircleY: Sets initial particle position and speed for a circle emitter.
*/
void D3DParticleInitPosSpeedCircleY(emitter *pEmitter, particle *pParticle)
{
   float angle = (float)(rand() % 360);
   float speed = 5.0f;

   pParticle->pos.x = pEmitter->pos.x;
   pParticle->pos.y = pEmitter->pos.y;
   pParticle->pos.z = pEmitter->pos.z;

   pParticle->velocity.y = 0.0f;
   if (angle < 90)
   {
      pParticle->velocity.z = (float)cos(angle) * speed;
      pParticle->velocity.x = (float)sin(angle) * speed;
   }
   else if (angle < 180)
   {
      pParticle->velocity.z = (float)-cos(angle) * speed;
      pParticle->velocity.x = (float)sin(angle) * speed;
   }
   else if (angle < 270)
   {
      pParticle->velocity.z = (float)cos(angle) * speed;
      pParticle->velocity.x = (float)-sin(angle) * speed;
   }
   else if (angle < 360)
   {
      pParticle->velocity.z = (float)-cos(angle) * speed;
      pParticle->velocity.x = (float)sin(angle) * speed;
   }
}

/*
* D3DParticleInitPosSpeedCircleY: Sets initial particle position and speed for a random circle emitter.
*/
void D3DParticleInitPosSpeedRandomCircle(emitter *pEmitter, particle *pParticle)
{
   int random = rand() % 3;
   switch (random)
   {
      case 0: return D3DParticleInitPosSpeedCircleX(pEmitter, pParticle);
      case 1: return D3DParticleInitPosSpeedCircleY(pEmitter, pParticle);
      case 2: return D3DParticleInitPosSpeedCircleZ(pEmitter, pParticle);
   }
}

/*
* D3DParticleRotationInit: Sets initial rotation for a particle.
*/
void D3DParticleInitRotation(emitter *pEmitter, particle *pParticle)
{
   pParticle->rotation.x = pEmitter->rotation.x;
   pParticle->rotation.y = pEmitter->rotation.y;
   pParticle->rotation.z = pEmitter->rotation.z;

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
}

/*
 * D3DParticleInitColorEnergy: Sets particle initial color and energy.
 */
void D3DParticleInitColorEnergy(emitter *pEmitter, particle *pParticle)
{
   pParticle->bgra.b = pEmitter->bgra.b;
   pParticle->bgra.g = pEmitter->bgra.g;
   pParticle->bgra.r = pEmitter->bgra.r;
   pParticle->bgra.a = pEmitter->bgra.a;

   pParticle->energy = pEmitter->energy;
   pEmitter->numParticles++;
   pEmitter->numAlive++;
}

/*
 * D3DParticleRandomizeEmitterPosition: Moves the emitter a small random x/y amount.
 */
void D3DParticleRandomizeEmitterPosition(emitter *pEmitter)
{
   if (pEmitter->pos.x > 0)
   {
      if ((int)rand() & 1)
         pEmitter->pos.x += (((int)rand() % 48));
      else
         pEmitter->pos.x -= (((int)rand() % 48));
   }
   if (pEmitter->pos.y > 0)
   {
      if ((int)rand() & 1)
         pEmitter->pos.y += (((int)rand() % 48));
      else
         pEmitter->pos.y -= (((int)rand() % 48));
   }
}

/*
 * D3DParticleVelocityUpdate: Updates the velocity and position of a particle.
 */
void D3DParticleVelocityUpdate(emitter *pEmitter, particle *pParticle)
{
   D3DMATRIX rotate, matrix;
   custom_xyzw velocity;

   if (pEmitter->emitterFlags & PS_GRAVITY)
      pParticle->velocity.z -= 0.5f;
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
}

/*
* D3DParticleAddToRenderer: Adds a particle to be rendered.
*/
void D3DParticleAddToRenderer(d3d_render_pool_new *pPool, particle *pParticle)
{
   d3d_render_packet_new *pPacket;
   d3d_render_chunk_new *pChunk;

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
#define SANDSTORM_TIMER ((int)rand() % 240)

   D3DParticleSystemReset(&gParticleSystemSand);
   // four corners, blowing around the perimeter
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      0, 500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      500.0f, 0, 0,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
       MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      0, -500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      -500.0f, 0, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);

   // four corners, blowing towards player
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      353.55f, 353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      353.55f, -353.55f, 0,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      -353.55f, -353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      -353.55f, 353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);

   // forward, left, right, and back, blowing towards player
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -1024.0f, 0, EMITTER_HEIGHT,
      500.0f, 0.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 1024.0f, 0, EMITTER_HEIGHT,
      -500.0f, 0, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      0, EMITTER_RADIUS * 1024.0f, EMITTER_HEIGHT,
      0, -500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      0, EMITTER_RADIUS * -1024.0f, EMITTER_HEIGHT,
      0, 500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);

   // four corners, blowing around the perimeter
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      0, 500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      500.0f, 0, 0,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      0, -500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      -500.0f, 0, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);

   // four corners, blowing towards player
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      353.55f, 353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      353.55f, -353.55f, 0,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * 724.0f, EMITTER_HEIGHT,
      -353.55f, -353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 724.0f, EMITTER_RADIUS * -724.0f, EMITTER_HEIGHT,
      -353.55f, 353.55f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);

   // forward, left, right, and back, blowing towards player
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * -1024.0f, 0, EMITTER_HEIGHT,
      500.0f, 0.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      EMITTER_RADIUS * 1024.0f, 0, EMITTER_HEIGHT,
      -500.0f, 0, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
       MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      0, EMITTER_RADIUS * 1024.0f, EMITTER_HEIGHT,
      0, -500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
   D3DParticleEmitterInit(&gParticleSystemSand,
      0, EMITTER_RADIUS * -1024.0f, EMITTER_HEIGHT,
      0, 500.0f, 0.0f,
      SANDSTORM_B, SANDSTORM_G, SANDSTORM_R, SANDSTORM_A,
      EMITTER_ENERGY, SANDSTORM_TIMER,
      0, -PI / 500.0f, -PI / 500.0f, 1024, 2,
      MAX_PARTICLES, PS_RANDOM_XY | PS_RANDOM_Z);
}

void RainInit(void)
{
#define RAIN_EMITTER_RADIUS	(16384)
#define RAIN_EMITTER_ENERGY	(400)
#define RAIN_EMITTER_HEIGHT	(2500)
#define RAIN_TIMER ((int)rand() % 240)

   D3DParticleSystemReset(&gParticleSystemRain);

   for (int i = 0; i < 8; i++)
   {
      D3DParticleEmitterInit(&gParticleSystemRain,
         0, 0, RAIN_EMITTER_HEIGHT,
         0.0f, 0, -300.0f,
         RAIN_B, RAIN_G, RAIN_R, RAIN_A,
         RAIN_EMITTER_ENERGY, RAIN_TIMER,
         0, 0, 0, RAIN_EMITTER_RADIUS, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
      D3DParticleEmitterInit(&gParticleSystemRain,
         0, 0, RAIN_EMITTER_HEIGHT * 2,
         0.0f, 0, -300.0f,
         RAIN_B, RAIN_G, RAIN_R, RAIN_A,
         RAIN_EMITTER_ENERGY, RAIN_TIMER,
         0, 0, 0, RAIN_EMITTER_RADIUS, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
      D3DParticleEmitterInit(&gParticleSystemRain,
         0, 0, RAIN_EMITTER_HEIGHT,
         0.0f, 0, -300.0f,
         RAIN_B, RAIN_G, RAIN_R, RAIN_A,
         RAIN_EMITTER_ENERGY, RAIN_TIMER,
         0, 0, 0, RAIN_EMITTER_RADIUS / 4, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
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
#define SNOW_TIMER ((int)rand() % 240)

   D3DParticleSystemReset(&gParticleSystemSnow);

   for (int i = 0; i < 9; i++)
   {
      // Normal height.
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         5.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, SNOW_TIMER,
         0, 0, 0, SNOW_EMITTER_RADIUS, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         -5.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, SNOW_TIMER,
         0, 0, 0, SNOW_EMITTER_RADIUS, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         0.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, SNOW_TIMER,
         0, 0, 0, SNOW_EMITTER_RADIUS, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         0.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, SNOW_TIMER,
         0, 0, 0, SNOW_EMITTER_RADIUS, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT,
         0.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY, SNOW_TIMER,
         0, 0, 0, SNOW_EMITTER_RADIUS, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
      // These ones are higher, but disappear faster (filler).
      D3DParticleEmitterInit(&gParticleSystemSnow,
         0.0f, 0.0f, SNOW_EMITTER_HEIGHT * 2,
         0.0f, 0.0f, -30.0f,
         SNOW_B, SNOW_G, SNOW_R, SNOW_A,
         SNOW_EMITTER_ENERGY / 3, SNOW_TIMER,
         0, 0, 0, SNOW_EMITTER_RADIUS, 0,
         MAX_PARTICLES, PS_RANDOM_XY | PS_GROUND_DESTROY | PS_WEATHER_EFFECT);
   }
}

void FireworksInit(void)
{
#define MAX_FIREWORK_PARTICLES	512
   // Distance from the player to spawn particles.
#define FIREWORKS_EMITTER_RADIUS	(512)
   // Amount of energy to give them.
#define FIREWORKS_EMITTER_ENERGY	(120)
   // Default height to spawn them at.
#define FIREWORKS_EMITTER_HEIGHT	(3600)
#define FIREWORKS_TIMER ((int)rand() % 120)

   D3DParticleSystemReset(&gParticleSystemFireworks);

   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, -3000.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, -2500.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, -2000.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, -1500.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, -1000.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, -500.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, 0.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, 500.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, 1000.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, 1500.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, 2000.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
   D3DParticleEmitterInit(&gParticleSystemFireworks,
      12000.0f, 2500.0f, FIREWORKS_EMITTER_HEIGHT,
      5.0f, 5.0f, 0.0f,
      FIREWORKS_B, FIREWORKS_G, FIREWORKS_R, FIREWORKS_A,
      FIREWORKS_EMITTER_ENERGY, FIREWORKS_TIMER,
      0, 0, 0, FIREWORKS_EMITTER_RADIUS, 0,
      MAX_FIREWORK_PARTICLES, PS_NO_FLAGS);
}
