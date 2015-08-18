// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * language.c:  Builds a menu containing available resource languages.
 *
 */

#include "client.h"
#include "merintr.h"

static HMENU language_menu;

// Need to know numeric positions of main menu top level things, silly as this may be
#define MENU_POSITION_LANGUAGE 4

typedef struct {
  char   *two_letter_code;
  char   *language_name;
  int    languageid;
} lang_table;

// Table of all languages we could possibly use.
static lang_table language_id_table[] = {
   { "en", "English",      0,   },
   { "de", "German",       1,   },
   { "ko", "Korean",       2,   },
   { "ru", "Russian",      3,   },
   { "sv", "Swedish",      4,   },
   { "ab", "Abkhazian",    5,   },
   { "aa", "Afar",         6,   },
   { "af", "Afrikaans",    7,   },
   { "ak", "Akan",         8,   },
   { "sq", "Albanian",     9,   },
   { "am", "Amharic",      10,  },
   { "ar", "Arabic",       11,  },
   { "an", "Aragonese",    12,  },
   { "hy", "Armenian",     13,  },
   { "as", "Assamese",     14,  },
   { "av", "Avaric",       15,  },
   { "ae", "Avestan",      16,  },
   { "ay", "Aymara",       17,  },
   { "az", "Azerbaijani",  18,  },
   { "bm", "Bambara",      19,  },
   { "ba", "Bashkir",      20,  },
   { "eu", "Basque",       21,  },
   { "be", "Belarusian",   22,  },
   { "bn", "Bengali",      23,  },
   { "bh", "Bihari",       24,  },
   { "bi", "Bislama",      25,  },
   { "nb", "Bokmål",       26,  },
   { "bs", "Bosnian",      27,  },
   { "br", "Breton",       28,  },
   { "bg", "Bulgarian",    29,  },
   { "my", "Burmese",      30,  },
   { "ca", "Catalan",      31,  },
   { "ch", "Chamorro",     32,  },
   { "ce", "Chechen",      33,  },
   { "zh", "Chinese",      34,  },
   { "cv", "Chuvash",      35,  },
   { "kw", "Cornish",      36,  },
   { "co", "Corse",        37,  },
   { "cr", "Cree",         38,  },
   { "hr", "Croatian",     39,  },
   { "cs", "Czech",        40,  },
   { "da", "Danish",       41,  },
   { "dv", "Divehi",       42,  },
   { "nl", "Dutch",        43,  },
   { "dz", "Dzongkha",     44,  },
   { "eo", "Esperanto",    45,  },
   { "et", "Estonian",     46,  },
   { "ee", "Ewe",          47,  },
   { "fo", "Faroese",      48,  },
   { "fj", "Fijian",       49,  },
   { "fi", "Finnish",      50,  },
   { "fr", "Français",     51,  },
   { "fy", "Frisian",      52,  },
   { "ff", "Fulah",        53,  },
   { "gd", "Gaelic",       54,  },
   { "gl", "Gallegan",     55,  },
   { "lg", "Ganda",        56,  },
   { "ka", "Georgian",     57,  },
   { "el", "Greek",        58,  },
   { "kl", "Greenlandic",  59,  },
   { "gn", "Guarani",      60,  },
   { "gu", "Gujarati",     61,  },
   { "ha", "Hausa",        62,  },
   { "he", "Hebrew",       63,  },
   { "hz", "Herero",       64,  },
   { "hi", "Hindi",        65,  },
   { "ho", "Hiri Motu",    66,  },
   { "hu", "Hungarian",    67,  },
   { "is", "Icelandic",    68,  },
   { "io", "Ido",          69,  },
   { "ig", "Igbo",         70,  },
   { "id", "Indonesian",   71,  },
   { "ia", "Interlingua",  72,  },
   { "ie", "Interlingue",  73,  },
   { "iu", "Inuktitut",    74,  },
   { "ik", "Inupiaq",      75,  },
   { "ga", "Irish",        76,  },
   { "it", "Italian",      77,  },
   { "ja", "Japanese",     78,  },
   { "jv", "Javanese",     79,  },
   { "kn", "Kannada",      80,  },
   { "kr", "Kanuri",       81,  },
   { "ks", "Kashmiri",     82,  },
   { "kk", "Kazakh",       83,  },
   { "km", "Khmer",        84,  },
   { "ki", "Kikuyu",       85,  },
   { "rw", "Kinyarwanda",  86,  },
   { "ky", "Kirghiz",      87,  },
   { "kv", "Komi",         88,  },
   { "kg", "Kongo",        89,  },
   { "kj", "Kuanyama",     90,  },
   { "ku", "Kurdish",      91,  },
   { "lo", "Lao",          92,  },
   { "la", "Latin",        93,  },
   { "lv", "Latvian",      94,  },
   { "lb", "Letzeburgesch",95,  },
   { "li", "Limburgan",    96,  },
   { "ln", "Lingala",      97,  },
   { "lt", "Lithuanian",   98,  },
   { "lu", "Luba-Katanga", 99,  },
   { "mk", "Macedonian",   100, },
   { "mg", "Malagasy",     101, },
   { "ms", "Malay",        102, },
   { "ml", "Malayalam",    103, },
   { "mt", "Maltese",      104, },
   { "gv", "Manx",         105, },
   { "mi", "Maori",        106, },
   { "mr", "Marathi",      107, },
   { "mh", "Marshallese",  108, },
   { "mo", "Moldavian",    109, },
   { "mn", "Mongolian",    110, },
   { "na", "Nauru",        111, },
   { "nv", "Navaho",       112, },
   { "nd", "Ndebele North",113, },
   { "nr", "Ndebele South",114, },
   { "ng", "Ndonga",       115, },
   { "ne", "Nepali",       116, },
   { "se", "Northern Sami",117, },
   { "no", "Norwegian",    118, },
   { "nn", "Nynorsk",      119, },
   { "ny", "Nyanja",       120, },
   { "oc", "Provençal",    121, },
   { "oj", "Ojibwa",       122, },
   { "cu", "Old Bulgarian",123, },
   { "or", "Oriya",        124, },
   { "om", "Oromo",        125, },
   { "os", "Ossetian",     126, },
   { "pi", "Pali",         127, },
   { "pa", "Panjabi",      128, },
   { "fa", "Persian",      129, },
   { "pl", "Polish",       130, },
   { "pt", "Portuguese",   131, },
   { "ps", "Pushto",       132, },
   { "qu", "Quechua",      133, },
   { "rm", "Raeto-Romance",134, },
   { "ro", "Romanian",     135, },
   { "rn", "Rundi",        136, },
   { "sm", "Samoan",       137, },
   { "sg", "Sango",        138, },
   { "sa", "Sanskrit",     139, },
   { "sc", "Sardinian",    140, },
   { "sr", "Serbian",      141, },
   { "sn", "Shona",        142, },
   { "ii", "Sichuan Yi",   143, },
   { "sd", "Sindhi",       144, },
   { "si", "Sinhalese",    145, },
   { "sk", "Slovak",       146, },
   { "sl", "Slovenian",    147, },
   { "so", "Somali",       148, },
   { "st", "Sotho",        149, },
   { "es", "Spanish",      150, },
   { "su", "Sundanese",    151, },
   { "sw", "Swahili",      152, },
   { "ss", "Swati",        153, },
   { "tl", "Tagalog",      154, },
   { "ty", "Tahitian",     155, },
   { "tg", "Tajik",        156, },
   { "ta", "Tamil",        157, },
   { "tt", "Tatar",        158, },
   { "te", "Telugu",       159, },
   { "th", "Thai",         160, },
   { "bo", "Tibetan",      161, },
   { "ti", "Tigrinya",     162, },
   { "to", "Tonga",        163, },
   { "ts", "Tsonga",       164, },
   { "tn", "Tswana",       165, },
   { "tr", "Turkish",      166, },
   { "tk", "Turkmen",      167, },
   { "tw", "Twi",          168, },
   { "ug", "Uighur",       169, },
   { "uk", "Ukrainian",    170, },
   { "ur", "Urdu",         171, },
   { "uz", "Uzbek",        172, },
   { "ve", "Venda",        173, },
   { "vi", "Vietnamese",   174, },
   { "vo", "Volapük",      175, },
   { "wa", "Walloon",      176, },
   { "cy", "Welsh",        177, },
   { "wo", "Wolof",        178, },
   { "xh", "Xhosa",        179, },
   { "yi", "Yiddish",      180, },
   { "yo", "Yoruba",       181, },
   { "za", "Zhuang",       182, },
   { "zu", "Zulu",         183, },
   {NULL,   NULL,          184  }
};

/* Local function prototypes. */
static void MenuAddLanguage(int lang_id);
/********************************************************************/
/*
 * LanguageInit:  Initialize language menu when game entered.
 */
void LanguageInit(void)
{
   language_menu = CreatePopupMenu();

   // Add "Language" menu item.
   if (language_menu != NULL)
   {
      InsertMenu(cinfo->main_menu, MENU_POSITION_LANGUAGE, MF_STRING | MF_POPUP | MF_BYPOSITION, 
         (UINT) language_menu, GetString(hInst, IDS_LANGUAGE_MENU));
      DrawMenuBar(cinfo->hMain);
   }

   // Get available languages and populate the menu.
   Bool *avail_languages = GetAvailableLanguages();

   for (int i = 0; i < MAX_LANGUAGE_ID; i++)
      if (avail_languages[i] == 1)
         MenuAddLanguage(i);

   // Add a check to the selected language.
   CheckMenuItem(language_menu, cinfo->config->language + ID_LANGUAGE, MF_CHECKED);
}
/********************************************************************/
/*
 * LanguageExit:  Free language menu when game exited.
 */
void LanguageExit(void)
{
   // Remove "Language" menu.
   if (language_menu != NULL)
   {
      RemoveMenu(cinfo->main_menu, MENU_POSITION_LANGUAGE, MF_BYPOSITION);
      DrawMenuBar(cinfo->hMain);
      DestroyMenu(language_menu);
   }

   language_menu = NULL;
}
/********************************************************************/
/*
 * MenuAddLanguage:  Add given language to Language menu.
 */
void MenuAddLanguage(int lang_id)
{
   int num, index, len;
   char item_name[MAXRSCSTRING + 1], *name = NULL;

   if (lang_id > MAX_LANGUAGE_ID)
      return;

   num = GetMenuItemCount(language_menu);

   for(int i = 0; i < MAX_LANGUAGE_ID; i++)
      if(language_id_table[i].languageid == lang_id)
      {
         name = language_id_table[i].language_name;

         break;
      }

   // Add in sorted order.
   for (index = 0; index < num; index++)
   {
      len = GetMenuString(language_menu, index, item_name, MAXRSCSTRING, MF_BYPOSITION);
      if (len == 0)
         continue;
      if (stricmp(item_name, name) >= 0)
         break;
   }

   // Check for adding to end of list.
   if (index == num)
      index = -1;

   InsertMenu(language_menu, index, MF_STRING | MF_BYPOSITION, 
         ID_LANGUAGE + lang_id, name);
}
/********************************************************************/
/*
 * MenuLanguageChosen:  User picked a language from the list.
 */
void MenuLanguageChosen(int id)
{
   if (language_menu == NULL)
      return;

   // Get the language ID by removing the Windows base ID (used for the menu).
   int lang_id = id - ID_LANGUAGE;

   if (lang_id < MAX_LANGUAGE_ID)
   {
      // Uncheck the currently selected language.
      CheckMenuItem(language_menu, cinfo->config->language + ID_LANGUAGE, MF_UNCHECKED);
      // Remove all spells from the client menu.
      MenuRemoveAllSpells();
      // Write language ID to config
      cinfo->config->language = lang_id;
      // Add the spells back (in the new language).
      MenuAddAllSpells();
      // Check the one we just selected (using the Windows menu ID).
      CheckMenuItem(language_menu, id, MF_CHECKED);
      // Get the room, player and inventory data again. Currently not much
      // of this data is affected by the language settings, but this may change.
      ResetUserData();
   }
}
