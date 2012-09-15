// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * char.h:  Main header file for character selection DLL
 */

#ifndef _CHAR_H
#define _CHAR_H

#include "charrc.h"

#define MODULE_ID 103  // Unique module id #

#define BK_TABCHANGED     (WM_USER + 101)

/* New game state */
enum {GAME_PICKCHAR = 100, };

/* Structures from server giving character information */
typedef struct {
   ID    id;           // Object id of character
   char *name;         // String name
   BYTE  flags;        // Extra info about character
} Character;

#define MAX_ADVERTISEMENTS 2
typedef struct {
   char filename[MAX_PATH + 1];    // Filename of ad AVI file
   char url[MAX_PATH + 1];         // URL associated with ad
} Advertisement;

void ChooseCharacter(Character *characters, WORD num_characters, char *motd,
		     BYTE num_ads, Advertisement *ads, BOOL quickstart);
void GoToGame(ID character);
void AbortCharDialogs(void);
void CharactersDestroy(Character *c, WORD num_characters);


#define MIN_CHARNAME 3        /* Minimum length of character name */

#define NUM_GENDERS 2

#define NUM_FACE_OVERLAYS  4   // Eyes, nose, mouth, hair

#define NUM_CHAR_STATS 6

#define SPELL_POINTS_INITIAL   45   // Initial # of available spell/skill points
#define STAT_POINTS_INITIAL    50   // Initial # of available stat points

typedef struct {
   int num_eyes;
   int num_noses;
   int num_mouths;
   int num_hair;

   ID head;             // Head resource ID
   ID *eyes;            // Array of eye resource IDs
   ID *noses;           // Array of nose resource IDs
   ID *mouths;          // Array of mouth resource IDs  
   ID *hair;            // Array of hair resource IDs  
} FaceInfo;

typedef struct {
   int gender_choice;   // Index of currently selected gender        
   int eye_choice;      // Index of currently selected eye
   int nose_choice;     // Index of currently selected nose
   int mouth_choice;    // Index of currently selected mouth
   int hair_choice;     // Index of currently selected hair

   BYTE hairt_choice;   // Index of currently selected hair translation
   BYTE facet_choice;   // Index of currently selected face translation

   BYTE num_hair_translations;
   BYTE *hair_translations;  // Array of hair palette translations

   BYTE num_face_translations;
   BYTE *face_translations;  // Array of face palette translations

   FaceInfo parts[NUM_GENDERS];  // Face part graphics for each gender setting
} CharAppearance;

typedef enum {
   SS_SHALILLE = 1,
   SS_QOR = 2,
   SS_KRAANAN = 3,
   SS_FAREN = 4,
   SS_RIIJA = 5,
   SS_JALA = 6,
   SS_DM_COMMAND = 7,
} SpellSchool;

// Info on each available spell
typedef struct {
   int  id;             // Spell ID of spell (NOT object ID)
   ID   name_res;       // Name resource of spell
   ID   desc_res;       // Resource ID of spell description string
   int  cost;           // Cost of choosing spell
   Bool chosen;         // True when user has chosen spell
   BYTE school;         // School of spell
} Spell;

// Info on each available skill
typedef struct {
   int  id;             // Skill ID of skill (NOT object ID)
   ID   name_res;       // Name resource of skill
   ID   desc_res;       // Resource ID of skill description string
   int  cost;           // Cost of choosing skill
   Bool chosen;         // True when user has chosen skill
} Skill;

void MakeChar(CharAppearance *ap_init, list_type spells_init, list_type skills_init);
char *VerifyCharName(char *name);
CharAppearance *CharAppearanceDestroy(CharAppearance *ap);
void CharTabPageCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
Bool VerifySettings(void);
void CharInfoValid(void);
void CharInfoInvalid(void);

BOOL CALLBACK CharNameDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void CharNameGetChoices(char *name, char *desc);

void CharFaceInit(void);
BOOL CALLBACK CharFaceDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void CharFaceGetChoices(CharAppearance *ap, int *stats, BYTE *gender);
void CharFaceExit(void);

BOOL CALLBACK CharStatsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void CharStatsGetChoices(int *buf);
int  CharStatsGetPoints(void);

BOOL CALLBACK CharSpellsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK CharSkillsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern int spell_points;                // Number of spells/skills points left

// Sending messages to server
extern client_message msg_table[];

extern ClientInfo *cinfo;         // Holds data passed from main client
extern Bool        exiting;       // True when module is exiting and should be unloaded

extern HINSTANCE hInst;  // module handle

/* messages to server */
#define RequestCharacters()          ToServer(BP_SEND_CHARACTERS, msg_table)
#define RequestUseCharacter(id)      ToServer(BP_USE_CHARACTER, msg_table, id)
#define RequestCharInfo()            ToServer(BP_SYSTEM, msg_table, BP_SEND_CHARINFO)
#define SendNewCharInfo(obj, name, desc, g, len1, parts, t1, t2, len2, stats, spells, skills) \
ToServer(BP_SYSTEM, msg_table, BP_NEW_CHARINFO, obj, name, desc, g, len1, parts, t1, t2, len2, stats, spells, skills)
#define SendAdSelected(num)          ToServer(BP_AD_SELECTED, msg_table, num)


#endif /* #ifndef _CHAR_H */
