// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * key.h:  Header file for key.c
 */

#ifndef _KEY_H
#define _KEY_H

/* Keys not defined by Windows, but which we need to use */
#define VK_SINGLEQUOTE	222
#define VK_LDBLCLK		5
#define VK_SEMICOLON	0xBA
#define VK_PERIOD		0xBE
#define VK_COMMA		0xBC
#define VK_SLASH		0xBF
#define VK_LBRACKET		0xDB
#define VK_BACKSLASH 0xDC
#define VK_RBRACKET		0xDD
#define VK_BACKQUOTE	0xC0

/* Modifier keys bit vector */
#define KEY_ALT    0x01
#define KEY_CTL    0x02
#define KEY_SHIFT  0x04
#define KEY_NONE   0x08
#define KEY_ANY    0x10

/* Translation table entry from key to action */
typedef struct {
   WORD  vk_code;  /* Virtual key code */ 
   WORD  flags;    /* Modifier keys */
   WORD  command;  /* Game command corresponding to key & flags */
   void *data;     /* Extra data associated with command */
} keymap, * KeyTable;

// translates ascii key name to vk code used by windows
typedef struct ascii_key
{
	char	asciiName[32];
	WORD	vkCode;
} ascii_key;

void KeyInit(void);
void KeyClose(void);
M59EXPORT void KeyAddTable(int game_state, KeyTable table);
M59EXPORT void KeyRemoveTable(int game_state, KeyTable table);

M59EXPORT int  TranslateKey(UINT vk_key, KeyTable table, void **data);
M59EXPORT void KeySetLastNorepeatTime(void);
void HandleKeys(Bool poll);
int  KeysDown(void);
void KeyUpHack(int vk);

#endif /* #ifndef _KEY_H */
