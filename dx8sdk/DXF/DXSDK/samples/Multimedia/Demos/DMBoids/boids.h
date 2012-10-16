//-----------------------------------------------------------------------------
// File: Boids.h
//
// Desc: 
//       
// Copyright (c) 1995-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef BOIDS_H
#define BOIDS_H




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class Boid 
{
public:
	D3DXMATRIX 	matWorld;		// matrix representing the boids location/orientation
	D3DXVECTOR3	vPos;		    // location
	D3DXVECTOR3	vDir;	    	// cur direction
	D3DXVECTOR3	vSeparationForce;
	D3DXVECTOR3	vAlignmentForce;
	D3DXVECTOR3	vCohesionForce;
	D3DXVECTOR3	vMigratoryForce;
	D3DXVECTOR3	vObstacleForce;
	DWORD       dwNumNeighbors;

	D3DXVECTOR3	vDeltaPos;	// change in position from flock centering
	D3DXVECTOR3	vDeltaDir;	// change in direction
	int			iDeltaCnt;	// number of boids that influence this delta_dir
	FLOAT		speed;
	FLOAT		yaw, pitch, roll, dyaw;
	D3DXVECTOR3	color;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct Obstacle
{
	D3DXVECTOR3 vPos;
	FLOAT       fRadius;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CFlock
{
public:
	DWORD		m_dwNumBoids;
	Boid*       m_Boids;
	DWORD		m_dwNumObstacles;
	Obstacle*   m_Obstacles;
	FLOAT**     m_afDist;	// 2-d array of boid distances, yuk what a waste
	D3DXVECTOR3	m_vGoal;

	// Functions
	VOID Update( FLOAT fElapsedTime );
};





#endif


