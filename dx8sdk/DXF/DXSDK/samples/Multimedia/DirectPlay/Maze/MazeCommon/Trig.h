//----------------------------------------------------------------------------
// File: trig.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _TRIG_H
#define _TRIG_H




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
typedef DWORD   ANGLE;

const   float   TRIG_ANGLE_SCALE = (3.1415926536f*2.0f) / 65536.0f;
#define TRIG_ANGLE_MASK 0xffff

inline float Sin( ANGLE angle ) { return float(sin(float(angle&0xffff)*TRIG_ANGLE_SCALE)); };
inline float Cos( ANGLE angle ) { return float(cos(float(angle&0xffff)*TRIG_ANGLE_SCALE)); };
inline float AngleToFloat( ANGLE angle ) { return float(angle&0xffff) * TRIG_ANGLE_SCALE; };




#endif
