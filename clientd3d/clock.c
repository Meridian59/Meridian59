// Meridian 59, Copyright 1994-2026 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <chrono>

//////////////////////
// Local Namespaces //
//////////////////////
using namespace std::chrono;

///////////////
// Variables //
///////////////
static time_point<steady_clock> lastEndFrame;

// Elapsed time between frames (in seconds).
static float deltaTime_s;

//////////////////////
// Public Functions //
//////////////////////

// Returns delta time in seconds.
float GetDeltaTime()
{
	return deltaTime_s;
}

void InitializeTime()
{
	deltaTime_s = 0.0f;

	lastEndFrame = steady_clock::now();
}

void UpdateTime()
{
	auto currentTime = steady_clock::now();

	auto frameTime = currentTime - lastEndFrame;
	lastEndFrame = currentTime;

	// Update delta time in seconds.
	deltaTime_s = duration<float>(frameTime).count();
}
