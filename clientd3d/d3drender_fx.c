// Meridian 59, Copyright 1994-2024 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

// Variables
static particle_system sandParticleSystem;
static particle_system rainParticleSystem;
static particle_system snowParticleSystem;

// Default colors for particles
static const custom_bgra SANDSTORM_COLOR = {6,153,226,255};
static const custom_bgra RAIN_COLOR = {249,228,175,150};
static const custom_bgra SNOW_COLOR = {255,255,255,220};

// Interfaces

static void SandstormInit(void);
static void RainInit(void);
static void SnowInit(void);

// Implementations

/**
* Initializes the particle effects, specifically the sandstorm particle emitters.
*/
void D3DFxInit()
{
	SandstormInit();
	RainInit();
	SnowInit();
}

/**
* Updates and renders all active particle emitters, including the sandstorm effect, 
* in the current frame.
*/
void D3DRenderParticles(const ParticleSystemStructure& pss)
{
	list_type	list;
	emitter		*pEmitter;

	for (list = sandParticleSystem.emitterList; list != NULL; list = list->next)
	{
		pEmitter = (emitter *)list->data;

		if (pEmitter)
			D3DParticleEmitterUpdate(pEmitter, pss.playerDeltaPos.x, pss.playerDeltaPos.y, pss.playerDeltaPos.z);
	}

	for (list = rainParticleSystem.emitterList; list != NULL; list = list->next)
	{
		pEmitter = (emitter *)list->data;

		if (pEmitter)
			D3DParticleEmitterUpdate(pEmitter, pss.playerDeltaPos.x, pss.playerDeltaPos.y, pss.playerDeltaPos.z);
	}

	for (list = snowParticleSystem.emitterList; list != NULL; list = list->next)
	{
		pEmitter = (emitter *)list->data;

		if (pEmitter)
			D3DParticleEmitterUpdate(pEmitter, pss.playerDeltaPos.x, pss.playerDeltaPos.y, pss.playerDeltaPos.z);
	}

	if (effects.sand)
	{
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, pss.vertexDeclaration);

		D3DParticleSystemUpdate(&sandParticleSystem, pss.particlePool, pss.particleCacheSystem);
	}

	if (effects.raining)
	{
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, pss.vertexDeclaration);

		D3DParticleSystemUpdate(&rainParticleSystem, pss.particlePool, pss.particleCacheSystem);
	}

	if (effects.snowing)
	{
		IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
		IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, pss.vertexDeclaration);

		D3DParticleSystemUpdate(&snowParticleSystem, pss.particlePool, pss.particleCacheSystem);
	}
}

/**
* Initializes the sandstorm particle emitters with predefined positions, energy, and colors for the 
* particle system.
*/
void SandstormInit(void)
{
	const int SAND_EMITTER_RADIUS = 12;
	const int SAND_EMITTER_ENERGY = 40;
	const int SAND_EMITTER_HEIGHT = 0;
	const int SAND_TIMER = 1;
	const int SAND_RAND_POS = 1;
	const int SAND_RAND_ROT = 2;
	const bool SAND_WEATHER_EFFECT = false;
	
	D3DParticleSystemReset(&sandParticleSystem);
	emitter* newEmitter = nullptr;
	// four corners, blowing around the perimeter
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 0, 500.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);
	
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 500.0f, 0, 0);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 0, -500.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, -500.0f, 0, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	// four corners, blowing towards player
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 353.55f, 353.55f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 353.55f, -353.55f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, -353.55f, -353.55f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, -353.55f, 353.55f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	// forward, left, right, and back, blowing towards player
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -1024.0f, 0, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 500.0f, 0.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 1024.0f, 0, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, -500.0f, 0.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, 0, SAND_EMITTER_RADIUS * 1024.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 0, -500.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, 0, SAND_EMITTER_RADIUS * -1024.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 0, 500.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	

	// four corners, blowing around the perimeter
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 0, 500.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);
	
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 500.0f, 0, 0);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	
	
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 0, -500.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);		

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, -500.0f, 0, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	// four corners, blowing towards player
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 353.55f, 353.55f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	
	
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 353.55f, -353.55f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);		
	
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, -353.55f, -353.55f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);		

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 724.0f, SAND_EMITTER_RADIUS * -724.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, -353.55f, 353.55f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	

	// forward, left, right, and back, blowing towards player
	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * -1024.0f, 0, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 500.0f, 0.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, SAND_EMITTER_RADIUS * 1024.0f, 0, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, -500.0f, 0.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);	

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, 0, SAND_EMITTER_RADIUS * 1024.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 0, -500.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);

	newEmitter = D3DParticleEmitterInit(&sandParticleSystem, SAND_EMITTER_ENERGY, SAND_TIMER, SAND_WEATHER_EFFECT);
	D3DParticleEmitterSetPos(newEmitter, 0, SAND_EMITTER_RADIUS * -1024.0f, SAND_EMITTER_HEIGHT);
	D3DParticleEmitterSetVel(newEmitter, 0, 500.0f, 0.0f);
	D3DParticleEmitterSetRot(newEmitter, 0, -PI / 500.0f, -PI / 500.0f);
	D3DParticleEmitterSetBGRA(newEmitter, SANDSTORM_COLOR);
	D3DParticleEmitterSetRandom(newEmitter, SAND_RAND_POS, SAND_RAND_ROT);
	D3DParticleEmitterAddToList(&sandParticleSystem, newEmitter);
}

/**
* Initializes the rain particle emitters with predefined positions, energy, and colors for the 
* particle system.
*/
void RainInit(void)
{
	const int RAIN_EMITTER_RADIUS = 15000;
	const int RAIN_EMITTER_ENERGY = 120;
	const int RAIN_EMITTER_HEIGHT = 3000;
	const int RAIN_TIMER = 1;
	const bool RAIN_WEATHER_EFFECT = true;
	const int RAIN_VELOCITY = -250;
	const int RAIN_RANDOM_ROT = 2;
	// Rain emitter position is offsetted since they aren't centered on the player.
	const int RAIN_EMITTER_OFFSET = 600;

	D3DParticleSystemReset(&rainParticleSystem);
	emitter* newEmitter = nullptr;
	for(int i = 0; i < 16; i++)
	{
		newEmitter = D3DParticleEmitterInit(&rainParticleSystem, RAIN_EMITTER_ENERGY, RAIN_TIMER, RAIN_WEATHER_EFFECT);
		D3DParticleEmitterSetPos(newEmitter, -RAIN_EMITTER_OFFSET, -RAIN_EMITTER_OFFSET, RAIN_EMITTER_HEIGHT);
		D3DParticleEmitterSetVel(newEmitter, 0, 0, RAIN_VELOCITY);
		D3DParticleEmitterSetRot(newEmitter, 0, 0, 0);
		D3DParticleEmitterSetBGRA(newEmitter, RAIN_COLOR);
		D3DParticleEmitterSetRandom(newEmitter, RAIN_EMITTER_RADIUS, RAIN_RANDOM_ROT);
		D3DParticleEmitterAddToList(&rainParticleSystem, newEmitter);
	}
}

/**
* Initializes the snow particle emitters with predefined positions, energy, and colors for the 
* particle system.
*/
void SnowInit(void)
{
	const int SNOW_EMITTER_RADIUS = 15000;
	const int SNOW_EMITTER_ENERGY = 400;
	const int SNOW_EMITTER_HEIGHT = 3000;
	const int SNOW_FALL_SPEED = -35;
	const int SNOW_TIMER = 1;
	const int SNOW_RANDOM_ROT = 2;
	const bool SNOW_WEATHER_EFFECT = true;
	// Snow emitter position is offsetted since they aren't centered on the player.
	const int SNOW_EMITTER_OFFSET = 600;
	
	D3DParticleSystemReset(&snowParticleSystem);
	emitter* newEmitter = nullptr;
	for(int i = 0; i < 16; i++)
	{
		newEmitter = D3DParticleEmitterInit(&snowParticleSystem, SNOW_EMITTER_ENERGY, SNOW_TIMER, SNOW_WEATHER_EFFECT);
		D3DParticleEmitterSetPos(newEmitter, -SNOW_EMITTER_OFFSET, -SNOW_EMITTER_OFFSET, SNOW_EMITTER_HEIGHT);
		D3DParticleEmitterSetVel(newEmitter, 0, 0, SNOW_FALL_SPEED);
		D3DParticleEmitterSetRot(newEmitter, 0, 0, 0);
		D3DParticleEmitterSetBGRA(newEmitter, SNOW_COLOR);
		D3DParticleEmitterSetRandom(newEmitter, SNOW_EMITTER_RADIUS, SNOW_RANDOM_ROT);
		D3DParticleEmitterAddToList(&snowParticleSystem, newEmitter);
	}
}

/**
* Applies post-render overlay effects such as alpha blending, screen flashes, and pain effects, 
* and resets the rendering cache.
*/
void D3DPostOverlayEffects(const FxRenderSystemStructure& fxrss)
{
	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, TRUE, 1, D3DCMP_GREATEREQUAL);
	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, FALSE);

	D3DCacheSystemReset(fxrss.objectCacheSystem);
	D3DRenderPoolReset(fxrss.objectPool, &D3DMaterialObjectPool);

	static DWORD			timeLastFrame = 0;
	DWORD					timeCurrent, timeDelta;
	int						i;
	d3d_render_chunk_new	*pChunk;
	d3d_render_packet_new	*pPacket;

	timeCurrent = timeGetTime();
	timeDelta = timeCurrent - timeLastFrame;
	timeLastFrame = timeCurrent;

	// Flash of XLAT.  Could be color, blindness, whatever.
	if (effects.flashxlat != XLAT_IDENTITY)
	{
		custom_bgra	bgra;

		effects.duration -= (int)timeDelta;
		switch (effects.flashxlat)
		{
			case XLAT_BLEND10RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND25RED:
			case XLAT_BLEND30RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND75RED:
			case XLAT_BLEND80RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND10WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND30WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND80WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25YELLOW:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 192;
			break;
			case XLAT_BLEND25BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 192;
			break;

			default:
				bgra.b = 0; bgra.g = 0; bgra.r = 0; bgra.a = 255;
			break;
		}

		if (effects.duration <= 0)
		{
			effects.flashxlat = XLAT_IDENTITY;
			effects.duration = 0;
		}

		pPacket = D3DRenderPacketFindMatch(fxrss.objectPool, NULL, NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, bgra.b, bgra.g, bgra.r, bgra.a);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	if (effects.xlatOverride > 0)
	{
		custom_bgra	bgra;

		switch (effects.xlatOverride)
		{
			case XLAT_BLEND10RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND25RED:
			case XLAT_BLEND30RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND75RED:
			case XLAT_BLEND80RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100RED:
				bgra.b = 0; bgra.g = 0; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND10WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 25;
			break;

			case XLAT_BLEND20WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 50;
			break;

			case XLAT_BLEND30WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 75;
			break;

			case XLAT_BLEND40WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 100;
			break;

			case XLAT_BLEND50WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 125;
			break;

			case XLAT_BLEND60WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 150;
			break;

			case XLAT_BLEND70WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 175;
			break;

			case XLAT_BLEND80WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 200;
			break;

			case XLAT_BLEND90WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 225;
			break;

			case XLAT_BLEND100WHITE:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25YELLOW:
				bgra.b = COLOR_MAX; bgra.g = COLOR_MAX; bgra.r = COLOR_MAX; bgra.a = 255;
			break;

			case XLAT_BLEND25GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75GREEN:
				bgra.b = 0; bgra.g = COLOR_MAX; bgra.r = 0; bgra.a = 192;
			break;
			case XLAT_BLEND25BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 64;
			break;
			case XLAT_BLEND50BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 128;
			break;
			case XLAT_BLEND75BLUE:
				bgra.b = COLOR_MAX; bgra.g = 0; bgra.r = 0; bgra.a = 192;
			break;

			default:
				bgra.b = 0; bgra.g = 0; bgra.r = 0; bgra.a = 255;
			break;
		}

		pPacket = D3DRenderPacketFindMatch(fxrss.objectPool, NULL, NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, bgra.b, bgra.g, bgra.r, bgra.a);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	// May be drawn over room or map.

	// whiteout
	if (effects.whiteout)
	{
		int	whiteout = min(effects.whiteout, 500);

		whiteout = whiteout * COLOR_MAX / 500;
		whiteout = max(whiteout, 200);

		pPacket = D3DRenderPacketFindMatch(fxrss.objectPool, NULL, NULL, 0, 0, 0);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		assert(pChunk);
		pChunk->numIndices = 4;
		pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
		pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
		MatrixIdentity(&pChunk->xForm);

		for (i = 0; i < 4; i++)
		{
			CHUNK_BGRA_SET(pChunk, i, COLOR_MAX, COLOR_MAX, COLOR_MAX, whiteout);
		}

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
			D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));

		pChunk->indices[0] = 1;
		pChunk->indices[1] = 2;
		pChunk->indices[2] = 0;
		pChunk->indices[3] = 3;
	}

	// Pain (always drawn last).
	if (config.pain)
	{
		if (effects.pain)
		{
			int	pain = min(effects.pain, 2000);

			pain = pain * 204 / 2000;

			pPacket = D3DRenderPacketFindMatch(fxrss.objectPool, NULL, NULL, 0, 0, 0);
			if (NULL == pPacket)
				return;
			pChunk = D3DRenderChunkNew(pPacket);
			assert(pChunk);
			pChunk->numIndices = 4;
			pChunk->numVertices = 4;
			pChunk->numPrimitives = pChunk->numVertices - 2;
			pPacket->pMaterialFctn = &D3DMaterialEffectPacket;
			pChunk->pMaterialFctn = &D3DMaterialEffectChunk;
			MatrixIdentity(&pChunk->xForm);

			for (i = 0; i < 4; i++)
			{
				CHUNK_BGRA_SET(pChunk, i, 0, 0, COLOR_MAX, pain);
			}

			CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));
			CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(0, fxrss.screenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
			CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(fxrss.screenHeight, fxrss.screenHeight));
			CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxrss.screenWidth, fxrss.screenWidth), 0,
				D3DRENDER_SCREEN_TO_CLIP_Y(0, fxrss.screenHeight));

			pChunk->indices[0] = 1;
			pChunk->indices[1] = 2;
			pChunk->indices[2] = 0;
			pChunk->indices[3] = 3;
		}
	}

	D3DCacheFill(fxrss.objectCacheSystem, fxrss.objectPool, 1);
	D3DCacheFlush(fxrss.objectCacheSystem, fxrss.objectPool, 1, D3DPT_TRIANGLESTRIP);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);
}

/**
* Creates a blur and wave distortion effects using frame buffers, applying it to the current screen 
* using a series of textures.
*/
void D3DFxBlurWaver(const FxRenderSystemStructure& fxRss)
{
	d3d_render_packet_new	*pPacket;
	d3d_render_chunk_new	*pChunk;
	int						i, t;
	static int				offset = 0;
	static int				offsetDir = 1;

	t = fxRss.frame & 7;

	if (fxRss.frame & 63)
	{
		offset += offsetDir;

		if ((offset > 31) || (offset < 0))
		{
			offsetDir = -offsetDir;
			offset += offsetDir;
		}
	}

	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, FALSE, 1, D3DCMP_GREATEREQUAL);
	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, FALSE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

	IDirect3DDevice9_SetVertexShader(gpD3DDevice, NULL);
	IDirect3DDevice9_SetVertexDeclaration(gpD3DDevice, fxRss.vertexDeclaration);

	D3DRenderFramebufferTextureCreate(fxRss.backBufferTexFull, fxRss.backBufferTex[t],
		fxRss.smallTextureSize, fxRss.smallTextureSize);

	D3DCacheSystemReset(fxRss.effectCacheSystem);
	D3DRenderPoolReset(fxRss.effectPool, &D3DMaterialBlurPool);

	for (i = 0; i <= 7; i++)
	{
		pPacket = D3DRenderPacketNew(fxRss.effectPool);
		if (NULL == pPacket)
			return;
		pChunk = D3DRenderChunkNew(pPacket);
		pPacket->pMaterialFctn = D3DMaterialBlurPacket;
		pChunk->pMaterialFctn = D3DMaterialBlurChunk;
		pPacket->pTexture = fxRss.backBufferTex[i];
		pChunk->numIndices = pChunk->numVertices = 4;
		pChunk->numPrimitives = pChunk->numVertices - 2;
		MatrixIdentity(&pChunk->xForm);

		CHUNK_XYZ_SET(pChunk, 0, D3DRENDER_SCREEN_TO_CLIP_X(-offset, fxRss.screenWidth),
			0, D3DRENDER_SCREEN_TO_CLIP_Y(-offset, fxRss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 1, D3DRENDER_SCREEN_TO_CLIP_X(-offset, fxRss.screenWidth),
			0, D3DRENDER_SCREEN_TO_CLIP_Y(fxRss.screenHeight + offset, fxRss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 2, D3DRENDER_SCREEN_TO_CLIP_X(fxRss.screenWidth + offset, fxRss.screenWidth),
			0, D3DRENDER_SCREEN_TO_CLIP_Y(fxRss.screenHeight + offset, fxRss.screenHeight));
		CHUNK_XYZ_SET(pChunk, 3, D3DRENDER_SCREEN_TO_CLIP_X(fxRss.screenWidth + offset, fxRss.screenWidth),
			0, D3DRENDER_SCREEN_TO_CLIP_Y(-offset, fxRss.screenHeight));

		CHUNK_ST0_SET(pChunk, 0, 0.0f, 0.0f);
		CHUNK_ST0_SET(pChunk, 1, 0.0f, 1.0f);
		CHUNK_ST0_SET(pChunk, 2, 1.0f, 1.0f);
		CHUNK_ST0_SET(pChunk, 3, 1.0f, 0.0f);

		CHUNK_BGRA_SET(pChunk, 0, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
		CHUNK_BGRA_SET(pChunk, 1, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
		CHUNK_BGRA_SET(pChunk, 2, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);
		CHUNK_BGRA_SET(pChunk, 3, COLOR_MAX, COLOR_MAX, COLOR_MAX, COLOR_MAX / 4);

		CHUNK_INDEX_SET(pChunk, 0, 1);
		CHUNK_INDEX_SET(pChunk, 1, 2);
		CHUNK_INDEX_SET(pChunk, 2, 0);
		CHUNK_INDEX_SET(pChunk, 3, 3);
	}

	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_WORLD, &fxRss.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_VIEW, &fxRss.transformMatrix);
	IDirect3DDevice9_SetTransform(gpD3DDevice, D3DTS_PROJECTION, &fxRss.transformMatrix);
	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, FALSE);

	D3DRENDER_SET_ALPHATEST_STATE(gpD3DDevice, FALSE, 1, D3DCMP_GREATEREQUAL);
	D3DRENDER_SET_ALPHABLEND_STATE(gpD3DDevice, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

	D3DCacheFill(fxRss.effectCacheSystem, fxRss.effectPool, 1);
	D3DCacheFlush(fxRss.effectCacheSystem, fxRss.effectPool, 1, D3DPT_TRIANGLESTRIP);

	IDirect3DDevice9_SetRenderState(gpD3DDevice, D3DRS_ZENABLE, TRUE);
}
