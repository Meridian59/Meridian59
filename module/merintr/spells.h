// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * spells.h:  Header file for spells.c
 */

#ifndef _SPELLS_H
#define _SPELLS_H

#define MAX_SPELLS 400              // Max # of spells in menu
#define MAX_SPELLS_PER_SCHOOL 50    // Max # of spells per school

// Return values for FindSpellByName
#define SPELL_NOMATCH    ((spell *) 0)
#define SPELL_AMBIGUOUS  ((spell *) -1)

typedef struct {
   object_node obj;
   BYTE        num_targets;
   BYTE        school;
} spell;     /* A magical spell--must have obj struct to use in owner drawn list box */

void SpellsInit(void);
void SpellsExit(void);
void MenuRemoveAllSpells(void);
void MenuAddAllSpells(void);
void SpellsGotSchools(BYTE num, ID *schools);
void SetSpells(list_type new_spells);
void FreeSpells(void);
void AddSpell(spell *sp);
void RemoveSpell(ID id);
spell *FindSpellByName(char *name);
spell *FindSpellByID(ID id);
char *GetSpellName(char *str, char **next);

void UserCastSpell(void);
void SpellCast(spell *sp);
void MenuSpellChosen(int index);

object_node* GetSpellObject( ID idFind );

#endif /* #ifndef _SPELLS_H */
