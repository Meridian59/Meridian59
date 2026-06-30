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

	// If delta time gets absurdly long (e.g. the window is paused), clamp it down to the cap.
	static constexpr float DELTA_TIME_CAP = 0.1f;
	deltaTime_s = std::min(deltaTime_s, DELTA_TIME_CAP);
}
