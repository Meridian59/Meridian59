// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * drawdefs.h:  Basic constants for 3D drawing, defining sizes and resolutions.
 *   This file is included by draw3d.h.
 */

#ifndef _DRAWDEFS_H
#define _DRAWDEFS_H

#define NUMDEGREES 4096
#define LOG_NUMDEGREES 12
#define NUMDEGREES_MASK 0xfff

#define PI      3.1415926f
#define PITWICE 6.2831853f
#define PIHALF  1.5707963f

/* Turning MACROs */
#define TURN_LEFT(angle) ((angle + 3 * NUMDEGREES / 4) % NUMDEGREES)
#define TURN_RIGHT(angle) ((angle + NUMDEGREES / 4) % NUMDEGREES)
#define TURN_BACK(angle) ((angle + NUMDEGREES / 2) % NUMDEGREES)

/* Original game clients view port size -- must be DWORD aligned for WinG */
#define CLASSIC_WIDTH 452
#define CLASSIC_HEIGHT 276

/* Scaler used to increase the classic view port x,y values */
#define VIEW_SCALER 5

/* Maximum values used to create view textures and buffers */
#define MAXX (CLASSIC_WIDTH * VIEW_SCALER)
#define MAXY (CLASSIC_HEIGHT * VIEW_SCALER)
#define MAXYX_ASPECT_RATIO 1.68
#define MAXXY_ASPECT_RATIO (1/MAXYX_ASPECT_RATIO)

/* Fineness of offset within a grid square. */
#define FINENESS     1024L       /* Keep track of rays to within 1/FINENESS */
#define LOG_FINENESS 10L         /* Discrete log of FINENESS */
#define FINE_MASK    0xfffffc00

/* Convert from fineness to (square, offset) and vice versa */
#define FineToSquare(x) ((x) >> LOG_FINENESS)
#define FineToOffset(x) ((x) & ~FINE_MASK)
#define SquareToFine(s, o) (((int)(s)) << LOG_FINENESS | (o))

/* Convert from kod (square, fineness) to client fineness */
#define KOD_FINENESS     64           // Kod has 64 units per grid square
#define LOG_KOD_FINENESS 6
#define SquareToFineKod(s, o) \
(((int)(s)) << LOG_FINENESS | ((o) << (LOG_FINENESS - LOG_KOD_FINENESS)))
#define FinenessKodToClient(f) (((int) (f)) << (LOG_FINENESS - LOG_KOD_FINENESS))
#define FinenessClientToKod(f) ((f) >> (LOG_FINENESS - LOG_KOD_FINENESS))

// Convert from room file/kod height to fineness height
#define HeightKodToClient(f) (((int) (f)) << (LOG_FINENESS - LOG_KOD_FINENESS))

#define VIEWER_DISTANCE  (FINENESS >> 1)         /* Distance from viewer to screen in pixels */
#define LOG_VIEWER_DISTANCE  (LOG_FINENESS - 1)

/* Convert from pseudo degrees to radians */
#define DegToRad(x) ((float) (x) * PITWICE / NUMDEGREES)
#define RadToDeg(x) ((long)  ((x) * NUMDEGREES / PITWICE))

#define NUM_COLORS 256
#define MAX_LIGHT 256  
#define LIGHT_LEVELS 64
#define KOD_LIGHT_LEVELS 256

/* Palette constants */
#define NUM_PALETTES (LIGHT_LEVELS + 1)
#define PALETTE_INVERT LIGHT_LEVELS    // Index of inverted (nuclear-effect) palette

#endif /* #ifdef _DRAWDEFS_H */
