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
static time_point<steady_clock> lastCappedEndFrame;

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
	lastCappedEndFrame = steady_clock::now();
}

void UpdateTime()
{
	auto currentTime = steady_clock::now();

	// This defines a ceiling threshold (250ms) to detect a severe time discontinuity
	// in case delta time gets absurdly long (e.g. when the window halts during a resize).
	static constexpr duration<float> MAX_STEP_THRESHOLD = duration<float>(0.25f);

	// If the threshold was reached, then a 1-second discontinuity step is reported
	// for both uncapped and capped delta time to help clear the 'time debt'.
	bool hasHitched = ((currentTime - lastEndFrame) > duration_cast<steady_clock::duration>(MAX_STEP_THRESHOLD));
	
	/////////////////////////
	// Uncapped Delta Time //
	/////////////////////////
	auto frameTime = currentTime - lastEndFrame;
	lastEndFrame = currentTime;

	if (hasHitched)
	{
		// Force the uncapped time to report just 1,000ms (1 sec) for this one frame.
		// This prevents a bug where a large delta time passed into the 'msSleep' calculation
		// for the FPS throttler created a speed burst in player movement.
		deltaTime_s = 1.0f; 
		deltaTime_ms = 1000;
		msAccumulator = 0.0f;
	}
	else
	{
		// Update the uncapped delta time, both in seconds and milliseconds.
		deltaTime_s = duration<float>(frameTime).count();

		// msAccumulator keeps track of fractional time so it isn't lost between frames.
		float totalMs = (duration<float, std::milli>(frameTime).count()) + msAccumulator;
		deltaTime_ms = static_cast<int>(totalMs);
		msAccumulator = totalMs - static_cast<float>(deltaTime_ms);
	}
	///////////////////////
	// Capped Delta Time //
	///////////////////////
	auto cappedFrameTime = currentTime - lastCappedEndFrame;

	// The limit for capped delta time is roughly 10FPS, or 100ms.
	// This prevents movement and graphical effects from "spiking" if the framerate drops.
	static constexpr duration<float> DELTA_TIME_CAP = duration<float>(0.1f);
	
	if (hasHitched)
	{
		// Force the capped delta time to match the 1-sec discontinuity step for one frame.
		// This prevents moving objects from 'sliding fast' to their destination by
		// triggering a coordinate snap within MoveSingle().
		cappedDeltaTime_s = 1.0f;
		cappedDeltaTime_ms = 1000;
		cappedMsAccumulator = 0.0f;
	}
	else
	{
		// Clamp the frame time if it reaches the delta time cap.
		cappedFrameTime = std::min(cappedFrameTime, duration_cast<steady_clock::duration>(DELTA_TIME_CAP));

		// Then update the capped delta time, also in seconds and milliseconds.
		cappedDeltaTime_s = duration<float>(cappedFrameTime).count();

		float totalCappedMs = (duration<float, std::milli>(cappedFrameTime).count()) + cappedMsAccumulator;
		cappedDeltaTime_ms = static_cast<int>(totalCappedMs);
		cappedMsAccumulator = totalCappedMs - static_cast<float>(cappedDeltaTime_ms);
	}
	// Always slide the capped anchor to 'currentTime' to clear historical lag debt.
	lastCappedEndFrame = currentTime;
}
