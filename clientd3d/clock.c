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
// Elapsed time between frames (in milliseconds) for legacy systems.
static int deltaTime_ms;
// Tracks fractional milliseconds to prevent drift between frames.
static float msAccumulator;

//////////////////////
// Public Functions //
//////////////////////

// Returns delta time in seconds.
float GetDeltaTime()
{
	return deltaTime_s;
}

// Returns delta time in milliseconds.
int GetDeltaTime_Ms()
{
	return deltaTime_ms;
}

void InitializeTime()
{
	deltaTime_s = 0.0f;
	deltaTime_ms = 0;
	msAccumulator = 0.0f;

	lastEndFrame = steady_clock::now();
}

void UpdateTime()
{
	auto currentTime = steady_clock::now();

	auto frameTime = currentTime - lastEndFrame;
	lastEndFrame = currentTime;

	// Update delta time in seconds.
	deltaTime_s = duration<float>(frameTime).count();

	// Also update delta time in milliseconds, and add fractional milliseconds from the last frame.
	float totalMs = (duration<float, std::milli>(frameTime).count()) + msAccumulator;
	deltaTime_ms = static_cast<int>(totalMs);
	// msAccumulator keeps track of fractional time so it isn't lost between frames.
	msAccumulator = totalMs - static_cast<float>(deltaTime_ms);
}
