// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * animate.h:  Header for animate.c
 */

#ifndef _ANIMATE_H
#define _ANIMATE_H

#define FLICKER_LEVEL (LIGHT_LEVELS / 2)

void AnimationTimerAbort(void);
void AnimationTimerStart(void);
M59EXPORT bool AnimateObject(object_node *obj, int dt);
void AnimationTimerProc(HWND hwnd, UINT timer);
void AnimateStop(Animate *a);
bool AnimateSingle(Animate *a, int num_groups, int dt);

DWORD GetFrameTime(void);

#endif /* #ifndef _ANIMATE_H */
