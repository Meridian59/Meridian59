/*
 * characternotes.c: Persistent notes associated with character names.
 */

#include "client.h"
#include "characternotes.h"

#include <map>

static const char* _szCharacterNotesFile = "mail\\characternotes.dat";

struct CharacterNameLess
{
   bool operator()(const std::string& left, const std::string& right) const
   {
      return _stricmp(left.c_str(), right.c_str()) < 0;
   }
};

static std::map<std::string, std::string, CharacterNameLess> characterNotes;

struct CharacterNoteDialogData
{
   std::string characterName;
   char note[MAX_CHARACTER_NOTE + 1];
};

const char *GetCharacterNote(const char *characterName)
{
   if (!characterName)
      return NULL;

   std::map<std::string, std::string, CharacterNameLess>::const_iterator note =
      characterNotes.find(characterName);
   return note == characterNotes.end() ? NULL : note->second.c_str();
}

void LoadCharacterNotes(void)
{
   FILE *file;
   char line[512];

   characterNotes.clear();
   file = fopen(_szCharacterNotesFile, "rt");
   if (!file)
   {
      debug(("LoadCharacterNotes(): cannot open %s\n", _szCharacterNotesFile));
      return;
   }

   while (fgets(line, sizeof(line), file))
   {
      size_t length = strlen(line);
      char *separator;
      char *note;

      if (length > 0 && line[length - 1] != '\n' && !feof(file))
      {
         int ch;
         while ((ch = fgetc(file)) != '\n' && ch != EOF)
            ;
         continue;
      }

      line[strcspn(line, "\r\n")] = '\0';
      separator = strchr(line, '\t');
      if (!separator)
         continue;

      *separator = '\0';
      note = separator + 1;
      if (!line[0] || strlen(line) > MAX_CHARNAME ||
          strlen(note) > MAX_CHARACTER_NOTE)
         continue;

      characterNotes[line] = note;
   }

   if (ferror(file))
      debug(("LoadCharacterNotes(): error reading %s\n", _szCharacterNotesFile));

   fclose(file);
}

BOOL SaveCharacterNotes(void)
{
   FILE *file = fopen(_szCharacterNotesFile, "wt");
   if (!file)
   {
      debug(("SaveCharacterNotes(): cannot open %s to save\n", _szCharacterNotesFile));
      return FALSE;
   }

   for (std::map<std::string, std::string, CharacterNameLess>::const_iterator note =
           characterNotes.begin();
        note != characterNotes.end(); ++note)
   {
      if (fprintf(file, "%s\t%s\n", note->first.c_str(), note->second.c_str()) < 0)
      {
         debug(("SaveCharacterNotes(): error writing %s\n", _szCharacterNotesFile));
         fclose(file);
         return FALSE;
      }
   }

   if (fclose(file) != 0)
   {
      debug(("SaveCharacterNotes(): error closing %s\n", _szCharacterNotesFile));
      return FALSE;
   }

   return TRUE;
}

void FreeCharacterNotes(void)
{
   characterNotes.clear();
}

static INT_PTR CALLBACK CharacterNoteDialogProc(HWND dialog, UINT message,
                                                WPARAM wParam, LPARAM lParam)
{
   CharacterNoteDialogData *data =
      (CharacterNoteDialogData *)GetWindowLongPtr(dialog, DWLP_USER);

   switch (message)
   {
   case WM_INITDIALOG:
   {
      data = (CharacterNoteDialogData *)lParam;
      SetWindowLongPtr(dialog, DWLP_USER, (LONG_PTR)data);
      char label[80];
      GetDlgItemText(dialog, IDC_CHARACTERNOTE_LABEL, label, sizeof(label));
      std::string characterLabel = std::string(label) + " " + data->characterName;
      SetDlgItemText(dialog, IDC_CHARACTERNOTE_LABEL, characterLabel.c_str());
      SendDlgItemMessage(dialog, IDC_CHARACTERNOTE_EDIT, EM_LIMITTEXT,
                         MAX_CHARACTER_NOTE, 0);
      SetDlgItemText(dialog, IDC_CHARACTERNOTE_EDIT, data->note);
      return TRUE;
   }

   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
      {
         char note[MAX_CHARACTER_NOTE + 1];
         std::string oldNote;
         std::map<std::string, std::string, CharacterNameLess>::const_iterator old =
            characterNotes.find(data->characterName);

         if (old != characterNotes.end())
            oldNote = old->second;

         GetDlgItemText(dialog, IDC_CHARACTERNOTE_EDIT, note, sizeof(note));
         for (char *ch = note; *ch; ++ch)
            if (*ch == '\t' || *ch == '\r' || *ch == '\n')
               *ch = ' ';

         if (oldNote != note)
         {
            if (note[0])
               characterNotes[data->characterName] = note;
            else
               characterNotes.erase(data->characterName);

            if (!SaveCharacterNotes())
            {
               if (oldNote.empty())
                  characterNotes.erase(data->characterName);
               else
                  characterNotes[data->characterName] = oldNote;

               MessageBox(dialog,
                          "Unable to save character notes to mail\\characternotes.dat.",
                          szAppName, MB_OK | MB_ICONERROR);
               return TRUE;
            }
         }

         EndDialog(dialog, IDOK);
         return TRUE;
      }

      case IDCANCEL:
         EndDialog(dialog, IDCANCEL);
         return TRUE;
      }
      break;
   }

   return FALSE;
}

void EditCharacterNote(HWND parent, const char *characterName)
{
   CharacterNoteDialogData data;
   const char *existingNote;

   if (!characterName || !*characterName)
      return;

   data.characterName = characterName;
   data.note[0] = '\0';
   existingNote = GetCharacterNote(characterName);
   if (existingNote)
      strncpy(data.note, existingNote, MAX_CHARACTER_NOTE);
   data.note[MAX_CHARACTER_NOTE] = '\0';

   if (SafeDialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHARACTERNOTE), parent,
                          CharacterNoteDialogProc, (LPARAM)&data) == -1)
      debug(("EditCharacterNote(): could not open note dialog\n"));
}
