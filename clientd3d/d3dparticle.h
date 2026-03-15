// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DPARTICLE_H__
#define __D3DPARTICLE_H__

constexpr int MAX_PARTICLES = 128;

struct particle
{
	int				energy;
	custom_xyz		position;
	custom_xyz		oldPosition;
	custom_xyz		velocity;
	custom_xyz		rotation;
	custom_bgra		bgra;
	float			size;
	float			weight;
	// Current age of the weather particle (in seconds)
	float			currentAge_s;
	// Max lifespan of the weather particle (in seconds)
	float			maxAge_s;
};

struct emitter
{
	int				numParticles;
	int				nextSlot;
	int				energy;
	float			timer_s;
	float			timerBase_s;

	custom_xyz		delta;
	
	// Base transform settings
	custom_xyz		position;
	custom_xyz		rotation;
	custom_xyz		velocity;
	
	// Randomization ranges to apply to the base transform settings.
	// Setting both min and max to 0 means no variance.
	custom_xyz		positionVarianceMin;
	custom_xyz		positionVarianceMax;
	custom_xyz		rotationVarianceMin;
	custom_xyz		rotationVarianceMax;
	custom_xyz		velocityVarianceMin;
	custom_xyz		velocityVarianceMax;
	
	custom_bgra		bgra;
	particle		particles[MAX_PARTICLES];
	
	bool			bDestroysOnSurface;  // If true, particle clears on hitting a ceiling or floor.
};

struct particle_system
{
	int			numParticles;
	list_type	emitterList;
};

void		D3DParticleEmitterUpdate(emitter *pEmitter, float posX, float posY, float posZ);
void		D3DParticleSystemReset(particle_system *pParticleSystem);

emitter*	D3DParticleEmitterInit(particle_system *pParticleSystem, float time);

void		D3DParticleSystemUpdate(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
							 d3d_render_cache_system *pCacheSystem);
void 		D3DParticleUpdate(emitter *pEmitter, particle *pParticle, d3d_render_pool_new *pPool);
void 		D3DParticleInitialize(emitter *pEmitter, particle *pParticle);
void		D3DParticleHide(particle *pParticle);

#endif
