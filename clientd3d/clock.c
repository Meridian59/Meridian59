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

// Elapsed time between frames but clamped around a max of 10 FPS.
// Keeps movement and graphical effects consistent during framerate drops.
static float cappedDeltaTime_s;
static int cappedDeltaTime_ms;
static float cappedMsAccumulator;

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
int GetDeltaTime_Ms()
{
	return deltaTime_ms;
}

// Returns capped delta time in seconds.
float GetCappedDeltaTime()
{
	return cappedDeltaTime_s;
}

// Returns capped delta time in milliseconds.
int GetCappedDeltaTime_Ms()
{
	return cappedDeltaTime_ms;
}

void InitializeTime()
{
	deltaTime_s = 0.0f;
	deltaTime_ms = 0;
	msAccumulator = 0.0f;

	cappedDeltaTime_s = 0.0f;
	cappedDeltaTime_ms = 0;
	cappedMsAccumulator = 0.0f;
	
	lastEndFrame = steady_clock::now();
}

void UpdateTime()
{
	// Get current time first.
	auto currentTime = steady_clock::now();
	// Calculate delta time (in seconds) since last frame.
	auto frameTime = currentTime - lastEndFrame;
	// Always slide the time window completely forward to prevent catch-up loops.
	lastEndFrame = currentTime;
	
	// First, update the uncapped delta time in seconds and milliseconds.
	deltaTime_s = duration<float>(frameTime).count();
	// msAccumulator keeps track of fractional time so it isn't lost between frames.
	float totalMs = (duration<float, std::milli>(frameTime).count()) + msAccumulator;
	deltaTime_ms = static_cast<int>(totalMs);
	msAccumulator = totalMs - static_cast<float>(deltaTime_ms);
	
	// Then calculate the capped delta time (at roughly 10FPS), also in seconds and milliseconds.
	// This prevents movement and graphical effects from "spiking" if the window halts (e.g. during a resize).
	static constexpr duration<float> DELTA_TIME_CAP = duration<float>(0.1f);
	auto cappedFrameTime = std::min(frameTime, duration_cast<steady_clock::duration>(DELTA_TIME_CAP));
	
	cappedDeltaTime_s = duration<float>(cappedFrameTime).count();
	float totalCappedMs = (duration<float, std::milli>(cappedFrameTime).count()) + cappedMsAccumulator;
	cappedDeltaTime_ms = static_cast<int>(totalCappedMs);
	cappedMsAccumulator = totalCappedMs - static_cast<float>(cappedDeltaTime_ms);
}
