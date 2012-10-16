//-----------------------------------------------------------------------------
// File: Flock.cpp
//
// Desc: 
//       
// Copyright (c) 1995-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <D3DX8.h>
#include <stdio.h>
#include "DXUtil.h"
#include "boids.h"
#include "music.h"

const float g_fInfluenceRadius = 10.0f;	// outside of this range forces are considered to be 0

const float CollisionFraction = 0.8f;
const float InvCollisionFraction = 1.0f/(1.0f-CollisionFraction);

const float g_fNormalSpeed = 0.1f;
const float	AngleTweak = 0.02f;
const float g_fPitchToSpeedRatio = 0.002f;

// More arbitray constants that look cool
const float	fSeparationScale	= 0.05f;
const float	fAlignmentScale		= 0.1f;
const float	fCohesionScale		= 1.0f;
const float	fMigratoryScale		= 0.4f;
const float	fObstacleScale		= 1.0f;




//-----------------------------------------------------------------------------
// Effects
//-----------------------------------------------------------------------------
extern BoidMusic g_Music;
extern BOOL      g_bSeparation;
extern BOOL      g_bAlignment;
extern BOOL      g_bCohesion;
extern BOOL      g_bMigratory;
extern BOOL      g_bObstacle;




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CFlock::Update( FLOAT fElapsedTime )
{
	DWORD   i, j;
	static DWORD lastobj = 0xffffffff;

	// First, update the dist array 0.0..1.0 with 0.0 being furthest away
	for( i=0; i<m_dwNumBoids; i++ ) 
	{
		for( j=i+1; j<m_dwNumBoids; j++ ) 
		{
			D3DXVECTOR3 vDiff = m_Boids[i].vPos - m_Boids[j].vPos;
			FLOAT       fDist = D3DXVec3Length( &vDiff );
			m_afDist[i][j] = m_afDist[j][i] = fDist;
		}
		m_afDist[i][i] = 0.0f;

		// Reset boid forces
		m_Boids[i].vSeparationForce = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		m_Boids[i].vAlignmentForce  = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		m_Boids[i].vCohesionForce   = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		m_Boids[i].vMigratoryForce  = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		m_Boids[i].vObstacleForce   = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		m_Boids[i].dwNumNeighbors   = 0;
	}

	// For each boid calculate the individual forces affecting it
	for( i=0; i<m_dwNumBoids; i++ ) 
	{
		// Add in effects from other boids
		for( j=i+1; j<m_dwNumBoids; j++ ) 
		{
			D3DXVECTOR3	vDiff = m_Boids[i].vPos - m_Boids[j].vPos;
			FLOAT       fDist = D3DXVec3Length( &vDiff );

			// if i is near j have them influence each other
			if( fDist < g_fInfluenceRadius ) 
			{
				// Sum seperation force
				m_Boids[i].vSeparationForce += vDiff/(fDist*fDist);
				m_Boids[j].vSeparationForce -= vDiff/(fDist*fDist);

				// sum alignment force (actually summing the directions of the neighbors)
				m_Boids[i].vAlignmentForce += m_Boids[j].vDir / fDist;
				m_Boids[j].vAlignmentForce += m_Boids[i].vDir / fDist;
				
				// sum cohesion force (actually we're summing neighbor locations)				
				m_Boids[i].vCohesionForce += m_Boids[j].vPos;
				m_Boids[j].vCohesionForce += m_Boids[i].vPos;

				m_Boids[i].dwNumNeighbors++;
				m_Boids[j].dwNumNeighbors++;
			}
		}

		// Add in any obstacle forces
		for( j=0; j<m_dwNumObstacles; j++ )
		{
			D3DXVECTOR3	vDiff     = m_Boids[i].vPos - m_Obstacles[j].vPos;
			FLOAT       fObRadius = m_Obstacles[j].fRadius * 1.5f;

			// Ignore object if already past
			if( D3DXVec3Dot( &vDiff, &m_Boids[i].vDir ) > 0.0f )
				continue;

			FLOAT fDist = D3DXVec3Length( &vDiff ) - fObRadius;

			if( fDist < g_fInfluenceRadius )
			{
				if( ( lastobj != j ) && ( fDist < 5.0f ) )
				{
					lastobj = j;
					g_Music.Transition();
				}
				vDiff /= fDist;	// normalize

				fDist -= fObRadius;
				if( fDist < 0.01f )
					fDist = 0.01f;
				
				m_Boids[i].vObstacleForce += vDiff;
			}
		}

		// Find cohesion force
		if( m_Boids[i].dwNumNeighbors ) 
		{
			m_Boids[i].vCohesionForce /= (FLOAT)m_Boids[i].dwNumNeighbors;	 // Find average location of neighbors
			D3DXVECTOR3	vDiff = m_Boids[i].vCohesionForce - m_Boids[i].vPos; // Find delta to center of flock
			FLOAT       fMag  = D3DXVec3Length( &vDiff );
			
			if( fMag > 0.0f)
				m_Boids[i].vCohesionForce = vDiff/fMag;	// normalized
			else
				m_Boids[i].vCohesionForce = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		}

		// Find the alignment force
		if( m_Boids[i].dwNumNeighbors != 0 ) 
		{
			m_Boids[i].vAlignmentForce /= (FLOAT)m_Boids[i].dwNumNeighbors;
			FLOAT fMag = D3DXVec3Length( &m_Boids[i].vAlignmentForce );
			
			if( fMag > 0.0f ) 
			{
				m_Boids[i].vAlignmentForce /= fMag;	// normalize
				
				D3DXVECTOR3	vDiff = m_Boids[i].vAlignmentForce - m_Boids[i].vDir;
				
				m_Boids[i].vAlignmentForce = vDiff / fMag;
			} 
		}

		// Finally, the migratory force
		m_Boids[i].vMigratoryForce  = m_vGoal - m_Boids[i].vPos;
		D3DXVec3Normalize( &m_Boids[i].vMigratoryForce, &m_Boids[i].vMigratoryForce );
	}

	// Update the boids
	for( i=0; i<m_dwNumBoids; i++ ) 
	{
		// Sum all the forces
		D3DXVECTOR3	vForce( 0.0f, 0.0f, 0.0f );
		if( !g_bObstacle )	 vForce += m_Boids[i].vObstacleForce;
		if( !g_bSeparation ) vForce += m_Boids[i].vSeparationForce;
		if( !g_bAlignment )	 vForce += m_Boids[i].vAlignmentForce * fAlignmentScale;
		if( !g_bCohesion )	 vForce += m_Boids[i].vCohesionForce;
		if( !g_bMigratory )	 vForce += m_Boids[i].vMigratoryForce;

		// Ok, now we have a final force to apply to the boid.
		// Normalize it if too big.
		FLOAT mag = D3DXVec3Length( &vForce );
		if( mag > 1.0f )
			vForce /= mag;

		// first deal with pitch changes
		if( vForce.y > 0.01f ) 
		{			// we're too low
			m_Boids[i].pitch += AngleTweak;
			if (m_Boids[i].pitch > 0.8f)
				m_Boids[i].pitch = 0.8f;
		} 
		else if( vForce.y < -0.01f ) 
		{	// we're too high
			m_Boids[i].pitch -= AngleTweak;
			if (m_Boids[i].pitch < -0.8f)
				m_Boids[i].pitch = -0.8f;
		}
		else
		{
			// add damping
			m_Boids[i].pitch *= 0.98f;
		}

		// speed up or slow down depending on angle of attack
		m_Boids[i].speed -= m_Boids[i].pitch * g_fPitchToSpeedRatio;
		// damp back to normal
		m_Boids[i].speed = (m_Boids[i].speed-g_fNormalSpeed)*0.99f + g_fNormalSpeed;

		// limit speed changes to +- 50% from normal
		if( m_Boids[i].speed < g_fNormalSpeed/2 ) 
			m_Boids[i].speed = g_fNormalSpeed/2;
		if( m_Boids[i].speed > g_fNormalSpeed*5 ) 
			m_Boids[i].speed = g_fNormalSpeed*5;

		// now figure out yaw changes
		D3DXVECTOR3 vOffset = vForce;
		vOffset.y = 0.0f;
		D3DXVECTOR3 vDelta = m_Boids[i].vDir;

		if( D3DXVec3Length( &vOffset ) > 0.0f )
			D3DXVec3Normalize( &vOffset, &vOffset );

		float	dot = D3DXVec3Dot( &vOffset, &vDelta );
		// speed up slightly if not turning much
		if (dot > 0.7f) 
		{
			dot -= 0.7f;
			m_Boids[i].speed += dot * 0.005f;
		}
		D3DXVec3Cross( &vOffset, &vOffset, &vDelta );
//		D3DXVec3Cross( &vOffset, &vDelta, &vOffset );
		dot = (1.0f-dot)/2.0f * 0.07f;
		if( vOffset.y > 0.05f ) 
		{
			m_Boids[i].dyaw = (m_Boids[i].dyaw*19.0f + dot) * 0.05f;
		} 
		else if( vOffset.y < -0.05f ) 
		{
			m_Boids[i].dyaw = (m_Boids[i].dyaw*19.0f - dot) * 0.05f;
		} 
		else
		{
			m_Boids[i].dyaw *= 0.98f;	// damp it
		}
		m_Boids[i].yaw += m_Boids[i].dyaw;
		m_Boids[i].roll = -m_Boids[i].dyaw * 20.0f;

		// Take new info and create a new world matrix
		// First translate into place, then set orientation, then scale (if needed)
		D3DXMATRIX matTrans, matRotateX, matRotateY, matRotateZ, matTemp1, matTemp2;
		D3DXMatrixTranslation( &matTrans, m_Boids[i].vPos.x, m_Boids[i].vPos.y, m_Boids[i].vPos.z );
		D3DXMatrixRotationX( &matRotateX, -m_Boids[i].pitch );
		D3DXMatrixRotationY( &matRotateY, -m_Boids[i].yaw );
		D3DXMatrixRotationZ( &matRotateZ, -m_Boids[i].roll );
		D3DXMatrixMultiply( &matTemp1, &matRotateX, &matRotateY );
		D3DXMatrixMultiply( &matTemp2, &matRotateZ, &matTemp1 );
		D3DXMatrixMultiply( &m_Boids[i].matWorld, &matTemp2, &matTrans );

		// Now extract the boid's direction out of the matrix
		m_Boids[i].vDir.x = m_Boids[i].matWorld._31;
		m_Boids[i].vDir.y = m_Boids[i].matWorld._32;
		m_Boids[i].vDir.z = m_Boids[i].matWorld._33;

		// And update the boid's location
		m_Boids[i].vPos += m_Boids[i].vDir * m_Boids[i].speed * 100 * fElapsedTime;
	}
}




