// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * idlist.h:  Header file for idlist.c
 */

#ifndef _IDLIST_H
#define _IDLIST_H

typedef list_type IDList;

M59EXPORT IDList IDListDelete(IDList id_list);
M59EXPORT Bool IDListMember(IDList id_list, ID name_res);
M59EXPORT IDList IDListAdd(IDList id_list, ID name_res);
M59EXPORT IDList IDListRemove(IDList id_list, ID name_res);

#endif /* #ifndef _IDLIST_H */
