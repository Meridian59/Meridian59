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

	lastEndFrame = steady_clock::now();
}

void UpdateTime()
{
	// Get current time first.
	auto currentTime = steady_clock::now();

	// Calculate delta time (in seconds) since last frame.
	auto frameTime = currentTime - lastEndFrame;
	deltaTime_s = duration<float>(frameTime).count();

	// Cap the max allowed delta time to roughly an equivalent to 10 FPS. This prevents things
	// like game logic or animations from "spiking" if the window halts (e.g. during a resize).
	static constexpr duration<float> DELTA_TIME_CAP = duration<float>(0.1f);

	// Sync frameTime duration back down to match the safety cap.
	frameTime = std::min(frameTime, duration_cast<steady_clock::duration>(DELTA_TIME_CAP));
	deltaTime_s = duration<float>(frameTime).count();

	// Always slide the time window completely forward to prevent catch-up loops.
	lastEndFrame = currentTime;

	// Also calculate delta time in milliseconds for systems that still use it.
	// msAccumulator keeps track of fractional time so it isn't lost between frames.
	float totalMs = (duration<float, std::milli>(frameTime).count()) + msAccumulator;
	deltaTime_ms = static_cast<int>(totalMs);
	msAccumulator = totalMs - static_cast<float>(deltaTime_ms);
}
