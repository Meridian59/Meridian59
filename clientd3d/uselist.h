// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * uselist.h:  Header file for uselist.c
 */

#ifndef _USELIST_H
#define _USELIST_H

void      UseListSet(list_type listusing);
void      UseListDelete(void);
list_type UseListGetObjects(list_type inv);
void      UseObject(ID obj_id);
void      UnuseObject(ID obj_id);


#endif /* #ifndef _USELIST_H */
