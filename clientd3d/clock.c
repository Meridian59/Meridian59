// Meridian 59, Copyright 1994-2026 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"
#include <chrono>

///////////////
// Variables //
///////////////
using steady_clock_time_point = std::chrono::time_point<std::chrono::steady_clock>;

static steady_clock_time_point lastEndFrame;

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
// Used in legacy systems that still use milliseconds instead of seconds.
int GetDeltaTimeMs()
{
	return deltaTime_ms;
}

void InitializeTime()
{
	deltaTime_s = 0.0f;
	deltaTime_ms = 0;
	msAccumulator = 0.0f;
	
	lastEndFrame = std::chrono::steady_clock::now();
}

void UpdateTime()
{
	// Get current time first.
	auto currentTime = std::chrono::steady_clock::now();
	
	// Calculate delta time (in seconds) since last frame.
	auto frameTime = currentTime - lastEndFrame;
	deltaTime_s = std::chrono::duration<float>(frameTime).count();
	
	// Cap the max allowed delta time to roughly an equivalent to 10 FPS. This prevents things
	// like game logic or animations from "spiking" if the window halts (e.g. during a resize).
	if (deltaTime_s > 0.1f)
	{
		deltaTime_s = 0.1f;
		
		// Sync frameTime duration back down to match our safety cap
		frameTime = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(0.1f));
	}

	// Also calculate delta time in milliseconds for systems that still use it.
	// msAccumulator keeps track of fractional time so it isn't lost between frames.
	float totalMs = (std::chrono::duration<float, std::milli>(frameTime).count()) + msAccumulator;
	deltaTime_ms = static_cast<int>(totalMs);
	msAccumulator = totalMs - static_cast<float>(deltaTime_ms);
	
	lastEndFrame = currentTime;
}