// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DPARTICLE_H__
#define __D3DPARTICLE_H__

///////////////
// Constants //
///////////////
static constexpr int MAX_PARTICLES_PER_EMITTER = 128;

////////////////
// Structures //
////////////////
struct particle
{
	custom_xyz	position;
	custom_xyz	oldPosition;
	custom_xyz	velocity;
	custom_xyz	rotation;
	custom_bgra	bgra;
	// If false, the particle isn't included in the rendering.
	bool		isActive;
	int			timeLeft;
};

struct emitter
{
	int			numParticles;
	int			nextSlot;
	int			particleLifetime;
	int			timer;
	int			timerBase;

	// The world position of the emitter.
	custom_xyz	position;

	// Base velocity/rotation for newly initialized particles.
	custom_xyz	velocity;
	custom_xyz	rotation;

	// Randomization ranges to apply to the base transform settings for particles.
	// Setting both min and max to 0 means no variance.
	custom_xyz		positionVarianceMin;
	custom_xyz		positionVarianceMax;
	custom_xyz		rotationVarianceMin;
	custom_xyz		rotationVarianceMax;
	custom_xyz		velocityVarianceMin;
	custom_xyz		velocityVarianceMax;

	custom_bgra	bgra;
	particle	particles[MAX_PARTICLES_PER_EMITTER];
};

struct particle_system
{
	std::vector<emitter*>	emitterList;
	// Points to an external bool that determines if this particle system is active.
	bool*					pIsActive;	
};

////////////////
// Prototypes //
////////////////
void	D3DParticleSystemReset(particle_system *pParticleSystem);
emitter* D3DParticleEmitterInit(particle_system *pParticleSystem, int time);
void	D3DParticleEmitterUpdate(emitter *pEmitter, custom_xyz deltaPosition);
void	D3DParticleSystemUpdate(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
							 d3d_render_cache_system *pCacheSystem);

#endif
