// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * select.h:  Header file for select.c
 */

#ifndef _SELECT_H
#define _SELECT_H

typedef void (*SelectCallback)(ID object2);

M59EXPORT void SetSelectCallback(SelectCallback fn);

void UserSelect(ID target);
M59EXPORT void SelectedObject(ID target_id);
void SetUseObject(ID id);

#endif /* #ifndef _SELECT_H */
