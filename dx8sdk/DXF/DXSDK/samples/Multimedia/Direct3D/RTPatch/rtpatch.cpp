//-----------------------------------------------------------------------------
// File: RTPatch.cpp
//
// Desc: Example code showing how to use patches in D3D.
//
// Copyright (c) 1995-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFile.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "resource.h"



//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
// Utah Teapot Bezier Patch Data
const float teapotData[] = 
{
    // xBody_Back
    -80.00f,  0.00f, 30.00f,     -80.00f,-44.80f, 30.00f,     -44.80f,-80.00f, 30.00f,       0.00f,-80.00f, 30.00f,
    -80.00f,  0.00f, 12.00f,     -80.00f,-44.80f, 12.00f,     -44.80f,-80.00f, 12.00f,       0.00f,-80.00f, 12.00f,
    -60.00f,  0.00f,  3.00f,     -60.00f,-33.60f,  3.00f,     -33.60f,-60.00f,  3.00f,       0.00f,-60.00f,  3.00f,
    -60.00f,  0.00f,  0.00f,     -60.00f,-33.60f,  0.00f,     -33.60f,-60.00f,  0.00f,       0.00f,-60.00f,  0.00f,

      0.00f,-80.00f, 30.00f,      44.80f,-80.00f, 30.00f,      80.00f,-44.80f, 30.00f,      80.00f,  0.00f, 30.00f,
      0.00f,-80.00f, 12.00f,      44.80f,-80.00f, 12.00f,      80.00f,-44.80f, 12.00f,      80.00f,  0.00f, 12.00f,
      0.00f,-60.00f,  3.00f,      33.60f,-60.00f,  3.00f,      60.00f,-33.60f,  3.00f,      60.00f,  0.00f,  3.00f,
      0.00f,-60.00f,  0.00f,      33.60f,-60.00f,  0.00f,      60.00f,-33.60f,  0.00f,      60.00f,  0.00f,  0.00f,

    -60.00f,  0.00f, 90.00f,     -60.00f,-33.60f, 90.00f,     -33.60f,-60.00f, 90.00f,       0.00f,-60.00f, 90.00f,
    -70.00f,  0.00f, 69.00f,     -70.00f,-39.20f, 69.00f,     -39.20f,-70.00f, 69.00f,       0.00f,-70.00f, 69.00f,
    -80.00f,  0.00f, 48.00f,     -80.00f,-44.80f, 48.00f,     -44.80f,-80.00f, 48.00f,       0.00f,-80.00f, 48.00f,
    -80.00f,  0.00f, 30.00f,     -80.00f,-44.80f, 30.00f,     -44.80f,-80.00f, 30.00f,       0.00f,-80.00f, 30.00f,

      0.00f,-60.00f, 90.00f,      33.60f,-60.00f, 90.00f,      60.00f,-33.60f, 90.00f,      60.00f,  0.00f, 90.00f,
      0.00f,-70.00f, 69.00f,      39.20f,-70.00f, 69.00f,      70.00f,-39.20f, 69.00f,      70.00f,  0.00f, 69.00f,
      0.00f,-80.00f, 48.00f,      44.80f,-80.00f, 48.00f,      80.00f,-44.80f, 48.00f,      80.00f,  0.00f, 48.00f,
      0.00f,-80.00f, 30.00f,      44.80f,-80.00f, 30.00f,      80.00f,-44.80f, 30.00f,      80.00f,  0.00f, 30.00f,

    -56.00f,  0.00f, 90.00f,     -56.00f,-31.36f, 90.00f,     -31.36f,-56.00f, 90.00f,       0.00f,-56.00f, 90.00f,
    -53.50f,  0.00f, 95.25f,     -53.50f,-29.96f, 95.25f,     -29.96f,-53.50f, 95.25f,       0.00f,-53.50f, 95.25f,
    -57.50f,  0.00f, 95.25f,     -57.50f,-32.20f, 95.25f,     -32.20f,-57.50f, 95.25f,       0.00f,-57.50f, 95.25f,
    -60.00f,  0.00f, 90.00f,     -60.00f,-33.60f, 90.00f,     -33.60f,-60.00f, 90.00f,       0.00f,-60.00f, 90.00f,

      0.00f,-56.00f, 90.00f,      31.36f,-56.00f, 90.00f,      56.00f,-31.36f, 90.00f,      56.00f,  0.00f, 90.00f,
      0.00f,-53.50f, 95.25f,      29.96f,-53.50f, 95.25f,      53.50f,-29.96f, 95.25f,      53.50f,  0.00f, 95.25f,
      0.00f,-57.50f, 95.25f,      32.20f,-57.50f, 95.25f,      57.50f,-32.20f, 95.25f,      57.50f,  0.00f, 95.25f,
      0.00f,-60.00f, 90.00f,      33.60f,-60.00f, 90.00f,      60.00f,-33.60f, 90.00f,      60.00f,  0.00f, 90.00f,

      // xBody_Front
     80.00f,  0.00f, 30.00f,      80.00f, 44.80f, 30.00f,      44.80f, 80.00f, 30.00f,       0.00f, 80.00f, 30.00f,
     80.00f,  0.00f, 12.00f,      80.00f, 44.80f, 12.00f,      44.80f, 80.00f, 12.00f,       0.00f, 80.00f, 12.00f,
     60.00f,  0.00f,  3.00f,      60.00f, 33.60f,  3.00f,      33.60f, 60.00f,  3.00f,       0.00f, 60.00f,  3.00f,
     60.00f,  0.00f,  0.00f,      60.00f, 33.60f,  0.00f,      33.60f, 60.00f,  0.00f,       0.00f, 60.00f,  0.00f,

      0.00f, 80.00f, 30.00f,     -44.80f, 80.00f, 30.00f,     -80.00f, 44.80f, 30.00f,     -80.00f,  0.00f, 30.00f,
      0.00f, 80.00f, 12.00f,     -44.80f, 80.00f, 12.00f,     -80.00f, 44.80f, 12.00f,     -80.00f,  0.00f, 12.00f,
      0.00f, 60.00f,  3.00f,     -33.60f, 60.00f,  3.00f,     -60.00f, 33.60f,  3.00f,     -60.00f,  0.00f,  3.00f,
      0.00f, 60.00f,  0.00f,     -33.60f, 60.00f,  0.00f,     -60.00f, 33.60f,  0.00f,     -60.00f,  0.00f,  0.00f,

     60.00f,  0.00f, 90.00f,      60.00f, 33.60f, 90.00f,      33.60f, 60.00f, 90.00f,       0.00f, 60.00f, 90.00f,
     70.00f,  0.00f, 69.00f,      70.00f, 39.20f, 69.00f,      39.20f, 70.00f, 69.00f,       0.00f, 70.00f, 69.00f,
     80.00f,  0.00f, 48.00f,      80.00f, 44.80f, 48.00f,      44.80f, 80.00f, 48.00f,       0.00f, 80.00f, 48.00f,
     80.00f,  0.00f, 30.00f,      80.00f, 44.80f, 30.00f,      44.80f, 80.00f, 30.00f,       0.00f, 80.00f, 30.00f,

      0.00f, 60.00f, 90.00f,     -33.60f, 60.00f, 90.00f,     -60.00f, 33.60f, 90.00f,     -60.00f,  0.00f, 90.00f,
      0.00f, 70.00f, 69.00f,     -39.20f, 70.00f, 69.00f,     -70.00f, 39.20f, 69.00f,     -70.00f,  0.00f, 69.00f,
      0.00f, 80.00f, 48.00f,     -44.80f, 80.00f, 48.00f,     -80.00f, 44.80f, 48.00f,     -80.00f,  0.00f, 48.00f,
      0.00f, 80.00f, 30.00f,     -44.80f, 80.00f, 30.00f,     -80.00f, 44.80f, 30.00f,     -80.00f,  0.00f, 30.00f,

     56.00f,  0.00f, 90.00f,      56.00f, 31.36f, 90.00f,      31.36f, 56.00f, 90.00f,       0.00f, 56.00f, 90.00f,
     53.50f,  0.00f, 95.25f,      53.50f, 29.96f, 95.25f,      29.96f, 53.50f, 95.25f,       0.00f, 53.50f, 95.25f,
     57.50f,  0.00f, 95.25f,      57.50f, 32.20f, 95.25f,      32.20f, 57.50f, 95.25f,       0.00f, 57.50f, 95.25f,
     60.00f,  0.00f, 90.00f,      60.00f, 33.60f, 90.00f,      33.60f, 60.00f, 90.00f,       0.00f, 60.00f, 90.00f,

      0.00f, 56.00f, 90.00f,     -31.36f, 56.00f, 90.00f,     -56.00f, 31.36f, 90.00f,     -56.00f,  0.00f, 90.00f,
      0.00f, 53.50f, 95.25f,     -29.96f, 53.50f, 95.25f,     -53.50f, 29.96f, 95.25f,     -53.50f,  0.00f, 95.25f,
      0.00f, 57.50f, 95.25f,     -32.20f, 57.50f, 95.25f,     -57.50f, 32.20f, 95.25f,     -57.50f,  0.00f, 95.25f,
      0.00f, 60.00f, 90.00f,     -33.60f, 60.00f, 90.00f,     -60.00f, 33.60f, 90.00f,     -60.00f,  0.00f, 90.00f,

    // Handle
    -64.00f,  0.00f, 75.00f,     -64.00f, 12.00f, 75.00f,     -60.00f, 12.00f, 84.00f,     -60.00f,  0.00f, 84.00f,
    -92.00f,  0.00f, 75.00f,     -92.00f, 12.00f, 75.00f,    -100.00f, 12.00f, 84.00f,    -100.00f,  0.00f, 84.00f,
   -108.00f,  0.00f, 75.00f,    -108.00f, 12.00f, 75.00f,    -120.00f, 12.00f, 84.00f,    -120.00f,  0.00f, 84.00f,
   -108.00f,  0.00f, 66.00f,    -108.00f, 12.00f, 66.00f,    -120.00f, 12.00f, 66.00f,    -120.00f,  0.00f, 66.00f,

    -60.00f,  0.00f, 84.00f,     -60.00f,-12.00f, 84.00f,     -64.00f,-12.00f, 75.00f,     -64.00f,  0.00f, 75.00f,
   -100.00f,  0.00f, 84.00f,    -100.00f,-12.00f, 84.00f,     -92.00f,-12.00f, 75.00f,     -92.00f,  0.00f, 75.00f,
   -120.00f,  0.00f, 84.00f,    -120.00f,-12.00f, 84.00f,    -108.00f,-12.00f, 75.00f,    -108.00f,  0.00f, 75.00f,
   -120.00f,  0.00f, 66.00f,    -120.00f,-12.00f, 66.00f,    -108.00f,-12.00f, 66.00f,    -108.00f,  0.00f, 66.00f,

   -108.00f,  0.00f, 66.00f,    -108.00f, 12.00f, 66.00f,    -120.00f, 12.00f, 66.00f,    -120.00f,  0.00f, 66.00f,
   -108.00f,  0.00f, 57.00f,    -108.00f, 12.00f, 57.00f,    -120.00f, 12.00f, 48.00f,    -120.00f,  0.00f, 48.00f,
   -100.00f,  0.00f, 39.00f,    -100.00f, 12.00f, 39.00f,    -106.00f, 12.00f, 31.50f,    -106.00f,  0.00f, 31.50f,
    -80.00f,  0.00f, 30.00f,     -80.00f, 12.00f, 30.00f,     -76.00f, 12.00f, 18.00f,     -76.00f,  0.00f, 18.00f,

   -120.00f,  0.00f, 66.00f,    -120.00f,-12.00f, 66.00f,    -108.00f,-12.00f, 66.00f,    -108.00f,  0.00f, 66.00f,
   -120.00f,  0.00f, 48.00f,    -120.00f,-12.00f, 48.00f,    -108.00f,-12.00f, 57.00f,    -108.00f,  0.00f, 57.00f,
   -106.00f,  0.00f, 31.50f,    -106.00f,-12.00f, 31.50f,    -100.00f,-12.00f, 39.00f,    -100.00f,  0.00f, 39.00f,
    -76.00f,  0.00f, 18.00f,     -76.00f,-12.00f, 18.00f,     -80.00f,-12.00f, 30.00f,     -80.00f,  0.00f, 30.00f,

    // Spout
     68.00f,  0.00f, 51.00f,      68.00f, 26.40f, 51.00f,      68.00f, 26.40f, 18.00f,      68.00f,  0.00f, 18.00f,
    104.00f,  0.00f, 51.00f,     104.00f, 26.40f, 51.00f,     124.00f, 26.40f, 27.00f,     124.00f,  0.00f, 27.00f,
     92.00f,  0.00f, 78.00f,      92.00f, 10.00f, 78.00f,      96.00f, 10.00f, 75.00f,      96.00f,  0.00f, 75.00f,
    108.00f,  0.00f, 90.00f,     108.00f, 10.00f, 90.00f,     132.00f, 10.00f, 90.00f,     132.00f,  0.00f, 90.00f,

     68.00f,  0.00f, 18.00f,      68.00f,-26.40f, 18.00f,      68.00f,-26.40f, 51.00f,      68.00f,  0.00f, 51.00f,
    124.00f,  0.00f, 27.00f,     124.00f,-26.40f, 27.00f,     104.00f,-26.40f, 51.00f,     104.00f,  0.00f, 51.00f,
     96.00f,  0.00f, 75.00f,      96.00f,-10.00f, 75.00f,      92.00f,-10.00f, 78.00f,      92.00f,  0.00f, 78.00f,
    132.00f,  0.00f, 90.00f,     132.00f,-10.00f, 90.00f,     108.00f,-10.00f, 90.00f,     108.00f,  0.00f, 90.00f,

    108.00f,  0.00f, 90.00f,     108.00f, 10.00f, 90.00f,     132.00f, 10.00f, 90.00f,     132.00f,  0.00f, 90.00f,
    112.00f,  0.00f, 93.00f,     112.00f, 10.00f, 93.00f,     141.00f, 10.00f, 93.75f,     141.00f,  0.00f, 93.75f,
    116.00f,  0.00f, 93.00f,     116.00f,  6.00f, 93.00f,     138.00f,  6.00f, 94.50f,     138.00f,  0.00f, 94.50f,
    112.00f,  0.00f, 90.00f,     112.00f,  6.00f, 90.00f,     128.00f,  6.00f, 90.00f,     128.00f,  0.00f, 90.00f,

    132.00f,  0.00f, 90.00f,     132.00f,-10.00f, 90.00f,     108.00f,-10.00f, 90.00f,     108.00f,  0.00f, 90.00f,
    141.00f,  0.00f, 93.75f,     141.00f,-10.00f, 93.75f,     112.00f,-10.00f, 93.00f,     112.00f,  0.00f, 93.00f,
    138.00f,  0.00f, 94.50f,     138.00f, -6.00f, 94.50f,     116.00f, -6.00f, 93.00f,     116.00f,  0.00f, 93.00f,
    128.00f,  0.00f, 90.00f,     128.00f, -6.00f, 90.00f,     112.00f, -6.00f, 90.00f,     112.00f,  0.00f, 90.00f,

    // Lip
     50.00f,  0.00f, 90.00f,      50.00f, 28.00f, 90.00f,      28.00f, 50.00f, 90.00f,       0.00f, 50.00f, 90.00f,
     52.00f,  0.00f, 90.00f,      52.00f, 29.12f, 90.00f,      29.12f, 52.00f, 90.00f,       0.00f, 52.00f, 90.00f,
     54.00f,  0.00f, 90.00f,      54.00f, 30.24f, 90.00f,      30.24f, 54.00f, 90.00f,       0.00f, 54.00f, 90.00f,
     56.00f,  0.00f, 90.00f,      56.00f, 31.36f, 90.00f,      31.36f, 56.00f, 90.00f,       0.00f, 56.00f, 90.00f,

      0.00f, 50.00f, 90.00f,     -28.00f, 50.00f, 90.00f,     -50.00f, 28.00f, 90.00f,     -50.00f,  0.00f, 90.00f,
      0.00f, 52.00f, 90.00f,     -29.12f, 52.00f, 90.00f,     -52.00f, 29.12f, 90.00f,     -52.00f,  0.00f, 90.00f,
      0.00f, 54.00f, 90.00f,     -30.24f, 54.00f, 90.00f,     -54.00f, 30.24f, 90.00f,     -54.00f,  0.00f, 90.00f,
      0.00f, 56.00f, 90.00f,     -31.36f, 56.00f, 90.00f,     -56.00f, 31.36f, 90.00f,     -56.00f,  0.00f, 90.00f,

    -50.00f,  0.00f, 90.00f,     -50.00f,-28.00f, 90.00f,     -28.00f,-50.00f, 90.00f,       0.00f,-50.00f, 90.00f,
    -52.00f,  0.00f, 90.00f,     -52.00f,-29.12f, 90.00f,     -29.12f,-52.00f, 90.00f,       0.00f,-52.00f, 90.00f,
    -54.00f,  0.00f, 90.00f,     -54.00f,-30.24f, 90.00f,     -30.24f,-54.00f, 90.00f,       0.00f,-54.00f, 90.00f,
    -56.00f,  0.00f, 90.00f,     -56.00f,-31.36f, 90.00f,     -31.36f,-56.00f, 90.00f,       0.00f,-56.00f, 90.00f,

      0.00f,-50.00f, 90.00f,      28.00f,-50.00f, 90.00f,      50.00f,-28.00f, 90.00f,      50.00f,  0.00f, 90.00f,
      0.00f,-52.00f, 90.00f,      29.12f,-52.00f, 90.00f,      52.00f,-29.12f, 90.00f,      52.00f,  0.00f, 90.00f,
      0.00f,-54.00f, 90.00f,      30.24f,-54.00f, 90.00f,      54.00f,-30.24f, 90.00f,      54.00f,  0.00f, 90.00f,
      0.00f,-56.00f, 90.00f,      31.36f,-56.00f, 90.00f,      56.00f,-31.36f, 90.00f,      56.00f,  0.00f, 90.00f,

    // Lid
      8.00f,  0.00f,102.00f,       8.00f,  4.48f,102.00f,       4.48f,  8.00f,102.00f,       0.00f,  8.00f,102.00f,
     16.00f,  0.00f, 96.00f,      16.00f,  8.96f, 96.00f,       8.96f, 16.00f, 96.00f,       0.00f, 16.00f, 96.00f,
     52.00f,  0.00f, 96.00f,      52.00f, 29.12f, 96.00f,      29.12f, 52.00f, 96.00f,       0.00f, 52.00f, 96.00f,
     52.00f,  0.00f, 90.00f,      52.00f, 29.12f, 90.00f,      29.12f, 52.00f, 90.00f,       0.00f, 52.00f, 90.00f,

      0.00f,  8.00f,102.00f,      -4.48f,  8.00f,102.00f,      -8.00f,  4.48f,102.00f,      -8.00f,  0.00f,102.00f,
      0.00f, 16.00f, 96.00f,      -8.96f, 16.00f, 96.00f,     -16.00f,  8.96f, 96.00f,     -16.00f,  0.00f, 96.00f,
      0.00f, 52.00f, 96.00f,     -29.12f, 52.00f, 96.00f,     -52.00f, 29.12f, 96.00f,     -52.00f,  0.00f, 96.00f,
      0.00f, 52.00f, 90.00f,     -29.12f, 52.00f, 90.00f,     -52.00f, 29.12f, 90.00f,     -52.00f,  0.00f, 90.00f,

     -8.00f,  0.00f,102.00f,      -8.00f, -4.48f,102.00f,      -4.48f, -8.00f,102.00f,       0.00f, -8.00f,102.00f,
    -16.00f,  0.00f, 96.00f,     -16.00f, -8.96f, 96.00f,      -8.96f,-16.00f, 96.00f,       0.00f,-16.00f, 96.00f,
    -52.00f,  0.00f, 96.00f,     -52.00f,-29.12f, 96.00f,     -29.12f,-52.00f, 96.00f,       0.00f,-52.00f, 96.00f,
    -52.00f,  0.00f, 90.00f,     -52.00f,-29.12f, 90.00f,     -29.12f,-52.00f, 90.00f,       0.00f,-52.00f, 90.00f,

      0.00f, -8.00f,102.00f,       4.48f, -8.00f,102.00f,       8.00f, -4.48f,102.00f,       8.00f,  0.00f,102.00f,
      0.00f,-16.00f, 96.00f,       8.96f,-16.00f, 96.00f,      16.00f, -8.96f, 96.00f,      16.00f,  0.00f, 96.00f,
      0.00f,-52.00f, 96.00f,      29.12f,-52.00f, 96.00f,      52.00f,-29.12f, 96.00f,      52.00f,  0.00f, 96.00f,
      0.00f,-52.00f, 90.00f,      29.12f,-52.00f, 90.00f,      52.00f,-29.12f, 90.00f,      52.00f,  0.00f, 90.00f,

    // Knob
      0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,
     32.00f,  0.00f,120.00f,      32.00f, 18.00f,120.00f,      18.00f, 32.00f,120.00f,       0.00f, 32.00f,120.00f,
      0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,
      8.00f,  0.00f,102.00f,       8.00f,  4.48f,102.00f,       4.48f,  8.00f,102.00f,       0.00f,  8.00f,102.00f,

      0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,
      0.00f, 32.00f,120.00f,     -18.00f, 32.00f,120.00f,     -32.00f, 18.00f,120.00f,     -32.00f,  0.00f,120.00f,
      0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,
      0.00f,  8.00f,102.00f,      -4.48f,  8.00f,102.00f,      -8.00f,  4.48f,102.00f,      -8.00f,  0.00f,102.00f,

      0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,
    -32.00f,  0.00f,120.00f,     -32.00f,-18.00f,120.00f,     -18.00f,-32.00f,120.00f,       0.00f,-32.00f,120.00f,
      0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,
     -8.00f,  0.00f,102.00f,      -8.00f, -4.48f,102.00f,      -4.48f, -8.00f,102.00f,       0.00f, -8.00f,102.00f,

      0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,       0.00f,  0.00f,120.00f,
      0.00f,-32.00f,120.00f,      18.00f,-32.00f,120.00f,      32.00f,-18.00f,120.00f,      32.00f,  0.00f,120.00f,
      0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,       0.00f,  0.00f,108.00f,
      0.00f, -8.00f,102.00f,       4.48f, -8.00f,102.00f,       8.00f, -4.48f,102.00f,       8.00f,  0.00f,102.00f,

      // Bottom. Not part of original data set.
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f, 60.00f,  0.00f,      33.60f, 60.00f,  0.00f,      60.00f, 33.60f,  0.00f,      60.00f,  0.00f,  0.00f, 

      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,-60.00f,  0.00f,     -33.60f,-60.00f,  0.00f,     -60.00f,-33.60f,  0.00f,     -60.00f,  0.00f,  0.00f, 

      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
     60.00f,  0.00f,  0.00f,      60.00f,-33.60f,  0.00f,      33.60f,-60.00f,  0.00f,       0.00f,-60.00f,  0.00f, 

      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
      0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,       0.00f,  0.00f,  0.00f,
    -60.00f,  0.00f,  0.00f,     -60.00f, 33.60f,  0.00f,     -33.60f, 60.00f,  0.00f,       0.00f, 60.00f,  0.00f, 

};

inline float TeapotData(UINT patch, UINT u, UINT v, UINT coordIndex) 
{
    return teapotData[patch * 16 * 3 + v * 4 * 3 + u * 3 + coordIndex];
}


// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z; // The original position of the vertex
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ )

// Surface handles have to be > 0, but can be assigned arbitrarily.
const UINT SURFACEHANDLEBASE = 1;




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*               m_pFont;          // Font for drawing text
    LPDIRECT3DVERTEXBUFFER8 m_pVB;            // Buffer to hold vertices
    DWORD                   m_hVShader;       // Handle to vertex shader
    CD3DArcBall             m_ArcBall;        // Mouse rotation utility

    UINT                    m_numSegments;      // Amount to tessellate
    BOOL                    m_bRetessellate;  // Need to retessellate patches
    UINT                    m_numPatches;
    BOOL                    m_bShowHelp;
    BOOL                    m_bWireframe;

    HRESULT ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();

public:
    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    CMyD3DApplication();
};




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    CMyD3DApplication d3dApp;

    if( FAILED( d3dApp.Create( hInst ) ) )
        return 0;
    return d3dApp.Run();
}




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_strWindowTitle    = _T("RTPatch: D3D Patch Example");
    m_bUseDepthBuffer   = TRUE;
    m_bShowCursorWhenFullscreen = TRUE;

    m_pFont             = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pVB               = NULL;
    m_hVShader          = 0;
    m_bRetessellate     = TRUE;
    m_numSegments       = 4;
    m_bShowHelp         = FALSE;
    m_bWireframe        = FALSE;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // Set cursor to indicate that user can move the object with the mouse
#ifdef _WIN64
    SetClassLongPtr( m_hWnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor( NULL, IDC_SIZEALL ) );
#else
    SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)LoadCursor( NULL, IDC_SIZEALL ) );
#endif
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // Set up viewing postion from ArcBall
    m_pd3dDevice->SetTransform( D3DTS_WORLD, m_ArcBall.GetRotationMatrix() );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    // Clear the viewport
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
                         0x000000ff, 1.0f, 0L );

    if( m_bWireframe )
        m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
    else
        m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        m_pd3dDevice->SetVertexShader( m_hVShader );
        m_pd3dDevice->SetStreamSource( 0, m_pVB, sizeof(CUSTOMVERTEX) );

        for( UINT i = 0; i < m_numPatches; i++ ) 
        {
            float numSegs[4];
            numSegs[0] = (FLOAT)m_numSegments;
            numSegs[1] = (FLOAT)m_numSegments;
            numSegs[2] = (FLOAT)m_numSegments;
            numSegs[3] = (FLOAT)m_numSegments;
            if ( m_bRetessellate ) 
            {
                // First time through, tesellate
                D3DRECTPATCH_INFO info;
                info.StartVertexOffsetWidth = 0;
                info.StartVertexOffsetHeight = i * 4;
                info.Width = 4;
                info.Height = 4;
                info.Stride = 4; // verticies to next row of verticies
                info.Basis = D3DBASIS_BEZIER;
                info.Order = D3DORDER_CUBIC;
                m_pd3dDevice->DrawRectPatch(SURFACEHANDLEBASE + i, numSegs, &info);
            }
            else 
            {
                // After the first time, use the existing surface handles
                m_pd3dDevice->DrawRectPatch(SURFACEHANDLEBASE + i, numSegs, 0);
            }
        }

        m_bRetessellate = FALSE;
        
        // Output statistics
        m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
        m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );
        TCHAR szSegments[150];
        wsprintf(szSegments, TEXT("%d segments per patch"), m_numSegments);
        m_pFont->DrawText( 2, 40, D3DCOLOR_ARGB(255,255,255,255), szSegments );

        if( m_bShowHelp )
        {
            m_pFont->DrawText( 2, 60, D3DCOLOR_ARGB(255,255,255,255), 
                TEXT("Up/Down arrows: change number of segments") );
            m_pFont->DrawText( 2, 80, D3DCOLOR_ARGB(255,255,255,255), 
                TEXT("W: toggle wireframe") );
        }
        else
        {
            m_pFont->DrawText( 2, 60, D3DCOLOR_ARGB(255,255,255,255), 
                TEXT("Press F1 for Help") );
        }

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: This creates all device-dependent managed objects, such as managed
//       textures and managed vertex buffers.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    // Initialize the font's internal textures
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    // Create a vertex buffer and fill it with our vertices.
    UINT numVerts = sizeof(teapotData) / (3 * sizeof(float));
    m_numPatches = numVerts / (4 * 4);

    if( FAILED( m_pd3dDevice->CreateVertexBuffer( numVerts*sizeof(CUSTOMVERTEX),
        D3DUSAGE_RTPATCHES, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &m_pVB ) ) )
    {
        return E_FAIL;
    }

    VOID* pVertices;
    if( FAILED( m_pVB->Lock( 0, numVerts*sizeof(CUSTOMVERTEX), (BYTE**)&pVertices, 0 ) ) )
        return E_FAIL;
    CUSTOMVERTEX* pVBase = (CUSTOMVERTEX*) pVertices;
    for( UINT patch = 0; patch < m_numPatches; patch++ ) 
    {
        for( UINT v = 0; v < 4; v++ ) 
        {
            for ( UINT u = 0; u < 4; u++ ) 
            {
                CUSTOMVERTEX* pV = pVBase + (patch * 16 + u + v * 4);
                pV->x = TeapotData(patch, u, v, 0);
                pV->y = TeapotData(patch, u, v, 1);
                pV->z = -TeapotData(patch, u, v, 2);    // Convert handedness of coordinate system
            }
        }
    }
    m_pVB->Unlock();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Restore device-memory objects and state after a device is created or
//       resized.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    HRESULT hr;

    m_pFont->RestoreDeviceObjects();


    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. 
    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( 0.0f, 3.0f,-400.0f ), 
                                  &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ), 
                                  &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // Set up world matrix
    D3DXMATRIX matWorld;
    D3DXMatrixIdentity( &matWorld );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    D3DXMATRIX matProj;
    FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 800.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,     TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );

    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00202020 );

    // Set up a material. The material here just has the diffuse and ambient
    // colors set to white.
    D3DMATERIAL8 mtrl;
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL8) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    m_pd3dDevice->SetMaterial( &mtrl );

    // Set up a light
    D3DLIGHT8 light;
    light.Type         = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r    = light.Diffuse.g  = light.Diffuse.b  = 1.0f;
    light.Specular.r   = light.Specular.g = light.Specular.b = 0.0f;
    light.Ambient.r    = light.Ambient.g  = light.Ambient.b  = 0.3f;
    light.Position     = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &D3DXVECTOR3( 0.0f, 1.0f, 1.0f ) );
    light.Attenuation0 = light.Attenuation1 = light.Attenuation2 = 0.0f;
    light.Range        = sqrtf(FLT_MAX);
    m_pd3dDevice->SetLight(0, &light );
    m_pd3dDevice->LightEnable(0, TRUE );

    m_ArcBall.SetWindow( m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height, 1.0f );
    m_ArcBall.SetRadius( 1.0f );

    DWORD decl[] =
    {
       /* Vertex Data */
        D3DVSD_STREAM(0),
        D3DVSD_REG( D3DVSDE_POSITION,  D3DVSDT_FLOAT3),

        /* Data generation section implemented as a virtual stream */
        D3DVSD_STREAM_TESS(),

        /* Generate normal _using_ the position input and copy it to the normal register (output) */
        D3DVSD_TESSNORMAL( D3DVSDE_POSITION, D3DVSDE_NORMAL ),

        D3DVSD_END()
    };

    hr = m_pd3dDevice->CreateVertexShader( decl, NULL, &m_hVShader, 0 );
    if( FAILED( hr ) )
        return hr;

    m_bRetessellate = TRUE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Called when the device-dependent objects are about to be lost.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();

    for( UINT i = 0; i < m_numPatches; i++ ) 
        m_pd3dDevice->DeletePatch(SURFACEHANDLEBASE + i);
    
    if( m_hVShader != 0 )
        m_pd3dDevice->DeleteVertexShader(m_hVShader);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    m_pFont->DeleteDeviceObjects();
    SAFE_RELEASE( m_pVB );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    SAFE_DELETE( m_pFont );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
    if( pCaps->DevCaps & D3DDEVCAPS_RTPATCHES )
        return S_OK;
    else
        return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Message proc function to handle key and menu input
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam )
{
    // Pass mouse messages to the ArcBall so it can build internal matrices
    m_ArcBall.HandleMouseMessages( hWnd, uMsg, wParam, lParam );

    if( uMsg == WM_COMMAND)
    {
        switch( LOWORD(wParam) )
        {
        case IDM_MORESEGMENTS:
            m_numSegments++;
            break;

        case IDM_LESSSEGMENTS:
            if( m_numSegments > 1 )
            m_numSegments--;
            break;

        case IDM_TOGGLEHELP:
            m_bShowHelp = !m_bShowHelp;
            break;

        case IDM_TOGGLEWIREFRAME:
            m_bWireframe = !m_bWireframe;
            break;
        }
    }

    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}
