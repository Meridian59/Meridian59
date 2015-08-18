// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* resource.h
 * Header file for resource.c
 */

#ifndef _RESOURCE_H
#define _RESOURCE_H

#define MAX_LANGUAGE_ID 184

Bool check_for_class_resource_string(id_type id);
void write_resources(char *fname);

#endif /* #ifndef _RESOURCE_H */
