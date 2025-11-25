// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DPARTICLE_H__
#define __D3DPARTICLE_H__

#define MAX_PARTICLES	512 // was 128

typedef struct particle
{
	int			energy;
	custom_xyz	pos;
	custom_xyz	oldPos;
	custom_xyz	velocity;
	custom_xyz	rotation;
	custom_bgra	bgra;
	float		size;
	float		weight;
} particle;

typedef struct emitter
{
	int				numParticles;
	int				energy;
	int				timer;
	int				timerBase;
	int				randomPos;
	int				randomRot;
	custom_xyz		pos;
	custom_xyz		delta;
	custom_xyz		velocity;
	custom_xyz		rotation;
	custom_bgra		bgra;
	particle		particles[MAX_PARTICLES];
	bool			bRandomize;
	bool			groundDestroy;
} emitter;

typedef struct particle_system
{
	int			numParticles;
	list_type	emitterList;
} particle_system;

void		D3DParticleEmitterUpdate(emitter *pEmitter, float posX, float posY, float posZ);
void		D3DParticleSystemReset(particle_system *pParticleSystem);

emitter*	D3DParticleEmitterInit(particle_system *pParticleSystem, int energy, int timerBase, 
							bool groundDestroy);
void		D3DParticleEmitterSetPos(emitter *pEmitter, float posX, float posY, float posZ);
void		D3DParticleEmitterSetVel(emitter *pEmitter, float velX, float velY, float velZ);
void		D3DParticleEmitterSetRot(emitter *pEmitter, float rotX, float rotY, float rotZ);
void		D3DParticleEmitterSetBGRA(emitter *pEmitter, const struct custom_bgra* newBGRA);
void		D3DParticleEmitterSetRandom(emitter *pEmitter, bool bRandomize, int randomPos, 
							float randomRot);
void		D3DParticleEmitterAddToList(particle_system *pParticleSystem, emitter *pEmitter);

void		D3DParticleEmitterUpdate(emitter *pEmitter, float posX, float posY, float posZ);
//void	D3DParticleSystemRoomInit(particle_system *pParticleSystem, room_type *room);
void		D3DParticleSystemUpdate(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
							 d3d_render_cache_system *pCacheSystem);

#endif
