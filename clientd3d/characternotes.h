#ifndef _CHARACTERNOTES_H
#define _CHARACTERNOTES_H

#define MAX_CHARACTER_NOTE 255

const char *GetCharacterNote(const char *characterName);
void LoadCharacterNotes(void);
BOOL SaveCharacterNotes(void);
void FreeCharacterNotes(void);
void EditCharacterNote(HWND parent, const char *characterName);

#endif
