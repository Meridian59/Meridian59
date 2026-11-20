/*
 * characternotes.c: persistent notes associated with character names.
 */

#include "client.h"
#include "characternotes.h"

#include <fstream>
#include <map>

/* Notes are shared by all accounts and characters using this client installation. */
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
   std::string note;
};

const char *GetCharacterNote(const char *characterName)
{
   if (!characterName)
      return NULL;

   std::map<std::string, std::string, CharacterNameLess>::const_iterator note =
      characterNotes.find(characterName);
   /* the returned pointer remains valid until this note is changed or erased  */
   return note == characterNotes.end() ? NULL : note->second.c_str();
}

void LoadCharacterNotes(void)
{
   std::ifstream file(_szCharacterNotesFile);

   characterNotes.clear();
   if (!file)
   {
      debug(("LoadCharacterNotes(): cannot open %s\n", _szCharacterNotesFile));
      return;
   }

   std::string line;
   while (std::getline(file, line))
   {
      /* Each record contains a character name, a tab, and a note of up to MAX_CHARACTER_NOTE characters. */
      size_t lineEnd = line.find_first_of("\r\n");
      if (lineEnd != std::string::npos)
         line.erase(lineEnd);

      size_t separator = line.find('\t');
      if (separator == std::string::npos)
         continue;

      std::string characterName = line.substr(0, separator);
      std::string note = line.substr(separator + 1);
      if (characterName.empty() || characterName.size() > MAX_CHARNAME ||
          note.size() > MAX_CHARACTER_NOTE)
         continue;

      characterNotes[characterName] = note;
   }

   if (file.bad())
      debug(("LoadCharacterNotes(): error reading %s\n", _szCharacterNotesFile));
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
      SetDlgItemText(dialog, IDC_CHARACTERNOTE_EDIT, data->note.c_str());
      return TRUE;
   }

   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
      {
         std::string note(MAX_CHARACTER_NOTE + 1, '\0');
         std::string oldNote;
         std::map<std::string, std::string, CharacterNameLess>::const_iterator old =
            characterNotes.find(data->characterName);

         if (old != characterNotes.end())
            oldNote = old->second;

         UINT noteLength = GetDlgItemText(dialog, IDC_CHARACTERNOTE_EDIT, &note[0],
                                          static_cast<int>(note.size()));
         note.resize(noteLength);
         /* Keep notes on one line so they remain a single file record. */
         for (size_t i = 0; i < note.size(); ++i)
            if (note[i] == '\t' || note[i] == '\r' || note[i] == '\n')
               note[i] = ' ';

         if (oldNote != note)
         {
            if (!note.empty())
               characterNotes[data->characterName] = note;
            else
               characterNotes.erase(data->characterName);

            if (!SaveCharacterNotes())
            {
               if (oldNote.empty())
                  characterNotes.erase(data->characterName);
               else
                  characterNotes[data->characterName] = oldNote;

               /* keep the in-memory note unchanged when saving fails. */
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
   existingNote = GetCharacterNote(characterName);
   if (existingNote)
   {
      data.note = existingNote;
      if (data.note.size() > MAX_CHARACTER_NOTE)
         data.note.resize(MAX_CHARACTER_NOTE);
   }

   if (SafeDialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHARACTERNOTE), parent,
                          CharacterNoteDialogProc, (LPARAM)&data) == -1)
      debug(("EditCharacterNote(): could not open note dialog\n"));
}
