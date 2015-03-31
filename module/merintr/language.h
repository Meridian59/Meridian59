// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * language.h:  Header file for language.c
 */

#ifndef _LANGUAGE_H
#define _LANGUAGE_H

#define ID_LANGUAGE 4500        // Windows base ID for language menu items

void LanguageInit(void);
void LanguageExit(void);

// User chose a language
void MenuLanguageChosen(int index);

#endif /* #ifndef _LANGUAGE_H */
