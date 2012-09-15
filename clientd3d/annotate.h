// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * annotate.h:  Header file for annotate.c
 */

#ifndef _ANNOTATE_H
#define _ANNOTATE_H

#define MAP_ANNOTATION_SIZE     (2 * FINENESS)    // Size of annotation indicator in FINENESS units
#define MAP_ANNOTATION_MIN_SIZE 14                // Min size of drawn annotation in pixels

#define MAX_ANNOTATIONS    20
#define MAX_ANNOTATION_LEN 100

typedef struct {
  int x, y;               // Coordinates in room for map annotation
  char text[MAX_ANNOTATION_LEN];
} MapAnnotation;

void MapAnnotationsInitialize(void);
void MapAnnotationGetText(TOOLTIPTEXT *ttt);
void MapMoveAnnotations( MapAnnotation *annotations, int x, int y, float scale, Bool bMiniMap );
void MapAnnotationClick(int x, int y);
void AbortAnnotateDialog(void);

#endif /* #ifndef _ANNOTATE_H */

