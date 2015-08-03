// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/************************************************************************/
/* 
 * actions.c
 * Action procedures for Blakod parser
 */
/************************************************************************/
#include "blakcomp.h"
#include "bkod.h"
#include "resource.h"

extern function_type Functions[];
extern int numfuncs;
extern id_struct BuiltinIds[];
extern int numbuiltins;
extern int lineno;

static int loop_depth = 0;
SymbolTable st;

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

/* Miscellaneous procedures */
/************************************************************************/
void initialize_parser(void)
{
   int i;

   st.globalvars = table_create(TABLESIZE);
   st.classvars = table_create(TABLESIZE);
   st.localvars = table_create(TABLESIZE);
   st.missingvars = table_create(TABLESIZE);

   /* Add function names to table of global identifiers */
   for (i=0; i < numfuncs; i++)
   {
      id_type id = (id_type) SafeMalloc(sizeof(id_struct));
      id->name = Functions[i].name;
      id->type = I_FUNCTION;
      id->idnum = i;  /* For functions, idnum is just index in table, not a real id # */
      if (table_insert(st.globalvars, (void *) id, id_hash, id_compare) != 0)
         simple_error("Duplicate built-in function name %s", id->name);
   }

   /* Add builtin identifiers to appropriate symbol tables */
   for (i=0; i < numbuiltins; i++)
      switch (BuiltinIds[i].type)
      {
      case I_MISSING:
         if (table_insert(st.missingvars, (void *) &BuiltinIds[i], id_hash, id_compare) != 0)
            simple_error("Duplicate builtin identifier name %s", BuiltinIds[i].name);
         break;

      case I_PROPERTY:
         if (table_insert(st.globalvars, (void *) &BuiltinIds[i], id_hash, id_compare) != 0)
            simple_error("Duplicate builtin identifier name %s", BuiltinIds[i].name);
         break;

      default:
         simple_error("Bad type on builtin identifier %s", BuiltinIds[i].name);
         break;
      }

   st.maxid = IDBASE; /* Base for user-defined ids; builtins have lower #s */
   st.maxresources = RESOURCEBASE;
   st.maxlocals = -1; /* So that first local is numbered 0 */
   st.maxclassvars = -1; /* So that first class variable is numbered 0 */
   // XXX not needed because of self
#if 0
   st.maxproperties = -1; /* So that first property is numbered 0 */
#endif

   st.recompile_list = NULL;
   st.constants = NULL;
   st.num_strings = 0;
   st.strings = NULL;
   st.override_classvars = NULL;
}
/************************************************************************/
/* Hash on an indentifier's name */
int id_hash(const void *info, int table_size)
{
   id_type id = (id_type) info;
   
   return string_hash(id->name, table_size);
}
/************************************************************************/
/* Compare two identifiers based on names */
int id_compare(void *info1, void *info2)
{
   id_type id1 = (id_type) info1;
   id_type id2 = (id_type) info2;
   
   return !stricmp(id1->name, id2->name);
}
/************************************************************************/
/* Compare a class with a recompile_info structure based on class names */
int recompile_compare(void *info1, void *info2)
{
   id_type c1 = (id_type) info1;
   recompile_type r2 = (recompile_type) info2;
   
   return !stricmp(c1->name, r2->class_id->name);
}
/************************************************************************/
/* Compare two classes based on name */
int class_compare(void *info1, void *info2)
{
   class_type class1 = (class_type) info1;
   class_type class2 = (class_type) info2;
   
   return !stricmp(class1->class_id->name, class2->class_id->name);
}
/************************************************************************/
/*
 * is_parent: returns True iff parent is a parent class of child, or if 
 *    parent and child are the same class.
 *    Class id #s are compared to see if two classes are equal.
 */
int is_parent(class_type parent, class_type child)
{
   if (child->class_id->idnum == parent->class_id->idnum)
      return True;

   if (child->superclass == NULL)
      return False;

   return is_parent(parent, child->superclass);
}
/************************************************************************/
id_type duplicate_id(id_type id)
{
   id_type temp = (id_type) SafeMalloc(sizeof(id_struct));
   
   temp->name = strdup(id->name);
   temp->type = id->type;
   temp->ownernum = id->ownernum;
   temp->idnum = id->idnum;
   return temp;
}
/************************************************************************/
/* 
 * lookup_id: Translate from an identifier name to an ID number.
 *   Returns id that matches the given id, or NULL if none is in table.
 */
id_type lookup_id(id_type id)
{
   id_type record;

   /* We MUST check the local table before the global one, because parameters are
    * entered in both.  We must find the local one first to get a local variable #
    * as the id # of the parameter.  If we found the global one first, we would get
    * the parameter's global id # and interpret it as a local variable #--an error
    */

   /* Check local variable table */
   record = (id_type) table_lookup(st.localvars, (void *) id, id_hash, id_compare);
   if (record != NULL)
   {
      id->type = record->type;
      id->idnum = record->idnum;
      id->ownernum = record->ownernum;
      id->source = record->source;
      return record;
   }

   /* Check class table */
   record = (id_type) table_lookup(st.classvars, (void *) id, id_hash, id_compare);
   if (record != NULL)
   {
      id->type = record->type;
      id->idnum = record->idnum;
      id->ownernum = record->ownernum;
      id->source = record->source;
      return record;
   }

   /* Check global table */
   record = (id_type) table_lookup(st.globalvars, (void *) id, id_hash, id_compare);
   if (record != NULL)
   {
      id->type = record->type;
      id->idnum = record->idnum;
      id->ownernum = record->ownernum;
      id->source = record->source;
      return record;
   }

   /* Check missing table */
   record = (id_type) table_lookup(st.missingvars, (void *) id, id_hash, id_compare);
   if (record != NULL)
   {
      id->type = record->type;
      id->idnum = record->idnum;
      id->ownernum = record->ownernum;
      id->source = record->source;
      return record;
   }

   /* Couldn't find the identifier name */
   id->type = I_UNDEFINED;
   return NULL;
}
/************************************************************************/
/* 
 * add_identifier: Add identifier to appropriate namespace, and assigns
 *                 it the next available id #.  Does nothing
 *                 if identifier is already in table, but returns nonzero.
 */
int add_identifier(id_type id, int type)
{
   /* Add identifier to appropriate table */
   
   id->type = type;

   switch(type) 
   {
   case I_CLASS:
      if (table_insert(st.globalvars, (void *) id, id_hash, id_compare) == 0)
	 id->idnum = ++st.maxid;
      else return 1;

      break;

   case I_MESSAGE:
      if (table_insert(st.globalvars, (void *) id, id_hash, id_compare) == 0)
	 id->idnum = ++st.maxid;
      else return 1;
      break;

   case I_CONSTANT:
      /* Don't give constants id #s */
      if (table_insert(st.classvars, (void *) id, id_hash, id_compare) != 0)
	 return 1;
      break;
      
   case I_RESOURCE:
      if (table_insert(st.globalvars, (void *) id, id_hash, id_compare) == 0)
	 id->idnum = ++st.maxresources;
      else return 1;
      break;

   case I_CLASSVAR:
      if (table_insert(st.classvars, (void *) id, id_hash, id_compare) == 0)
	 id->idnum = ++st.maxclassvars;
      else return 1;
      break;

   case I_PROPERTY:
      if (table_insert(st.classvars, (void *) id, id_hash, id_compare) == 0)
	 id->idnum = ++st.maxproperties;
      else return 1;
      break;

   case I_LOCAL:
      if (table_insert(st.localvars, (void *) id, id_hash, id_compare) == 0)
	 id->idnum = ++st.maxlocals;
      else return 1;

      break;

   case I_PARAMETER:
      if (table_insert(st.globalvars, (void *) id, id_hash, id_compare) == 0)
	 id->idnum = ++st.maxid;
      else return 1;

      break;

   case I_MISSING:
      if (table_insert(st.missingvars, (void *) id, id_hash, id_compare) == 0)
	 id->idnum = ++st.maxid;
      else return 1;
      break;

   default:
      /* The default action is not to insert I_UNDEFINED ids into
       * any table.
       */
      break;
   }
   return 0;
}
/************************************************************************/
/* Action procedures */

/************************************************************************/
const_type make_numeric_constant(int num)
{
   const_type c = (const_type) SafeMalloc(sizeof(const_struct));

   c->type = C_NUMBER;
   c->value.numval = num;
   return c;
}
/************************************************************************/
const_type make_nil_constant(void)
{
   const_type c = (const_type) SafeMalloc(sizeof(const_struct));

   c->type = C_NIL;
   return c;
}
/************************************************************************/
const_type make_number_from_constant_id(id_type id)
{
   const_type c = (const_type) SafeMalloc(sizeof(const_struct));

   lookup_id(id);
   switch(id->type)
   {

   case I_CONSTANT:
      c->type = C_NUMBER;
      c->value.numval = id->source;  /* Value is stored in source field */
      break;

   case I_RESOURCE:
      /* Turn resource id reference into the resource # itself */
      c->type = C_RESOURCE;
      c->value.numval = id->idnum;       
      break;

   default:
      action_error("Identifier %s can only be a constant here", id->name);
      return NULL;      
   }
      
   return c;
}
/************************************************************************/
/* make a "debugging string"; the string has a numerical id, which is an
 * index to the list of all debugging strings encountered in this file */
const_type make_string_constant(char *str)
{
   const_type c = (const_type) SafeMalloc(sizeof(const_struct));

   c->type = C_STRING;
   /* Add string to symbol table, and remember index */
   c->value.numval = st.num_strings++;
   st.strings = list_add_item(st.strings, str);

   return c;
}
/************************************************************************/
const_type make_string_resource(char *str)
{
   const_type c = (const_type) SafeMalloc(sizeof(const_struct));

   c->type = C_STRING;
   c->value.stringval = str;
   return c;
}
/************************************************************************/
const_type make_fname_resource(char *fname)
{
   const_type c = (const_type) SafeMalloc(sizeof(const_struct));

   c->type = C_FNAME;
   c->value.stringval = fname;
   return c;
}
/************************************************************************/
id_type make_identifier(char *name)
{
   id_type id = (id_type) SafeMalloc(sizeof(id_struct));

   id->type = I_UNDEFINED;   /* Type is undefined until it's looked up */
   id->name = name;
   id->source = COMPILE;     /* Id came from a newly compiled file */
   return id;
}
/************************************************************************/
const_type make_literal_class(id_type id)
{
   const_type c = (const_type) SafeMalloc(sizeof(const_struct));

   lookup_id(id);		
   switch (id->type)
   {
   case I_CLASS:
      break;

   case I_MISSING:
      if (id->source != I_CLASS)
	 action_error("Identifier %s was referenced earlier with different type", id->name);
      break;

   default: 
      add_identifier(id, I_MISSING);
      id->source = I_CLASS;
      break;     
   }
   /* Make a constant expression whose value is the id # */
   c->type = C_CLASS;
   c->value.numval = id->idnum; 

   return c;
}
/************************************************************************/
const_type make_literal_message(id_type id)
{
   const_type c = (const_type) SafeMalloc(sizeof(const_struct));

   lookup_id(id);		
   switch (id->type)
   {
   case I_MESSAGE:
      break;

   case I_MISSING:
      if (id->source != I_MESSAGE)
	 action_error("Identifier %s was referenced earlier with different type", id->name);
      break;

   default: 
      add_identifier(id, I_MISSING);
      id->source = I_MESSAGE;
      break;     
   }
   /* Make a constant expression whose value is the id # */
   c->type = C_MESSAGE;
   c->value.numval = id->idnum; 

   return c;
}
/************************************************************************/
expr_type make_expr_from_id(id_type id)
{
   expr_type e = (expr_type) SafeMalloc(sizeof(expr_struct));
   
   /* Id must be a parameter, local, property, constant, or resource */
   lookup_id(id);		
   switch(id->type)
   {
   case I_LOCAL:
   case I_PROPERTY:
   case I_CLASSVAR:
      e->type = E_IDENTIFIER;
      e->value.idval = id;
      break;

   case I_RESOURCE:
   {
      const_type c = (const_type) SafeMalloc(sizeof(const_struct));

      /* Turn resource id reference into the resource # itself */
      c->type = C_RESOURCE;
      c->value.numval = id->idnum; 

      e->type = E_CONSTANT;
      e->value.constval = c;
      return e;
   }

   case I_CONSTANT:
   {
      const_type c = (const_type) SafeMalloc(sizeof(const_struct));
      id_type temp;

      /* Turn constant id reference into the constant itself */
      c->type = C_NUMBER;

      temp = (id_type) list_find_item(st.constants, id, id_compare);
      c->value.numval = temp->source; /* Value is stored in source field */

      e->type = E_CONSTANT;
      e->value.constval = c;
      break;
   }
   case I_UNDEFINED:
   case I_MISSING:
      action_error("Unknown identifier %s", id->name);
      /* Put in something so that compilation can continue */
      e = make_expr_from_constant(make_nil_constant());
      break;

   default:
      action_error("Identifier %s in expression has wrong type", id->name);
      /* Put in something so that compilation can continue */
      e = make_expr_from_constant(make_nil_constant());
      break;
   }
   e->lineno = lineno;
   return e;
}
/************************************************************************/
expr_type make_expr_from_call(stmt_type call_stmt)
{
   expr_type e = (expr_type) SafeMalloc(sizeof(expr_struct));

   e->type = E_CALL;
   e->value.callval = (void *) call_stmt; /* See blakcomp.h for reason for cast */
   e->lineno = lineno;
   return e;
}
/************************************************************************/
expr_type make_expr_from_constant(const_type c)
{
   expr_type e = (expr_type) SafeMalloc(sizeof(expr_struct));
   
   e->type = E_CONSTANT;
   e->value.constval = c;
   e->lineno = lineno;
   return e;
}
/************************************************************************/
expr_type make_bin_op(expr_type expr1, int op, expr_type expr2)
{
   expr_type e = (expr_type) SafeMalloc(sizeof(expr_struct));
   
   e->type = E_BINARY_OP;
   e->value.binary_opval.left_exp = expr1;
   e->value.binary_opval.right_exp = expr2;
   e->value.binary_opval.op = op;
   e->lineno = lineno;

   SimplifyExpression(e);
   return e;
}
/************************************************************************/
expr_type make_un_op(int op, expr_type expr1)
{
   expr_type e = (expr_type) SafeMalloc(sizeof(expr_struct));
   
   e->type = E_UNARY_OP;
   e->value.unary_opval.exp = expr1;
   e->value.unary_opval.op = op;
   e->lineno = lineno;

   SimplifyExpression(e);
   return e;
}
/************************************************************************/
arg_type make_arg_from_expr(expr_type expr)
{
   arg_type arg = (arg_type) SafeMalloc(sizeof(arg_struct));
   arg->type = ARG_EXPR;
   arg->value.expr_val = expr;
   return arg;
}
/************************************************************************/
arg_type make_arg_from_setting(id_type id, expr_type expr)
{
   setting_type s = (setting_type) SafeMalloc(sizeof(setting_struct));
   arg_type arg = (arg_type) SafeMalloc(sizeof(arg_struct));

   s->id = id;
   s->expr = expr;
   
   arg->type = ARG_SETTING;
   arg->value.setting_val = s;
   return arg;
}
/************************************************************************/
id_type make_constant_id(id_type id, expr_type expr)
{
   int numeric_val;

   /* Right hand side must be a number or a negative number */
   switch (expr->type)
   {
   case E_CONSTANT:
   {
      const_type c = (const_type) expr->value.constval;
      if (c->type == C_NUMBER)
	 numeric_val = c->value.numval;
      else action_error("Right hand side must be a numeric constant");
      break;
   }
   case E_UNARY_OP:
   {
      const_type c;
      expr_type sub_expr = expr->value.unary_opval.exp;
      int op = expr->value.unary_opval.op;
      
      if (op != NEG_OP || sub_expr->type != E_CONSTANT)
      {
         action_error("Right hand side must be a numeric constant");
         break;
      }
      
      c = (const_type) sub_expr->value.constval;
      if (c->type == C_NUMBER)
         numeric_val = - c->value.numval;
      else action_error("Right hand side must be a numeric constant");
      break;
   }
   default:
      action_error("Right hand side must be a numeric constant");
   }
   

   lookup_id(id);

   /* Left hand side cannot have appeared before */
   switch (id->type)
   {
   case I_UNDEFINED:
      id->ownernum = st.curclass;
      /* Store value in source field.  This is kind of a hack, but now we can 
	 insert just the id in st.constants, making it easy to find later. */
      id->source = numeric_val;
      add_identifier(id, I_CONSTANT);
      break;
      
   default:
      action_error("Duplicate identifier %s", id->name);
      break;
   }

   /* Add to list of constants in this class */
   st.constants = list_add_item(st.constants, id);
   
   return id;
}
/************************************************************************/
param_type make_parameter(id_type id, expr_type e)
{
   param_type p = (param_type) SafeMalloc(sizeof(param_struct));

   if (e->type != E_CONSTANT)
   {
      action_error("Parameter can only be initialized to a constant");
      return p;
   }

   lookup_id(id);

   /* Left-hand side must not have appeared before, except perhaps as a parameter */
   switch (id->type) {
   case I_MISSING:
      /* The parameter has been referenced in a function call, but not declared anywhere 
       * We should use the existent id # and remove the id from the missing list.
       * First we must make sure that the missing id is supposed to be a missing parameter.
       */
      if (id->source != I_PARAMETER)
      {
	 action_error("Parameter %s was referenced elsewhere with different type", id->name);
	 break;
      }
      
      /* Insert directly into global table to preserve id # */
      id->type = I_PARAMETER;
      id->source = COMPILE;
      table_insert(st.globalvars, (void *) id, id_hash, id_compare);

      /* Remove from missing list */
      table_delete_item(st.missingvars, id, id_hash, id_compare);
      break;

   case I_UNDEFINED:   /* New parameter # */
      id->ownernum = st.curmessage;
      add_identifier(id, I_PARAMETER);
      break;

   case I_PARAMETER:
      /* Legal only if it hasn't yet appeared in this message */
      if (id->ownernum == st.curmessage && id->source == COMPILE)
	 action_error("Parameter %s appears twice", id->name);
      break;

   default:            /* Other types indicate name already used */
      action_error("Duplicate identifier %s", id->name);
   }

   p->lhs = id;
   p->rhs = e->value.constval;
   return p;
}
/************************************************************************/
id_type make_var(id_type id)
{
   /* Add to list of local variables, if it hasn't been defined */
   lookup_id(id);

   switch(id->type)
   {
   case I_LOCAL:
      action_error("Duplicate identifier %s", id->name);

   default:
      id->ownernum = st.curmessage;
      add_identifier(id, I_LOCAL);
      break;

   }
   return id;
}
/************************************************************************/
classvar_type make_classvar(id_type id, expr_type e)
{
   classvar_type cv;

   cv = (classvar_type) SafeMalloc(sizeof(classvar_struct));

   if (e->type != E_CONSTANT)
   {
      action_error("Classvar can only be initialized to a constant");
      return cv;
   }

   lookup_id(id);
   switch(id->type) 
   {
   case I_CONSTANT:
   case I_PROPERTY:
      action_error("Duplicate identifier %s", id->name);
      break;
      
      /* Legal if it hasn't yet appeared in this class */
   case I_CLASSVAR:
      if (id->ownernum == st.curclass && id->source == COMPILE)
	 action_error("Class variable %s appears twice", id->name);
      else 
      {
	 id->ownernum = st.curclass;
	 id->source = COMPILE;
	 add_identifier(id, I_CLASSVAR);
      }
      break;

   default:
      id->source = COMPILE;
      id->ownernum = st.curclass;
      add_identifier(id, I_CLASSVAR);
      break;
   }

   cv->id = id;
   cv->rhs = e->value.constval;
   return cv;   
}
/************************************************************************/
property_type make_property(id_type id, expr_type e)
{
   property_type p = (property_type) SafeMalloc(sizeof(property_struct));

   if (e->type != E_CONSTANT)
   {
      action_error("Property can only be initialized to a constant");
      return p;
   }

   /* Left-hand side must not have appeared as a property before, except possibly as a
    * property of one of our superclasses.  Properties shadow other global names. */
   lookup_id(id);
   switch(id->type) {

   case I_CONSTANT:
      action_error("Duplicate identifier %s", id->name);
      break;
      
      /* Legal if it hasn't yet appeared in this class */
   case I_PROPERTY:
      if (id->ownernum == st.curclass && id->source == COMPILE)
	 action_error("Property %s appears twice", id->name);
      else 
      {
	 id->ownernum = st.curclass;
	 id->source = COMPILE;
	 add_identifier(id, I_PROPERTY);
      }
      break;

   case I_CLASSVAR:
      if (id->ownernum == st.curclass && id->source == COMPILE)
	 action_error("Property and classvar %s both appear in same class\n", id->name);
      else
      {
	 classvar_type new_cv;
	 id_type new_id;
	 const_type new_const;
	 
	 // Override classvar with special tag value
	 new_cv = (classvar_type) SafeMalloc(sizeof(classvar_struct));
	 new_const = (const_type) SafeMalloc(sizeof(const_struct));
	 new_id = duplicate_id(id);

	 new_cv->id = new_id;
	 new_cv->rhs = new_const;
	 new_id->ownernum = st.curclass;
	 new_id->source = COMPILE;
	 new_const->type = C_OVERRIDE;
	 
	 // Replace classvar with property in table
	 table_delete_item(st.classvars, id, id_hash, id_compare);
	 id->ownernum = st.curclass;
	 id->source = COMPILE;
	 add_identifier(id, I_PROPERTY);

	 // Store # of property in class var
	 new_const->value.numval = id->idnum;

	 st.override_classvars = list_add_item(st.override_classvars, new_cv);	 
      }
      break;

   default:
      id->source = COMPILE;
      id->ownernum = st.curclass;
      add_identifier(id, I_PROPERTY);
      break;
   }

   p->id = id;
   p->rhs = e->value.constval;
   return p;   
}
/*******************************************************************************/
/*  
 * make_language_id: Compares str to an array of 2 - letter ISO - 639 - 1
 * language codes, and returns an identifying integer.
 */
/******************************************************************************/
int make_language_id(char *str)
{
   int lid = 0;

   for(int i = 0; i < MAX_LANGUAGE_ID; i++)
   {
      if(!strcmp(language_id_table[i].two_letter_code, str))
      {
         lid = language_id_table[i].languageid;

         return lid;
      }
   }

   // If we get here, lid should be 0 and we can log an error.
   if (lid == 0)
      action_error("Invalid language code %s!", str);

   return lid;
}
/************************************************************************/
resource_type make_resource(id_type id, const_type c, int la_id)
{
   id_type old_id;
   list_type l;
   class_type cl = NULL;
   resource_type r = NULL;

   id->ownernum = st.curclass;

   /* Left-hand side must not have appeared before, except maybe in dbase */
   old_id = lookup_id(id);

   /* Check if this resource is already present in another class.
    * old_id will be NULL if it isn't, and if it is defined in the
    * same class, old_id->ownernum is st.curclass (class ID). */
   if (old_id != NULL && old_id->ownernum != st.curclass)
      action_error("Resource already defined in another class!\n");

   switch(id->type) {
   case I_UNDEFINED:
      id->source = COMPILE;
      add_identifier(id, I_RESOURCE);
      break;

   case I_RESOURCE:
      /* Allow redefinition of resources listed in database file */
      if (id->source != COMPILE)
         old_id->source = COMPILE;

      id->source = COMPILE;
      break;
      
   default:
      action_error("Resource defined without type I_UNDEFINED or I_RESOURCE! %s",
         id->name);
   }

   // Get data for this class
   for (l = st.classes; l != NULL; l = l->next)
   {
      cl = (class_type) (l->data);
      if (st.curclass == cl->class_id->idnum)
         break;
   }

   // Try to get the old resource if present.
   for (l = cl->resources; l != NULL; l = l->next)
   {
      r = (resource_type) (l->data);
      if (r->lhs->idnum == id->idnum)
         break;
      r = NULL;
   }

   // If the resource list for this class is empty and thus we don't have
   // a resource, allocate a new one and put our data in it. Also add this
   // to the resources list for this class, so we can check other resources
   // against it.
   if (!cl->resources || !r)
   {
      r = (resource_type)SafeMalloc(sizeof(resource_struct));
      r->lhs = id;
      for (int i = 0; i < MAX_LANGUAGE_ID; i++)
      {
         if (i == la_id)
            r->resource[i] = c;
         else
         {
            //r->resource[i] = (const_type) SafeMalloc(sizeof(const_struct));
            r->resource[i] = NULL;
         }
      }
      cl->resources = list_add_item(cl->resources, r);
   }
   else
   {
      // If we have a valid resource from the class resource list,
      // check the appropriate language ID in this resource and if
      // empty, store our data in it. Working with the resource list
      // so don't return this resource (will be duplicated).
      if (r->resource[la_id])
         action_error("Resource already defined with language ID %i\n",la_id);
      else
         r->resource[la_id] = c;

      return NULL;
   }

   return r;
}
/************************************************************************/
void check_break(void)
{
   if (loop_depth == 0)
      action_error("Break statement must appear inside a loop");
}
/************************************************************************/
void check_continue(void)
{
   if (loop_depth == 0)
      action_error("Continue statement must appear inside a loop");
}
/************************************************************************/
stmt_type make_if_stmt(expr_type condition, list_type then_stmts, list_type else_stmts, stmt_type elseif_stmts)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   if_stmt_type s = (if_stmt_type) SafeMalloc(sizeof(if_stmt_struct));

   s->condition = condition;
   s->then_clause = then_stmts;
   s->else_clause = else_stmts;
   s->elseif_clause = (void *) elseif_stmts;

   stmt->type = S_IF;
   stmt->value.if_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type make_assign_stmt(id_type id, expr_type expr)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   assign_stmt_type s = (assign_stmt_type) SafeMalloc(sizeof(assign_stmt_struct));

   /* Left-hand side must be a local or property */
   lookup_id(id);
   if (id->type == I_UNDEFINED || id->type == I_MISSING)
      action_error("Unknown identifier %s", id->name);
   else
      if (id->type != I_PROPERTY && id->type != I_LOCAL)
	 action_error("Identifier %s cannot be assigned to", id->name);

   /* Can't assign to SELF */
   if (id->type == I_PROPERTY && id->idnum == 0)
      action_error("Can't assign to self");

   s->lhs = id;
   s->rhs = expr;

   stmt->type = S_ASSIGN;
   stmt->value.assign_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type make_foreach_stmt(id_type id, expr_type expr, list_type stmts)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   foreach_stmt_type s = (foreach_stmt_type) SafeMalloc(sizeof(foreach_stmt_struct));

   /* Loop variable must be a local, property or parameter */
   lookup_id(id);
   if (id->type == I_UNDEFINED || id->type == I_MISSING)
      action_error("Unknown identifier %s", id->name);
   else
      if (id->type != I_LOCAL && id->type != I_PROPERTY)
	 action_error("Loop variable %s has wrong type", id->name);

   s->id = id;
   s->condition = expr;
   s->body = stmts;

   stmt->type = S_FOREACH;
   stmt->value.foreach_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type make_while_stmt(expr_type condition, list_type stmts)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   while_stmt_type s = (while_stmt_type) SafeMalloc(sizeof(while_stmt_struct));

   s->condition = condition;
   s->body = stmts;

   stmt->type = S_WHILE;
   stmt->value.while_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type make_do_while_stmt(expr_type condition, list_type stmts)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   while_stmt_type s = (while_stmt_type) SafeMalloc(sizeof(while_stmt_struct));

   s->condition = condition;
   s->body = stmts;

   stmt->type = S_DOWHILE;
   stmt->value.while_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type make_case_stmt(expr_type condition, list_type stmts, bool defaultcase)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   case_stmt_type s = (case_stmt_type) SafeMalloc(sizeof(case_stmt_struct));

   s->body = stmts;

   if (defaultcase)
   {
      stmt->type = S_DEFAULTCASE;
      s->condition = condition;
   }
   else
   {
      stmt->type = S_CASE;
      s->condition = condition;
   }
   stmt->value.case_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type make_switch_stmt(expr_type condition, list_type stmts)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   switch_stmt_type s = (switch_stmt_type) SafeMalloc(sizeof(switch_stmt_struct));

   s->condition = condition;
   s->body = stmts;

   stmt->type = S_SWITCH;
   stmt->value.switch_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type make_for_stmt(list_type init_assign, expr_type condition, list_type assign, list_type stmts)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   for_stmt_type s = (for_stmt_type) SafeMalloc(sizeof(for_stmt_struct));

   s->condition = condition;
   s->body = stmts;
   s->initassign = init_assign;
   s->assign = assign;

   stmt->type = S_FOR;
   stmt->value.for_stmt_val = s;
   return stmt;
}
/************************************************************************/
/* Make a call to the LIST function from a list of expressions */
stmt_type make_list_call(list_type l)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   call_stmt_type s = (call_stmt_type) SafeMalloc(sizeof(call_stmt_struct));
   
   s->function = MLIST;
   s->args = NULL;
   
   for ( ; l != NULL; l = l->next)
      s->args = list_add_item(s->args, make_arg_from_expr((expr_type) l->data));

   stmt->type = S_CALL;
   stmt->value.call_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type make_call(id_type function_id, list_type args)
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   call_stmt_type s = (call_stmt_type) SafeMalloc(sizeof(call_stmt_struct));
   int argnum = 0, argtype, continuation = ANONE, minargs = 0, i, index;
   const char *fname;
   arg_type arg;
   id_type id, new_id;

   s->args = NULL;

   /* Verify that id is the name of a built-in function */
   lookup_id(function_id);

   if (function_id->type != I_FUNCTION)
   {
      action_error("Expecting a function name; found %s", function_id->name);
      return stmt;
   }

   index = function_id->idnum;  /* index # of function to call */

   fname = Functions[index].name;
   s->function = Functions[index].opcode;
      
   /* Check that types of arguments match "function prototype" in table */
   for ( ; args != NULL; args = args->next)
   {
      argtype = Functions[index].params[argnum];

      /* See if we are looking for one or more expressions or parameters */
      if (continuation != ANONE)
	 argtype = continuation;

      arg = (arg_type) args->data;
      switch (argtype) 
      {
      case ANONE:
	 action_error("Extra argument #%d to function %s", argnum+1, fname);
	 return stmt; /* Ignore additional extra arguments */
	 break;

      case AEXPRESSIONS:
	 continuation = AEXPRESSION;
	 /* fall through */
      case AEXPRESSION:
	 /* Must have an expression here */
	 if (arg->type != ARG_EXPR)
	    action_error("Expecting expression in argument #%d",
			 argnum+1, fname);
	 else
	    s->args = list_add_item(s->args, (void *) arg);
	 break;

      case ASETTINGS:
	 continuation = ASETTING;
	 /* fall through */
      case ASETTING:
	 if (arg->type != ARG_SETTING)
	 {
	    action_error("Expecting setting (i.e. #a=b ) in argument #%d",
			 argnum+1, fname);
	    break;
	 }
	 
	 /* Find parameter id */
	 id = arg->value.setting_val->id;
	 lookup_id(id);
	 
	 switch(id->type)
	 {
	 case I_UNDEFINED:
	    /* If parameter hasn't been defined yet, put on missing list */
	    add_identifier(id, I_MISSING);

	    /* Use source field to indicate where id came from for later checking */
	    id->source = I_PARAMETER;

	    s->args = list_add_item(s->args, (void *) arg);
	    break;
	    
	 case I_MISSING:
	    if (id->source != I_PARAMETER)
	    {
	       action_error("Duplicate identifier %s", id->name);
	       break;
	    }
	    s->args = list_add_item(s->args, (void *) arg);
	    break;

	    /* The tag here should be I_PARAMETER, but parameters also inserted as locals.
	     */
	 case I_LOCAL: 
	    /* Unfortunately, we want the parameter id #, and not the local id #.  So
	     * we have to look up the id again in the global table. */
	    new_id = (id_type) table_lookup(st.globalvars, (void *) id, id_hash, id_compare);

	    /* If it isn't there, then there happens to be a local variable of the same
	     * name as this parameter, and the parameter hasn't appeared before.  Thus the
	     * parameter id should be added as a missing variable.  We can't use the normal
	     * lookup_id and add_identifier procs, since the local variable will mask
	     * the missing one.
	     */
	    if (new_id == NULL)
	    {
	       /* Of course, the id might already have been inserted as a missing var */
	       new_id = (id_type) table_lookup(st.missingvars, (void *) id, id_hash, id_compare);
	       if (new_id == NULL)
	       {
		  id->source = I_PARAMETER;
		  id->type = I_MISSING;
		  id->idnum = ++st.maxid;
		  table_insert(st.missingvars, (void *) id, id_hash, id_compare);
	       }
	       else 
	       {
		  /* If it was already there, make sure that it was a parameter */
		  if (new_id->source != I_PARAMETER)
		     action_error("Duplicate identifier %s", id->name);
		  id->source = new_id->source;
		  id->type = I_MISSING;
		  id->source = I_PARAMETER;
		  id->idnum = new_id->idnum;
	       }
	       
	    }
	    else 
	    {
	       id = new_id;
	       if (id->type != I_PARAMETER)
		  action_error("Can't find parameter %s", id->name);
	    }

	    arg->value.setting_val->id = id;
	    /* If literal exists, proceed normally */
	    s->args = list_add_item(s->args, (void *) arg);
	    break;

	    /* But if the identifier is from another handler, might not be local */
	 case I_PARAMETER:
	    s->args = list_add_item(s->args, (void *) arg);
	    break;

	 default:
	    action_error("Literal %s is a duplicate identifier", id->name);
	 }
	 break;
      }
      
      argnum++;
   }
   /* Check that no arguments left out */
   for (i=0; i < MAXARGS; i++)
   {
      argtype = Functions[index].params[i];
      if (argtype == ANONE || argtype == ASETTINGS || argtype == AEXPRESSIONS)
	 break; /* Zero or more arguments follow */
      
      minargs++;
   }
   if (argnum < minargs)
      action_error("Expecting %d arguments to %s; found %d", minargs, fname, argnum);

   /* Sort parameter arguments in increasing order for server efficiency */
   s->args = SortArgumentList(s->args);

   stmt->type = S_CALL;
   stmt->value.call_stmt_val = s;
   return stmt;
}
/************************************************************************/
stmt_type allocate_statement()
{
   return (stmt_type) SafeMalloc(sizeof(stmt_struct));
}
/************************************************************************/
list_type add_statement(list_type l, stmt_type s)
{
   /* Put in statement's line # */
   if (s != NULL)
      s->lineno = get_statement_line(s, lineno);
   
   return list_add_item(l, s);
}
/************************************************************************/
stmt_type make_prop_stmt()
{
   stmt_type stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   list_type l;

   /* Find current class */
   for (l = st.classes; l != NULL; l = l->next)
      if (st.curclass == ((class_type) (l->data))->class_id->idnum)
	 break;
      
   if ( ((class_type) (l->data))->superclass == NULL)
      action_error("Can't PROPAGATE in a class with no superclass");
   stmt->type = S_PROP; 
   return stmt;
}
/************************************************************************/
void enter_loop(void)
{
   loop_depth++;
}
/************************************************************************/
void leave_loop(void)
{
   loop_depth--;
}
/************************************************************************/
message_handler_type make_message_handler(message_header_type header, char *comment,
					  list_type locals, list_type stmts)
{
   message_handler_type h = (message_handler_type) SafeMalloc(sizeof(message_handler_struct));
   stmt_type stmt;

   /* Last statement of body must be PROPAGATE or RETURN */
   stmt = (stmt_type) list_last_item(stmts);

   if (stmt == NULL || (stmt->type != S_RETURN && stmt->type != S_PROP)) 
      action_error("Last statement must be PROPAGATE or RETURN");
   
   h->header  = header;
   h->locals  = locals;
   h->body    = stmts;
   if (comment == NULL)
      h->comment = NULL;
   else h->comment = make_string_constant(comment);

   /* Clean out local variable table.  We must do this at the end of 
    * each handler, since in make_message_header we have already checked
    * parameters names against old variables, which includes parameters
    * from the current function in the local table.
    */
   table_delete(st.localvars);
   st.maxlocals = -1; /* So that first local is numbered 0 */

   return h;
}
/************************************************************************/
message_header_type make_message_header(id_type id, list_type args)
{
   message_header_type s = (message_header_type) SafeMalloc(sizeof(message_header_struct));
   id_type old_id, temp_id;
   param_type param;
   list_type l;

   /* Messsage name must be unique in this class, but may be the same
    * as a message in a different class. */
   old_id = lookup_id(id);

   switch (id->type) {
   case I_MISSING:
      /* The message has been referenced in a function call, but not declared anywhere.
       * We should use the existent message # and remove the id from the missing list 
       */
      if (id->source != I_MESSAGE)
      {
	 action_error("Message %s was referenced elsewhere with different type", id->name);
	 break;
      }

      /* Insert directly into global table to preserve id # */
      id->type = I_MESSAGE;
      id->ownernum = st.curclass;
      id->source = COMPILE;
      table_insert(st.globalvars, (void *) id, id_hash, id_compare);

      /* Remove from missing list */
      table_delete_item(st.missingvars, id, id_hash, id_compare);

      break;

   case I_UNDEFINED:   /* New message # */
      id->ownernum = st.curclass;
      add_identifier(id, I_MESSAGE);
      break;

   case I_MESSAGE:     /* Make sure message hasn't appeared in this class */
      if (id->ownernum == st.curclass && id->source == COMPILE)
	 action_error("Message handler %s defined twice in same class", id->name);
      old_id->ownernum = st.curclass;
      old_id->source = COMPILE;
      break;

   default:            /* Other types indicate name already used */
      action_error("Duplicate identifier %s", id->name);
   }

   s->message_id = id;
   /* Sort parameters in increasing id # order */
   s->params = SortParameterList(args);

   /* Add parameters as handler's local variables--this must be done AFTER sorting */
   for (l = s->params; l != NULL; l = l->next)
   {
      param = (param_type) l->data;
      
      /* Make a copy of the id for the local table */
      temp_id = duplicate_id(param->lhs);
      temp_id->type = I_LOCAL;

      if (table_insert(st.localvars, (void *) temp_id, id_hash, id_compare) == 0)
	 temp_id->idnum = ++st.maxlocals;      
   }

   st.curmessage = id->idnum;

   return s;
}
/************************************************************************/
/*
 * add_parent_properties: Add properties of a class's superclasses to the
 *   class's properties.  
 */
void add_parent_properties(class_type base, class_type parent)
{
   list_type p;

   if (parent == NULL)
      return;

   for (p = parent->properties; p != NULL; p = p->next)
   {
      property_type prop = (property_type) (p->data);
      /* Parser error recovery may have made a NULL property */
      if (prop != NULL)
      {
	 /* Add property id to base class's property table, using table_insert
	  * so that original id #s are preserved.  Only increment # of properties
	  * if wasn't already in table.  
	  * If already in table, it's not an error, since a property might be 
	  * listed many times in this class's ancestors.
	  */
	 if (table_insert(st.classvars, (void *) prop->id, id_hash, id_compare) == 0)
	    st.maxproperties++;
      }
   }

   add_parent_properties(base, parent->superclass);
}
/************************************************************************/
/*
 * add_parent_classvars: Add classvars of a class's superclasses to the
 *   class's classvars.  
 *   cv_list is a list used just for holding classvars; we need a separate list
 *   to handle classvars that are overridden with properties in a parent class.
 */
void add_parent_classvars(list_type cv_list, class_type base, class_type parent)
{
   list_type cv;

   if (parent == NULL)
   {
      list_delete(cv_list);
      return;
   }

   for (cv = parent->classvars; cv != NULL; cv = cv->next)
   {
      classvar_type classvar = (classvar_type) (cv->data);
      /* Parser error recovery may have made a NULL property */
      if (classvar != NULL)
      {
	 // Increment classvar count even for classvars that are overridden by properties.
	 // Only add non-overridden classvars to symbol table, though, so that references
	 // to the name will map to the property, not the classvar.

	 if (list_find_item(cv_list, classvar->id, id_compare) == NULL)
	 {
	    cv_list = list_add_item(cv_list, classvar->id);
	    st.maxclassvars++;
	 }

	 // Insert to table will fail if property with same name is already there
	 table_insert(st.classvars, (void *) classvar->id, id_hash, id_compare);
      }
   }

   add_parent_classvars(cv_list, base, parent->superclass);
}
/************************************************************************/
class_type make_class_signature(id_type class_id, id_type superclass_id)
{
   list_type l;
   id_type old_id;
   class_type c = (class_type) SafeMalloc(sizeof(class_struct));
   c->superclass = NULL;  /* Will be set for real below */

   c->class_id = class_id;

   /* Class name must not have appeared before */
   old_id = lookup_id(class_id);

   switch(class_id->type)
   {
   case I_MISSING:
      /* The class has been referenced in a function call, but not declared anywhere 
       * We should use the existent class # and remove the id from the missing list 
       */
      if (class_id->source != I_CLASS)
      {
	 action_error("Class %s was referenced elsewhere with different type", class_id->name);
	 break;
      }

      /* Insert directly into global table to preserve id # */
      class_id->type = I_CLASS;
      class_id->source = COMPILE;
      table_insert(st.globalvars, (void *) class_id, id_hash, id_compare);

      /* Remove from missing list */
      table_delete_item(st.missingvars, class_id, id_hash, id_compare);

      break;
      
   case I_UNDEFINED:
      /* New class name--continue normally */
      add_identifier(class_id, I_CLASS);
      break;

   case I_CLASS:
      /* We are recompiling a previously existing class.  This is rather ugly, since
       * we want to replace all the old class's data with our new data.  However, to give
       * previously compiled subclasses a chance of working with the new version, we
       * should try to match class numbers, message #s, etc. with the old class.
       *   At this point, all the old class's data have been inserted into our hash
       * tables.  The new class will be inserted into st.globalvars.  We must remove the old class 
       * from the class list.
       */

      /* If class is given twice in source files, that's an error */
      if (class_id->source == COMPILE)
	 action_error("Class %s is given twice", class_id->name);
      else 
	 /* Change the existent class id to reflect the fact that the class has now
	  * been recompiled. */
	 old_id->source = COMPILE;

      class_id->source = COMPILE;

      /* Delete self from list of existent classes.  Give warnings for subclasses. */
	 
      for (l = st.classes; l != NULL; l = l->next)
	 if (is_parent(c, (class_type) l->data))
	 {
	    if (((class_type) l->data)->class_id->idnum == c->class_id->idnum)
	       st.classes = list_delete_item(st.classes, l->data, class_compare);
	    else
	       /* Don't give warning if deleted self from list */
	    {
	       recompile_type recompile_info = 
		  (recompile_type) SafeMalloc(sizeof(recompile_struct));
	       recompile_info->class_id = ((class_type) (l->data))->class_id;
	       recompile_info->superclass = c->class_id;
	       
	       /* Only add to list if it isn't already there */
	       if (list_find_item(st.recompile_list, recompile_info->class_id, 
				  recompile_compare) == NULL)
		  st.recompile_list = list_add_item(st.recompile_list, (void *) recompile_info);
	    }
	    
	 }
      break;
      
   default:
      action_error("Duplicate identifier %s", class_id->name);
   }

   /* Delete this class from list of those that need to be recompiled */
   st.recompile_list = 
      list_delete_item(st.recompile_list, (void *) c->class_id, recompile_compare);

   c->resources  = NULL;
   c->properties = NULL;
   c->messages   = NULL;
   c->is_new = True; /* We should generate code for this class */

   /* Superclass must be defined, if one is given */
   if (superclass_id != NULL)
   {
      lookup_id(superclass_id);
      if (superclass_id->type != I_CLASS)
	 action_error("Can't find superclass %s", superclass_id->name);
      else if (class_id->idnum == superclass_id->idnum)
	 action_error("Can't subclass from self", superclass_id->name);	 
      else 
      {
	 /* Find superclass's data and store a pointer to it. */
	 for (l = st.classes; l != NULL; l = l->next)
	    if (superclass_id->idnum == ((class_type) (l->data))->class_id->idnum)
	    {
	       c->superclass = (class_type) l->data;
	       break;
	    }

	 if (c->superclass == NULL)
	    action_error("Unable to find superclass %s", superclass_id->name);
      }
      
   }
   else c->superclass = NULL;

   st.curclass = class_id->idnum;

   /* Now add superclasses' properties to our own property table, but delete
    * them from our property list.  This way the table will hold all properties 
    * that can be referenced in the class, but the property list will only 
    * hold those that were declared in this class (need to know this for code gen) */
   add_parent_properties(c, c->superclass);

   /* Similarly, add superclasses' classvars.  This must be done AFTER adding the properties,
    * since some properties in a superclass could be overriding classvars in a superclass */
   add_parent_classvars(NULL, c, c->superclass);

   /* Add to list of classes */
   st.classes = list_add_item(st.classes, (void *) c);

   return c;   
}
/************************************************************************/
class_type make_class(class_type c, list_type resources, list_type classvars,
		      list_type properties, list_type messages)
{
   int has_constructor = False;

   c->resources = resources;
   c->classvars = list_append(st.override_classvars, classvars);
   c->properties = properties;

   /* Sort message handlers by id # */
   c->messages = SortMessageHandlerList(messages);

   /* Save away total # of properties in this class + parents */
   c->numproperties = st.maxproperties;

   /* Save away total # of class variables in this class + parents */
   c->numclassvars  = st.maxclassvars + 1;

   /* Clean out class variable table */
   table_delete(st.classvars);
   st.override_classvars = NULL;

   /* Erase constant list */
   st.constants = list_delete(st.constants);

   st.maxproperties = 0;  // Property #0 is reserved for SELF
   st.maxclassvars  = -1;

   return c;   
}
