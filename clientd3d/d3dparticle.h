// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef __D3DPARTICLE_H__
#define __D3DPARTICLE_H__

#define MAX_PARTICLES	512
#define SANDSTORM_R		226
#define SANDSTORM_G		153
#define SANDSTORM_B		6
#define SANDSTORM_A		255

#define RAIN_R			175
#define RAIN_G			228
#define RAIN_B			249
#define RAIN_A			100

#define SNOW_R			255
#define SNOW_G			255
#define SNOW_B			255
#define SNOW_A			220

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
	int			numParticles;
	int			energy;
	int			timer;
	int			timerBase;
	int			randomPos;
	int			randomRot;
	custom_xyz	pos;
	custom_xyz	delta;
	custom_xyz	velocity;
	custom_xyz	rotation;
	custom_bgra	bgra;
	particle	particles[MAX_PARTICLES];
	Bool		bRandomizeXY;
	Bool		bRandomizeZ;
	Bool		bGroundDestroy;
	Bool		bWeatherEffect;
} emitter;

typedef struct particle_system
{
	int			numParticles;
	list_type	emitterList;
} particle_system;

void	D3DParticleSystemReset(particle_system *pParticleSystem);
void	D3DParticleEmitterInit(particle_system *pParticleSystem, float posX, float posY, float posZ,
							float velX, float velY, float velZ, unsigned char b, unsigned char g,
							unsigned char r, unsigned char a, int energy, int timerBase,
							float rotX, float rotY, float rotZ, Bool bRandomizeXY, Bool bRandomizeZ,
							int randomPos, int randomRot, Bool bGroundDestroy, Bool bWeatherEffect);
void	D3DParticleEmitterUpdate(emitter *pEmitter, float posX, float posY, float posZ);
//void	D3DParticleSystemRoomInit(particle_system *pParticleSystem, room_type *room);
void	D3DParticleSystemUpdate(particle_system *pParticleSystem, d3d_render_pool_new *pPool,
							 d3d_render_cache_system *pCacheSystem, Draw3DParams *params);

#endif
